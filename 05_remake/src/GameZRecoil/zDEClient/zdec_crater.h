// SUBSYSTEM: declient
// Declarations for src/GameZRecoil/zDEClient/zdec_crater.cpp.
#pragma once

namespace recoil {

// 0x00458aa0
int __fastcall DEClient_SetCurrentWorld(int ecx, int edx);
// 0x00458ac0
int __fastcall World_GetPartitionCell(int ecx, int edx);
// 0x00458ae0
int __fastcall DEClient_GetCurrentWorld(int ecx, int edx);
// 0x00458af0
int __fastcall DEClient_SetVec00575dc8(int ecx, int edx);
// 0x00458b20
int __fastcall DEClient_SetByteList00575dd4(int ecx, int edx);
// 0x00458a70
int __fastcall Record52_FillN(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0045ae30
int __fastcall Script_AdvanceRecord(int ecx, int edx, int arg1);
// 0x0045c2f0
int __fastcall Script_Rewind(int ecx, int edx);
// 0x0045bb00
int __fastcall Script_CmdResumeLocalScript(int ecx, int edx);
// 0x0045bbb0
int __fastcall Script_CmdStopLocalScript(int ecx, int edx);
// 0x0045c240
int __fastcall Script_CmdAnimNodeSetState(int ecx, int edx);
// 0x00459510
int __fastcall Script_CmdWorldSetup(int ecx, int edx);
// 0x00459cb0
int __fastcall Script_CmdSetScale(int ecx, int edx);
// 0x0045b210
int __fastcall Script_CmdSetObjectFlags(int ecx, int edx);
// 0x0045b440
int __fastcall Script_CmdSetAnimFrame(int ecx, int edx, int arg1);
// 0x0045c6b0
int __fastcall Script_SkipToQuote(int ecx, int edx, int arg1);
// 0x0045c6e0
int __fastcall Script_CmdNop(int ecx, int edx, int arg1);
// 0x0045b280
int __fastcall Script_CmdValueRamp(int ecx, int edx, int arg1);
// 0x00458a30
int __fastcall Record52_CopyRange(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0045c6f0
int __fastcall Script_CmdCallback(int ecx, int edx, int arg1);
// 0x00457c50
int __fastcall DEClient_DispatchRecords(int ecx, int edx);
// 0x00457040
int __fastcall DEObjectB_Create(int ecx, int edx);
// 0x00458c10
int __fastcall Anim_PlaceAndScaleAlongSegment_Static(int ecx, int edx, int arg1);
// 0x00458ce0
int __fastcall Anim_PlaceAndScaleAlongSegment_Animated(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00457b40
int __fastcall DEClient_BuildHazardSaveRecords(int ecx, int edx);
// 0x0045c310
int __fastcall Script_CmdWaitOrLoop(int ecx, int edx, int arg1);
// 0x00456c80
int __fastcall DEObjectB_BuildCraterDisc(int ecx, int edx);
// 0x004570e0
int __fastcall DEObjectB_RebuildClip(int ecx, int edx);
// 0x00457840
int __fastcall DEClient_AddHazardRecord(int ecx, int edx);
// 0x0045c710
int __fastcall Script_CmdColourFade(int ecx, int edx, int arg1);
// 0x0045cbc0
int __fastcall Script_CmdShowMessage(int ecx, int edx);
// 0x0045c640
int __fastcall Anim_SquaredDistanceToEventAnchor(int ecx, int edx);
// 0x0045b4a0
int __fastcall AnimEvent_StretchSegmentBetweenAnchors(int ecx, int edx, int arg1);
// 0x0045b3b0
int __fastcall Script_CmdAttach(int ecx, int edx);
// 0x0045b410
int __fastcall Script_CmdDetach(int ecx, int edx);
// 0x00459580
int __fastcall Script_CmdCameraParams(int ecx, int edx, int arg1);
// 0x004596c0
int __fastcall AnimEvent_AnimateCameraOptics(int ecx, int edx, int arg1);
// 0x00459ae0
int __fastcall Script_CmdSetRotation(int ecx, int edx);
// 0x00459ce0
int __fastcall Script_CmdSetPosition(int ecx, int edx);
// 0x0045a9d0
int __fastcall AnimEvent_AnimateObjectTransform(int ecx, int edx, int arg1);
// 0x0045ae90
int __fastcall Script_ExecuteKeyframe(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00457140
int __fastcall DEObjectB_TesselateCrater(int ecx, int edx);
// 0x0045b120
int __fastcall Script_RunTimed(int ecx, int edx, int arg1);
// 0x0045c920
int __fastcall AnimEvent_ScreenShakeRect(int ecx, int edx, int arg1);
// 0x00459e30
int __fastcall Script_CmdSetActive(int ecx, int edx);
// 0x0045bf60
int __fastcall DEScene_DeactivateLights(int ecx, int edx);
// 0x0045bfd0
int __fastcall DEScene_DeactivateSounds(int ecx, int edx);
// 0x00458f70
int __fastcall Script_CmdSoundNodeControl(int ecx, int edx);
// 0x00459080
int __fastcall Script_CmdLightControl(int ecx, int edx);
// 0x00459280
int __fastcall AnimEvent_FadeLightColorAndRange(int ecx, int edx, int arg1);
// 0x00458b50
int __fastcall DEObject_TickTimer(int ecx, int edx, int arg1);
// 0x00458e10
int __fastcall Script_CmdPlaySound(int ecx, int edx);
// 0x00458eb0
int __fastcall Script_CmdCamera(int ecx, int edx);
// 0x00459e70
int __fastcall AnimEvent_SpawnPhysicsDebris(int ecx, int edx, int arg1);
// 0x0045a920
int __fastcall Ground_FindSurfaceNear(int ecx, int edx);
// 0x0045b8b0
int __fastcall Script_CmdSpawnFromAnimNode(int ecx, int edx);
// 0x0045bc60
int __fastcall Script_CmdSpawnAnimNode(int ecx, int edx, int arg1);
// 0x0045c040
int __fastcall Effect_StopOrFade(int ecx, int edx);
// 0x0045c100
int __fastcall Script_CmdAnimNodeStopOrFade(int ecx, int edx);
// 0x0045c1a0
int __fastcall Script_CmdAnimNodeCall45d6b0(int ecx, int edx);
// 0x0045c3c0
int __fastcall AnimEvent_EvaluateConditionChain(int ecx, int edx, int arg1);
// 0x0045c530
int __fastcall Anim_ProximityClearanceTest(int ecx, int edx, int arg1, int arg2);
// 0x00456b20
int __fastcall DEClient_InstanceCrater(int ecx, int edx);
// 0x00456c50
int __fastcall Query_RunWithGateHook(int ecx, int edx);
// 0x004575f0
int __fastcall DEClient_RegisterAllEntries(int ecx, int edx);
// 0x00457660
int __fastcall DEClient_ResetState00539df0(int ecx, int edx);
// 0x004576a0
int __fastcall DEClient_RegisterAtExit(int ecx, int edx);
// 0x004576e0
int __fastcall DEClient_Tree_Destroy(int ecx, int edx);
// 0x00457750
int __fastcall DEClient_DestroyAllObjects(int ecx, int edx);
// 0x00457ae0
int __fastcall DEClient_ClearTree(int ecx, int edx);
// 0x00457cc0
int __fastcall DEClient_Tree_Construct(int ecx, int edx, int arg1, int arg2);
// 0x00457d90
int __fastcall DEClient_Tree_InsertUnique(int ecx, int edx, int arg1, int arg2);
// 0x00457e80
int __fastcall DEClient_Tree_EraseRange(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x00457fe0
int __fastcall DEClient_Tree_EraseNode(int ecx, int edx, int arg1, int arg2);
// 0x00458510
int __fastcall DEClient_Tree_EraseSubtree(int ecx, int edx, int arg1);
// 0x004585a0
int __fastcall DEClient_Tree_InsertNode(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004588c0
int __fastcall DEClient_TreeIterator_Increment(int ecx, int edx);
// 0x00458970
int __fastcall DEClient_TreeIterator_Decrement(int ecx, int edx);
// 0x004cad80
int __fastcall EH_Unwind_DEClient_Tree_Construct_0(int ecx, int edx);
// 0x004cada0
int __fastcall EH_Unwind_DEClient_Tree_EraseRange_0(int ecx, int edx);
// 0x004cada9
int __fastcall EH_Unwind_DEClient_Tree_EraseRange_1(int ecx, int edx);
// 0x004cadc0
int __fastcall EH_Unwind_DEClient_Tree_EraseNode_0(int ecx, int edx);
// 0x004cade0
int __fastcall EH_Unwind_DEClient_Tree_EraseSubtree_0(int ecx, int edx);
// 0x004cae00
int __fastcall EH_Unwind_DEClient_Tree_InsertNode_0(int ecx, int edx);
// 0x004cae20
int __fastcall EH_Unwind_DEClient_TreeIterator_Increment_0(int ecx, int edx);
// 0x004cae40
int __fastcall EH_Unwind_DEClient_TreeIterator_Decrement_0(int ecx, int edx);
// 0x004cad89
int __fastcall EH_Handler_DEClient_Tree_Construct(int ecx, int edx);

// 0x004cadb2
int __fastcall EH_Handler_DEClient_Tree_EraseRange(int ecx, int edx);

// 0x004cadc9
int __fastcall EH_Handler_DEClient_Tree_EraseNode(int ecx, int edx);

// 0x004cade9
int __fastcall EH_Handler_DEClient_Tree_EraseSubtree(int ecx, int edx);

// 0x004cae09
int __fastcall EH_Handler_DEClient_Tree_InsertNode(int ecx, int edx);

// 0x004cae29
int __fastcall EH_Handler_DEClient_TreeIterator_Increment(int ecx, int edx);

// 0x004cae49
int __fastcall EH_Handler_DEClient_TreeIterator_Decrement(int ecx, int edx);

}  // namespace recoil
