// SUBSYSTEM: render_frame
// Declarations for src/unattributed/render_frame.cpp.
#pragma once

namespace recoil {

// 0x00475fa0
int __fastcall ModelCache_Clear(int ecx, int edx);
// 0x0046d4d0
int __fastcall TextureManager_FindSlotByName(int ecx, int edx);
// 0x00479cc0
int __fastcall Texture_IsSoftwareOnly(int ecx, int edx);
// 0x004762a0
int __fastcall RenderState_SetValue5C(int ecx, int edx, int arg1);
// 0x004761e0
int __fastcall RenderState_SetRangeAEnd(int ecx, int edx, int arg1);
// 0x00476220
int __fastcall RenderState_SetRangeBFirst(int ecx, int edx, int arg1);
// 0x00476260
int __fastcall RenderState_SetRangeBSecond(int ecx, int edx, int arg1);
// 0x004762b0
int __fastcall RenderState_SetMode34(int ecx, int edx);
// 0x00476190
int __fastcall RenderState_SetRangeAStart(int ecx, int edx, int arg1);
// 0x00476180
int __fastcall Render_GetGlobal_0057d930(int ecx, int edx);
// 0x00476400
int __fastcall Render_IsNodeTypeEnabledForRender(int ecx, int edx);
// 0x00478fc0
int __fastcall Model_UpdateOncePerFrame(int ecx, int edx);
// 0x00452650
int __fastcall Render_ComputeBoundingSphereFromCorners(int ecx, int edx, int arg1);
// 0x004762c0
int __fastcall RenderState_SetColour(int ecx, int edx);
// 0x00449ba0
int __fastcall Render_SetFrameRateCap(int ecx, int edx, int arg1);
// 0x00476300
int __fastcall Render_SetSmallPolygonRejectArea(int ecx, int edx, int arg1);
// 0x004766a0
int __fastcall Render_ProjectToScreenScaled(int ecx, int edx);
// 0x00478c70
int __fastcall Render_TestBoundSphereAgainstFrustum(int ecx, int edx, int arg1);
// 0x00479c50
int __fastcall Decal_SetImageIndex(int ecx, int edx);
// 0x00479f90
int __fastcall RenderInset_SetSourceRect(int ecx, int edx);
// 0x00490340
int __fastcall zRndr_SetFramebuffer(int ecx, int edx, int arg1, int arg2);
// 0x004903c0
int __fastcall Render_SetViewportSizeFromRect(int ecx, int edx);
// 0x004903e0
int __fastcall Render_SetStrideScale(int ecx, int edx);
// 0x00490430
int __fastcall zRndr_SetPerspectiveSubdivFixed(int ecx, int edx);
// 0x00490480
int __fastcall zRndr_SetPerspectiveSubdivAdaptive(int ecx, int edx, int arg1);
// 0x004904a0
int __fastcall zRndr_SetReciprocalParam_0057de68(int ecx, int edx, int arg1);
// 0x004904d0
int __fastcall zRndr_SetPerspectiveInverseZTolerance(int ecx, int edx, int arg1);
// 0x00490600
int __fastcall zRndr_ClearClipPolys(int ecx, int edx);
// 0x0049a830
int __fastcall Queue_AppendPoint(int ecx, int edx, int arg1);
// 0x0049a8b0
int __fastcall Backend_GetItemCount(int ecx, int edx);
// 0x0049a910
int __fastcall PointQueue_Clear(int ecx, int edx);
// 0x0048ff60
int __fastcall zRndrShutdown(int ecx, int edx);
// 0x0046e290
int __fastcall Tex_PickLevel(int ecx, int edx);
// 0x00476340
int __fastcall ObjectLights_SetPending(int ecx, int edx);
// 0x00490590
int __fastcall zRndr_BeginFrame(int ecx, int edx);
// 0x00490710
int __fastcall zRndr_AddClipPoly(int ecx, int edx);
// 0x0049a9c0
int __fastcall Backend_CollectActiveItems(int ecx, int edx);
// 0x0049aa30
int __fastcall Backend_ItemScreenRay(int ecx, int edx);
// 0x004c8070
int __fastcall Palette_ApplyBrightness(int ecx, int edx);
// 0x00490610
int __fastcall Render_AddScreenRectClipPoly(int ecx, int edx, int arg1);
// 0x004a66f0
int __fastcall Render_ApplyResolutionAndNotify(int ecx, int edx);
// 0x004762f0
int __fastcall RenderState_ApplyColour(int ecx, int edx);
// 0x00479660
int __fastcall Decal_StampOntoTexture(int ecx, int edx);
// 0x0048d7a0
int __fastcall Backend_ScreenTintOverlay(int ecx, int edx);
// 0x0048fd80
int __fastcall zRndrInit(int ecx, int edx);
// 0x00498cb0
int __fastcall SW_PlotPoint16(int ecx, int edx, int arg1);
// 0x0049a920
int __fastcall Backend_CompactAndCollectItems(int ecx, int edx);
// 0x0049a8c0
int __fastcall PointQueue_FlushAll(int ecx, int edx, int arg1);
// 0x0044d320
int __fastcall Camera_UpdateFollowNodes(int ecx, int edx);
// 0x00476320
int __fastcall Render_InitRecordBytes00FFFFFF(int ecx, int edx);
// 0x00476480
int __fastcall Render_ProjectWorldPointToScreen(int ecx, int edx);
// 0x00476700
int __fastcall Occlusion_SphereVisibleInSpanBuffer(int ecx, int edx, int arg1);
// 0x00489540
int __fastcall Shade_DistanceFadeSphere(int ecx, int edx, int arg1);
// 0x0048ff70
int __fastcall zRndr_ResetState(int ecx, int edx);
// 0x00475e70
int __fastcall Render_InitSceneNodePool(int ecx, int edx);
// 0x0044d3a0
int __fastcall Render_CameraFrame_SW(int ecx, int edx);
// 0x00475c40
int __fastcall Render_InitRenderState(int ecx, int edx);
// 0x0049a2b0
int __fastcall Backend_FlushSortedQueue(int ecx, int edx);
// 0x0049a490
int __fastcall Backend_FlushPolyQueue(int ecx, int edx);
// 0x0049aa90
int __fastcall Render_DrawScreenSprite(int ecx, int edx, int arg1, int arg2);
// 0x0049afb0
int __fastcall Backend_ItemDistanceFade(int ecx, int edx);
// 0x0049b020
int __fastcall LensFlare_Draw(int ecx, int edx, int arg1);
// 0x0049b1a0
int __fastcall Backend_FadeKeptItems(int ecx, int edx);
// 0x0044d600
int __fastcall Render_CameraFrame_HW(int ecx, int edx);
// 0x0044f630
int __fastcall Render_DispatchCameraList(int ecx, int edx);
}  // namespace recoil
