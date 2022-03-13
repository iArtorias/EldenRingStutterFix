#include "pch.h"
#include "d3d12.h"
#include <string>

extern "C" UINT_PTR mProcs[19] = { 0 };

extern "C" void D3D12CoreCreateLayeredDevice_wrapper();
extern "C" void D3D12CoreGetLayeredDeviceSize_wrapper();
extern "C" void D3D12CoreRegisterLayers_wrapper();
extern "C" void D3D12CreateDevice_wrapper();
extern "C" void D3D12CreateRootSignatureDeserializer_wrapper();
extern "C" void D3D12CreateVersionedRootSignatureDeserializer_wrapper();
extern "C" void D3D12DeviceRemovedExtendedData_wrapper();
extern "C" void D3D12EnableExperimentalFeatures_wrapper();
extern "C" void D3D12GetDebugInterface_wrapper();
extern "C" void D3D12GetInterface_wrapper();
extern "C" void D3D12PIXEventsReplaceBlock_wrapper();
extern "C" void D3D12PIXGetThreadInfo_wrapper();
extern "C" void D3D12PIXNotifyWakeFromFenceSignal_wrapper();
extern "C" void D3D12PIXReportCounter_wrapper();
extern "C" void D3D12SerializeRootSignature_wrapper();
extern "C" void D3D12SerializeVersionedRootSignature_wrapper();
extern "C" void GetBehaviorValue_wrapper();
extern "C" void SetAppCompatStringPointer_wrapper();
extern "C" void ExportByOrdinal99();

LPCSTR mImportNames[] = {
  "D3D12CoreCreateLayeredDevice",
  "D3D12CoreGetLayeredDeviceSize",
  "D3D12CoreRegisterLayers",
  "D3D12CreateDevice",
  "D3D12CreateRootSignatureDeserializer",
  "D3D12CreateVersionedRootSignatureDeserializer",
  "D3D12DeviceRemovedExtendedData",
  "D3D12EnableExperimentalFeatures",
  "D3D12GetDebugInterface",
  "D3D12GetInterface",
  "D3D12PIXEventsReplaceBlock",
  "D3D12PIXGetThreadInfo",
  "D3D12PIXNotifyWakeFromFenceSignal",
  "D3D12PIXReportCounter",
  "D3D12SerializeRootSignature",
  "D3D12SerializeVersionedRootSignature",
  "GetBehaviorValue",
  "SetAppCompatStringPointer",
  (LPCSTR)99,
};



#define CHECK_NULL(val) if (!val) { MessageBoxA(nullptr, "Unexpected null value: "#val, "Error", MB_OK); return false; }

PFN_D3D12_CREATE_DEVICE D3D12CreateDevice_real;
HMODULE realDll;

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
    
        char realDllPath[MAX_PATH];
        GetSystemDirectoryA(realDllPath, MAX_PATH);
        strcat_s(realDllPath, "\\d3d12.dll");
        realDll = LoadLibraryA(realDllPath);
        CHECK_NULL(realDll);

        for (int i = 0; i < 19; ++i)
        {
            mProcs[i] = (UINT_PTR)GetProcAddress(realDll, mImportNames[i]);
            if (!mProcs[i])
            {
                std::string errorMsg = "Proc not found: ";
                if (HIWORD(mImportNames[i]) == 0)
                {
                    errorMsg += std::to_string(LOWORD(mImportNames[i]));
                }
                else
                {
                    errorMsg += mImportNames[i];
                }
                MessageBoxA(nullptr, errorMsg.c_str(), "Error", MB_OK);
                return FALSE;
            }
        }

        D3D12CreateDevice_real = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(realDll, "D3D12CreateDevice");
        CHECK_NULL(D3D12CreateDevice_real);
        MessageBoxA(nullptr, "Hooked d3d12.dll", "Debug", MB_OK);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        FreeLibrary(realDll);
    }
    return TRUE;
}
