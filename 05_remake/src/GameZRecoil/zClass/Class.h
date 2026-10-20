// SUBSYSTEM: class_api
// Declarations for src/GameZRecoil/zClass/Class.cpp.
#pragma once

namespace recoil {

// 0x00447a70
int __fastcall gwNodeFree(int ecx, int edx);
// 0x00447bc0
int __fastcall gwNodeFindByName(int ecx, int edx);
// 0x00447d20
int __fastcall gwNodeSetFlag10000(int ecx, int edx);
// 0x00447d70
int __fastcall gwNodeSetFlag20000(int ecx, int edx);
// 0x00447dc0
int __fastcall gwNodeSetName(int ecx, int edx);
// 0x00447e30
int __fastcall gwNodeValidate(int ecx, int edx);
// 0x00447f00
int __fastcall gwNodeGetModel(int ecx, int edx);
// 0x00448100
int __fastcall gwNodeSetFlag08(int ecx, int edx);
// 0x00448140
int __fastcall gwNodeGetFlag08(int ecx, int edx);
// 0x00448180
int __fastcall gwNodeGetByte30(int ecx, int edx);
// 0x004481b0
int __fastcall gwNodeSetFlag10(int ecx, int edx);
// 0x004481f0
int __fastcall gwNodeGetFlag10(int ecx, int edx);
// 0x00448230
int __fastcall gwNodeSetFlag20(int ecx, int edx);
// 0x00448270
int __fastcall gwNodeGetFlag20(int ecx, int edx);
// 0x004482b0
int __fastcall gwNodeSetFlag40(int ecx, int edx);
// 0x004482f0
int __fastcall gwNodeSetFlag80(int ecx, int edx);
// 0x00448330
int __fastcall gwNodeSetByte30(int ecx, int edx);
// 0x00448360
int __fastcall gwNodeClearFlag1000000(int ecx, int edx);
// 0x004483a0
int __fastcall gwNodeSetFlag800000(int ecx, int edx);
// 0x00448760
int __fastcall gwNodeGetBox(int ecx, int edx);
// 0x004487c0
int __fastcall gwNodeGetLocalCorners(int ecx, int edx);
// 0x00448920
int __fastcall Render_ExpandNodeAABBToCorners(int ecx, int edx);
// 0x00449420
int __fastcall gwNodeUpdateModelBox(int ecx, int edx);
// 0x00449af0
int __fastcall gwNodeGetRootBelowWorld(int ecx, int edx);
// 0x00449b40
int __fastcall gwNodeSetSubtreeFlag80000(int ecx, int edx);
// 0x00449ab0
int __fastcall gwNodeGetRoot(int ecx, int edx);
// 0x004491b0
int __fastcall gwNodeUpdateChildrenBox(int ecx, int edx);
// 0x00447fe0
int __fastcall gwNodeSetActionCallbackFirst(int ecx, int edx);
// 0x004478c0
int __fastcall gwNodeAlloc(int ecx, int edx);
// 0x00447f30
int __fastcall gwNodeSetActionCallback(int ecx, int edx);
// 0x00448090
int __fastcall gwNodeSetActionList(int ecx, int edx);
// 0x00447e60
int __fastcall gwNodeSetModel(int ecx, int edx);
// 0x004484d0
int __fastcall gwNodeAttachChild(int ecx, int edx);
// 0x00448660
int __fastcall gwNodeDetachChild(int ecx, int edx);
// 0x00447b60
int __fastcall gwNodeDelete(int ecx, int edx);
// 0x00448e90
int __fastcall gwNodeUpdateBoxAndPropagate(int ecx, int edx);
// 0x00448cc0
int __fastcall gwNodeUpdate(int ecx, int edx);
// 0x00449480
int __fastcall gwNodeBuildNodeToAncestorMatrix(int ecx, int edx);
// 0x004497b0
int __fastcall gwNodeGetWorldPosition(int ecx, int edx);
// 0x004498e0
int __fastcall gwNodeGetWorldOrientation(int ecx, int edx, int arg1);
// 0x00449850
int __fastcall gwNodeLocalToWorld(int ecx, int edx);
// 0x004483f0
int __fastcall gwNodeAttachChildByClass(int ecx, int edx);
// 0x00448570
int __fastcall gwNodeDetachChildByClass(int ecx, int edx);
// 0x00447980
int __fastcall gwNodeDestroy(int ecx, int edx);
// 0x00447c60
int __fastcall gwNodeSetActive(int ecx, int edx);
}  // namespace recoil
