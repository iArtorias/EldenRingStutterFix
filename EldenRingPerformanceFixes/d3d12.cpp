#include "pch.h"
#include "d3d12.h"
#include <string>
#include <wrl/client.h>
#include <initguid.h>
#include <mutex>
#include <iomanip>
#include <fstream>
#include <sstream>

using Microsoft::WRL::ComPtr;

std::mutex logMutex;
void log(std::string msg)
{
    std::lock_guard<std::mutex> lock(logMutex);
    std::ofstream os("perf.log", std::ios_base::app);
    os << msg;
}

#define COMMAND_ALLOCATOR_CACHE_CAPACITY 8

struct CommandAllocatorCacheEntry
{
    D3D12_COMMAND_LIST_TYPE type;
    ID3D12CommandAllocator* commandAllocator;
};

class D3D12DeviceExt : public IUnknown
{
private:
    volatile ULONG m_cref = 0;
public:
    CommandAllocatorCacheEntry commandAllocatorCache[COMMAND_ALLOCATOR_CACHE_CAPACITY];
    std::mutex cacheMutex;

    D3D12DeviceExt() : commandAllocatorCache()
    {

    }

    virtual HRESULT QueryInterface(REFIID riid, void** ppvObject) override
    {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }

    virtual ULONG AddRef() override
    {
        return InterlockedIncrement(&m_cref);
    }

    virtual ULONG Release() override
    {
        ULONG cref = InterlockedDecrement(&m_cref);
        if (cref == 0)
        {
            delete this;
        }
        return cref;
    }
};

class D3D12CommandAllocatorExt : public IUnknown
{
private:
    volatile ULONG m_cref = 0;
public:
    ID3D12Device* device;
    D3D12_COMMAND_LIST_TYPE type;

    D3D12CommandAllocatorExt(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
        : device(device), type(type)
    {

    }

    virtual HRESULT QueryInterface(REFIID riid, void** ppvObject) override
    {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }

    virtual ULONG AddRef() override
    {
        return InterlockedIncrement(&m_cref);
    }

    virtual ULONG Release() override
    {
        ULONG cref = InterlockedDecrement(&m_cref);
        if (cref == 0)
        {
            delete this;
        }
        return cref;
    }
};

// {4AD2A837-234C-46B4-87DF-0833EC1D7E75}
DEFINE_GUID(GUID_D3D12CommandListExt, 0x4ad2a837, 0x234c, 0x46b4, 0x87, 0xdf, 0x8, 0x33, 0xec, 0x1d, 0x7e, 0x75);

// {07E5F7AB-5CE8-4AA1-A4D4-ABE8428A2A74}
DEFINE_GUID(GUID_D3D12DeviceExt, 0x7e5f7ab, 0x5ce8, 0x4aa1, 0xa4, 0xd4, 0xab, 0xe8, 0x42, 0x8a, 0x2a, 0x74);

namespace IUnknown_hook
{
    typedef ULONG(*PFN_Release)(IUnknown* __this);
}

namespace ID3D12CommandAllocator_hook
{
    IUnknown_hook::PFN_Release Release_real = nullptr;
    ULONG Release_hook(ID3D12CommandAllocator* __this)
    {
        ULONG cref = Release_real(__this);

        if (cref == 1)
        {
            UINT dataSize = sizeof(D3D12CommandAllocatorExt*);
            ComPtr<D3D12CommandAllocatorExt> commandAllocatorExt;
            if (SUCCEEDED(__this->GetPrivateData(GUID_D3D12CommandListExt, &dataSize, static_cast<D3D12CommandAllocatorExt**>(&commandAllocatorExt))))
            {
                dataSize = sizeof(D3D12DeviceExt*);
                ComPtr<D3D12DeviceExt> deviceExt;
                if (SUCCEEDED(commandAllocatorExt->device->GetPrivateData(GUID_D3D12DeviceExt, &dataSize, static_cast<D3D12DeviceExt**>(&deviceExt))))
                {
                    {
                        std::lock_guard<std::mutex> lock(deviceExt->cacheMutex);
                        for (int i = 0; i < COMMAND_ALLOCATOR_CACHE_CAPACITY; i++)
                        {
                            if (deviceExt->commandAllocatorCache[i].commandAllocator == nullptr)
                            {
                                if (SUCCEEDED(__this->Reset()))
                                {
                                    deviceExt->commandAllocatorCache[i].commandAllocator = __this;
                                    deviceExt->commandAllocatorCache[i].type = commandAllocatorExt->type;
                                    return cref - 1;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                    }

                    // if there's no room in the cache, destroy the allocator
                    return Release_real(__this);
                }
            }
        }

        return cref;
    }
}

namespace ID3D12Device_hook
{
    typedef HRESULT(*PFN_CreateCommandAllocator)(ID3D12Device* __this, D3D12_COMMAND_LIST_TYPE type, const IID& riid, void** ppCommandAllocator);
    PFN_CreateCommandAllocator CreateCommandAllocator_real = nullptr;
    std::mutex vtMutex;

    bool GetCachedCommandAllocator(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, void** ppCommandAllocator)
    {
        if (ID3D12CommandAllocator_hook::Release_real != nullptr)
        {
            UINT dataSize = sizeof(D3D12DeviceExt*);
            ComPtr<D3D12DeviceExt> deviceExt;
            if (SUCCEEDED(device->GetPrivateData(GUID_D3D12DeviceExt, &dataSize, static_cast<D3D12DeviceExt**>(&deviceExt))))
            {
                std::lock_guard<std::mutex> lock(deviceExt->cacheMutex);
                for (int i = 0; i < COMMAND_ALLOCATOR_CACHE_CAPACITY; i++)
                {
                    if (deviceExt->commandAllocatorCache[i].commandAllocator != nullptr && deviceExt->commandAllocatorCache[i].type == type)
                    {
                        *ppCommandAllocator = deviceExt->commandAllocatorCache[i].commandAllocator;
                        deviceExt->commandAllocatorCache[i].commandAllocator->AddRef();
                        deviceExt->commandAllocatorCache[i].commandAllocator = nullptr;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    HRESULT CreateCommandAllocator_hook(ID3D12Device* __this, D3D12_COMMAND_LIST_TYPE type, const IID& riid, void** ppCommandAllocator)
    {
        if (GetCachedCommandAllocator(__this, type, ppCommandAllocator))
        {
            return S_OK;
        }

        HRESULT hr = CreateCommandAllocator_real(__this, type, riid, ppCommandAllocator);

        if (SUCCEEDED(hr))
        {
            ID3D12CommandAllocator* pCommandAllocator = static_cast<ID3D12CommandAllocator*>(*ppCommandAllocator);

            // hook ID3D12CommandAllocator::Release
            if (ID3D12CommandAllocator_hook::Release_real == nullptr)
            {
                std::lock_guard<std::mutex> lock(vtMutex);
                if (ID3D12CommandAllocator_hook::Release_real == nullptr)
                {
                    // modify vtable
                    void** vt = *reinterpret_cast<void***>(pCommandAllocator);
                    void** pfn = &vt[2];
                    ID3D12CommandAllocator_hook::Release_real = static_cast<IUnknown_hook::PFN_Release>(*pfn);

                    DWORD OldProtection;
                    VirtualProtect(pfn, sizeof(void*), PAGE_READWRITE, &OldProtection);
                    *pfn = ID3D12CommandAllocator_hook::Release_hook;
                    VirtualProtect(pfn, sizeof(void*), OldProtection, &OldProtection);
                }
            }

            // add a ref to keep it alive
            pCommandAllocator->AddRef();
            pCommandAllocator->SetPrivateDataInterface(GUID_D3D12CommandListExt, new D3D12CommandAllocatorExt(__this, type));
        }

        return hr;
    }
}

HRESULT WINAPI D3D12CreateDevice_proxy(
    _In_opt_ IUnknown* pAdapter,
    D3D_FEATURE_LEVEL MinimumFeatureLevel,
    _In_ REFIID riid, // Expected: ID3D12Device
    _COM_Outptr_opt_ void** ppDevice)
{
    HRESULT hr = D3D12CreateDevice_real(pAdapter, MinimumFeatureLevel, riid, ppDevice);
    
    if (hr == S_OK && ppDevice != nullptr)
    {
        ID3D12Device* device = static_cast<ID3D12Device*>(*ppDevice);

        // hook ID3D12Device::CreateCommandAllocator
        if (ID3D12Device_hook::CreateCommandAllocator_real == nullptr)
        {
            // modify vtable
            void** vt = *reinterpret_cast<void***>(device);
            void** pfn = &vt[9];
            ID3D12Device_hook::CreateCommandAllocator_real = static_cast<ID3D12Device_hook::PFN_CreateCommandAllocator>(*pfn);

            DWORD OldProtection;
            VirtualProtect(pfn, sizeof(void*), PAGE_READWRITE, &OldProtection);
            *pfn = ID3D12Device_hook::CreateCommandAllocator_hook;
            VirtualProtect(pfn, sizeof(void*), OldProtection, &OldProtection);
        }

        device->SetPrivateDataInterface(GUID_D3D12DeviceExt, new D3D12DeviceExt());
    }

    return hr;
}