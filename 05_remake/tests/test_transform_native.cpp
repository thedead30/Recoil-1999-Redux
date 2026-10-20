// Native L1 for the transform stack: the same operations are run on the ORIGINAL functions (acting on the
// original's globals in the mapped image) and on the port (acting on its own globals); afterwards the
// whole logical state must be identical - depth, every flag slot, every pointer slot (as an index into
// the caller-owned matrix pool), every pool matrix bit for bit, matrices A and B, and any output.
#include "test.h"
#include "native_oracle.h"
#include "unattributed/transform.h"
#include "unattributed/view.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <random>

namespace {
constexpr int kPool = 40;
constexpr std::uintptr_t kOrigFlagSP = 0x004e0e84, kOrigMatSP = 0x004e0e88;
constexpr std::uintptr_t kOrigMatStack = 0x00566868, kOrigFlagStack = 0x00566950;
constexpr std::uintptr_t kOrigA = 0x005668e8, kOrigB = 0x00566920;

template <class T> T& at(std::uintptr_t va) { return *reinterpret_cast<T*>(va); }

struct Side {
    float pool[kPool][12];
};
Side O, P;
int g_depth = 0;
std::mt19937 rng(0x7F5A);

int pool_index(const Side& s, const float* m)
{
    for (int i = 0; i < kPool; ++i)
        if (m == s.pool[i]) return i;
    return m ? -2 : -1;
}
void set_both(float& o, float& p, std::uint32_t bits) { std::memcpy(&o, &bits, 4); std::memcpy(&p, &bits, 4); }
float rndf(float lo, float hi)
{
    const float edges[] = {0.0f, -0.0f, 1.0f, -1.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(),
                           -std::numeric_limits<float>::infinity(), 1e-40f, 1e19f, -1e19f};
    if (rng() % 12 == 0) return edges[rng() % 10];
    return std::uniform_real_distribution<float>(lo, hi)(rng);
}
std::uint32_t bits_of(float f) { std::uint32_t b; std::memcpy(&b, &f, 4); return b; }

// Seed both sides with equal contents (raw random bits when `raw`, else plausible floats) and reset
// both stacks to their load-time state (CONFIRMED-DATA initial pointer values).
void reset_both(bool raw)
{
    for (int i = 0; i < kPool; ++i)
        for (int k = 0; k < 12; ++k) set_both(O.pool[i][k], P.pool[i][k], raw ? static_cast<std::uint32_t>(rng()) : bits_of(rndf(-10, 10)));
    for (int k = 0; k < 12; ++k) {
        set_both(*reinterpret_cast<float*>(kOrigA + 4 * k), recoil::g_ViewMatrixA_005668e8[k], raw ? static_cast<std::uint32_t>(rng()) : bits_of(rndf(-10, 10)));
        set_both(*reinterpret_cast<float*>(kOrigB + 4 * k), recoil::g_CameraWorldB_00566920[k], raw ? static_cast<std::uint32_t>(rng()) : bits_of(rndf(-10, 10)));
    }
    std::memset(reinterpret_cast<void*>(kOrigMatStack), 0, 4 * recoil::kTransformStackSlots);
    std::memset(reinterpret_cast<void*>(kOrigFlagStack), 0, 4 * recoil::kTransformStackSlots);
    std::memset(recoil::g_TransformMatrixStack_00566868, 0, sizeof recoil::g_TransformMatrixStack_00566868);
    std::memset(recoil::g_TransformFlagStack_00566950, 0, sizeof recoil::g_TransformFlagStack_00566950);
    at<std::uint32_t>(kOrigFlagSP) = static_cast<std::uint32_t>(kOrigFlagStack);
    at<std::uint32_t>(kOrigMatSP) = static_cast<std::uint32_t>(kOrigMatStack);
    recoil::Transform_ResetState();
    g_depth = 0;
}

bool same_state()
{
    const int od = static_cast<int>((at<std::uint32_t>(kOrigMatSP) - kOrigMatStack) / 4);
    const int of = static_cast<int>((at<std::uint32_t>(kOrigFlagSP) - kOrigFlagStack) / 4);
    const int pd = static_cast<int>(recoil::g_TransformMatrixSP_004e0e88 - recoil::g_TransformMatrixStack_00566868);
    const int pf = static_cast<int>(recoil::g_TransformFlagSP_004e0e84 - recoil::g_TransformFlagStack_00566950);
    bool ok = od == g_depth && of == g_depth && pd == g_depth && pf == g_depth;
    for (int i = 0; ok && i < recoil::kTransformStackSlots; ++i) {
        ok = at<std::uint32_t>(kOrigFlagStack + 4 * i) == recoil::g_TransformFlagStack_00566950[i]
          && pool_index(O, reinterpret_cast<float*>(static_cast<std::uintptr_t>(at<std::uint32_t>(kOrigMatStack + 4 * i))))
                 == pool_index(P, recoil::g_TransformMatrixStack_00566868[i]);
    }
    return ok && std::memcmp(O.pool, P.pool, sizeof O.pool) == 0
              && std::memcmp(reinterpret_cast<void*>(kOrigA), recoil::g_ViewMatrixA_005668e8, 48) == 0
              && std::memcmp(reinterpret_cast<void*>(kOrigB), recoil::g_CameraWorldB_00566920, 48) == 0;
}

using VoidFn = void(__cdecl*)();
using PtrFn = void(__fastcall*)(float*);
const auto oPushRef = rt::original<PtrFn>(0x00472f30);
const auto oPushCopy = rt::original<PtrFn>(0x00472ef0);
const auto oIdent = rt::original<VoidFn>(0x004732f0);
}  // namespace

TEST(native_transform_stack_primitives_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using CPtrFn = void(__fastcall*)(const float*);
    using StoreFn = void(__stdcall*)(float*);
    using GetM = float*(__cdecl*)();
    using GetF = std::uint32_t(__cdecl*)();
    auto oPop = rt::original<VoidFn>(0x00472f60);
    auto oLoadA = rt::original<VoidFn>(0x00472f90);
    auto oLoadB = rt::original<VoidFn>(0x00472fa0);
    auto oStore = rt::original<StoreFn>(0x00473210);
    auto oGetM = rt::original<GetM>(0x00473230);
    auto oGetF = rt::original<GetF>(0x00473240);
    auto oLoad = rt::original<CPtrFn>(0x00473250);
    auto oSetR = rt::original<CPtrFn>(0x00473280);

    reset_both(true);
    int bad_state = 0, bad_ret = 0;
    for (int n = 0; n < 20000; ++n) {
        int op = static_cast<int>(rng() % 11);
        if (g_depth == 0) op = rng() % 2;                                   // the base slot holds no matrix
        if (op <= 1 && g_depth >= recoil::kTransformStackSlots - 1) op = 2; // stay inside the stack
        const int k = static_cast<int>(rng() % kPool);
        switch (op) {
        case 0:
            if (g_depth == 0) { oPushRef(O.pool[k]); recoil::zTransformStackPushRef(P.pool[k]); }
            else { oPushCopy(O.pool[k]); recoil::zTransformStackPushAndCopy(P.pool[k]); }
            ++g_depth; break;
        case 1: oPushRef(O.pool[k]); recoil::zTransformStackPushRef(P.pool[k]); ++g_depth; break;
        case 2: oPop(); recoil::zTransformStackPop(); --g_depth; break;
        case 3: oLoadA(); recoil::zTransformLoadView(); break;
        case 4: oLoadB(); recoil::zTransformLoadCameraWorld(); break;
        case 5: { float o1[12], o2[12]; oStore(o1); recoil::zTransformStore(o2); if (std::memcmp(o1, o2, 48)) ++bad_ret; } break;
        case 6: if (pool_index(O, oGetM()) != pool_index(P, recoil::zTransformGetCurrent())) ++bad_ret; break;
        case 7: if (oGetF() != recoil::zTransformIsIdentity()) ++bad_ret; break;
        case 8: oLoad(O.pool[k]); recoil::zTransformLoad(P.pool[k]); break;   // includes loading a matrix onto itself
        case 9: { const int off = static_cast<int>(rng() % 4); oSetR(O.pool[k] + off); recoil::zTransformSetRotation(P.pool[k] + off); } break;
        default: oIdent(); recoil::zTransformLoadIdentity(); break;
        }
        if (!same_state()) ++bad_state;
        if (bad_state || bad_ret) break;
    }
    CHECK_EQ(bad_state, 0);
    CHECK_EQ(bad_ret, 0);
}

TEST(native_transform_x87_ports_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using ConcatFn = void(__fastcall*)(const float*, int);
    using V3Fn = void(__stdcall*)(float, float, float);
    using AngFn = void(__stdcall*)(float);
    using VecFn = void(__fastcall*)(float*, int);
    using VecToFn = void(__fastcall*)(const float*, float*, int);
    auto oConcat = rt::original<ConcatFn>(0x00473370);
    auto oScale = rt::original<V3Fn>(0x00473690);
    auto oTrans = rt::original<V3Fn>(0x004737e0);
    auto oRotX = rt::original<AngFn>(0x00473970);
    auto oRotY = rt::original<AngFn>(0x00473b10);
    auto oRotZ = rt::original<AngFn>(0x00473cc0);
    auto oRotV = rt::original<VecFn>(0x004745e0);
    auto oRotVT = rt::original<VecFn>(0x00474670);
    auto oRotTo = rt::original<VecToFn>(0x00474710);
    auto oPts = rt::original<VecFn>(0x004747d0);
    auto oViewParent = rt::original<VoidFn>(0x004731f0);

    int bad[11] = {}, bad_mode2_translation = 0, mode2_calls = 0;
    for (int n = 0; n < 6000; ++n) {
        reset_both(false);
        // 1..3 levels; the current one is sometimes an identity level (flag 1) or an inherited copy.
        const int levels = 1 + static_cast<int>(rng() % 3);
        for (int l = 0; l < levels; ++l) {
            const int k = l * 10 + static_cast<int>(rng() % 10);
            if (l > 0 && rng() % 3 == 0) { oPushCopy(O.pool[k]); recoil::zTransformStackPushAndCopy(P.pool[k]); }
            else { oPushRef(O.pool[k]); recoil::zTransformStackPushRef(P.pool[k]); }
            ++g_depth;
            if (rng() % 3 == 0) { oIdent(); recoil::zTransformLoadIdentity(); }
        }
        const int op = n % 11;
        const float x = rndf(-7, 7), y = rndf(-7, 7), z = rndf(-7, 7);
        float vo[3 * 6], vp[3 * 6], to[3 * 6] = {}, tp[3 * 6] = {};
        for (int i = 0; i < 18; ++i) set_both(vo[i], vp[i], bits_of(rndf(-100, 100)));
        const int count = static_cast<int>(rng() % 7);  // 0..6 (a negative count makes 0x00474710 REP MOVS a huge block in the original too)
        switch (op) {
        case 0: {
            const int modes[] = {1, 3, 0, -1, 2};
            const int mode = modes[rng() % 5];
            const int k = static_cast<int>(rng() % kPool);   // may be the current matrix itself
            if (mode == 2 && at<std::uint32_t>(at<std::uint32_t>(kOrigFlagSP)) == 0) {
                // KG-25: the original's translation comes from stack residue; the port keeps the current one.
                ++mode2_calls;
                const float* cur = recoil::zTransformGetCurrent();
                float before[3];
                std::memcpy(before, cur + 9, 12);
                oConcat(O.pool[k], mode); recoil::zTransformConcatenateLocal(P.pool[k], mode);
                if (std::memcmp(before, cur + 9, 12) && pool_index(P, cur) != k) ++bad_mode2_translation;
                float* ocur = reinterpret_cast<float*>(static_cast<std::uintptr_t>(at<std::uint32_t>(at<std::uint32_t>(kOrigMatSP))));
                std::memcpy(ocur + 9, cur + 9, 12);  // compare everything else bit for bit
            } else {
                oConcat(O.pool[k], mode); recoil::zTransformConcatenateLocal(P.pool[k], mode);
            }
        } break;
        case 1: oScale(x, y, z); recoil::zTransformScaleLocal(x, y, z); break;
        case 2: oTrans(x, y, z); recoil::Transform_TranslateLocal(x, y, z); break;
        case 3: oRotX(x); recoil::Transform_RotateXLocal(x); break;
        case 4: oRotY(x); recoil::Transform_RotateYLocal(x); break;
        case 5: oRotZ(x); recoil::Transform_RotateZLocal(x); break;
        case 6: oRotV(vo, count); recoil::zTransformRotateVectors(vp, count); break;
        case 7: oRotVT(vo, count); recoil::zTransformRotateVectorsTransposed(vp, count); break;
        case 8: oRotTo(vo, to, count); recoil::zTransformRotateVectorsTo(vp, tp, count); break;
        case 9: oPts(vo, count); recoil::zTransformPoints(vp, count); break;
        default:
            if (g_depth < 2) break;  // needs a parent level
            oViewParent(); recoil::zTransformLoadViewConcatParent(); break;
        }
        if (!same_state() || std::memcmp(vo, vp, sizeof vo) || std::memcmp(to, tp, sizeof to)) ++bad[op];
    }
    for (int i = 0; i < 11; ++i) CHECK_EQ(bad[i], 0);
    CHECK_EQ(bad_mode2_translation, 0);
    CHECK(mode2_calls > 0);
}

TEST(native_transform_trs_and_quaternions_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using TrsFn = void(__fastcall*)(const float*, const float*, const float*);
    using QEFn = void(__fastcall*)(float*, int, float, float, float);
    using QPFn = void(__fastcall*)(const float*, const float*, float*);
    auto oTrs = rt::original<TrsFn>(0x00474010);
    auto oQE = rt::original<QEFn>(0x004757c0);
    auto oQP = rt::original<QPFn>(0x004759d0);
    int bad[3] = {};
    for (int n = 0; n < 6000; ++n) {
        reset_both(false);
        const int levels = 1 + static_cast<int>(rng() % 3);
        for (int l = 0; l < levels; ++l) {
            const int k = l * 10 + static_cast<int>(rng() % 10);
            oPushRef(O.pool[k]); recoil::zTransformStackPushRef(P.pool[k]);
            ++g_depth;
            if (rng() % 3 == 0) { oIdent(); recoil::zTransformLoadIdentity(); }
        }
        float ang[3], tr[3], sc[3];
        for (float& x : ang) x = rndf(-7, 7);
        for (float& x : tr) x = rndf(-50, 50);
        for (float& x : sc) x = (rng() % 3 == 0) ? 1.0f : rndf(-3, 3);  // 1.0 skips the row scale
        oTrs(ang, tr, sc); recoil::zTransformTRSLocal(ang, tr, sc);
        if (!same_state()) ++bad[0];
        {   float q1[4] = {5, 5, 5, 5}, q2[4] = {5, 5, 5, 5};
            oQE(q1, 0, ang[0], ang[1], ang[2]); recoil::Quat_FromEuler(q2, 0, ang[0], ang[1], ang[2]);
            if (std::memcmp(q1, q2, 16)) ++bad[1];
        }
        {   float a[4], b[4], o1[4] = {}, o2[4] = {};
            for (int i = 0; i < 4; ++i) { a[i] = rndf(-2, 2); b[i] = rndf(-2, 2); }
            oQP(a, b, o1); recoil::Quat_Product(a, b, o2);
            if (std::memcmp(o1, o2, 16)) ++bad[2];
        }
    }
    for (int i = 0; i < 3; ++i) CHECK_EQ(bad[i], 0);
}

TEST(native_transform_screen_and_billboards_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using ScrFn = void(__fastcall*)(const float*, float*, int);
    using BbYFn = void(__stdcall*)(float);
    auto oS2V = rt::original<ScrFn>(0x00474bc0);
    auto oS2W = rt::original<ScrFn>(0x00474c20);
    auto oBbY = rt::original<BbYFn>(0x00472fb0);
    auto oBbF = rt::original<VoidFn>(0x00473060);
    int bad[4] = {};
    for (int n = 0; n < 6000; ++n) {
        reset_both(false);
        // view state: projection block, screen centre, default angles - equal on both sides
        for (int i = 0; i < 12; ++i) set_both(*reinterpret_cast<float*>(0x00566838 + 4 * i), recoil::g_ViewParams_00566838[i], bits_of(rndf(-0.01f, 0.01f)));
        for (int i = 0; i < 2; ++i) set_both(*reinterpret_cast<float*>(0x005761e0 + 4 * i), recoil::g_ScreenCentre_005761e0[i], bits_of(rndf(0, 640)));
        for (int i = 0; i < 3; ++i) set_both(*reinterpret_cast<float*>(0x005669d8 + 4 * i), recoil::g_DefaultAngles_005669d8[i], bits_of(n % 2 ? 0.0f : rndf(-3, 3)));
        const int levels = 2 + static_cast<int>(rng() % 2);   // billboards read the parent level
        for (int l = 0; l < levels; ++l) {
            const int k = l * 10 + static_cast<int>(rng() % 10);
            oPushRef(O.pool[k]); recoil::zTransformStackPushRef(P.pool[k]);
            ++g_depth;
            if (rng() % 3 == 0) { oIdent(); recoil::zTransformLoadIdentity(); }
        }
        const int op = n % 4;
        float in[3 * 4], o1[3 * 4] = {}, o2[3 * 4] = {};
        for (int i = 0; i < 12; ++i) in[i] = (i % 3 == 2) ? rndf(0.5f, 50) : rndf(0, 640);
        switch (op) {
        case 0: { const int c = static_cast<int>(rng() % 5); oS2V(in, o1, c); recoil::zTransformScreenToView(in, o2, c); } break;
        case 1: oS2W(in, o1, 1); recoil::zTransformScreenToWorld(in, o2, 1); break;   // callers always pass 1
        case 2: { const float yaw = rndf(-4, 4); oBbY(yaw); recoil::zTransformBillboardYaw(yaw); } break;
        default: oBbF(); recoil::zTransformBillboardFull(); break;
        }
        if (!same_state() || std::memcmp(o1, o2, sizeof o1)) ++bad[op];
    }
    for (int i = 0; i < 4; ++i) CHECK_EQ(bad[i], 0);
}
