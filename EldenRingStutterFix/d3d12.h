#pragma once

#include <d3d12.h>

HRESULT WINAPI D3D12CreateDevice_proxy(
    _In_opt_ IUnknown* pAdapter,
    D3D_FEATURE_LEVEL MinimumFeatureLevel,
    _In_ REFIID riid, // Expected: ID3D12Device
    _COM_Outptr_opt_ void** ppDevice);

extern PFN_D3D12_CREATE_DEVICE D3D12CreateDevice_real;
