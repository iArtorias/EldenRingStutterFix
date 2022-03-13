#pragma once

#include <d3d12.h>

HRESULT WINAPI D3D12CreateDevice_hook(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice);

extern PFN_D3D12_CREATE_DEVICE D3D12CreateDevice_real;
