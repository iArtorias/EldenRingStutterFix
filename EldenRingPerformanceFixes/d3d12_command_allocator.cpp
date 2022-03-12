#include "pch.h"
#include "d3d12_command_allocator.h"
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;


ID3D12CommandAllocator_proxy::ID3D12CommandAllocator_proxy(ComPtr<ID3D12CommandAllocator> orig)
    : m_orig(orig)
{

}

int ID3D12CommandAllocator_proxy::Unk()
{
    return 1;
}


HRESULT ID3D12CommandAllocator_proxy::QueryInterface(const IID& riid, void** ppvObject)
{
    /*
    if (riid == __uuidof(ID3D12CommandAllocator))
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        LPWSTR buf;
        if (StringFromIID(riid, &buf) == S_OK)
        {
            MessageBoxW(nullptr, buf, L"Error", MB_OK);
        }
        else
        {
            MessageBoxA(nullptr, __FUNCTION__, "Error", MB_OK);
        }
    }
    */

    return m_orig->QueryInterface(riid, ppvObject);
}

ULONG ID3D12CommandAllocator_proxy::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG ID3D12CommandAllocator_proxy::Release()
{
    ULONG refs = InterlockedDecrement(&m_cRef);
    if (refs == 0)
    {
        delete this;
    }
    return refs;
}

HRESULT __stdcall ID3D12CommandAllocator_proxy::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData)
{
    return m_orig->GetPrivateData(guid, pDataSize, pData);
}

HRESULT __stdcall ID3D12CommandAllocator_proxy::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData)
{
    return m_orig->SetPrivateData(guid, DataSize, pData);
}

HRESULT __stdcall ID3D12CommandAllocator_proxy::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData)
{
    return m_orig->SetPrivateDataInterface(guid, pData);
}

HRESULT __stdcall ID3D12CommandAllocator_proxy::SetName(LPCWSTR Name)
{
    return m_orig->SetName(Name);
}

HRESULT __stdcall ID3D12CommandAllocator_proxy::GetDevice(REFIID riid, void** ppvDevice)
{
    return m_orig->GetDevice(riid, ppvDevice);
}

HRESULT __stdcall ID3D12CommandAllocator_proxy::Reset(void)
{
    return m_orig->Reset();
}