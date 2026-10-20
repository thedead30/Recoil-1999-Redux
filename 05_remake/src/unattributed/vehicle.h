// SUBSYSTEM: vehicle
// Declarations for src/unattributed/vehicle.cpp.
#pragma once

namespace recoil {

// 0x0041bab0
int __fastcall Vehicle_UpdateFireRequests(int ecx, int edx);
// 0x00426350
int __fastcall Sign_Float(int ecx, int edx, int arg1);
// 0x004283f0
int __fastcall Vehicle_CockpitThrottle(int ecx, int edx);
// 0x004290f0
int __fastcall Terrain_FindGroundHeight(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00429240
int __fastcall Vehicle_ApplyBodyWobble(int ecx, int edx, int arg1);
// 0x00429430
int __fastcall Vehicle_ApplyKnockback(int ecx, int edx, int arg1, int arg2);
// 0x004294d0
int __fastcall Vehicle_ExtractAxesFromMatrix(int ecx, int edx);
// 0x00429f20
int __fastcall VehicleGlobals_StaticInit(int ecx, int edx);
// 0x0042b810
int __fastcall Vehicle_SyncPlayerTransformFromObject(int ecx, int edx);
// 0x0042b8c0
int __fastcall Vehicle_ComputeGroundContactHeight(int ecx, int edx);
// 0x0042b970
int __fastcall Vehicle_BuildBasisFromUpForward(int ecx, int edx);
// 0x0042bab0
int __fastcall Vehicle_ComputeAimYawToPoint(int ecx, int edx);
// 0x0042bed0
int __fastcall Vehicle_ZeroMotion(int ecx, int edx);
// 0x0042c420
int __fastcall Vehicle_ApplySlopeForces(int ecx, int edx);
// 0x0042cbd0
int __fastcall SurfaceMask_AndThree(int ecx, int edx, int arg1);
// 0x0042cde0
int __fastcall Vehicle_PlaneHeightAt(int ecx, int edx, int arg1);
// 0x0042ce50
int __fastcall Vehicle_SetUpFromTriangle(int ecx, int edx, int arg1, int arg2);
// 0x0042cf60
int __fastcall Vehicle_BuildActiveContactList(int ecx, int edx);
// 0x0042d560
int __fastcall Vec3_Midpoint(int ecx, int edx, int arg1);
// 0x0043afd0
int __fastcall Vehicle_ComposeLocalMatrix(int ecx, int edx);
// 0x004b28e0
int __fastcall Globals_SetPair779a80(int ecx, int edx);
// 0x0042cc00
int __fastcall Vehicle_SelectTopContacts(int ecx, int edx);
// 0x00439b70
int __fastcall Vehicle_StopAlarmSounds(int ecx, int edx);
// 0x00428c20
int __fastcall Vehicle_SubVerticalSpeed(int ecx, int edx);
// 0x0042aa50
int __fastcall Vehicle_DrawDebugInfo(int ecx, int edx, int arg1);
// 0x0042c2e0
int __fastcall Vehicle_TrackVerticalVelocity(int ecx, int edx);
// 0x0042da40
int __fastcall Vehicle_AlignToUpNormal(int ecx, int edx);
// 0x0043b5d0
int __fastcall Vehicle_AdjustHealth(int ecx, int edx, int arg1);
// 0x0042d320
int __fastcall Vehicle_ThirdContactSnap(int ecx, int edx);
// 0x00426330
int __fastcall Vehicle_ResetLookOffset(int ecx, int edx);
// 0x00439990
int __fastcall Vehicle_ClearFireState(int ecx, int edx);
// 0x0043b790
int __fastcall Vehicle_ApplyEffectSlotHit(int ecx, int edx, int arg1);
// 0x0042be00
int __fastcall Vehicle_Teleport(int ecx, int edx, int arg1);
// 0x00405650
int __fastcall Vehicle_ResetChaseCamera(int ecx, int edx);
// 0x0042be70
int __fastcall Vehicle_TeleportToNode(int ecx, int edx);
// 0x00429f50
int __fastcall Vehicle_FreeVector_00429f50(int ecx, int edx);
// 0x00429f40
int __fastcall VehicleGlobals_StaticAtexit(int ecx, int edx);
// 0x00429f10
int __fastcall StaticInitWrapper_00429f10(int ecx, int edx);
// 0x00438690
int __fastcall Vehicle_StopModeTransitionSoundSlot(int ecx, int edx, int arg1);
// 0x00429ef0
int __fastcall Vehicle_EndSkid(int ecx, int edx);
// 0x0042b5a0
int __fastcall Copters_Activate(int ecx, int edx);
// 0x0042b630
int __fastcall Copters_ResolveAndDeactivate(int ecx, int edx);
// 0x00438540
int __fastcall Vehicle_ActivateModeConfigNode(int ecx, int edx, int arg1);
// 0x0042b4c0
int __fastcall Vehicle_TransitionToFlyModeUnused(int ecx, int edx);
// 0x00405c90
int __fastcall Vehicle_SetCameraViewState(int ecx, int edx);
// 0x004266b0
int __fastcall Vehicle_DispatchMovementPhysics(int ecx, int edx);
// 0x00426770
int __fastcall Vehicle_UpdatePhysicsFrame(int ecx, int edx);
// 0x00427140
int __fastcall Vehicle_UpdateHoverModePhysics(int ecx, int edx);
// 0x00427440
int __fastcall Vehicle_HoverSuspension(int ecx, int edx);
// 0x004279f0
int __fastcall Vehicle_UpdateAmphibModePhysics(int ecx, int edx);
// 0x00427ec0
int __fastcall Vehicle_AmphibGroundSnap(int ecx, int edx);
// 0x00428120
int __fastcall Vehicle_UpdateBasicModePhysics(int ecx, int edx);
// 0x00428350
int __fastcall Vehicle_BasicGroundFollow(int ecx, int edx);
// 0x00428490
int __fastcall Vehicle_BasicYawInput(int ecx, int edx);
// 0x00428520
int __fastcall Vehicle_UpdateSubModePhysics(int ecx, int edx);
// 0x004289f0
int __fastcall Vehicle_UpdateGroundClearanceAndAutoAmphib(int ecx, int edx);
// 0x00428d60
int __fastcall Vehicle_GatherGroundContacts(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00429560
int __fastcall Vehicle_AutoTurnToHeading(int ecx, int edx);
// 0x00429750
int __fastcall Vehicle_IntegrateSteering(int ecx, int edx);
// 0x00429870
int __fastcall Vehicle_IntegrateThrottle(int ecx, int edx);
// 0x00429b40
int __fastcall Vehicle_ComputeLateralSlipAccel(int ecx, int edx);
// 0x00429d30
int __fastcall Vehicle_IntegrateSkid(int ecx, int edx);
// 0x00429ed0
int __fastcall Vehicle_StartSkid(int ecx, int edx);
// 0x0042ac90
int __fastcall Vehicle_TransitionToTrackMode(int ecx, int edx);
// 0x0042aeb0
int __fastcall Vehicle_TransitionToAmphibMode(int ecx, int edx, int arg1);
// 0x0042b0f0
int __fastcall Vehicle_TransitionToHoverMode(int ecx, int edx);
// 0x0042b2a0
int __fastcall Vehicle_TransitionToSubMode(int ecx, int edx);
// 0x0042b4a0
int __fastcall Vehicle_ReleaseF48(int ecx, int edx);
// 0x0042b520
int __fastcall Vehicle_RequestModeTransitionByEnum(int ecx, int edx, int arg1);
// 0x0042bf90
int __fastcall Vehicle_UpdateTrackModeContactAndBuoyancy(int ecx, int edx);
// 0x0042c0d0
int __fastcall Vehicle_TrackContactDispatch(int ecx, int edx);
// 0x0042c520
int __fastcall Vehicle_SingleContactSnap(int ecx, int edx);
// 0x0042c640
int __fastcall Vehicle_TwoContactSnap(int ecx, int edx);
// 0x0042c8d0
int __fastcall Vehicle_ApplyContactTorque(int ecx, int edx, int arg1);
// 0x0042ca40
int __fastcall Vehicle_SnapToContactPlane(int ecx, int edx);
// 0x0042cb50
int __fastcall Vehicle_StopRotation_EngineWhine(int ecx, int edx);
// 0x0042cf90
int __fastcall Vehicle_TrackContactsGather(int ecx, int edx);
// 0x0042d5c0
int __fastcall Vehicle_UpdateWaterBuoyancyAndAutoTransition(int ecx, int edx);
// 0x004386c0
int __fastcall Vehicle_UpdateEngineSound(int ecx, int edx, int arg1);
// 0x004399c0
int __fastcall Vehicle_HealthTick(int ecx, int edx);
// 0x00439b20
int __fastcall Vehicle_SetLowHealthAlarm(int ecx, int edx);
// 0x0043b730
int __fastcall Vehicle_StartDamageOverTime(int ecx, int edx, int arg1);
// 0x0043b810
int __fastcall NetVehicle_TakeDamage(int ecx, int edx, int arg1, int arg2);
// 0x0043bc40
int __fastcall Vehicle_PlayerDied(int ecx, int edx);
// 0x0043bcc0
int __fastcall Vehicle_TakeDamage(int ecx, int edx, int arg1, int arg2);
// 0x0043c010
int __fastcall Vehicle_CheckDeath(int ecx, int edx);
// 0x0043c850
int __fastcall Vehicle_DeactivatePartsOnDeath(int ecx, int edx);
// 0x0041b950
int __fastcall Vehicle_PostPhysicsTick(int ecx, int edx);
// 0x00426390
int __fastcall Vehicle_MainUpdateTick(int ecx, int edx);
}  // namespace recoil
