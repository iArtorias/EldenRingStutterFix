#include "pch.h"
#include "d3d12.h"
#include <string>
#include <wrl/client.h>
#include <initguid.h>
#include <mutex>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <thread>

using Microsoft::WRL::ComPtr;

#ifdef LOG_STATS
std::mutex logMutex;
void log(std::string msg)
{
    std::lock_guard<std::mutex> lock(logMutex);
    std::ofstream os("d3d12_stats.log", std::ios_base::app);
    os << msg;
}

volatile UINT commandAllocatorsCreated = 0;
volatile UINT commandAllocatorCacheHits = 0;
volatile UINT createPsoOverrides = 0;
volatile UINT createCommittedResourceOverrides = 0;

bool loggingThreadRunning = false;
void startLoggingThread(unsigned int interval)
{
    loggingThreadRunning = true;
    std::thread([interval]()
    {
        while (true)
        {
            auto nextLogTime = std::chrono::steady_clock::now() + std::chrono::seconds(interval);

            std::stringstream ss;
            time_t now = std::time(nullptr);
            struct tm timebuf;
            localtime_s(&timebuf, &now);
            ss << std::put_time(&timebuf, "%F %T ");
            ss << "\nCommand allocators created:        " << commandAllocatorsCreated;
            ss << "\nCommand allocator cache hits:      " << commandAllocatorCacheHits;
            ss << "\nCreatePipelineLibrary overrides:   " << createPsoOverrides;
            ss << "\nCreateCommittedResource overrides: " << createCommittedResourceOverrides;
            ss << "\n\n";
            log(ss.str());

            std::this_thread::sleep_until(nextLogTime);
        }
    }).detach();
}

#endif


////////// Ext classes //////////
//
// Extended info that's attached to D3D12 objects.
// Implemented as COM classes so D3D12 can automatically clean them up.
//

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


////////// Hooks //////////
//
// These functions are injected into D3D12 vtables
//

namespace IUnknown_hook
{
    typedef ULONG(*PFN_Release)(IUnknown* __this);
}

namespace ID3D12CommandAllocator_hook
{
    ///// ID3D12CommandAllocator::Release
    // Cache command allocators instead of destroying them

    IUnknown_hook::PFN_Release Release_real = nullptr;
    ULONG Release_hook(ID3D12CommandAllocator* __this)
    {
        ULONG cref = Release_real(__this);

        // there are actually 0 refs since we added a ref when creating the command allocator
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

        return cref ? cref - 1 : 0;
    }
}

namespace ID3D12Device_hook
{
    ///// ID3D12Device::CreateCommandAllocator
    // Retrieve CommandAllocators from the cache to deal with CreateCommandAllocator spam

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
#ifdef LOG_STATS
            InterlockedIncrement(&commandAllocatorCacheHits);
#endif
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
                    void** pRelease = &vt[2];
                    ID3D12CommandAllocator_hook::Release_real = static_cast<IUnknown_hook::PFN_Release>(*pRelease);

                    DWORD OldProtection;
                    VirtualProtect(vt, sizeof(void*) * 3, PAGE_READWRITE, &OldProtection);
                    *pRelease = ID3D12CommandAllocator_hook::Release_hook;
                    VirtualProtect(vt, sizeof(void*) * 3, OldProtection, &OldProtection);
                }
            }

            // add a ref to keep it alive
            pCommandAllocator->AddRef();
            pCommandAllocator->SetPrivateDataInterface(GUID_D3D12CommandListExt, new D3D12CommandAllocatorExt(__this, type));

#ifdef LOG_STATS
            InterlockedIncrement(&commandAllocatorsCreated);
#endif
        }

        return hr;
    }


    ///// ID3D12Device::CreateCommittedResource
    // Add D3D12_HEAP_FLAG_CREATE_NOT_ZEROED flag to speed up creating committed resources

    typedef HRESULT(*PFN_CreateCommittedResource)(
        ID3D12Device* __this,
        const D3D12_HEAP_PROPERTIES* pHeapProperties,
        D3D12_HEAP_FLAGS HeapFlags,
        const D3D12_RESOURCE_DESC* pDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        const D3D12_CLEAR_VALUE* pOptimizedClearValue,
        REFIID riidResource,
        void** ppvResource);

    PFN_CreateCommittedResource CreateCommittedResource_real = nullptr;

    HRESULT CreateCommittedResource_hook(
        ID3D12Device* __this,
        const D3D12_HEAP_PROPERTIES* pHeapProperties,
        D3D12_HEAP_FLAGS HeapFlags,
        const D3D12_RESOURCE_DESC* pDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        const D3D12_CLEAR_VALUE* pOptimizedClearValue,
        REFIID riidResource,
        void** ppvResource)
    {
#ifdef LOG_STATS
        if (!(HeapFlags & D3D12_HEAP_FLAG_CREATE_NOT_ZEROED))
        {
            InterlockedIncrement(&createCommittedResourceOverrides);
        }
#endif

        HeapFlags |= D3D12_HEAP_FLAG_CREATE_NOT_ZEROED;
        return CreateCommittedResource_real(__this, pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
    }


    ///// ID3D12Device::CreatePipelineLibrary
    // Ignore mismatch driver/device since Elden Ring doesn't detect the proper error code

    typedef HRESULT(*PFN_CreatePipelineLibrary)(
        ID3D12Device* __this,
        const void* pLibraryBlob,
        SIZE_T BlobLength,
        REFIID riid,
        void** ppPipelineLibrary);

    PFN_CreatePipelineLibrary CreatePipelineLibrary_real = nullptr;

    HRESULT CreatePipelineLibrary_hook(
        ID3D12Device* __this,
        const void* pLibraryBlob,
        SIZE_T BlobLength,
        REFIID riid,
        void** ppPipelineLibrary)
    {
        HRESULT hr = CreatePipelineLibrary_real(__this, pLibraryBlob, BlobLength, riid, ppPipelineLibrary);
        if (hr == D3D12_ERROR_ADAPTER_NOT_FOUND || hr == D3D12_ERROR_DRIVER_VERSION_MISMATCH)
        {
#ifdef LOG_STATS
            InterlockedIncrement(&createPsoOverrides);
#endif
            hr = CreatePipelineLibrary_real(__this, pLibraryBlob, 0, riid, ppPipelineLibrary);
            hr = S_OK;
        }
        return hr;
    }
}


////////// Exports //////////
//
// Functions exported by the DLL.
// Unmodified functions are defined in d3d12_asm.asm
// 

HRESULT WINAPI D3D12CreateDevice_hook(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice)
{
#ifdef LOG_STATS
    if (!loggingThreadRunning)
    {
        startLoggingThread(30);
    }
#endif

    HRESULT hr = D3D12CreateDevice_real(pAdapter, MinimumFeatureLevel, riid, ppDevice);
    
    if (SUCCEEDED(hr) && ppDevice != nullptr)
    {
        ID3D12Device* device = static_cast<ID3D12Device*>(*ppDevice);

        // hook ID3D12Device::CreateCommandAllocator
        if (ID3D12Device_hook::CreateCommandAllocator_real == nullptr)
        {
            // modify vtable
            void** vt = *reinterpret_cast<void***>(device);
            void** pCreateCommandAllocator = &vt[9];
            ID3D12Device_hook::CreateCommandAllocator_real = static_cast<ID3D12Device_hook::PFN_CreateCommandAllocator>(*pCreateCommandAllocator);
            void** pCreateCommittedResource = &vt[27];
            ID3D12Device_hook::CreateCommittedResource_real = static_cast<ID3D12Device_hook::PFN_CreateCommittedResource>(*pCreateCommittedResource);
            void** pCreatePipelineLibrary = &vt[44];
            ID3D12Device_hook::CreatePipelineLibrary_real = static_cast<ID3D12Device_hook::PFN_CreatePipelineLibrary>(*pCreatePipelineLibrary);

            DWORD OldProtection;
            VirtualProtect(vt, sizeof(void*) * 45, PAGE_READWRITE, &OldProtection);
            *pCreateCommandAllocator = ID3D12Device_hook::CreateCommandAllocator_hook;
            *pCreateCommittedResource = ID3D12Device_hook::CreateCommittedResource_hook;
            *pCreatePipelineLibrary = ID3D12Device_hook::CreatePipelineLibrary_hook;
            VirtualProtect(vt, sizeof(void*) * 45, OldProtection, &OldProtection);
        }

        device->SetPrivateDataInterface(GUID_D3D12DeviceExt, new D3D12DeviceExt());
    }

    return hr;
}