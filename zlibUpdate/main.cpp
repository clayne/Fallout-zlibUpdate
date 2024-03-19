#include "nvse/PluginAPI.h"
#include "zlib.h"

#pragma comment(lib, "zlibstat.lib")

NVSEInterface* g_nvseInterface{};

static void SafeWrite32(UInt32 addr, UInt32 data) {
	UInt32	oldProtect;

	VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	*((UInt32*)addr) = data;
	VirtualProtect((void*)addr, 4, oldProtect, &oldProtect);
}

void SafeWrite16(UInt32 addr, UInt32 data) {
	UInt32	oldProtect;

	VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	*((UInt16*)addr) = data;
	VirtualProtect((void*)addr, 4, oldProtect, &oldProtect);
}
template <typename T>
DECLSPEC_NOINLINE void ReplaceCall(UInt32 jumpSrc, T jumpTgt)
{
	SafeWrite32(jumpSrc + 1, UInt32(jumpTgt) - jumpSrc - 1 - 4);
}

static int __cdecl inflateInit_Ex(z_streamp strm, const char* version, int stream_size) {
	return inflateInit2_(strm, 15, ZLIB_VERSION, stream_size);
}

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "zlib";
	info->version = 131;

	return true;
}

bool NVSEPlugin_Load(NVSEInterface* nvse)
{

constexpr UInt32 zLibAllocSize = 0x1C08;

	if (!nvse->isEditor) {
		ReplaceCall(0x4742AC, inflateInit_Ex); // TESFile::DecompressCurrentForm
		ReplaceCall(0xAFC537, inflateInit_Ex); // CompressedArchiveFile::CompressedArchiveFile

		// Inflate
		ReplaceCall(0x47434F, inflate); // TESFile::DecompressCurrentForm
		ReplaceCall(0xAFC1F4, inflate); // CompressedArchiveFile::CompressedArchiveFile

		// End
		for (UInt32 uiAddr : { 0x4742CA, 0x474388, 0x4743D5, 0x474419 })
			ReplaceCall(uiAddr, inflateEnd); // TESFile::DecompressCurrentForm

		for (UInt32 uiAddr : { 0xAFC00E, 0xAFC21B, 0xAFC552 })
			ReplaceCall(uiAddr, inflateEnd); // CompressedArchiveFile::CompressedArchiveFile, CompressedArchiveFile::StandardReadF

		SafeWrite16(0xAFC4A2, zLibAllocSize);	// Increase allocation size
	}
	else {
		ReplaceCall(0x4DFB34, inflateInit_Ex); // TESFile::DecompressCurrentForm
		ReplaceCall(0x8AAF17, inflateInit_Ex); // CompressedArchiveFile::CompressedArchiveFile

		// Inflate
		ReplaceCall(0x4DFB9E, inflate); // TESFile::DecompressCurrentForm
		ReplaceCall(0x8AABD4, inflate); // CompressedArchiveFile::CompressedArchiveFile

		// End
		for (UInt32 uiAddr : { 0x4DFB45, 0x4DFC1F, 0x4DFBD5, 0x4DFBC4 })
			ReplaceCall(uiAddr, inflateEnd); // TESFile::DecompressCurrentForm

		for (UInt32 uiAddr : { 0x8AA9EE, 0x8AABFB, 0x8AAF32 })
			ReplaceCall(uiAddr, inflateEnd); // CompressedArchiveFile::~CompressedArchiveFile, CompressedArchiveFile::StandardReadF

		SafeWrite16(0x8AAE81, zLibAllocSize); // Increase allocation size
	}

	return true;
}
