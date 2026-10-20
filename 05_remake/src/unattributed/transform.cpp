// SUBSYSTEM: transform
// Functions of subsystem `transform` with no attributed original source file (ledger orig_file empty).
// Their placement here is a layout choice, not a provenance claim (STAGE2.md section 2).
// Spec: 04_spec/systems/transform_stack.md
//
// Matrix copies move 32-bit words, not floats: an x87 FLD/FSTP round trip would quiet signalling NaNs,
// which REP MOVSD / MOV in the original never do. Copies run forward, word by word, like the original.
#include "unattributed/transform.h"

namespace recoil {

float* g_TransformMatrixStack_00566868[kTransformStackSlots];
std::uint32_t g_TransformFlagStack_00566950[kTransformStackSlots];
float** g_TransformMatrixSP_004e0e88 = g_TransformMatrixStack_00566868;
std::uint32_t* g_TransformFlagSP_004e0e84 = g_TransformFlagStack_00566950;
float g_ViewMatrixA_005668e8[12];
float g_CameraWorldB_00566920[12];

namespace {
constexpr int kMatrixWords = 12;    // CONFIRMED-BINARY: MOV ECX,0xc before REP MOVSD at 0x00472f1f, 0x0047325b
constexpr int kRotationWords = 9;   // CONFIRMED-BINARY: nine MOV stores in 0x00473280..0x004732dd
constexpr std::uint32_t kFlagNotIdentity = 0;  // CONFIRMED-BINARY: MOV [..],0x0 at 0x00472f51
constexpr std::uint32_t kFlagIdentity = 1;     // CONFIRMED-BINARY: MOV [ECX],0x1 at 0x00473364
constexpr std::uint32_t kOneBits = 0x3f800000u;  // CONFIRMED-BINARY: 1.0f immediates at 0x004732f7/0x0047331b/0x0047333f
constexpr std::uint32_t kZeroBits = 0;           // CONFIRMED-BINARY: 0.0f immediates in 0x004732f0

void copy_words(void* dst, const void* src, int n)
{
    auto* d = static_cast<volatile std::uint32_t*>(dst);
    auto* s = static_cast<const volatile std::uint32_t*>(src);
    for (int i = 0; i < n; ++i) d[i] = s[i];
}
}  // namespace

void Transform_ResetState()
{
    g_TransformMatrixSP_004e0e88 = g_TransformMatrixStack_00566868;
    g_TransformFlagSP_004e0e84 = g_TransformFlagStack_00566950;
}

// 0x00472ef0 zTransformStackPushAndCopy - CONFIRMED-BINARY (listing 0x00472ef0..0x00472f2d).
void __fastcall zTransformStackPushAndCopy(float* m)
{
    std::uint32_t* f = ++g_TransformFlagSP_004e0e84;
    float** p = ++g_TransformMatrixSP_004e0e88;
    f[0] = f[-1];
    p[0] = m;
    copy_words(p[0], p[-1], kMatrixWords);
}

// 0x00472f30 zTransformStackPushRef - CONFIRMED-BINARY (listing 0x00472f30..0x00472f57).
void __fastcall zTransformStackPushRef(float* m)
{
    ++g_TransformFlagSP_004e0e84;
    *++g_TransformMatrixSP_004e0e88 = m;
    *g_TransformFlagSP_004e0e84 = kFlagNotIdentity;
}

// 0x00472f60 zTransformStackPop - CONFIRMED-BINARY (listing 0x00472f60..0x00472f81): no store.
void zTransformStackPop()
{
    --g_TransformFlagSP_004e0e84;
    --g_TransformMatrixSP_004e0e88;
}

// 0x00473250 - CONFIRMED-BINARY (listing 0x00473250..0x00473270): copy, then flag := 0.
void __fastcall zTransformLoad(const float* src)
{
    copy_words(*g_TransformMatrixSP_004e0e88, src, kMatrixWords);
    *g_TransformFlagSP_004e0e84 = kFlagNotIdentity;
}

// 0x00472f90 - CONFIRMED-BINARY: MOV ECX,0x5668e8; JMP 0x00473250.
void zTransformLoadView() { zTransformLoad(g_ViewMatrixA_005668e8); }

// 0x00472fa0 - CONFIRMED-BINARY: MOV ECX,0x566920; JMP 0x00473250.
void zTransformLoadCameraWorld() { zTransformLoad(g_CameraWorldB_00566920); }

// 0x00473210 - CONFIRMED-BINARY (listing 0x00473210..0x00473228, RET 4). The flag is untouched.
// EAX on return = out (MOV EAX,[ESP+0xc]; MOV EDI,EAX at 0x0047321e): gwNodeBuildNodeToAncestorMatrix 0x00449480 does
// MOV ESI,EAX after the call and copies the 12 words from there into the node's cached matrix (class data +0x60).
// Returning void left EAX = out+0x30, so that cache got 12 words of neighbouring stack (KG-51: turret not drawn).
float* __stdcall zTransformStore(float* out)
{
    copy_words(out, *g_TransformMatrixSP_004e0e88, kMatrixWords);
    return out;
}

// 0x00473230 - CONFIRMED-BINARY: MOV EAX,[0x004e0e88]; MOV EAX,[EAX]; RET.
float* zTransformGetCurrent() { return *g_TransformMatrixSP_004e0e88; }

// 0x00473240 - CONFIRMED-BINARY: MOV EAX,[0x004e0e84]; MOV EAX,[EAX]; RET.
std::uint32_t zTransformIsIdentity() { return *g_TransformFlagSP_004e0e84; }

// 0x00473280 - CONFIRMED-BINARY (listing 0x00473280..0x004732ec): each word read then written, in
// order 0..8 (so a forward word copy); translation untouched; flag := 0.
void __fastcall zTransformSetRotation(const float* src)
{
    copy_words(*g_TransformMatrixSP_004e0e88, src, kRotationWords);
    *g_TransformFlagSP_004e0e84 = kFlagNotIdentity;
}

// 0x004732f0 - CONFIRMED-BINARY (listing 0x004732f0..0x0047336a): 1.0 at words 0, 4, 8, 0.0 elsewhere
// (translation included); flag := 1.
void zTransformLoadIdentity()
{
    auto* m = reinterpret_cast<volatile std::uint32_t*>(*g_TransformMatrixSP_004e0e88);
    for (int i = 0; i < kMatrixWords; ++i) m[i] = (i == 0 || i == 4 || i == 8) ? kOneBits : kZeroBits;
    *g_TransformFlagSP_004e0e84 = kFlagIdentity;
}

// 0x004731f0 - CONFIRMED-BINARY: CALL 0x00472f90; MOV EAX,[0x004e0e88]; MOV EDX,1; MOV ECX,[EAX-4];
// JMP 0x00473370 (tail call).
void zTransformLoadViewConcatParent()
{
    zTransformLoadView();
    constexpr int kModeGeneral = 1;  // CONFIRMED-BINARY: MOV EDX,0x1 at 0x004731fa
    zTransformConcatenateLocal(g_TransformMatrixSP_004e0e88[-1], kModeGeneral);
}

}  // namespace recoil
