.code
extern mProcs:QWORD
D3D12CoreCreateLayeredDevice_wrapper proc
  jmp mProcs[0 * 8]
D3D12CoreCreateLayeredDevice_wrapper endp
D3D12CoreGetLayeredDeviceSize_wrapper proc
  jmp mProcs[1 * 8]
D3D12CoreGetLayeredDeviceSize_wrapper endp
D3D12CoreRegisterLayers_wrapper proc
  jmp mProcs[2 * 8]
D3D12CoreRegisterLayers_wrapper endp
D3D12CreateDevice_wrapper proc
  jmp mProcs[3 * 8]
D3D12CreateDevice_wrapper endp
D3D12CreateRootSignatureDeserializer_wrapper proc
  jmp mProcs[4 * 8]
D3D12CreateRootSignatureDeserializer_wrapper endp
D3D12CreateVersionedRootSignatureDeserializer_wrapper proc
  jmp mProcs[5 * 8]
D3D12CreateVersionedRootSignatureDeserializer_wrapper endp
D3D12DeviceRemovedExtendedData_wrapper proc
  jmp mProcs[6 * 8]
D3D12DeviceRemovedExtendedData_wrapper endp
D3D12EnableExperimentalFeatures_wrapper proc
  jmp mProcs[7 * 8]
D3D12EnableExperimentalFeatures_wrapper endp
D3D12GetDebugInterface_wrapper proc
  jmp mProcs[8 * 8]
D3D12GetDebugInterface_wrapper endp
D3D12GetInterface_wrapper proc
  jmp mProcs[9 * 8]
D3D12GetInterface_wrapper endp
D3D12PIXEventsReplaceBlock_wrapper proc
  jmp mProcs[10 * 8]
D3D12PIXEventsReplaceBlock_wrapper endp
D3D12PIXGetThreadInfo_wrapper proc
  jmp mProcs[11 * 8]
D3D12PIXGetThreadInfo_wrapper endp
D3D12PIXNotifyWakeFromFenceSignal_wrapper proc
  jmp mProcs[12 * 8]
D3D12PIXNotifyWakeFromFenceSignal_wrapper endp
D3D12PIXReportCounter_wrapper proc
  jmp mProcs[13 * 8]
D3D12PIXReportCounter_wrapper endp
D3D12SerializeRootSignature_wrapper proc
  jmp mProcs[14 * 8]
D3D12SerializeRootSignature_wrapper endp
D3D12SerializeVersionedRootSignature_wrapper proc
  jmp mProcs[15 * 8]
D3D12SerializeVersionedRootSignature_wrapper endp
GetBehaviorValue_wrapper proc
  jmp mProcs[16 * 8]
GetBehaviorValue_wrapper endp
SetAppCompatStringPointer_wrapper proc
  jmp mProcs[17 * 8]
SetAppCompatStringPointer_wrapper endp
ExportByOrdinal99 proc
  jmp mProcs[18 * 8]
ExportByOrdinal99 endp
end