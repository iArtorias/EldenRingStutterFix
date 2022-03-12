#include "pch.h"
#include "d3d12_device.h"
#include "d3d12_command_allocator.h"
#include <initguid.h>
#include <string>
#include <detours/detours.h>

using Microsoft::WRL::ComPtr;

ID3D12Device_proxy::ID3D12Device_proxy(ComPtr<ID3D12Device5> orig)
    : m_orig(orig)
{

}

// 74eaee3f-2f4b-476d-82ba-2b85cb49e310
DEFINE_GUID(IID_ID3D12DeviceDownlevel, 0x74eaee3fL, 0x2f4b, 0x476d, 0x82, 0xba, 0x2b, 0x85, 0xcb, 0x49, 0xe3, 0x10);

HRESULT ID3D12Device_proxy::QueryInterface(const IID& riid, void** ppvObject)
{
    if (riid == __uuidof(ID3D12Device)
        || riid == __uuidof(ID3D12Device1)
        || riid == __uuidof(ID3D12Device2)
        || riid == __uuidof(ID3D12Device3)
        || riid == __uuidof(ID3D12Device4)
        || riid == __uuidof(ID3D12Device5))
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else if (riid == IID_ID3D12DeviceDownlevel)
    {
        // pass
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

    return m_orig->QueryInterface(riid, ppvObject);
}

ULONG ID3D12Device_proxy::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG ID3D12Device_proxy::Release()
{
    ULONG refs = InterlockedDecrement(&m_cRef);
    if (refs == 0)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (int i = 0; i < m_commandAllocatorCacheSize; i++)
            {
                m_commandAllocatorCache[i]->Release();
            }
        }

        delete this;
    }
    return refs;
}

HRESULT ID3D12Device_proxy::GetPrivateData(const GUID& guid, UINT* pDataSize, void* pData)
{
    return m_orig->GetPrivateData(guid, pDataSize, pData);
}

HRESULT ID3D12Device_proxy::SetPrivateData(const GUID& guid, UINT DataSize, const void* pData)
{
    return m_orig->SetPrivateData(guid, DataSize, pData);
}

HRESULT ID3D12Device_proxy::SetPrivateDataInterface(const GUID& guid, const IUnknown* pData)
{
    return m_orig->SetPrivateDataInterface(guid, pData);
}

HRESULT ID3D12Device_proxy::SetName(LPCWSTR Name)
{
    return m_orig->SetName(Name);
}

UINT ID3D12Device_proxy::GetNodeCount()
{
    return m_orig->GetNodeCount();
}

HRESULT ID3D12Device_proxy::CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC* pDesc, const IID& riid, void** ppCommandQueue)
{
    return m_orig->CreateCommandQueue(pDesc, riid, ppCommandQueue);
}

HRESULT ID3D12Device_proxy::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, const IID& riid, void** ppCommandAllocator)
{
    /*
    if (riid != __uuidof(ID3D12CommandAllocator))
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

    ComPtr<ID3D12CommandAllocator> cmdAllocator;
    HRESULT hr = m_orig->CreateCommandAllocator(type, riid, &cmdAllocator);

    if (hr == S_OK)
    {
        ComPtr<ID3D12CommandAllocator_proxy> cmdAllocatorProxy(new ID3D12CommandAllocator_proxy(cmdAllocator));
        *ppCommandAllocator = cmdAllocatorProxy.Detach();
    }
    */

    /*
    if (type != D3D12_COMMAND_LIST_TYPE_BUNDLE)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (int i = 0; i < m_commandAllocatorCacheSize; i++)
        {
            if (m_commandAllocatorCache[i]->AddRef() == 2)
            {
                m_commandAllocatorCache[i]->Release();
                m_commandAllocatorCache[i]->Reset();
                *ppCommandAllocator = m_commandAllocatorCache[i];
                return S_OK;
            }
            else
            {
                m_commandAllocatorCache[i]->Release();
            }
        }
    }
    */

    HRESULT hr = m_orig->CreateCommandAllocator(type, riid, ppCommandAllocator);

    if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
    {
        void** vt2 = *reinterpret_cast<void***>(ppCommandAllocator);
        static void** vt = vt2;
        if (vt != vt2)
        {
            MessageBoxA(nullptr, std::to_string(reinterpret_cast<ULONGLONG>(vt2)).c_str(), "INFO", MB_OK);
            vt = vt2;
        }

    }


    /*
    if (hr == S_OK && type != D3D12_COMMAND_LIST_TYPE_BUNDLE && m_commandAllocatorCacheSize < COMMAND_ALLOCATOR_CACHE_CAPACITY)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_commandAllocatorCache[m_commandAllocatorCacheSize] = static_cast<ID3D12CommandAllocator*>(*ppCommandAllocator);
        m_commandAllocatorCache[m_commandAllocatorCacheSize]->AddRef();
        m_commandAllocatorCacheSize++;
    }
    */

    return hr;
}

HRESULT ID3D12Device_proxy::CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, const IID& riid, void** ppPipelineState)
{
    return m_orig->CreateGraphicsPipelineState(pDesc, riid, ppPipelineState);
}

HRESULT ID3D12Device_proxy::CreateComputePipelineState(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc, const IID& riid, void** ppPipelineState)
{
    return m_orig->CreateComputePipelineState(pDesc, riid, ppPipelineState);
}

HRESULT ID3D12Device_proxy::CreateCommandList(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pInitialState, const IID& riid, void** ppCommandList)
{
    return m_orig->CreateCommandList(nodeMask, type, pCommandAllocator, pInitialState, riid, ppCommandList);
}

HRESULT ID3D12Device_proxy::CheckFeatureSupport(D3D12_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize)
{
    return m_orig->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
}

HRESULT ID3D12Device_proxy::CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, const IID& riid, void** ppvHeap)
{
    return m_orig->CreateDescriptorHeap(pDescriptorHeapDesc, riid, ppvHeap);
}

UINT ID3D12Device_proxy::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType)
{
    return m_orig->GetDescriptorHandleIncrementSize(DescriptorHeapType);
}

HRESULT ID3D12Device_proxy::CreateRootSignature(UINT nodeMask, const void* pBlobWithRootSignature, SIZE_T blobLengthInBytes, const IID& riid, void** ppvRootSignature)
{
    return m_orig->CreateRootSignature(nodeMask, pBlobWithRootSignature, blobLengthInBytes, riid, ppvRootSignature);
}

void ID3D12Device_proxy::CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    m_orig->CreateConstantBufferView(pDesc, DestDescriptor);
}

void ID3D12Device_proxy::CreateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    m_orig->CreateShaderResourceView(pResource, pDesc, DestDescriptor);
}

void ID3D12Device_proxy::CreateUnorderedAccessView(ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    m_orig->CreateUnorderedAccessView(pResource, pCounterResource, pDesc, DestDescriptor);
}

void ID3D12Device_proxy::CreateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    m_orig->CreateRenderTargetView(pResource, pDesc, DestDescriptor);
}

void ID3D12Device_proxy::CreateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    m_orig->CreateDepthStencilView(pResource, pDesc, DestDescriptor);
}

void ID3D12Device_proxy::CreateSampler(const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    m_orig->CreateSampler(pDesc, DestDescriptor);
}

void ID3D12Device_proxy::CopyDescriptors(UINT NumDestDescriptorRanges, const D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts, const UINT* pDestDescriptorRangeSizes, UINT NumSrcDescriptorRanges, const D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts, const UINT* pSrcDescriptorRangeSizes, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
    m_orig->CopyDescriptors(NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes, NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes, DescriptorHeapsType);
}

void ID3D12Device_proxy::CopyDescriptorsSimple(UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart, D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
    m_orig->CopyDescriptorsSimple(NumDescriptors, DestDescriptorRangeStart, SrcDescriptorRangeStart, DescriptorHeapsType);
}

D3D12_RESOURCE_ALLOCATION_INFO ID3D12Device_proxy::GetResourceAllocationInfo(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs)
{
    return m_orig->GetResourceAllocationInfo(visibleMask, numResourceDescs, pResourceDescs);
}

D3D12_HEAP_PROPERTIES ID3D12Device_proxy::GetCustomHeapProperties(UINT nodeMask, D3D12_HEAP_TYPE heapType)
{
    return m_orig->GetCustomHeapProperties(nodeMask, heapType);
}

HRESULT ID3D12Device_proxy::CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID& riidResource, void** ppvResource)
{
    return m_orig->CreateCommittedResource(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
}

HRESULT ID3D12Device_proxy::CreateHeap(const D3D12_HEAP_DESC* pDesc, const IID& riid, void** ppvHeap)
{
    return m_orig->CreateHeap(pDesc, riid, ppvHeap);
}

HRESULT ID3D12Device_proxy::CreatePlacedResource(ID3D12Heap* pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID& riid, void** ppvResource) {
    return m_orig->CreatePlacedResource(pHeap, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
}

HRESULT ID3D12Device_proxy::CreateReservedResource(const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID& riid, void** ppvResource)
{
    return m_orig->CreateReservedResource(pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
}

HRESULT ID3D12Device_proxy::CreateSharedHandle(ID3D12DeviceChild* pObject, const SECURITY_ATTRIBUTES* pAttributes, DWORD Access, LPCWSTR Name, HANDLE* pHandle)
{
    return m_orig->CreateSharedHandle(pObject, pAttributes, Access, Name, pHandle);
}

HRESULT ID3D12Device_proxy::OpenSharedHandle(HANDLE NTHandle, const IID& riid, void** ppvObj)
{
    return m_orig->OpenSharedHandle(NTHandle, riid, ppvObj);
}

HRESULT ID3D12Device_proxy::OpenSharedHandleByName(LPCWSTR Name, DWORD Access, HANDLE* pNTHandle)
{
    return m_orig->OpenSharedHandleByName(Name, Access, pNTHandle);
}

HRESULT ID3D12Device_proxy::MakeResident(UINT NumObjects, ID3D12Pageable* const* ppObjects)
{
    return m_orig->MakeResident(NumObjects, ppObjects);
}

HRESULT ID3D12Device_proxy::Evict(UINT NumObjects, ID3D12Pageable* const* ppObjects)
{
    return m_orig->Evict(NumObjects, ppObjects);
}

HRESULT ID3D12Device_proxy::CreateFence(UINT64 InitialValue, D3D12_FENCE_FLAGS Flags, const IID& riid, void** ppFence)
{
    return m_orig->CreateFence(InitialValue, Flags, riid, ppFence);
}

HRESULT ID3D12Device_proxy::GetDeviceRemovedReason()
{
    return m_orig->GetDeviceRemovedReason();
}

void ID3D12Device_proxy::GetCopyableFootprints(const D3D12_RESOURCE_DESC* pResourceDesc, UINT FirstSubresource, UINT NumSubresources, UINT64 BaseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, UINT* pNumRows, UINT64* pRowSizeInBytes, UINT64* pTotalBytes)
{
    m_orig->GetCopyableFootprints(pResourceDesc, FirstSubresource, NumSubresources, BaseOffset, pLayouts, pNumRows, pRowSizeInBytes, pTotalBytes);
}

HRESULT ID3D12Device_proxy::CreateQueryHeap(const D3D12_QUERY_HEAP_DESC* pDesc, const IID& riid, void** ppvHeap)
{
    return m_orig->CreateQueryHeap(pDesc, riid, ppvHeap);
}

HRESULT ID3D12Device_proxy::SetStablePowerState(BOOL Enable)
{
    return m_orig->SetStablePowerState(Enable);
}

HRESULT ID3D12Device_proxy::CreateCommandSignature(const D3D12_COMMAND_SIGNATURE_DESC* pDesc, ID3D12RootSignature* pRootSignature, const IID& riid, void** ppvCommandSignature)
{
    return m_orig->CreateCommandSignature(pDesc, pRootSignature, riid, ppvCommandSignature);
}

void ID3D12Device_proxy::GetResourceTiling(ID3D12Resource* pTiledResource, UINT* pNumTilesForEntireResource, D3D12_PACKED_MIP_INFO* pPackedMipDesc, D3D12_TILE_SHAPE* pStandardTileShapeForNonPackedMips, UINT* pNumSubresourceTilings, UINT FirstSubresourceTilingToGet, D3D12_SUBRESOURCE_TILING* pSubresourceTilingsForNonPackedMips)
{
    m_orig->GetResourceTiling(pTiledResource, pNumTilesForEntireResource, pPackedMipDesc, pStandardTileShapeForNonPackedMips, pNumSubresourceTilings, FirstSubresourceTilingToGet, pSubresourceTilingsForNonPackedMips);
}

LUID ID3D12Device_proxy::GetAdapterLuid()
{
    return m_orig->GetAdapterLuid();
}

HRESULT ID3D12Device_proxy::CreatePipelineLibrary(const void* pLibraryBlob, SIZE_T BlobLength, const IID& riid, void** ppPipelineLibrary)
{
    return m_orig->CreatePipelineLibrary(pLibraryBlob, BlobLength, riid, ppPipelineLibrary);
}

HRESULT ID3D12Device_proxy::SetEventOnMultipleFenceCompletion(ID3D12Fence* const* ppFences, const UINT64* pFenceValues, UINT NumFences, D3D12_MULTIPLE_FENCE_WAIT_FLAGS Flags, HANDLE hEvent)
{
    return m_orig->SetEventOnMultipleFenceCompletion(ppFences, pFenceValues, NumFences, Flags, hEvent);
}

HRESULT ID3D12Device_proxy::SetResidencyPriority(UINT NumObjects, ID3D12Pageable* const* ppObjects, const D3D12_RESIDENCY_PRIORITY* pPriorities)
{
    return m_orig->SetResidencyPriority(NumObjects, ppObjects, pPriorities);
}

HRESULT ID3D12Device_proxy::CreatePipelineState(const D3D12_PIPELINE_STATE_STREAM_DESC* pDesc, const IID& riid, void** ppPipelineState)
{
    return m_orig->CreatePipelineState(pDesc, riid, ppPipelineState);
}

HRESULT ID3D12Device_proxy::OpenExistingHeapFromAddress(const void* pAddress, const IID& riid, void** ppvHeap)
{
    return m_orig->OpenExistingHeapFromAddress(pAddress, riid, ppvHeap);
}

HRESULT ID3D12Device_proxy::OpenExistingHeapFromFileMapping(HANDLE hFileMapping, const IID& riid, void** ppvHeap)
{
    return m_orig->OpenExistingHeapFromFileMapping(hFileMapping, riid, ppvHeap);
}

HRESULT ID3D12Device_proxy::EnqueueMakeResident(D3D12_RESIDENCY_FLAGS Flags, UINT NumObjects, ID3D12Pageable* const* ppObjects, ID3D12Fence* pFenceToSignal, UINT64 FenceValueToSignal)
{
    return m_orig->EnqueueMakeResident(Flags, NumObjects, ppObjects, pFenceToSignal, FenceValueToSignal);
}

HRESULT ID3D12Device_proxy::CreateCommandList1(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_LIST_FLAGS flags, const IID& riid, void** ppCommandList)
{
    return m_orig->CreateCommandList1(nodeMask, type, flags, riid, ppCommandList);
}

HRESULT ID3D12Device_proxy::CreateProtectedResourceSession(const D3D12_PROTECTED_RESOURCE_SESSION_DESC* pDesc, const IID& riid, void** ppSession)
{
    return m_orig->CreateProtectedResourceSession(pDesc, riid, ppSession);
}

HRESULT ID3D12Device_proxy::CreateCommittedResource1(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, const IID& riidResource, void** ppvResource)
{
    return m_orig->CreateCommittedResource1(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, pProtectedSession, riidResource, ppvResource);
}

HRESULT ID3D12Device_proxy::CreateHeap1(const D3D12_HEAP_DESC* pDesc, ID3D12ProtectedResourceSession* pProtectedSession, const IID& riid, void** ppvHeap)
{
    return m_orig->CreateHeap1(pDesc, pProtectedSession, riid, ppvHeap);
}

HRESULT ID3D12Device_proxy::CreateReservedResource1(const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, const IID& riid, void** ppvResource)
{
    return m_orig->CreateReservedResource1(pDesc, InitialState, pOptimizedClearValue, pProtectedSession, riid, ppvResource);
}

D3D12_RESOURCE_ALLOCATION_INFO ID3D12Device_proxy::GetResourceAllocationInfo1(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO1* pResourceAllocationInfo1)
{
    return m_orig->GetResourceAllocationInfo1(visibleMask, numResourceDescs, pResourceDescs, pResourceAllocationInfo1);
}

HRESULT ID3D12Device_proxy::CreateLifetimeTracker(ID3D12LifetimeOwner* pOwner, const IID& riid, void** ppvTracker)
{
    return m_orig->CreateLifetimeTracker(pOwner, riid, ppvTracker);
}

void ID3D12Device_proxy::RemoveDevice()
{
    m_orig->RemoveDevice();
}

HRESULT ID3D12Device_proxy::EnumerateMetaCommands(UINT* pNumMetaCommands, D3D12_META_COMMAND_DESC* pDescs)
{
    return m_orig->EnumerateMetaCommands(pNumMetaCommands, pDescs);
}

HRESULT ID3D12Device_proxy::EnumerateMetaCommandParameters(const GUID& CommandId, D3D12_META_COMMAND_PARAMETER_STAGE Stage, UINT* pTotalStructureSizeInBytes, UINT* pParameterCount, D3D12_META_COMMAND_PARAMETER_DESC* pParameterDescs)
{
    return m_orig->EnumerateMetaCommandParameters(CommandId, Stage, pTotalStructureSizeInBytes, pParameterCount, pParameterDescs);
}

HRESULT ID3D12Device_proxy::CreateMetaCommand(const GUID& CommandId, UINT NodeMask, const void* pCreationParametersData, SIZE_T CreationParametersDataSizeInBytes, const IID& riid, void** ppMetaCommand)
{
    return m_orig->CreateMetaCommand(CommandId, NodeMask, pCreationParametersData, CreationParametersDataSizeInBytes, riid, ppMetaCommand);
}

HRESULT ID3D12Device_proxy::CreateStateObject(const D3D12_STATE_OBJECT_DESC* pDesc, const IID& riid, void** ppStateObject)
{
    return m_orig->CreateStateObject(pDesc, riid, ppStateObject);
}

void ID3D12Device_proxy::GetRaytracingAccelerationStructurePrebuildInfo(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS* pDesc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO* pInfo)
{
    m_orig->GetRaytracingAccelerationStructurePrebuildInfo(pDesc, pInfo);
}

D3D12_DRIVER_MATCHING_IDENTIFIER_STATUS ID3D12Device_proxy::CheckDriverMatchingIdentifier(D3D12_SERIALIZED_DATA_TYPE SerializedDataType, const D3D12_SERIALIZED_DATA_DRIVER_MATCHING_IDENTIFIER* pIdentifierToCheck)
{
    return m_orig->CheckDriverMatchingIdentifier(SerializedDataType, pIdentifierToCheck);
}