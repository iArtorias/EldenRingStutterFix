#include "pch.h"
#include "d3d12.h"
#include "d3d12_device.h"
#include <string>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

ComPtr<ID3D12Device_proxy> g_device;

HRESULT WINAPI D3D12CreateDevice_proxy(
    _In_opt_ IUnknown* pAdapter,
    D3D_FEATURE_LEVEL MinimumFeatureLevel,
    _In_ REFIID riid, // Expected: ID3D12Device
    _COM_Outptr_opt_ void** ppDevice)
{
    /*
    * Check which ID3D12Device version the game uses
    * Answer: ID3D12Device5
    GUID riids[] = {
        __uuidof(ID3D12Device),
        __uuidof(ID3D12Device1),
        __uuidof(ID3D12Device2),
        __uuidof(ID3D12Device3),
        __uuidof(ID3D12Device4),
        __uuidof(ID3D12Device5),
        __uuidof(ID3D12Device6),
        __uuidof(ID3D12Device7),
        __uuidof(ID3D12Device8),
        __uuidof(ID3D12Device9)
    };
    for (int i = 0; i < 10; i++)
    {
        if (riid == riids[i])
        {
            std::string msg = "ID3D12Device" + std::to_string(i);
            MessageBoxA(nullptr, msg.c_str(), "D3D12CreateDevice", MB_OK);
        }
    }
    */

    HRESULT hr;
    if (ppDevice == nullptr)
    {
        hr = D3D12CreateDevice_real(pAdapter, MinimumFeatureLevel, riid, ppDevice);
    }
    else
    {
        ComPtr<ID3D12Device5> dev;
        hr = D3D12CreateDevice_real(pAdapter, MinimumFeatureLevel, riid, &dev);

        if (riid != __uuidof(ID3D12Device5))
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

        ComPtr<ID3D12Device_proxy> devProxy(new ID3D12Device_proxy(dev));
        g_device = devProxy;
        *ppDevice = devProxy.Detach();
        static int count = 0;
        if (count > 0)
        {
        MessageBoxA(nullptr, __FUNCTION__, "INFO", MB_OK);

        }
        count++;

    }

    return hr;
}