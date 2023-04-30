#pragma once
#include <vector> 
#include <string>
#include <Windows.h>
#include <ImageHlp.h>
#include <iostream>
#pragma comment (lib, "ImageHlp.lib")
#define PTR(x) reinterpret_cast<void*>(x)
#define NT_CHECK(x) if(!x) \
					return FALSE;

namespace exports {
	const auto OHdl = GetCurrentProcess();
	struct ExportData {
		const std::string Name;
		UINT_PTR Address;
	};
	using VecType = std::vector<ExportData>;
	namespace get {
		auto GetExports(
			IN const std::string DllName, 
			IN OPTIONAL const BOOL ReadOnly = TRUE
		) -> const VecType 
		{
			_IMAGE_EXPORT_DIRECTORY* ImgExport = nullptr;
			auto Img = _LOADED_IMAGE{};
			auto ExportNames = VecType{};
			auto Hdl = GetModuleHandleA(DllName.c_str());
			if (
				MapAndLoad(
					DllName.c_str(), 
					nullptr,
					&Img, 
					TRUE,
					ReadOnly
				) &&
				Hdl
				)
			{
				auto Sz = ULONG(NULL);
				ImgExport = (_IMAGE_EXPORT_DIRECTORY*)
					ImageDirectoryEntryToData(
						Img.MappedAddress,
						false, 
						IMAGE_DIRECTORY_ENTRY_EXPORT, 
						&Sz
					);
				auto RVAs = (DWORD*)
					ImageRvaToVa(
					Img.FileHeader,
					Img.MappedAddress,
					ImgExport->AddressOfNames, nullptr
				);
				for (size_t idx = 0; idx < ImgExport->NumberOfNames; idx++) {
					auto ExportName = reinterpret_cast<const char*>(
						ImageRvaToVa(
							Img.FileHeader,
							Img.MappedAddress,
							RVAs[idx], nullptr
						)
					);
					ExportNames.push_back({
						std::string(ExportName),
						reinterpret_cast<UINT_PTR>(GetProcAddress(Hdl,ExportName))
						}
					);
				}
			}
			return ExportNames;
		}
		auto GetExportByName(
			IN std::string Function, 
			IN std::string DllName, 
			IN OPTIONAL const BOOL ReadOnly = TRUE
		) -> const ExportData 
		{
			auto Exports = GetExports(DllName, ReadOnly);
			for (const auto& Export : Exports) {
				if (Export.Name == Function) {
					return Export;
				}
			}
			return {};
		}
	}
	namespace fix {
		auto FixExport(
			IN const HANDLE ProcessHdl, 
			IN const ExportData Exp
		) -> BOOL 
		{
			BYTE Bytes[5]; 
			BYTE OBytes[5]; 
			DWORD OldProc;
			DWORD NewProc;
			NT_CHECK(
				ReadProcessMemory(
					OHdl,
					PTR(Exp.Address),
					&OBytes,
					sizeof(OBytes),
					nullptr
				)
			)
			NT_CHECK(
				ReadProcessMemory(
					ProcessHdl, 
					PTR(Exp.Address), 
					&Bytes, 
					sizeof(Bytes), 
					nullptr
				)
			)
			NT_CHECK(
				VirtualProtectEx(
					ProcessHdl,
					PTR(Exp.Address), 
					sizeof(OBytes), 
					PAGE_EXECUTE_READWRITE, 
					&OldProc
				)
			)
			NT_CHECK(
				WriteProcessMemory(
					ProcessHdl,
					PTR(Exp.Address), 
					OBytes, 
					sizeof(OBytes), 
					nullptr
				)
			)
			NT_CHECK(
				VirtualProtectEx(ProcessHdl, 
					PTR(Exp.Address),
					sizeof(OBytes), 
					OldProc, 
					&NewProc
				)
			)
			return TRUE;
		}
	}
}