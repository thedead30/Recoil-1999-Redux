// SUBSYSTEM: ui_widgets
// Declarations for src/unattributed/ui_widgets.cpp. Spec: 04_spec/systems/ui_widgets.md
#pragma once

namespace recoil {

// 0x00404e80 release-build error reporter: callers pass (level, __FILE__, __LINE__, message, ...) with the
// cdecl convention (caller pops); the body is a single RET, so nothing is reported and EAX is unchanged.
void Debug_ReportNoop();

// 0x0046f130
int __fastcall Font_ScanGlyphs(int ecx, int edx);
// 0x00404ca0
int __fastcall Widget_Slot_CallVirtual08(int ecx, int edx);
// 0x00404d10
int __fastcall Widget_ReturnTrue(int ecx, int edx, int arg1, int arg2);
// 0x00404d50
int __fastcall Widget_GetX(int ecx, int edx);
// 0x00404d60
int __fastcall Widget_GetY(int ecx, int edx);
// 0x00404d90
int __fastcall ImageWidget_GetX(int ecx, int edx);
// 0x00404dd0
int __fastcall ImageWidget_GetY(int ecx, int edx);
// 0x00407140
int __fastcall Widget_ReturnZero(int ecx, int edx);
// 0x00407150
int __fastcall Widget_EmptyVirtual(int ecx, int edx, int arg1);
// 0x00409550
int __fastcall UiScreen_Slot_ClearOwnerADS8(int ecx, int edx);
// 0x0040bdf0
int __fastcall Stub_NoOp_TwoStackArgs(int ecx, int edx, int arg1, int arg2);
// 0x0040bea0
int __fastcall TextWidget_GetFont(int ecx, int edx);
// 0x0040beb0
int __fastcall TextWidget_SetFontHandle(int ecx, int edx, int arg1);
// 0x0040bec0
int __fastcall ListLabel_SetFixedRect(int ecx, int edx, int arg1);
// 0x0041a290
int __fastcall Widget_Slot_TailVirtual8C(int ecx, int edx);
// 0x004353e0
int __fastcall UiScreen_Slot_TailOwnerSlot0C(int ecx, int edx);
// 0x004b3da0
int __fastcall ImageWidget_ReleaseOwnedImage(int ecx, int edx);
// 0x004b40c0
int __fastcall Widget_CopyCtor(int ecx, int edx, int arg1);
// 0x004b4120
int __fastcall Widget_Assign(int ecx, int edx, int arg1);
// 0x004b4180
int __fastcall Widget_OrFlagsWithGlobal(int ecx, int edx);
// 0x004b41b0
int __fastcall Widget_SetBoundsRect(int ecx, int edx, int arg1);
// 0x004b4280
int __fastcall Widget_SetLifetime(int ecx, int edx, int arg1);
// 0x004b4410
int __fastcall TextBuffer_GetText(int ecx, int edx);
// 0x004b4420
int __fastcall EditField_SetCursor(int ecx, int edx, int arg1);
// 0x004b4560
int __fastcall ListCursor_DecNoNotify_004b4560(int ecx, int edx);
// 0x004b4570
int __fastcall ListCursor_Next_004b4570(int ecx, int edx);
// 0x004b4590
int __fastcall EditField_OpenGap(int ecx, int edx, int arg1, int arg2);
// 0x004b45e0
int __fastcall EditField_CloseGap(int ecx, int edx, int arg1, int arg2);
// 0x004b47a0
int __fastcall Widget_BaseDtor(int ecx, int edx);
// 0x004b70b0
int __fastcall Widget_GetSubobjectCC(int ecx, int edx);
// 0x004b7f20
int __fastcall Cycler_SetIndex(int ecx, int edx, int arg1);
// 0x004b7f80
int __fastcall Cycler_SetRange(int ecx, int edx, int arg1, int arg2);
// 0x004ba350
int __fastcall ScreenBase_CloseConfig(int ecx, int edx, int arg1);
// 0x004ba400
int __fastcall ListLabel_GetFixedRect(int ecx, int edx);
// 0x004ba4d0
int __fastcall PtrVector_EraseRange(int ecx, int edx, int arg1, int arg2);
// 0x004bc4e0
int __fastcall Widget_HitTestCircle(int ecx, int edx, int arg1, int arg2);
// 0x004bc550
int __fastcall Widget_SetField10(int ecx, int edx, int arg1);
// 0x004bc560
int __fastcall Widget_GetField10(int ecx, int edx);
// 0x004bc760
int __fastcall Widget_SetGlobalDirtyMode(int ecx, int edx);
// 0x004bc780
int __fastcall WidgetContainer_Ctor(int ecx, int edx);
// 0x004bc7b0
int __fastcall WidgetContainer_Dtor(int ecx, int edx);
// 0x004bc7c0
int __fastcall Widget_AppendChild(int ecx, int edx, int arg1);
// 0x004bc810
int __fastcall WidgetList_FindPrev(int ecx, int edx, int arg1, int arg2);
// 0x004bc8d0
int __fastcall WidgetContainer_SetChildFlags(int ecx, int edx, int arg1);
// 0x004bc930
int __fastcall ListLabel_StartBlink(int ecx, int edx, int arg1);
// 0x004bd3d0
int __fastcall MessagePanel_SetColours(int ecx, int edx, int arg1, int arg2);
// 0x004bd470
int __fastcall DrawQueue_Remove(int ecx, int edx);
// 0x004bd800
int __fastcall Vec3_LerpToZ(int ecx, int edx, int arg1);
// 0x004bd9c0
int __fastcall Seg_ClipToX(int ecx, int edx, int arg1);
// 0x004bdb30
int __fastcall Seg_ClipToY(int ecx, int edx, int arg1);
// 0x004bdee0
int __fastcall ScreenParticle_Respawn(int ecx, int edx, int arg1, int arg2);
// 0x004be210
int __fastcall Points_AllInsideIntRect(int ecx, int edx, int arg1, int arg2);
// 0x004bee00
int __fastcall CompositePanel_SetSlot(int ecx, int edx, int arg1, int arg2);
// 0x004bee20
int __fastcall Object_SetRect18(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004bf7c0
int __fastcall Panel_SetMode1_004bf7c0(int ecx, int edx);
// 0x004bf7e0
int __fastcall Panel_SetMode2_004bf7e0(int ecx, int edx);
// 0x004bfe20
int __fastcall AviWidget_SetColourKey(int ecx, int edx, int arg1);
// 0x00404cd0
int __fastcall Widget_SetPosition(int ecx, int edx, int arg1, int arg2);
// 0x00404cf0
int __fastcall Widget_SetX(int ecx, int edx, int arg1);
// 0x00404d00
int __fastcall Widget_SetY(int ecx, int edx, int arg1);
// 0x00404d20
int __fastcall Widget_SetVisible(int ecx, int edx, int arg1);
// 0x00404e10
int __fastcall ImageWidget_Slot_UpdateBoundsFromImage(int ecx, int edx);
// 0x00409010
int __fastcall ScrollGroup_ActivateChild(int ecx, int edx, int arg1);
// 0x0040db90
int __fastcall UiScreen_Slot_NotifyVisibleChildren(int ecx, int edx);
// 0x0040f400
int __fastcall UiScreen_Slot_Virtual08_NotifyThreeChildren(int ecx, int edx);
// 0x0040fa10
int __fastcall Widget_Slot_ForwardToMember34Slot0(int ecx, int edx, int arg1);
// 0x004b3dd0
int __fastcall ImageWidget_SetPosition(int ecx, int edx, int arg1, int arg2);
// 0x004b3e30
int __fastcall ImageWidget_SetImage(int ecx, int edx, int arg1);
// 0x004b3e70
int __fastcall ImageWidget_SetImageNoOwn(int ecx, int edx, int arg1);
// 0x004b3e90
int __fastcall Widget_AddDirtyRect(int ecx, int edx, int arg1);
// 0x004b4030
int __fastcall Widget_HitTestRect(int ecx, int edx, int arg1, int arg2);
// 0x004b4190
int __fastcall Widget_SetField1CAndNotify(int ecx, int edx, int arg1, int arg2);
// 0x004b41e0
int __fastcall Widget_Tick(int ecx, int edx, int arg1);
// 0x004b42c0
int __fastcall Widget_GetExtentRect(int ecx, int edx, int arg1);
// 0x004b44e0
int __fastcall EditField_InsertChar(int ecx, int edx, int arg1);
// 0x004b4ba0
int __fastcall EditField_SetEnabled(int ecx, int edx, int arg1);
// 0x004b52f0
int __fastcall DeleteWidgetPtr(int ecx, int edx, int arg1);
// 0x004b5310
int __fastcall Widget_MarkDirtyAndRedrawChildren(int ecx, int edx);
// 0x004b8100
int __fastcall Cycler_SetItemFont(int ecx, int edx, int arg1, int arg2);
// 0x004b9330
int __fastcall ListScreen_ScrollTo(int ecx, int edx, int arg1);
// 0x004b9520
int __fastcall ListItem_Slot_ForwardValueToTarget(int ecx, int edx);
// 0x004ba070
int __fastcall ScreenBase_BindButtonImpl(int ecx, int edx, int arg1, int arg2);
// 0x004ba3a0
int __fastcall WidgetContainer_RedrawChildren(int ecx, int edx);
// 0x004ba3c0
int __fastcall Widget_SetField14CAndRedraw(int ecx, int edx, int arg1);
// 0x004ba3e0
int __fastcall Widget_ForwardSlot88(int ecx, int edx);
// 0x004bb440
int __fastcall ListLabel_GetText(int ecx, int edx);
// 0x004bb710
int __fastcall ListLabel_GetLineHeight(int ecx, int edx);
// 0x004bb980
int __fastcall ListWidget_BroadcastIfVisible(int ecx, int edx, int arg1);
// 0x004bbaa0
int __fastcall ListWidget_PrintfCurrent(int ecx, int edx);
// 0x004bbac0
int __fastcall ListWidget_ForwardToCurrentItem(int ecx, int edx);
// 0x004bbed0
int __fastcall ListWidget_ClearRange(int ecx, int edx, int arg1, int arg2);
// 0x004bc900
int __fastcall Manager_BroadcastVirtual24(int ecx, int edx, int arg1);
// 0x004bcd40
int __fastcall Widget_SetField1CAndRect(int ecx, int edx, int arg1, int arg2);
// 0x004bd110
int __fastcall WidgetArray_SetRectAll(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004bd2a0
int __fastcall MessagePanel_Clear(int ecx, int edx);
// 0x004bd410
int __fastcall MessagePanel_SetCentreX(int ecx, int edx, int arg1);
// 0x004bd440
int __fastcall MessagePanel_SetTopY(int ecx, int edx, int arg1);
// 0x004bdb60
int __fastcall Widget_Slot_DrawWithStyle(int ecx, int edx);
// 0x004bdc00
int __fastcall Widget_SetRectAndParams(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7);
// 0x004bed50
int __fastcall ScreenFX_SetPrimaryRect(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004bf8b0
int __fastcall LineWidget_SetPoint(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004bff00
int __fastcall ImageWidget_Slot_ComputeBounds(int ecx, int edx);
// 0x004bffb0
int __fastcall ImageWidget_SetPositionAndAnchor(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x00404e60
int __fastcall Widget_Slot_HitTestVia004bc4e0(int ecx, int edx, int arg1, int arg2);
// 0x00409410
int __fastcall UiScreen_Slot_BroadcastToGroups(int ecx, int edx, int arg1);
// 0x0040be90
int __fastcall ListLabel_MarkDirty(int ecx, int edx);
// 0x004b4070
int __fastcall Widget_BaseCtor(int ecx, int edx, int arg1, int arg2);
// 0x004b43d0
int __fastcall TextBuffer_Assign(int ecx, int edx, int arg1);
// 0x004b4530
int __fastcall ListCursor_Prev_004b4530(int ecx, int edx);
// 0x004b4550
int __fastcall ListCursor_Refresh_004b4550(int ecx, int edx);
// 0x004b4810
int __fastcall EditField_SetCaretShape(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004b4ed0
int __fastcall EditField_GetText(int ecx, int edx);
// 0x004b5350
int __fastcall Button_GetHitRect(int ecx, int edx);
// 0x004b5740
int __fastcall ToggleButton_Slot_RefreshVisuals(int ecx, int edx);
// 0x004b5860
int __fastcall Button_Reset(int ecx, int edx);
// 0x004b70c0
int __fastcall ToggleImageButton_Slot_RefreshVisuals(int ecx, int edx);
// 0x004b72c0
int __fastcall Toggle_SetState(int ecx, int edx, int arg1);
// 0x004b7e60
int __fastcall RadioGroup_Slot_UpdateButtons(int ecx, int edx, int arg1);
// 0x004b8a90
int __fastcall ScrollGroupChild_SetSelected(int ecx, int edx, int arg1);
// 0x004b9320
int __fastcall Thunk_004b9330(int ecx, int edx, int arg1);
// 0x004ba0c0
int __fastcall ScreenBase_BindButton(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004ba0e0
int __fastcall ScreenBase_BindWidget(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004bb3d0
int __fastcall ListWidget_HitTest(int ecx, int edx, int arg1, int arg2);
// 0x004bb740
int __fastcall ListWidget_GetRect(int ecx, int edx, int arg1);
// 0x004bb9f0
int __fastcall ListWidget_SetPosition(int ecx, int edx, int arg1, int arg2);
// 0x004bbb20
int __fastcall ListWidget_Slot_AppendRow(int ecx, int edx);
// 0x004bbe90
int __fastcall ListWidget_ClearAll(int ecx, int edx);
// 0x004bc510
int __fastcall ScreenRoot_Ctor(int ecx, int edx, int arg1);
// 0x004bc540
int __fastcall ScreenRoot_Dtor(int ecx, int edx);
// 0x004bc860
int __fastcall WidgetList_Remove(int ecx, int edx, int arg1);
// 0x004bc980
int __fastcall ListLabel_StartBlinkMode1(int ecx, int edx, int arg1);
// 0x004bc9b0
int __fastcall ListLabel_StartColourAnim(int ecx, int edx, int arg1, int arg2);
// 0x004bcc80
int __fastcall TextWidget_Assign(int ecx, int edx, int arg1);
// 0x004bd160
int __fastcall MessagePanel_PushLine(int ecx, int edx, int arg1, int arg2);
// 0x004bd880
int __fastcall Seg_ClipAgainstXBounds(int ecx, int edx, int arg1, int arg2);
// 0x004bd9f0
int __fastcall Seg_ClipAgainstYBounds(int ecx, int edx, int arg1, int arg2);
// 0x004bed30
int __fastcall Manager_0056bd58_ResetTimer(int ecx, int edx, int arg1);
// 0x004bed90
int __fastcall ScreenFX_AllocateRectSlot(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7);
// 0x004beee0
int __fastcall ScreenFX_QueueFade(int ecx, int edx, int arg1, int arg2);
// 0x004bef40
int __fastcall GlobalCompositePanel_SetSlot(int ecx, int edx);
// 0x004bef50
int __fastcall SetRect_Object0056bd58(int ecx, int edx, int arg1, int arg2);
// 0x004b4e60
int __fastcall EditField_SetText(int ecx, int edx, int arg1);
// 0x0041a2d0
int __fastcall IntEditField_ParseClamped(int ecx, int edx);
// 0x004b3d00
int __fastcall ImageWidget_Ctor(int ecx, int edx, int arg1);
// 0x004b87e0
int __fastcall Toggle_OnReleaseIfOff(int ecx, int edx);
// 0x004b8af0
int __fastcall Cycler_GetLabel(int ecx, int edx);
// 0x004b8cf0
int __fastcall ScrollGroup_Select(int ecx, int edx, int arg1);
// 0x004bc480
int __fastcall RingWidget_Ctor(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004bd280
int __fastcall MessageOverlay_Show(int ecx, int edx, int arg1);
// 0x004bef10
int __fastcall ScreenFX_QueueFlashRect(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x004bef70
int __fastcall Manager_0056bd58_Tick(int ecx, int edx, int arg1);
// 0x0040c9c0
int __fastcall OptionToggle_Refresh_Bit10(int ecx, int edx);
// 0x0040ca20
int __fastcall OptionToggle_Refresh_Bit08(int ecx, int edx);
// 0x0040ca80
int __fastcall OptionToggle_Refresh_00408360Is2(int ecx, int edx);
// 0x0040cab0
int __fastcall OptionCycler_Refresh_From00408030(int ecx, int edx);
// 0x0040caf0
int __fastcall OptionCycler_Refresh_From00408100(int ecx, int edx);
// 0x0040cb30
int __fastcall OptionCycler_Refresh_HWCardGated(int ecx, int edx);
// 0x0040cb90
int __fastcall OptionToggle_Refresh_Not00408060(int ecx, int edx);
// 0x0040cbd0
int __fastcall OptionCycler_Refresh_From004080d0(int ecx, int edx);
// 0x0040cc10
int __fastcall OptionControl_Refresh_FloatFrom00408090(int ecx, int edx);
// 0x0040cc60
int __fastcall OptionToggle_Refresh_From00408220(int ecx, int edx);
// 0x004bcd80
int __fastcall TextWidget_UpdateBoundsFromText(int ecx, int edx);
// 0x004bcdc0
int __fastcall TextWidget_MeasureWidth(int ecx, int edx);
// 0x004bcea0
int __fastcall TextWidget_HitTest(int ecx, int edx, int arg1, int arg2);
// 0x004bcff0
int __fastcall Widget_Slot_Virtual08_Then004936d0IfField4C(int ecx, int edx);
// 0x004bf900
int __fastcall LineWidget_Slot_Draw(int ecx, int edx);
// 0x00435a10
int __fastcall ListScreen_Slot_ReselectIfActive(int ecx, int edx);
// 0x00403c80
int __fastcall Thunk_Widget_Slot_Virtual08_Then00498fb0(int ecx, int edx);
// 0x004b47b0
int __fastcall LineWidget_Slot_TickBlink(int ecx, int edx, int arg1);
// 0x004bcdf0
int __fastcall TextWidget_CentreInBox(int ecx, int edx);
// 0x004bb540
int __fastcall ListLabel_Printf(int ecx, int edx);
// 0x004bb5e0
int __fastcall ListLabel_VPrintf(int ecx, int edx);
// 0x004bb680
int __fastcall ListLabel_SetText(int ecx, int edx, int arg1);
// 0x004bccf0
int __fastcall TextWidget_Printf(int ecx, int edx);
// 0x004a5b40
int __fastcall Call004a5bf0_If004a5b20(int ecx, int edx);
// 0x004bfba0
int __fastcall AnimImageWidget_CaptureFrame(int ecx, int edx, int arg1, int arg2);
// 0x004bdbc0
int __fastcall Widget_Slot_Call0048d6d0(int ecx, int edx);
// 0x00403cb0
int __fastcall RingWidget_Slot_UpdateShrink(int ecx, int edx, int arg1);
// 0x00404d70
int __fastcall Widget_BaseScalarDeletingDtor(int ecx, int edx, int arg1);
// 0x00407170
int __fastcall ScalarDeletingDtor_00407170(int ecx, int edx, int arg1);
// 0x0040cd30
int __fastcall OptionCycler_Refresh_DisplayMode(int ecx, int edx);
// 0x0040ed20
int __fastcall ScrollingWidget_Slot_Tick(int ecx, int edx, int arg1);
// 0x0041ebb0
int __fastcall ScalarDeletingDtor_004b47a0(int ecx, int edx, int arg1);
// 0x004b4370
int __fastcall KeyDispatch_Dtor(int ecx, int edx);
// 0x004b4390
int __fastcall EditField_ResizeBuffer(int ecx, int edx, int arg1);
// 0x004b4460
int __fastcall KeyDispatch_OnChar(int ecx, int edx, int arg1);
// 0x004b86b0
int __fastcall Slider_SetValue(int ecx, int edx, int arg1);
// 0x004ba470
int __fastcall PtrVector_Free(int ecx, int edx);
// 0x004ba510
int __fastcall PtrVector_InsertN(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004bb0c0
int __fastcall Color16_Lerp(int ecx, int edx, int arg1);
// 0x004bbfa0
int __fastcall WidgetArray_Destroy(int ecx, int edx);
// 0x004bcf80
int __fastcall Quad_SetVertex(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004bdc40
int __fastcall Widget_Slot_Call0048daf0(int ecx, int edx);
// 0x004bfae0
int __fastcall AnimImageWidget_AllocFrameImage(int ecx, int edx);
// 0x004bfb70
int __fastcall ImageWidget_SetPositionAndSync(int ecx, int edx, int arg1, int arg2);
// 0x004c0280
int __fastcall NamedTable_AddIfAbsent(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5);
// 0x00407100
int __fastcall UiScreen_Slot_004434b0_0(int ecx, int edx);
// 0x0040ba90
int __fastcall UiButton_Slot_0040b980(int ecx, int edx, int arg1);
// 0x0041a2a0
int __fastcall EditField_OnChar_DigitsOnly(int ecx, int edx, int arg1);
// 0x0041be70
int __fastcall UiScreen_Slot_00443160_4f3e78(int ecx, int edx);
// 0x004348f0
int __fastcall EditField_OnChar_NameChars(int ecx, int edx, int arg1);
// 0x004b42f0
int __fastcall TextBuffer_Ctor(int ecx, int edx, int arg1);
// 0x004b4b50
int __fastcall EditField_OnChar(int ecx, int edx, int arg1);
// 0x004b4e40
int __fastcall EditField_SetMaxLength(int ecx, int edx, int arg1);
// 0x004bfa50
int __fastcall ImageWidget_SetImageAndNotify(int ecx, int edx, int arg1);
// 0x004bfa70
int __fastcall ImageWidget_SetImageAndNotify_B(int ecx, int edx, int arg1);
// 0x004bfa90
int __fastcall AnimImageWidget_SetEnabled(int ecx, int edx, int arg1);
// 0x004bffe0
int __fastcall Timer_RegisterIfEnabled(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004ba9e0
int __fastcall ListLabel_Assign(int ecx, int edx, int arg1);
// 0x004babb0
int __fastcall TextWidget_SetFont(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7);
// 0x004bac10
int __fastcall ListLabel_Slot_RenderTextImage(int ecx, int edx);
// 0x004bb1c0
int __fastcall TextLabel_MeasurePrefix(int ecx, int edx, int arg1, int arg2);
// 0x004bb2a0
int __fastcall ListLabel_Slot_ComputeLayout(int ecx, int edx);
// 0x0040ccc0
int __fastcall OptionSlider_Refresh_From004a27f0(int ecx, int edx);
// 0x004b4ca0
int __fastcall EditField_Slot_TickCaret(int ecx, int edx, int arg1);
// 0x004bbbe0
int __fastcall ListWidget_SetFontAll(int ecx, int edx, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7);
// 0x004bc320
int __fastcall ListItem_CopyRange(int ecx, int edx, int arg1);
// 0x004bc3a0
int __fastcall ListLabelAnimated_Assign(int ecx, int edx, int arg1);
// 0x00403c90
int __fastcall Widget_Slot_DrawImageAt24(int ecx, int edx);
// 0x00404cb0
int __fastcall Widget_Slot_DrawImageAt18(int ecx, int edx);
// 0x004b3fb0
int __fastcall ImageWidget_Slot_Draw(int ecx, int edx);
// 0x004b8520
int __fastcall ScrollbarPair_Slot_Draw(int ecx, int edx);
// 0x004ba380
int __fastcall ScreenBase_DrawBackground(int ecx, int edx);
// 0x004bb460
int __fastcall ListLabel_Slot_Draw(int ecx, int edx);
// 0x004bfc60
int __fastcall Widget_Slot_DrawImageAt18_B(int ecx, int edx);
// 0x004bfe90
int __fastcall Widget_Slot_Virtual08_Then0048f500IfField0D(int ecx, int edx);
// 0x004bfec0
int __fastcall Widget_Slot_DrawImageClampedY(int ecx, int edx);
// 0x004c7f00
int __fastcall BitmapFont_DrawString(int ecx, int edx, int arg1, int arg2);
// 0x004038a0
int __fastcall ImageWidget_Slot_DrawWithFade(int ecx, int edx);
// 0x0040f040
int __fastcall UiScreen_Slot_Virtual20_Virtual74_004bb460(int ecx, int edx);
// 0x00434950
int __fastcall UiScreen_Slot_Call004bb460_ThenVirtual78(int ecx, int edx);
// 0x004ba410
int __fastcall ListWidget_Slot_ComputeBounds(int ecx, int edx);
// 0x004bce30
int __fastcall TextWidget_Slot_Draw(int ecx, int edx);
// 0x004bd4d0
int __fastcall DrawItem_Render(int ecx, int edx);
// 0x004bfc50
int __fastcall Thunk_ImageWidget_Slot_Draw(int ecx, int edx);
// 0x004bd660
int __fastcall UiTimers_Tick(int ecx, int edx);
// Compiler-generated array construct / destruct iterators (SEH frames dropped; hand-ported, see ui_widgets.cpp).
using ElementDtor = void(__thiscall*)(void*);
// 0x004c5ec0, 43 callers: args array base, element size, count, destructor (ret 0x10)
int __stdcall ArrayDtor_Eh2(char* array, unsigned elementSize, int count, ElementDtor dtor);
// 0x004c5f70, 2 callers: same, but the first argument is the END pointer (ret 0x10)
int __stdcall ArrayDtor_Eh(char* end, unsigned elementSize, int count, ElementDtor dtor);
// 0x004c6000, 13 callers: array base, element size, count, constructor, destructor for SEH unwind only (ret 0x14)
int __stdcall ArrayCtor_Eh(char* array, unsigned elementSize, int count, ElementDtor ctor, ElementDtor dtor);

// 0x004bc9f0
int __fastcall ListLabel_Slot_TickAnimated(int ecx, int edx, int arg1);
// 0x004be2f0
int __fastcall ScreenParticleOverlay_Slot_Tick(int ecx, int edx, int arg1);
// 0x004be880
int __fastcall ScreenParticleOverlay_Slot_TickRespawnAll(int ecx, int edx, int arg1);
// 0x004bf630
int __fastcall MessageBoxScreen_RunModal(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004c5f39
int __fastcall ArrayDtor_Eh2_Unwind(int ecx, int edx);
// 0x004c6078
int __fastcall ArrayCtor_Eh_Unwind(int ecx, int edx);
// 0x004bdfd0
int __fastcall FadeOverlay_Slot_Draw(int ecx, int edx);
// 0x00407160
int __fastcall Slot_ReturnTrue_Ret8_00407160(int ecx, int edx, int arg1, int arg2);
// 0x0048eb80
int __fastcall Raster_MaskRect16_0048eb80(int ecx, int edx);
// 0x004b4b30
int __fastcall UiWidget_ForwardVirtual84_004b4b30(int ecx, int edx);
// 0x004ba4a0
int __fastcall UiObject_ResetFields_004ba4a0(int ecx, int edx);
// 0x004ba4c0
int __fastcall UiObject_ClearFirstWord_004ba4c0(int ecx, int edx);
// 0x004bdbe0
int __fastcall UiWidget_Subclass_Ctor_004bdbe0(int ecx, int edx);
// 0x004bfc80
int __fastcall UiWidget_Subclass_Ctor_004bfc80(int ecx, int edx);
// 0x004c5fe0
int __fastcall Seh_CxxExceptionFilter_004c5fe0(int ecx, int edx);
// 0x00423450
int __fastcall Widget_Slot_ThunkMember34To0048eb80(int ecx, int edx);
// 0x004b4c50
int __fastcall EditField_SetFocus(int ecx, int edx, int arg1);
// 0x004c5a50
int __fastcall Thunk_MFC42_Ord5265_004c5a50(int ecx, int edx);
// 0x004b7250
int __fastcall Button_OnRelease(int ecx, int edx);
// 0x00403d70
int __fastcall ScalarDeletingDtor_00403d70(int ecx, int edx, int arg1);
// 0x00403eb0
int __fastcall ScalarDeletingDtor_00403eb0(int ecx, int edx, int arg1);
// 0x004070e0
int __fastcall UiButton_Slot_004434b0_0_ThenActivate(int ecx, int edx);
// 0x00408c20
int __fastcall UiScreen_Slot_Call0040bda0_Then004b5900(int ecx, int edx);
// 0x00409160
int __fastcall UiButton_Slot_004434b0_0_ThenActivate_B(int ecx, int edx);
// 0x00409180
int __fastcall UiScreen_Slot_004434b0_1_Set4f3d7c(int ecx, int edx);
// 0x00409360
int __fastcall ScalarDeletingDtor_004091e0(int ecx, int edx, int arg1);
// 0x00409570
int __fastcall TextPanel_LoadFromConfig(int ecx, int edx, int arg1, int arg2);
// 0x0040a590
int __fastcall ScalarDeletingDtor_0040a590(int ecx, int edx, int arg1);
// 0x0040b0a0
int __fastcall ScalarDeletingDtor_0040a940(int ecx, int edx, int arg1);
// 0x0040b0c0
int __fastcall ScalarDeletingDtor_0040aa30(int ecx, int edx, int arg1);
// 0x0040b0e0
int __fastcall ScalarDeletingDtor_0040ab20(int ecx, int edx, int arg1);
// 0x0040b100
int __fastcall ScalarDeletingDtor_0040ac10(int ecx, int edx, int arg1);
// 0x0040b120
int __fastcall ScalarDeletingDtor_0040ad00(int ecx, int edx, int arg1);
// 0x0040b140
int __fastcall CommandsDialog_Tick_WaitForKeyOrButton_0040b140(int ecx, int edx, int arg1);
// 0x0040b930
int __fastcall UiScreen_Slot_0042a550_004716b0_0040b680(int ecx, int edx);
// 0x0040b960
int __fastcall OptionCycler_Apply_0040b680(int ecx, int edx);
// 0x0040ba30
int __fastcall OptionRadio_Select1(int ecx, int edx);
// 0x0040ba60
int __fastcall UiScreen_Slot_0040b3e0_004b9330_Field430(int ecx, int edx);
// 0x0040bab0
int __fastcall OptionRadio_Select2(int ecx, int edx);
// 0x0040bae0
int __fastcall UiButton_Slot_0040b460_Field430(int ecx, int edx);
// 0x0040bb00
int __fastcall OptionRadio_Select3(int ecx, int edx);
// 0x0040bb30
int __fastcall UiButton_Slot_0040b4e0_Field430(int ecx, int edx);
// 0x0040bb50
int __fastcall OptionRadio_Select4_IfNotLocked(int ecx, int edx);
// 0x0040bb80
int __fastcall UiButton_Slot_0040b560_IfNotLocked(int ecx, int edx);
// 0x0040bba0
int __fastcall UiButton_Slot_0040b5e0_Plus1(int ecx, int edx);
// 0x0040bbc0
int __fastcall UiButton_Slot_0040b5e0_Minus1(int ecx, int edx);
// 0x0040bbe0
int __fastcall UiButton_Slot_0040b630_Plus1(int ecx, int edx);
// 0x0040bc00
int __fastcall UiButton_Slot_0040b630_Minus1(int ecx, int edx);
// 0x0040c260
int __fastcall ScalarDeletingDtor_0040c280(int ecx, int edx, int arg1);
// 0x0040c6e0
int __fastcall OptionButton_Apply_HUDType(int ecx, int edx);
// 0x0040c9e0
int __fastcall OptionToggle_Apply_GfxFlagBit_A(int ecx, int edx);
// 0x0040ca40
int __fastcall OptionToggle_Apply_GfxFlagBit_B(int ecx, int edx);
// 0x0040caa0
int __fastcall Thunk_Toggle_Flip(int ecx, int edx);
// 0x0040cad0
int __fastcall OptionCycler_Apply_ObjectLOD(int ecx, int edx);
// 0x0040cb10
int __fastcall OptionCycler_Apply_TextureMemory(int ecx, int edx);
// 0x0040cb70
int __fastcall OptionCycler_Apply_EffectsLevel(int ecx, int edx);
// 0x0040cbb0
int __fastcall OptionToggle_Apply_MuteSound(int ecx, int edx);
// 0x0040cbf0
int __fastcall OptionCycler_Apply_SoundLOD(int ecx, int edx);
// 0x0040cc30
int __fastcall OptionSlider_Apply_SoundVolume(int ecx, int edx);
// 0x0040cc80
int __fastcall OptionToggle_Apply_CDAudio(int ecx, int edx);
// 0x0040cd00
int __fastcall OptionSlider_Apply_004a2880(int ecx, int edx);
// 0x0040ce80
int __fastcall OptionCycler_Apply_Detail_0x00415670(int ecx, int edx);
// 0x0040daa0
int __fastcall ScalarDeletingDtor_0040d590(int ecx, int edx, int arg1);
// 0x0040dbd0
int __fastcall ScalarDeletingDtor_0040d780(int ecx, int edx, int arg1);
// 0x0040f2b0
int __fastcall ScalarDeletingDtor_0040d610(int ecx, int edx, int arg1);
// 0x0040fa20
int __fastcall ScalarDeletingDtor_0040fa40(int ecx, int edx, int arg1);
// 0x00414f40
int __fastcall UiScreen_Slot_Call00409b00_Then004b5900(int ecx, int edx);
// 0x00414f60
int __fastcall UiScreen_Slot_Call00435f50_Then004b5900(int ecx, int edx);
// 0x00414f80
int __fastcall UiScreen_Slot_Call0041c6c0_Then004b5900(int ecx, int edx);
// 0x00414fa0
int __fastcall UiButton_Slot_004434b0_0_Activate_Then00413630(int ecx, int edx);
// 0x00414fc0
int __fastcall UiScreen_Slot_Call0040d1c0_Then004b5900(int ecx, int edx);
// 0x00414fe0
int __fastcall UiScreen_Slot_Call004159b0_Then004b5900(int ecx, int edx);
// 0x00415000
int __fastcall UiScreen_Slot_Call00408ff0_Then004b5900(int ecx, int edx);
// 0x00415140
int __fastcall UiScreen_Slot_00435f80_Activate(int ecx, int edx);
// 0x00415740
int __fastcall UiScreen_Slot_Set4edc50_Reset_Back(int ecx, int edx);
// 0x00419800
int __fastcall UiScreen_Slot_00443160_4f3e48_0041ad80(int ecx, int edx);
// 0x00419830
int __fastcall UiScreen_Slot_Activate_00443160_4f3e78(int ecx, int edx);
// 0x0041a160
int __fastcall UiScreen_Slot_004434b0_0_00443160_4f3e78(int ecx, int edx);
// 0x0041a350
int __fastcall IntSpinButton_Slot_Step(int ecx, int edx);
// 0x0041a570
int __fastcall ScalarDeletingDtor_0041a570(int ecx, int edx, int arg1);
// 0x0041a590
int __fastcall ScalarDeletingDtor_0041a590(int ecx, int edx, int arg1);
// 0x0041a7b0
int __fastcall EditField_Slot_Focus(int ecx, int edx);
// 0x0041a820
int __fastcall NetGameSetup_Slot_ModeNext(int ecx, int edx);
// 0x0041a9c0
int __fastcall NetGameSetup_Slot_ModePrev(int ecx, int edx);
// 0x0041bf10
int __fastcall UiScreen_Slot_Virtual40_Global4f32c0Slot4_Back(int ecx, int edx);
// 0x0041bf40
int __fastcall UiScreen_Slot_EnterWithMouseRestore(int ecx, int edx);
// 0x0041bfa0
int __fastcall UiScreen_Slot_LeaveWithMouseReset(int ecx, int edx);
// 0x0041c270
int __fastcall UiScreen_Slot_0041c500IfOwner_Activate(int ecx, int edx);
// 0x0041c3b0
int __fastcall UiScreen_Slot_Init0x15_004b4e60(int ecx, int edx);
// 0x0041c480
int __fastcall ScalarDeletingDtor_0041c480(int ecx, int edx, int arg1);
// 0x0041c4a0
int __fastcall ScalarDeletingDtor_0041c4a0(int ecx, int edx, int arg1);
// 0x0041c4c0
int __fastcall ScalarDeletingDtor_0041c4c0(int ecx, int edx, int arg1);
// 0x004348b0
int __fastcall EditField_Slot_LoadTextAndSelect(int ecx, int edx);
// 0x00435160
int __fastcall ListScreen_Slot_SelectNext(int ecx, int edx);
// 0x004351b0
int __fastcall ListScreen_Slot_SelectPrev(int ecx, int edx);
// 0x00435220
int __fastcall UiScreen_Slot_Call00435a70IfOwner_ThenActivate(int ecx, int edx);
// 0x004b3ce0
int __fastcall ScalarDeletingDtor_004b3ce0(int ecx, int edx, int arg1);
// 0x004b3d50
int __fastcall ImageWidget_Dtor(int ecx, int edx);
// 0x004b4620
int __fastcall Caret_Ctor(int ecx, int edx);
// 0x004b49e0
int __fastcall EditField_Ctor(int ecx, int edx);
// 0x004b4a90
int __fastcall ScalarDeletingDtor_004b4a90(int ecx, int edx, int arg1);
// 0x004b4ac0
int __fastcall EditField_Dtor(int ecx, int edx);
// 0x004b4c90
int __fastcall Widget_SetField36CAndClose(int ecx, int edx);
// 0x004b4ee0
int __fastcall Button_Ctor(int ecx, int edx);
// 0x004b50a0
int __fastcall ScalarDeletingDtor_004b50a0(int ecx, int edx, int arg1);
// 0x004b50c0
int __fastcall Button_Dtor(int ecx, int edx);
// 0x004b5630
int __fastcall Button_Highlight(int ecx, int edx);
// 0x004b5900
int __fastcall Button_Activate(int ecx, int edx);
// 0x004b59f0
int __fastcall Button_LoadFromConfig(int ecx, int edx, int arg1, int arg2);
// 0x004b6fc0
int __fastcall Toggle_Ctor(int ecx, int edx);
// 0x004b7000
int __fastcall ScalarDeletingDtor_004b7000(int ecx, int edx, int arg1);
// 0x004b7020
int __fastcall Toggle_Dtor(int ecx, int edx);
// 0x004b7210
int __fastcall Button_OnHover(int ecx, int edx);
// 0x004b7340
int __fastcall Button_LoadStatesFromConfig(int ecx, int edx, int arg1, int arg2);
// 0x004b7d60
int __fastcall RadioGroup_Ctor(int ecx, int edx);
// 0x004b7dc0
int __fastcall ScalarDeletingDtor_004b7dc0(int ecx, int edx, int arg1);
// 0x004b7de0
int __fastcall RadioGroup_Dtor(int ecx, int edx);
// 0x004b7ee0
int __fastcall Cycler_Next(int ecx, int edx);
// 0x004b7fd0
int __fastcall Cycler_SetItemImage(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004b8200
int __fastcall ImageList_SetItemImage(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004b82e0
int __fastcall Cycler_LoadFromConfig(int ecx, int edx, int arg1, int arg2);
// 0x004b8450
int __fastcall ToggleImage_Ctor(int ecx, int edx);
// 0x004b84b0
int __fastcall ScalarDeletingDtor_004b84b0(int ecx, int edx, int arg1);
// 0x004b84d0
int __fastcall ToggleImage_Dtor(int ecx, int edx);
// 0x004b85c0
int __fastcall Slider_LoadFromConfig(int ecx, int edx, int arg1, int arg2);
// 0x004b8650
int __fastcall Slider_SetFromMouse(int ecx, int edx);
// 0x004b8760
int __fastcall Slider_Ctor(int ecx, int edx);
// 0x004b87a0
int __fastcall ScalarDeletingDtor_004b87c0(int ecx, int edx, int arg1);
// 0x004b87c0
int __fastcall Slider_Dtor(int ecx, int edx);
// 0x004b87d0
int __fastcall Toggle_OnHoverIfOff(int ecx, int edx);
// 0x004b87f0
int __fastcall Scrollbar_Slot_ApplyToTarget(int ecx, int edx);
// 0x004b8850
int __fastcall Toggle_LoadFromConfig(int ecx, int edx, int arg1, int arg2);
// 0x004b8b10
int __fastcall ScrollGroup_Ctor(int ecx, int edx);
// 0x004b8b40
int __fastcall ScalarDeletingDtor_004b8b40(int ecx, int edx, int arg1);
// 0x004b8b60
int __fastcall ScrollGroup_Dtor(int ecx, int edx);
// 0x004b8be0
int __fastcall ScrollGroup_LoadFromConfig(int ecx, int edx, int arg1, int arg2);
// 0x004b8d30
int __fastcall OptionListScreen_Ctor(int ecx, int edx);
// 0x004b8de0
int __fastcall ListScreen_LoadFromConfig(int ecx, int edx, int arg1, int arg2);
// 0x004b90e0
int __fastcall ListScreen_CreateRows(int ecx, int edx, int arg1, int arg2);
// 0x004b92a0
int __fastcall ListLabel_Subclass_Ctor_004b92a0(int ecx, int edx);
// 0x004b92c0
int __fastcall ListItem_VectorDeletingDtor(int ecx, int edx, int arg1);
// 0x004b9710
int __fastcall ScalarDeletingDtor_004b9710(int ecx, int edx, int arg1);
// 0x004b9850
int __fastcall ScreenBase_SetActive(int ecx, int edx, int arg1);
// 0x004ba020
int __fastcall ListLabel_Subclass_Ctor_004ba020(int ecx, int edx);
// 0x004ba740
int __fastcall ListLabel_Ctor(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004ba830
int __fastcall ScalarDeletingDtor_004ba830(int ecx, int edx, int arg1);
// 0x004ba850
int __fastcall ListLabel_CopyCtor(int ecx, int edx, int arg1);
// 0x004bab40
int __fastcall ListLabel_Dtor(int ecx, int edx);
// 0x004bb790
int __fastcall ListWidget_Ctor(int ecx, int edx, int arg1);
// 0x004bb960
int __fastcall ScalarDeletingDtor_00403e20(int ecx, int edx, int arg1);
// 0x004bbca0
int __fastcall ListWidget_SetItemCount(int ecx, int edx, int arg1);
// 0x004bbff0
int __fastcall ListItemVector_InsertN(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004bc410
int __fastcall ListLabelAnimated_CopyCtor(int ecx, int edx, int arg1);
// 0x004bcb50
int __fastcall TextWidget_Ctor(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004bcbe0
int __fastcall TextWidget_CopyCtor(int ecx, int edx, int arg1);
// 0x004bcf20
int __fastcall TextField_Ctor(int ecx, int edx);
// 0x004bd020
int __fastcall ChatInputPanel_Ctor(int ecx, int edx);
// 0x004bd100
int __fastcall ListLabel_CtorDefault_004bd100(int ecx, int edx);
// 0x004bd2d0
int __fastcall MessageLinesPanel_Ctor(int ecx, int edx);
// 0x004bdc70
int __fastcall SnowFX_Construct(int ecx, int edx, int arg1);
// 0x004bde20
int __fastcall ScalarDeletingDtor_004bde20(int ecx, int edx, int arg1);
// 0x004bde40
int __fastcall Overlay_Dtor_Base(int ecx, int edx);
// 0x004be280
int __fastcall ScreenParticleOverlay_Ctor(int ecx, int edx, int arg1);
// 0x004be2c0
int __fastcall ScalarDeletingDtor_004be2c0(int ecx, int edx, int arg1);
// 0x004be810
int __fastcall ScreenParticleOverlay_CtorRespawnAll(int ecx, int edx, int arg1);
// 0x004be850
int __fastcall ScalarDeletingDtor_004be870(int ecx, int edx, int arg1);
// 0x004be870
int __fastcall Overlay_Dtor(int ecx, int edx);
// 0x004bee40
int __fastcall StaticInitWrapper_004bee40(int ecx, int edx);
// 0x004bee50
int __fastcall GlobalCompositePanel_StaticInit(int ecx, int edx);
// 0x004bee60
int __fastcall GlobalCompositePanel_StaticAtexit(int ecx, int edx);
// 0x004bee70
int __fastcall AtexitStub_StaticObj_Dtor_5Widgets_004bee70(int ecx, int edx);
// 0x004bee80
int __fastcall StaticObj_Dtor_5Widgets_004bee80(int ecx, int edx);
// 0x004bef90
int __fastcall CompositePanel_Ctor(int ecx, int edx);
// 0x004bf800
int __fastcall UiScreen_Slot_Owner200Slot0C_ThenActivate(int ecx, int edx);
// 0x004bf820
int __fastcall UiScreen_Slot_Owner200Slot10_ThenActivate(int ecx, int edx);
// 0x004bf840
int __fastcall LineWidget_Ctor(int ecx, int edx);
// 0x004bf980
int __fastcall AnimImageWidget_Ctor(int ecx, int edx, int arg1, int arg2);
// 0x004bfa00
int __fastcall ScalarDeletingDtor_004bfa00(int ecx, int edx, int arg1);
// 0x004bfa20
int __fastcall AnimImageWidget_Dtor(int ecx, int edx);
// 0x004c86e0
int __fastcall EH_Unwind_TextPanel_LoadFromConfig_0(int ecx, int edx);
// 0x004c86eb
int __fastcall EH_Unwind_TextPanel_LoadFromConfig_1(int ecx, int edx);
// 0x004cb200
int __fastcall EH_Unwind_ImageWidget_Dtor_0(int ecx, int edx);
// 0x004cb220
int __fastcall EH_Unwind_Caret_Ctor_0(int ecx, int edx);
// 0x004cb240
int __fastcall EH_Unwind_EditField_Ctor_0(int ecx, int edx);
// 0x004cb248
int __fastcall EH_Unwind_EditField_Ctor_1(int ecx, int edx);
// 0x004cb256
int __fastcall EH_Unwind_EditField_Ctor_2(int ecx, int edx);
// 0x004cb270
int __fastcall EH_Unwind_EditField_Dtor_0(int ecx, int edx);
// 0x004cb278
int __fastcall EH_Unwind_EditField_Dtor_1(int ecx, int edx);
// 0x004cb286
int __fastcall EH_Unwind_EditField_Dtor_2(int ecx, int edx);
// 0x004cb2a0
int __fastcall EH_Unwind_Button_Ctor_0(int ecx, int edx);
// 0x004cb2a8
int __fastcall EH_Unwind_Button_Ctor_1(int ecx, int edx);
// 0x004cb2b6
int __fastcall EH_Unwind_Button_Ctor_2(int ecx, int edx);
// 0x004cb2c4
int __fastcall EH_Unwind_Button_Ctor_3(int ecx, int edx);
// 0x004cb2d2
int __fastcall EH_Unwind_Button_Ctor_4(int ecx, int edx);
// 0x004cb2f0
int __fastcall EH_Unwind_Button_Dtor_0(int ecx, int edx);
// 0x004cb2f8
int __fastcall EH_Unwind_Button_Dtor_1(int ecx, int edx);
// 0x004cb306
int __fastcall EH_Unwind_Button_Dtor_2(int ecx, int edx);
// 0x004cb314
int __fastcall EH_Unwind_Button_Dtor_3(int ecx, int edx);
// 0x004cb322
int __fastcall EH_Unwind_Button_Dtor_4(int ecx, int edx);
// 0x004cb340
int __fastcall EH_Unwind_Button_LoadFromConfig_0(int ecx, int edx);
// 0x004cb34b
int __fastcall EH_Unwind_Button_LoadFromConfig_1(int ecx, int edx);
// 0x004cb356
int __fastcall EH_Unwind_Button_LoadFromConfig_2(int ecx, int edx);
// 0x004cb361
int __fastcall EH_Unwind_Button_LoadFromConfig_3(int ecx, int edx);
// 0x004cb36c
int __fastcall EH_Unwind_Button_LoadFromConfig_4(int ecx, int edx);
// 0x004cb377
int __fastcall EH_Unwind_Button_LoadFromConfig_5(int ecx, int edx);
// 0x004cb382
int __fastcall EH_Unwind_Button_LoadFromConfig_6(int ecx, int edx);
// 0x004cb38d
int __fastcall EH_Unwind_Button_LoadFromConfig_7(int ecx, int edx);
// 0x004cb3b0
int __fastcall EH_Unwind_Toggle_Dtor_0(int ecx, int edx);
// 0x004cb3d0
int __fastcall EH_Unwind_Button_LoadStatesFromConfig_0(int ecx, int edx);
// 0x004cb3db
int __fastcall EH_Unwind_Button_LoadStatesFromConfig_1(int ecx, int edx);
// 0x004cb3e6
int __fastcall EH_Unwind_Button_LoadStatesFromConfig_2(int ecx, int edx);
// 0x004cb3f1
int __fastcall EH_Unwind_Button_LoadStatesFromConfig_3(int ecx, int edx);
// 0x004cb3fc
int __fastcall EH_Unwind_Button_LoadStatesFromConfig_4(int ecx, int edx);
// 0x004cb420
int __fastcall EH_Unwind_RadioGroup_Dtor_0(int ecx, int edx);
// 0x004cb440
int __fastcall EH_Unwind_Cycler_SetItemImage_0(int ecx, int edx);
// 0x004cb460
int __fastcall EH_Unwind_ImageList_SetItemImage_0(int ecx, int edx);
// 0x004cb480
int __fastcall EH_Unwind_ScrollGroup_Dtor_0(int ecx, int edx);
// 0x004cb4a0
int __fastcall EH_Unwind_ScrollGroup_LoadFromConfig_0(int ecx, int edx);
// 0x004cb4c0
int __fastcall EH_Unwind_OptionListScreen_Ctor_0(int ecx, int edx);
// 0x004cb4e0
int __fastcall EH_Unwind_ListScreen_CreateRows_0(int ecx, int edx);
// 0x004cb500
int __fastcall EH_Unwind_ScreenBase_Ctor_0(int ecx, int edx);
// 0x004cb508
int __fastcall EH_Unwind_ScreenBase_Ctor_1(int ecx, int edx);
// 0x004cb513
int __fastcall EH_Unwind_ScreenBase_Ctor_2(int ecx, int edx);
// 0x004cb549
int __fastcall EH_Unwind_ScreenBase_Ctor_4(int ecx, int edx);
// 0x004cb570
int __fastcall EH_Unwind_ScreenBase_Dtor_0(int ecx, int edx);
// 0x004cb578
int __fastcall EH_Unwind_ScreenBase_Dtor_1(int ecx, int edx);
// 0x004cb583
int __fastcall EH_Unwind_ScreenBase_Dtor_2(int ecx, int edx);
// 0x004cb5b9
int __fastcall EH_Unwind_ScreenBase_Dtor_4(int ecx, int edx);
// 0x004cb5e0
int __fastcall EH_Unwind_ListLabel_Ctor_0(int ecx, int edx);
// 0x004cb600
int __fastcall EH_Unwind_ListLabel_CopyCtor_0(int ecx, int edx);
// 0x004cb620
int __fastcall EH_Unwind_ListLabel_Dtor_0(int ecx, int edx);
// 0x004cb640
int __fastcall EH_Unwind_ListWidget_Ctor_0(int ecx, int edx);
// 0x004cb64b
int __fastcall EH_Unwind_ListWidget_Ctor_1(int ecx, int edx);
// 0x004cb65c
int __fastcall EH_Unwind_ListWidget_Ctor_2(int ecx, int edx);
// 0x004cb680
int __fastcall EH_Unwind_ListWidget_SetItemCount_0(int ecx, int edx);
// 0x004cb6a0
int __fastcall EH_Unwind_TextWidget_Ctor_0(int ecx, int edx);
// 0x004cb6c0
int __fastcall EH_Unwind_TextWidget_CopyCtor_0(int ecx, int edx);
// 0x004cb6e0
int __fastcall EH_Unwind_TextField_Ctor_0(int ecx, int edx);
// 0x004cb700
int __fastcall EH_Unwind_ChatInputPanel_Ctor_0(int ecx, int edx);
// 0x004cb708
int __fastcall EH_Unwind_ChatInputPanel_Ctor_1(int ecx, int edx);
// 0x004cb730
int __fastcall EH_Unwind_MessageLinesPanel_Ctor_0(int ecx, int edx);
// 0x004cb738
int __fastcall EH_Unwind_MessageLinesPanel_Ctor_1(int ecx, int edx);
// 0x004cb760
int __fastcall EH_Unwind_SnowFX_Construct_0(int ecx, int edx);
// 0x004cb780
int __fastcall EH_Unwind_Overlay_Dtor_Base_0(int ecx, int edx);
// 0x004cb7a0
int __fastcall EH_Unwind_StaticObj_Dtor_5Widgets_0(int ecx, int edx);
// 0x004cb7a8
int __fastcall EH_Unwind_StaticObj_Dtor_5Widgets_1(int ecx, int edx);
// 0x004cb7c0
int __fastcall EH_Unwind_CompositePanel_Ctor_0(int ecx, int edx);
// 0x004cb7c8
int __fastcall EH_Unwind_CompositePanel_Ctor_1(int ecx, int edx);
// 0x004cb7d3
int __fastcall EH_Unwind_CompositePanel_Ctor_2(int ecx, int edx);
// 0x004cb808
int __fastcall EH_Unwind_MessageBoxScreen_Ctor_1(int ecx, int edx);
// 0x004cb816
int __fastcall EH_Unwind_MessageBoxScreen_Ctor_2(int ecx, int edx);
// 0x004cb824
int __fastcall EH_Unwind_MessageBoxScreen_Ctor_3(int ecx, int edx);
// 0x004cb832
int __fastcall EH_Unwind_MessageBoxScreen_Ctor_4(int ecx, int edx);
// 0x004cb840
int __fastcall EH_Unwind_MessageBoxScreen_Ctor_5(int ecx, int edx);
// 0x004cb868
int __fastcall EH_Unwind_MessageBoxScreen_Dtor_1(int ecx, int edx);
// 0x004cb876
int __fastcall EH_Unwind_MessageBoxScreen_Dtor_2(int ecx, int edx);
// 0x004cb884
int __fastcall EH_Unwind_MessageBoxScreen_Dtor_3(int ecx, int edx);
// 0x004cb892
int __fastcall EH_Unwind_MessageBoxScreen_Dtor_4(int ecx, int edx);
// 0x004cb8a0
int __fastcall EH_Unwind_MessageBoxScreen_Dtor_5(int ecx, int edx);
// 0x004cb8c0
int __fastcall EH_Unwind_LineWidget_Ctor_0(int ecx, int edx);
// 0x004cb8e0
int __fastcall EH_Unwind_AnimImageWidget_Ctor_0(int ecx, int edx);
// 0x004cb900
int __fastcall EH_Unwind_AviWidget_Dtor_0(int ecx, int edx);
// 0x004cb920
int __fastcall EH_Unwind_AviWidget_Open_0(int ecx, int edx);
// 0x004c86f6
int __fastcall EH_Handler_TextPanel_LoadFromConfig(int ecx, int edx);

// 0x004cb208
int __fastcall EH_Handler_ImageWidget_Dtor(int ecx, int edx);

// 0x004cb228
int __fastcall EH_Handler_Caret_Ctor(int ecx, int edx);

// 0x004cb264
int __fastcall EH_Handler_EditField_Ctor(int ecx, int edx);

// 0x004cb294
int __fastcall EH_Handler_EditField_Dtor(int ecx, int edx);

// 0x004cb2e0
int __fastcall EH_Handler_Button_Ctor(int ecx, int edx);

// 0x004cb330
int __fastcall EH_Handler_Button_Dtor(int ecx, int edx);

// 0x004cb398
int __fastcall EH_Handler_Button_LoadFromConfig(int ecx, int edx);

// 0x004cb3b8
int __fastcall EH_Handler_Toggle_Dtor(int ecx, int edx);

// 0x004cb407
int __fastcall EH_Handler_Button_LoadStatesFromConfig(int ecx, int edx);

// 0x004cb428
int __fastcall EH_Handler_RadioGroup_Dtor(int ecx, int edx);

// 0x004cb44b
int __fastcall EH_Handler_Cycler_SetItemImage(int ecx, int edx);

// 0x004cb46b
int __fastcall EH_Handler_ImageList_SetItemImage(int ecx, int edx);

// 0x004cb488
int __fastcall EH_Handler_ScrollGroup_Dtor(int ecx, int edx);

// 0x004cb4ab
int __fastcall EH_Handler_ScrollGroup_LoadFromConfig(int ecx, int edx);

// 0x004cb4c8
int __fastcall EH_Handler_OptionListScreen_Ctor(int ecx, int edx);

// 0x004cb4eb
int __fastcall EH_Handler_ListScreen_CreateRows(int ecx, int edx);

// 0x004cb5e8
int __fastcall EH_Handler_ListLabel_Ctor(int ecx, int edx);

// 0x004cb608
int __fastcall EH_Handler_ListLabel_CopyCtor(int ecx, int edx);

// 0x004cb628
int __fastcall EH_Handler_ListLabel_Dtor(int ecx, int edx);

// 0x004cb667
int __fastcall EH_Handler_ListWidget_Ctor(int ecx, int edx);

// 0x004cb68b
int __fastcall EH_Handler_ListWidget_SetItemCount(int ecx, int edx);

// 0x004cb6a8
int __fastcall EH_Handler_TextWidget_Ctor(int ecx, int edx);

// 0x004cb6c8
int __fastcall EH_Handler_TextWidget_CopyCtor(int ecx, int edx);

// 0x004cb6e8
int __fastcall EH_Handler_TextField_Ctor(int ecx, int edx);

// 0x004cb721
int __fastcall EH_Handler_ChatInputPanel_Ctor(int ecx, int edx);

// 0x004cb751
int __fastcall EH_Handler_MessageLinesPanel_Ctor(int ecx, int edx);

// 0x004cb768
int __fastcall EH_Handler_SnowFX_Construct(int ecx, int edx);

// 0x004cb788
int __fastcall EH_Handler_Overlay_Dtor_Base(int ecx, int edx);

// 0x004cb7b3
int __fastcall EH_Handler_StaticObj_Dtor_5Widgets(int ecx, int edx);

// 0x004cb7e9
int __fastcall EH_Handler_CompositePanel_Ctor(int ecx, int edx);

// 0x004cb8c8
int __fastcall EH_Handler_LineWidget_Ctor(int ecx, int edx);

// 0x004cb8e8
int __fastcall EH_Handler_AnimImageWidget_Ctor(int ecx, int edx);

// 0x004cb908
int __fastcall EH_Handler_AviWidget_Dtor(int ecx, int edx);

// 0x004cb92b
int __fastcall EH_Handler_AviWidget_Open(int ecx, int edx);

// 0x0041ebd0
int __fastcall Thunk_0041ebe0_0041ebd0(int ecx, int edx);
// 0x0041ec00
int __fastcall Thunk_0041ec10_0041ec00(int ecx, int edx);
// 0x0041ebe0
int __fastcall StaticInit_ZeroList_004f3340(int ecx, int edx);
// 0x0041ec10
int __fastcall StaticInit_ZeroList_004f3a78(int ecx, int edx);
// 0x004b7290
int __fastcall Toggle_Flip(int ecx, int edx);
// 0x004bc4c0
int __fastcall Widget_Slot_Virtual08_Then00498fb0(int ecx, int edx);
// 0x004c5f33
int __fastcall DataTarget_004c5f33(int ecx, int edx);
// 0x004c5fb0
int __fastcall DataTarget_004c5fb0(int ecx, int edx);
// 0x004c5fbd
int __fastcall DataTarget_004c5fbd(int ecx, int edx, int arg1, int arg2, int arg3, int arg4);
// 0x004c606f
int __fastcall DataTarget_004c606f(int ecx, int edx);
// 0x00423440
int __fastcall Widget_Slot_ThunkMember34To0048ea20(int ecx, int edx);
// 0x00435140
int __fastcall UiScreen_Slot_Activate_00434fb0_1(int ecx, int edx);
// 0x00435200
int __fastcall UiScreen_Slot_Call00435240IfOwner_ThenActivate(int ecx, int edx);
// 0x004b9540
int __fastcall ScreenBase_Ctor(int ecx, int edx);
// 0x004b9740
int __fastcall ScreenBase_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x004b9760
int __fastcall ScreenBase_Dtor(int ecx, int edx);
// 0x004b98d0
int __fastcall ScreenBase_LoadConfigFile(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004b9900
int __fastcall ScreenBase_LoadFromConfig(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x004bf060
int __fastcall MessageBoxScreen_Ctor(int ecx, int edx, int arg1, int arg2);
// 0x004bf540
int __fastcall MessageBoxScreen_scalar_deleting_dtor(int ecx, int edx, int arg1);
// 0x004bf560
int __fastcall MessageBoxScreen_Dtor(int ecx, int edx);
// 0x004bfcb0
int __fastcall ScalarDeletingDtor_004bfcd0(int ecx, int edx, int arg1);
// 0x004bfcd0
int __fastcall AviWidget_Dtor(int ecx, int edx);
// 0x004bfd40
int __fastcall AviWidget_Open(int ecx, int edx, int arg1);
// 0x004bfe40
int __fastcall AnimWidget_Slot_Tick(int ecx, int edx, int arg1);
// 0x004cb52e
int __fastcall EH_Unwind_ScreenBase_Ctor_3(int ecx, int edx);
// 0x004cb59e
int __fastcall EH_Unwind_ScreenBase_Dtor_3(int ecx, int edx);
// 0x004cb800
int __fastcall EH_Unwind_MessageBoxScreen_Ctor_0(int ecx, int edx);
// 0x004cb860
int __fastcall EH_Unwind_MessageBoxScreen_Dtor_0(int ecx, int edx);
// 0x004cb561
int __fastcall EH_Handler_ScreenBase_Ctor(int ecx, int edx);

// 0x004cb5d1
int __fastcall EH_Handler_ScreenBase_Dtor(int ecx, int edx);

// 0x004cb84e
int __fastcall EH_Handler_MessageBoxScreen_Ctor(int ecx, int edx);

// 0x004cb8ae
int __fastcall EH_Handler_MessageBoxScreen_Dtor(int ecx, int edx);

// 0x0041a5b0
int __fastcall NetGameSetup_Slot_Start(int ecx, int edx);
}  // namespace recoil
