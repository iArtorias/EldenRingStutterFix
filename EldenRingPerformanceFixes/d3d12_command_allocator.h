#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <unordered_map>
#include <mutex>

class ID3D12Device_proxy;

typedef struct
{
	//volatile ULONG m_cRef = 0;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_orig;
	ID3D12Device_proxy* dev;
} ID3D12CommandAllocator_ext;

class ID3D12CommandAllocator_proxy : public ID3D12CommandAllocator
{
private:
	volatile ULONG m_cRef = 0;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_orig;
	//static std::unordered_map<ID3D12CommandAllocator_proxy*, ID3D12CommandAllocator_ext*> m_extMap;
	//std::mutex mutex;


public:
	// Extensions
	ID3D12CommandAllocator_proxy(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> orig);

	// Inherited via ID3D12CommandAllocator
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
	virtual ULONG __stdcall AddRef(void) override;
	virtual ULONG __stdcall Release(void) override;
	virtual HRESULT __stdcall GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override;
	virtual HRESULT __stdcall SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override;
	virtual HRESULT __stdcall SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override;
	virtual HRESULT __stdcall SetName(LPCWSTR Name) override;
	virtual HRESULT __stdcall GetDevice(REFIID riid, void** ppvDevice) override;
	virtual HRESULT __stdcall Reset(void) override;
	virtual int __stdcall Unk();
};
