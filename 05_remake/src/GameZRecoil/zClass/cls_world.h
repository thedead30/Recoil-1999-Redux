// SUBSYSTEM: cls_world
// Declarations for src/GameZRecoil/zClass/cls_world.cpp.
#pragma once

namespace recoil {

// 0x00450650
int __fastcall World_PointToCellClamped(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
// 0x00450790
int __fastcall World_PointToCellClampedSimple(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00450840
int __fastcall World_BoxToCell(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00450a00
int __fastcall World_GetCell(int ecx, int edx, int arg1);
// 0x00450ae0
int __fastcall World_SetParam10(int ecx, int edx);
// 0x00450af0
int __fastcall World_SetParams14(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00450b20
int __fastcall World_SetParams28(int ecx, int edx, int arg1, int arg2);
// 0x00450b40
int __fastcall World_SetParams20(int ecx, int edx, int arg1, int arg2);
// 0x00450b60
int __fastcall World_SetParam30(int ecx, int edx, int arg1);
// 0x00450b80
int __fastcall World_GetParam30(int ecx, int edx);
// 0x00450b90
int __fastcall World_GetParam10(int ecx, int edx);
// 0x00450ba0
int __fastcall World_GetParams14(int ecx, int edx, int arg1, int arg2);
// 0x00450bc0
int __fastcall World_GetParams20(int ecx, int edx, int arg1);
// 0x00450be0
int __fastcall World_GetParams28(int ecx, int edx, int arg1);
// 0x00450c00
int __fastcall World_SetOrigin(int ecx, int edx, int arg1, int arg2);
// 0x00450c30
int __fastcall World_SetExtent(int ecx, int edx, int arg1, int arg2);
// 0x00450e40
int __fastcall World_FreeGrid(int ecx, int edx);
// 0x00450f00
int __fastcall World_SetCellMargins(int ecx, int edx, int arg1, int arg2);
// 0x00450f20
int __fastcall World_SetPartitionMax(int ecx, int edx);
// 0x00451360
int __fastcall World_AddLight(int ecx, int edx);
// 0x00451410
int __fastcall World_RemoveLight(int ecx, int edx);
// 0x00451590
int __fastcall World_AddSound(int ecx, int edx);
// 0x00451640
int __fastcall World_RemoveSound(int ecx, int edx);
// 0x004500b0
int __fastcall World_RefreshCellBounds(int ecx, int edx);
// 0x00450c60
int __fastcall World_BuildGrid(int ecx, int edx, int arg1, int arg2);
// 0x00476170
int __fastcall Render_SetGlobal_0057d930(int ecx, int edx);
// 0x00450030
int __fastcall World_QueueDirtyCell(int ecx, int edx, int arg1);
// 0x00450a70
int __fastcall World_RefreshCell(int ecx, int edx, int arg1);
// 0x00450f60
int __fastcall World_InsertChildInCell(int ecx, int edx, int arg1, int arg2);
// 0x004510e0
int __fastcall World_AttachChild(int ecx, int edx);
// 0x00451240
int __fastcall World_DetachChild(int ecx, int edx);
// 0x004501c0
int __fastcall World_Create(int ecx, int edx);
// 0x00450240
int __fastcall World_Destroy(int ecx, int edx);
// 0x004517a0
int __fastcall GWWorld_BuildSaveRecords(int ecx, int edx);
// 0x00451540
int __fastcall Light_PrepareFrameLightList(int ecx, int edx);
// 0x00450530
int __fastcall World_Update(int ecx, int edx);
// 0x004502b0
int __fastcall World_RebucketBorderCells(int ecx, int edx);
// 0x00450510
int __fastcall World_SetRebucket50(int ecx, int edx);
// 0x00451560
int __fastcall Light_UpdateAllInNode(int ecx, int edx);
// 0x00451770
int __fastcall Sound_ProcessWorldNodeEmitters(int ecx, int edx);
}  // namespace recoil
