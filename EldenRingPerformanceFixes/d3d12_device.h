#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <mutex>

#define COMMAND_ALLOCATOR_CACHE_CAPACITY 8

class ID3D12Device_proxy : public ID3D12Device5
{
private:
	friend class ID3DCommandAllocator_proxy;
	volatile ULONG m_cRef = 0;
	Microsoft::WRL::ComPtr<ID3D12Device5> m_orig;
	std::mutex m_mutex;
	ID3D12CommandAllocator* m_commandAllocatorCache[COMMAND_ALLOCATOR_CACHE_CAPACITY] = {};
	int m_commandAllocatorCacheSize = 0;

public:
	// Extensions
	ID3D12Device_proxy(Microsoft::WRL::ComPtr<ID3D12Device5> orig);

	// Inherited via ID3D12Device5
	virtual HRESULT QueryInterface(const IID& riid, void** ppvObject);
	virtual ULONG AddRef();
	virtual ULONG Release();
	virtual HRESULT GetPrivateData(const GUID& guid, UINT* pDataSize, void* pData);
	virtual HRESULT SetPrivateData(const GUID& guid, UINT DataSize, const void* pData);
	virtual HRESULT SetPrivateDataInterface(const GUID& guid, const IUnknown* pData);
	virtual HRESULT SetName(LPCWSTR Name);
	virtual UINT GetNodeCount();
	virtual HRESULT CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC* pDesc, const IID& riid, void** ppCommandQueue);
	virtual HRESULT CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, const IID& riid, void** ppCommandAllocator);
	virtual HRESULT CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, const IID& riid, void** ppPipelineState);
	virtual HRESULT CreateComputePipelineState(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc, const IID& riid, void** ppPipelineState);
	virtual HRESULT CreateCommandList(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pInitialState, const IID& riid, void** ppCommandList);
	virtual HRESULT CheckFeatureSupport(D3D12_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize);
	virtual HRESULT CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, const IID& riid, void** ppvHeap);
	virtual UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType);
	virtual HRESULT CreateRootSignature(UINT nodeMask, const void* pBlobWithRootSignature, SIZE_T blobLengthInBytes, const IID& riid, void** ppvRootSignature);
	virtual void CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
	virtual void CreateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
	virtual void CreateUnorderedAccessView(ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
	virtual void CreateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
	virtual void CreateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
	virtual void CreateSampler(const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
	virtual void CopyDescriptors(UINT NumDestDescriptorRanges, const D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts, const UINT* pDestDescriptorRangeSizes, UINT NumSrcDescriptorRanges, const D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts, const UINT* pSrcDescriptorRangeSizes, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType);
	virtual void CopyDescriptorsSimple(UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart, D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType);
	virtual D3D12_RESOURCE_ALLOCATION_INFO GetResourceAllocationInfo(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs);
	virtual D3D12_HEAP_PROPERTIES GetCustomHeapProperties(UINT nodeMask, D3D12_HEAP_TYPE heapType);
	virtual HRESULT CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID& riidResource, void** ppvResource);
	virtual HRESULT CreateHeap(const D3D12_HEAP_DESC* pDesc, const IID& riid, void** ppvHeap);
	virtual HRESULT CreatePlacedResource(ID3D12Heap* pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID& riid, void** ppvResource);
	virtual HRESULT CreateReservedResource(const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID& riid, void** ppvResource);
	virtual HRESULT CreateSharedHandle(ID3D12DeviceChild* pObject, const SECURITY_ATTRIBUTES* pAttributes, DWORD Access, LPCWSTR Name, HANDLE* pHandle);
	virtual HRESULT OpenSharedHandle(HANDLE NTHandle, const IID& riid, void** ppvObj);
	virtual HRESULT OpenSharedHandleByName(LPCWSTR Name, DWORD Access, HANDLE* pNTHandle);
	virtual HRESULT MakeResident(UINT NumObjects, ID3D12Pageable* const* ppObjects);
	virtual HRESULT Evict(UINT NumObjects, ID3D12Pageable* const* ppObjects);
	virtual HRESULT CreateFence(UINT64 InitialValue, D3D12_FENCE_FLAGS Flags, const IID& riid, void** ppFence);
	virtual HRESULT GetDeviceRemovedReason();
	virtual void GetCopyableFootprints(const D3D12_RESOURCE_DESC* pResourceDesc, UINT FirstSubresource, UINT NumSubresources, UINT64 BaseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, UINT* pNumRows, UINT64* pRowSizeInBytes, UINT64* pTotalBytes);
	virtual HRESULT CreateQueryHeap(const D3D12_QUERY_HEAP_DESC* pDesc, const IID& riid, void** ppvHeap);
	virtual HRESULT SetStablePowerState(BOOL Enable);
	virtual HRESULT CreateCommandSignature(const D3D12_COMMAND_SIGNATURE_DESC* pDesc, ID3D12RootSignature* pRootSignature, const IID& riid, void** ppvCommandSignature);
	virtual void GetResourceTiling(ID3D12Resource* pTiledResource, UINT* pNumTilesForEntireResource, D3D12_PACKED_MIP_INFO* pPackedMipDesc, D3D12_TILE_SHAPE* pStandardTileShapeForNonPackedMips, UINT* pNumSubresourceTilings, UINT FirstSubresourceTilingToGet, D3D12_SUBRESOURCE_TILING* pSubresourceTilingsForNonPackedMips);
	virtual LUID GetAdapterLuid();
	virtual HRESULT CreatePipelineLibrary(const void* pLibraryBlob, SIZE_T BlobLength, const IID& riid, void** ppPipelineLibrary);
	virtual HRESULT SetEventOnMultipleFenceCompletion(ID3D12Fence* const* ppFences, const UINT64* pFenceValues, UINT NumFences, D3D12_MULTIPLE_FENCE_WAIT_FLAGS Flags, HANDLE hEvent);
	virtual HRESULT SetResidencyPriority(UINT NumObjects, ID3D12Pageable* const* ppObjects, const D3D12_RESIDENCY_PRIORITY* pPriorities);
	virtual HRESULT CreatePipelineState(const D3D12_PIPELINE_STATE_STREAM_DESC* pDesc, const IID& riid, void** ppPipelineState);
	virtual HRESULT OpenExistingHeapFromAddress(const void* pAddress, const IID& riid, void** ppvHeap);
	virtual HRESULT OpenExistingHeapFromFileMapping(HANDLE hFileMapping, const IID& riid, void** ppvHeap);
	virtual HRESULT EnqueueMakeResident(D3D12_RESIDENCY_FLAGS Flags, UINT NumObjects, ID3D12Pageable* const* ppObjects, ID3D12Fence* pFenceToSignal, UINT64 FenceValueToSignal);
	virtual HRESULT CreateCommandList1(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_LIST_FLAGS flags, const IID& riid, void** ppCommandList);
	virtual HRESULT CreateProtectedResourceSession(const D3D12_PROTECTED_RESOURCE_SESSION_DESC* pDesc, const IID& riid, void** ppSession);
	virtual HRESULT CreateCommittedResource1(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, const IID& riidResource, void** ppvResource);
	virtual HRESULT CreateHeap1(const D3D12_HEAP_DESC* pDesc, ID3D12ProtectedResourceSession* pProtectedSession, const IID& riid, void** ppvHeap);
	virtual HRESULT CreateReservedResource1(const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, const IID& riid, void** ppvResource);
	virtual D3D12_RESOURCE_ALLOCATION_INFO GetResourceAllocationInfo1(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO1* pResourceAllocationInfo1);
	virtual HRESULT CreateLifetimeTracker(ID3D12LifetimeOwner* pOwner, const IID& riid, void** ppvTracker);
	virtual void RemoveDevice();
	virtual HRESULT EnumerateMetaCommands(UINT* pNumMetaCommands, D3D12_META_COMMAND_DESC* pDescs);
	virtual HRESULT EnumerateMetaCommandParameters(const GUID& CommandId, D3D12_META_COMMAND_PARAMETER_STAGE Stage, UINT* pTotalStructureSizeInBytes, UINT* pParameterCount, D3D12_META_COMMAND_PARAMETER_DESC* pParameterDescs);
	virtual HRESULT CreateMetaCommand(const GUID& CommandId, UINT NodeMask, const void* pCreationParametersData, SIZE_T CreationParametersDataSizeInBytes, const IID& riid, void** ppMetaCommand);
	virtual HRESULT CreateStateObject(const D3D12_STATE_OBJECT_DESC* pDesc, const IID& riid, void** ppStateObject);
	virtual void GetRaytracingAccelerationStructurePrebuildInfo(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS* pDesc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO* pInfo);
	virtual D3D12_DRIVER_MATCHING_IDENTIFIER_STATUS CheckDriverMatchingIdentifier(D3D12_SERIALIZED_DATA_TYPE SerializedDataType, const D3D12_SERIALIZED_DATA_DRIVER_MATCHING_IDENTIFIER* pIdentifierToCheck);
};

