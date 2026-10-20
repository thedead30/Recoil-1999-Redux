// SUBSYSTEM: sysinfo
// Declarations for src/unattributed/sysinfo.cpp. Spec: 04_spec/systems/sysinfo.md
#pragma once

namespace recoil {

// 0x004c6100 Crt_ChkStk: the statically linked MSVC stack probe (_chkstk): EAX = bytes to allocate; touches each
// 4 KB page below ESP, moves ESP down by EAX and returns to the caller with ESP lowered (custom convention).
void Crt_ChkStk();

// 0x004b2fa0
int __fastcall SysInfo_ReleaseObject0056bcbc(int ecx, int edx);
// 0x004b2fc0
int __fastcall SysInfo_DirectSoundGetCaps(int ecx, int edx);
// 0x004b2fe0
int __fastcall SysInfo_HasCpuid(int ecx, int edx);
// 0x004b3020
int __fastcall SysInfo_CpuidMmxBit(int ecx, int edx);
// 0x004b3050
int __fastcall Cpu_IsP6Model3Plus(int ecx, int edx);
// 0x004b3090
int __fastcall SysInfo_CopyInfoBlock(int ecx, int edx);
// 0x004b3210
int __fastcall SysInfo_ReturnZero(int ecx, int edx);
// 0x004b3220
int __fastcall SysInfo_HasPositiveVideoValue(int ecx, int edx);
// 0x004b3230
int __fastcall SysInfo_TotalPhysMemKB(int ecx, int edx);
// 0x004b33f0
int __fastcall SysInfo_CanToggleEflagsId(int ecx, int edx);
// 0x004b3510
int __fastcall SysInfo_DivFlagsProbe(int ecx, int edx);
// 0x004b3550
int __fastcall SysInfo_Is8086Probe(int ecx, int edx);
// 0x004b35a0
int __fastcall SysInfo_Is286Probe(int ecx, int edx);
// 0x004b35f0
int __fastcall SysInfo_Is386Probe(int ecx, int edx);
// 0x004b3640
int __fastcall SysInfo_CpuidFamily(int ecx, int edx);
// 0x004b37f0
int __fastcall SysInfo_SpeedByBsfLoop(int ecx, int edx, int arg1);
// 0x004b38e0
int __fastcall SysInfo_SpeedByTscQpc(int ecx, int edx, int arg1);
// 0x004b3b00
int __fastcall SysInfo_ReadCmosSeconds(int ecx, int edx);
// 0x004b3b20
int __fastcall SysInfo_ReadTsc(int ecx, int edx);
// 0x004b3ca0
int __fastcall SysInfo_Sub64(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004c60b0
int __fastcall crt_onexit(int ecx, int edx);
// 0x004b3160
int __fastcall SysInfo_GetCpuVendorString(int ecx, int edx);
// 0x004b31f0
int __fastcall SysInfo_HasMmx(int ecx, int edx);
// 0x004b3420
int __fastcall SysInfo_GetCpuFamily(int ecx, int edx);
// 0x004b3480
int __fastcall SysInfo_CpuidIsGenuineIntel(int ecx, int edx);
// 0x004b3b50
int __fastcall SysInfo_SpeedByTscCmos(int ecx, int edx, int arg1);
// 0x004c60e0
int __fastcall crt_atexit(int ecx, int edx);
// 0x004b31b0
int __fastcall SysInfo_GetCpuFamilyWord(int ecx, int edx);
// 0x004b36f0
int __fastcall SysInfo_MeasureCpuSpeed(int ecx, int edx, int arg1);
// 0x004b31c0
int __fastcall SysInfo_GetField0CFrom004b36f0(int ecx, int edx);
// 0x004b2f50
int __fastcall SysInfo_GetDirectSound(int ecx, int edx);
// 0x004b30b0
int __fastcall SysInfo_Collect(int ecx, int edx);
}  // namespace recoil
