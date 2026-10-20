// Structured native L1s for the weapon functions on heap blocks, record arrays and node trees (P3 weapon, ported in
// the cloud):
//  - PlayerEntry_Construct (0x004383e0): clears the entry's words, mallocs a zeroed 0x10C4-byte block into +4; returns
//    the entry. PlayerEntry_AddMount (0x004384e0): mallocs a zeroed 0xC4-byte mount, makes it +8 when that is empty,
//    appends it to the list +0x14 head / +0x18 tail / +0x1C count (link at +0); returns it.
//  - Weapon_FindMountByName (0x004ae3c0) / Weapon_FindMountByField10 (0x004ae450): first record of the mount array
//    [0x00778928] (count [0x00778924], 0x164 bytes each) whose name ([0], case-sensitive) matches / whose [0] is set
//    and +0x10 equals ECX; the record or 0.
//  - Weapon_FreeBeamInstance (0x004b1f90): free(ECX), returns 0.
//  - Weapon_PropagateTargetTag (0x004b25f0) / Weapon_ClearTargetTag (0x004b2670): recursive over the children (+0x5C
//    count, +0x60 array); set +0xBC = EDX on untagged nodes (tagged subtrees are skipped) / clear +0xBC where it is EDX.
//  - Weapon_RecordFirePoseAndDispatch (0x004b2880; ret 4): when [0x00778968] == 1 copies the vec3 at ECX and the one at
//    EDX + 0xC into 0x00778940..0x00778954; then calls [[EDX+0x24]+0xBC] +0xC with ECX = its +8 and the stack arg.
// Compared: returns and every word written, pointers by role; the heap blocks' contents; freed pointers by role.
#include "test.h"
#include "cloud_harness.h"
#include "GameZRecoil/zWeapon/zwep_init.h"
#include "unattributed/weapon.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

TEST(native_weapon_player_entry_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = std::uint32_t(__fastcall*)(std::uint32_t*, int);
    const F ctor[2] = {rt::original<F>(0x004383e0), reinterpret_cast<F>(&recoil::PlayerEntry_Construct)};
    const F add[2] = {rt::original<F>(0x004384e0), reinterpret_cast<F>(&recoil::PlayerEntry_AddMount)};
    std::mt19937 rng(0x4384e0);
    for (int it = 0; it < 200; ++it) {
        const int mounts = static_cast<int>(rng() % 6);
        const bool construct = rng() % 4 != 0;
        std::uint32_t init[12];
        for (auto& w : init) w = rng();
        if (!construct) { init[2] = 0; init[5] = 0; init[6] = 0; init[7] = 0; }  // an entry built elsewhere: empty list
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            ch::Roles rl;
            std::uint32_t e[12];
            std::memcpy(e, init, sizeof e);
            rl.set(e, 1);
            std::vector<std::uint32_t> blocks;
            if (construct) {
                snap[side].push_back(rl(ctor[side](e, 0)));
                blocks.push_back(e[1]);
                const auto* b = ch::at(e[1]);
                bool zero = true;
                for (int k = 0; k < 0x10c4 / 4; ++k) zero = zero && b[k] == 0;
                snap[side].push_back(zero);
            }
            for (int m = 0; m < mounts; ++m) {
                const std::uint32_t p = add[side](e, 0);
                snap[side].push_back(rl(p));
                blocks.push_back(p);
            }
            for (std::uint32_t w : e) snap[side].push_back(rl(w));
            for (std::size_t k = construct ? 1 : 0; k < blocks.size(); ++k)
                for (int w = 0; w < 0xc4 / 4; ++w) snap[side].push_back(rl(ch::at(blocks[k])[w]));
            for (std::uint32_t b : blocks) if (b) ch::c_free(b);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

TEST(native_weapon_find_mount_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = std::uint32_t(__fastcall*)(std::uint32_t, int);
    const F by_name[2] = {rt::original<F>(0x004ae3c0), reinterpret_cast<F>(&recoil::Weapon_FindMountByName)};
    const F by_field[2] = {rt::original<F>(0x004ae450), reinterpret_cast<F>(&recoil::Weapon_FindMountByField10)};
    static const char* const names[] = {"", "gun", "Gun", "gun1", "gun2", "rocket", "rocketpod", "gu"};
    std::mt19937 rng(0x4ae3c0);
    for (int it = 0; it < 3000; ++it) {
        const int count = static_cast<int>(rng() % 7) - (rng() % 10 == 0 ? 3 : 0);
        std::vector<std::uint32_t> recs(7 * 0x164 / 4, 0);
        for (int r = 0; r < 7; ++r) {
            recs[r * 0x59] = rng() % 5 ? ch::addr(names[rng() % 8]) : 0;
            recs[r * 0x59 + 4] = rng() % 4;
            recs[r * 0x59 + 1] = rng();
        }
        const bool name_query = rng() % 2 != 0;
        const std::string query = names[rng() % 8];
        const std::uint32_t field = rng() % 4;
        std::uint32_t ret[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            *ch::img(side, 0x00778924) = static_cast<std::uint32_t>(count);
            *ch::img(side, 0x00778928) = ch::addr(recs.data());
            const std::uint32_t r = name_query ? by_name[side](ch::addr(query.c_str()), 0) : by_field[side](field, 0);
            ret[side] = r ? (r - ch::addr(recs.data())) / 0x164 + 1 : 0;
        }
        CHECK_EQ(ret[0], ret[1]);
    }
    rt::restore_pristine();
}

TEST(native_weapon_free_beam_instance_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(void*, int);
    const F fn[2] = {rt::original<F>(0x004b1f90), reinterpret_cast<F>(&recoil::Weapon_FreeBeamInstance)};
    for (int it = 0; it < 20; ++it) {
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            void* p = it % 5 ? ch::c_malloc(0x40) : nullptr;
            ch::Roles rl;
            rl.set(p, 1);
            ch::freed().clear();
            int ret;
            {
                ch::FreeHook hook;
                ret = fn[side](p, 0);
            }
            snap[side].push_back(static_cast<std::uint32_t>(ret));
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

TEST(native_weapon_target_tags_match_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t*, std::uint32_t);
    const F fn[2][2] = {
        {rt::original<F>(0x004b25f0), reinterpret_cast<F>(&recoil::Weapon_PropagateTargetTag)},
        {rt::original<F>(0x004b2670), reinterpret_cast<F>(&recoil::Weapon_ClearTargetTag)},
    };
    std::mt19937 rng(0x4b25f0);
    for (int it = 0; it < 1000; ++it) {
        // a random tree of up to 24 nodes, each 0xC0 bytes; children arrays per node
        const int n = 1 + static_cast<int>(rng() % 24);
        std::vector<int> parent(n, -1);
        for (int k = 1; k < n; ++k) parent[k] = static_cast<int>(rng() % k);
        std::vector<std::uint32_t> tag0(n);
        const std::uint32_t tag = 0x100 + rng() % 3;
        for (auto& t : tag0) t = rng() % 3 == 0 ? 0 : rng() % 2 ? tag : 0x100 + rng() % 3;
        const int which = static_cast<int>(rng() % 2), root = static_cast<int>(rng() % n);
        const bool zero_count_with_array = rng() % 5 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            std::vector<std::vector<std::uint32_t>> node(n, std::vector<std::uint32_t>(0xc0 / 4, 0));
            std::vector<std::vector<std::uint32_t>> kids(n);
            for (int k = 1; k < n; ++k) kids[parent[k]].push_back(ch::addr(node[k].data()));
            for (int k = 0; k < n; ++k) {
                node[k][0x5c / 4] = static_cast<std::uint32_t>(kids[k].size());
                node[k][0x60 / 4] = kids[k].empty() ? (zero_count_with_array ? 0x1u : 0u) : ch::addr(kids[k].data());
                node[k][0xbc / 4] = tag0[k];
            }
            fn[which][side](node[root].data(), tag);  // returns what EAX held
            for (int k = 0; k < n; ++k) snap[side].push_back(node[k][0xbc / 4]);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

namespace {
std::vector<std::uint32_t>& dispatched()
{
    static std::vector<std::uint32_t> v;
    return v;
}
int __fastcall fire_handler(std::uint32_t ecx, std::uint32_t, std::uint32_t arg)
{
    dispatched().push_back(ecx);
    dispatched().push_back(arg);
    return 0;
}
}  // namespace

TEST(native_weapon_record_fire_pose_and_dispatch_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(const float*, std::uint32_t*, std::uint32_t);
    const F fn[2] = {rt::original<F>(0x004b2880), reinterpret_cast<F>(&recoil::Weapon_RecordFirePoseAndDispatch)};
    std::mt19937 rng(0x4b2880);
    for (int it = 0; it < 500; ++it) {
        const std::uint32_t mode = rng() % 3 == 0 ? rng() : rng() % 2, arg = rng(), ctx = rng();
        float pose[3], obj_pos[3];
        for (float& f : pose) f = static_cast<float>(rng() % 2000) / 7.0f;
        for (float& f : obj_pos) f = static_cast<float>(rng() % 2000) / 3.0f;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::uint32_t target[4] = {0, 0, ctx, ch::addr(reinterpret_cast<void*>(&fire_handler))};
            std::uint32_t owner[0xc0 / 4] = {};
            owner[0xbc / 4] = ch::addr(target);
            std::uint32_t obj[0x28 / 4] = {};
            std::memcpy(&obj[3], obj_pos, 12);
            obj[0x24 / 4] = ch::addr(owner);
            *ch::img(side, 0x00778968) = mode;
            dispatched().clear();
            fn[side](pose, obj, arg);
            snap[side] = dispatched();
            for (std::uint32_t k = 0; k < 6; ++k) snap[side].push_back(*ch::img(side, 0x00778940 + 4 * k));
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
    rt::restore_pristine();
}

// Vehicle_StopWeaponSounds (0x00438b60): for the nine weapon slots of the block at [ECX+4] (+0x740 - 0x54 and +0x740,
// stride 0xAC) frees each non-null beam instance through Weapon_FreeBeamInstance. Compared: freed pointers by role.
TEST(native_vehicle_stop_weapon_sounds_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t*, int);
    const F fn[2] = {rt::original<F>(0x00438b60), reinterpret_cast<F>(&recoil::Vehicle_StopWeaponSounds)};
    std::mt19937 rng(0x438b60);
    for (int it = 0; it < 200; ++it) {
        std::vector<bool> used(18);
        for (std::size_t k = 0; k < used.size(); ++k) used[k] = rng() % 2 != 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            ch::Roles rl;
            std::vector<std::uint32_t> blk(0xd00 / 4, 0);
            for (int s = 0; s < 9; ++s)
                for (int h = 0; h < 2; ++h) {
                    const std::uint32_t off = 0x740 - (h ? 0 : 0x54) + 0xac * s;
                    if (!used[2 * s + h]) continue;
                    blk[off / 4] = ch::addr(ch::c_malloc(0x20));
                    rl.set(blk[off / 4], 1 + 2 * s + h);
                }
            std::uint32_t obj[2] = {0, ch::addr(blk.data())};
            ch::freed().clear();
            {
                ch::FreeHook hook;
                fn[side](obj, 0);
            }
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
            for (std::uint32_t w : blk) if (w && !ch::was_freed(w)) ch::c_free(w);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

// Weapon_TagTargetTree (0x004b25a0; ECX owner, EDX node, stack value; ret 4): a node without a tag record gets a
// calloc'd 16-byte one; unless the record's +4 is already set: +0 = owner, +4 = value, the record propagates down the
// tree (Weapon_PropagateTargetTag) and the node gets flag 0x40 (gwNodeSetFlag40). Weapon_TagTargetTreeAlt (0x004b26b0):
// the same with +8 / +0xC and no flag. Weapon_UntagTargetTree (0x004b2630; ECX node): clears the record from the tree,
// drops flag 0x40 when +4 is set, frees the record. Sequences of 1..4 calls on random trees (untag at the root). Compared: every node's
// tag (records by role) and flags, the records' words, freed records by role.
TEST(native_weapon_target_tree_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using T = int(__fastcall*)(std::uint32_t, std::uint32_t*, std::uint32_t);
    using U = int(__fastcall*)(std::uint32_t*, int);
    const T tag[2] = {rt::original<T>(0x004b25a0), reinterpret_cast<T>(&recoil::Weapon_TagTargetTree)};
    const T tag_alt[2] = {rt::original<T>(0x004b26b0), reinterpret_cast<T>(&recoil::Weapon_TagTargetTreeAlt)};
    const U untag[2] = {rt::original<U>(0x004b2630), reinterpret_cast<U>(&recoil::Weapon_UntagTargetTree)};
    std::mt19937 rng(0x4b25a0);
    for (int it = 0; it < 500; ++it) {
        const int n = 1 + static_cast<int>(rng() % 12);
        std::vector<int> parent(n, -1);
        for (int k = 1; k < n; ++k) parent[k] = static_cast<int>(rng() % k);
        std::vector<std::uint32_t> flags(n);
        for (auto& f : flags) f = rng() & ~0x40u;
        struct Op { int kind, node; std::uint32_t owner, value; };
        std::vector<Op> ops(1 + rng() % 4);
        // untag only at the root: it clears the record from the whole tree before freeing it, so no node keeps a
        // pointer to a freed record (an untag lower down would leave the ancestors' copies dangling)
        for (Op& o : ops) {
            o = Op{static_cast<int>(rng() % 3), static_cast<int>(rng() % n), rng(), rng() % 3 ? rng() : 0};
            if (o.kind == 2) o.node = 0;
        }
        const bool null_untag = rng() % 8 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            ch::Roles rl;
            std::vector<std::vector<std::uint32_t>> node(n, std::vector<std::uint32_t>(0xc0 / 4, 0));
            std::vector<std::vector<std::uint32_t>> kids(n);
            for (int k = 1; k < n; ++k) kids[parent[k]].push_back(ch::addr(node[k].data()));
            for (int k = 0; k < n; ++k) {
                rl.set(node[k].data(), 0x100 + k);
                node[k][0x24 / 4] = flags[k];
                node[k][0x5c / 4] = static_cast<std::uint32_t>(kids[k].size());
                node[k][0x60 / 4] = kids[k].empty() ? 0u : ch::addr(kids[k].data());
            }
            std::vector<std::uint32_t> records;
            ch::freed().clear();
            {
                ch::FreeHook hook;
                for (const Op& o : ops) {
                    int ret;
                    if (o.kind == 0) ret = tag[side](o.owner, node[o.node].data(), o.value);
                    else if (o.kind == 1) ret = tag_alt[side](o.owner, node[o.node].data(), o.value);
                    else ret = untag[side](null_untag ? nullptr : node[o.node].data(), 0);
                    snap[side].push_back(static_cast<std::uint32_t>(ret));
                    for (int k = 0; k < n; ++k) {
                        const std::uint32_t r = node[k][0xbc / 4];
                        snap[side].push_back(rl(r));
                        snap[side].push_back(node[k][0x24 / 4]);
                        if (r && !ch::was_freed(r)) {
                            bool seen = false;
                            for (std::uint32_t x : records) seen = seen || x == r;
                            if (!seen) records.push_back(r);
                        }
                    }
                    for (std::uint32_t r : records)
                        if (!ch::was_freed(r)) for (int w = 0; w < 4; ++w) snap[side].push_back(ch::at(r)[w]);
                }
            }
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
            for (std::uint32_t r : records) if (!ch::was_freed(r)) ch::c_free(r);
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}

// Vehicle_BindGunPoints (0x004390d0; ECX vehicle, EDX release flag): in the model tree [block+0xED0] (block = [ECX+4])
// finds the child named by 0x004dcfc8 (Node_FindChildByNameRecursive) into +0xEE0 and copies its local position
// (Object3D_GetLocalMatrix +0x24) to +0xEA8; then three named fire points (0x004dd210 / 0x004dd208 / 0x004dd200, via
// gwNodeFindByName under it) give their positions (Object3D_GetPosition) to +0xD80 / +0xD98 / +0xD8C; with the flag,
// each fire point's model is detached (gwNodeSetModel(node, 0)) and a model with references is released and freed.
// Real trees of Object3D nodes (class 5, class data from random floats) up to depth 3 with names from a pool that holds
// the ledger's gun / fpnt_c / fpnt_l / fpnt_r strings plus near misses; the nodes carry no model and flag bit 0 (already
// registered), so gwNodeSetModel touches no model pool or node list - the model release branch is not exercised here.
// Compared: the vehicle block words written (+0xD80..+0xDA4, +0xEA8..+0xEB0, +0xEE0 by node index) and every node's
// flag, change and model words.
TEST(native_vehicle_bind_gun_points_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t*, int);
    const F fn[2] = {rt::original<F>(0x004390d0), reinterpret_cast<F>(&recoil::Vehicle_BindGunPoints)};
    static const char* const names[] = {"gun", "fpnt_c", "fpnt_l", "fpnt_r", "body", "Gun", "fpnt", "gun2"};
    std::mt19937 rng(0x4390d0);
    int compared = 0, bound = 0;
    for (int it = 0; it < 2000; ++it) {
        std::vector<int> parent{-1}, depth{0}, name{static_cast<int>(rng() % 8)};
        for (std::size_t i = 0; i < parent.size() && parent.size() < 30; ++i) {
            if (depth[i] >= 3) continue;
            const int k = static_cast<int>(rng() % 4);
            for (int c = 0; c < k; ++c) {
                parent.push_back(static_cast<int>(i));
                depth.push_back(depth[i] + 1);
                name.push_back(static_cast<int>(rng() % (depth[i] == 0 ? 2 : 8)));
            }
        }
        const std::size_t n = parent.size();
        std::vector<std::uint32_t> data0(n * 0x40);
        for (auto& w : data0) w = ch::fbits(static_cast<float>(static_cast<int>(rng() % 20001) - 10000) / 16.0f);
        std::vector<std::uint32_t> flags0(n);
        for (auto& f : flags0) f = (rng() & ~0x200u) | 1u;
        const int release = static_cast<int>(rng() % 2);
        const bool no_tree = rng() % 10 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<std::vector<std::uint32_t>> node(n, std::vector<std::uint32_t>(0xc4 / 4, 0)), kids(n);
            std::vector<std::uint32_t> data = data0;
            for (std::size_t i = 0; i < n; ++i) std::strcpy(reinterpret_cast<char*>(node[i].data()), names[name[i]]);
            for (std::size_t i = 1; i < n; ++i) kids[parent[i]].push_back(ch::addr(node[i].data()));
            for (std::size_t i = 0; i < n; ++i) {
                node[i][0x24 / 4] = flags0[i];
                node[i][0x34 / 4] = 5;
                node[i][0x38 / 4] = ch::addr(&data[i * 0x40]);
                node[i][0x5c / 4] = static_cast<std::uint32_t>(kids[i].size());
                node[i][0x60 / 4] = kids[i].empty() ? 0u : ch::addr(kids[i].data());
            }
            std::vector<std::uint32_t> blk(0xf00 / 4, 0x77777777u);
            blk[0xed0 / 4] = no_tree ? 0u : ch::addr(node[0].data());
            std::uint32_t veh[2] = {0, ch::addr(blk.data())};
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](veh, release)));
            auto index = [&](std::uint32_t p) -> std::uint32_t {
                for (std::size_t i = 0; i < n; ++i) if (p == ch::addr(node[i].data())) return static_cast<std::uint32_t>(i + 1);
                return p ? 0xbad : 0;
            };
            for (std::uint32_t off = 0xd80; off < 0xda4; off += 4) snap[side].push_back(blk[off / 4]);
            for (std::uint32_t off = 0xea8; off < 0xeb4; off += 4) snap[side].push_back(blk[off / 4]);
            snap[side].push_back(index(blk[0xee0 / 4]));
            for (std::size_t i = 0; i < n; ++i) {
                snap[side].push_back(node[i][0x24 / 4]);
                snap[side].push_back(node[i][0x2c / 4]);
                snap[side].push_back(node[i][0x3c / 4]);
            }
            if (side == 0) bound += blk[0xee0 / 4] != 0;
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Vehicle_BindGunPoints calls %d, %d with a gun node\n", compared, bound);
    CHECK(bound > 200);
}

// Weapon_ComputeSecondaryMuzzle (0x0043acf0; ECX vehicle, EDX out pointer): needs the gun node +0xEE0 and the node
// +0xEDC of the block [ECX+4]; when +0x5C0 is set re-binds the gun points first (Vehicle_BindGunPoints(vehicle, 0)) and
// clears it; composes the local matrix (Vehicle_ComposeLocalMatrix); by the fire-point index +0xD48 (0, 1, 2) transforms
// fire point +0xD80 / +0xD98 / +0xD8C into +0xDC8 (index 0 with descriptor [+0x5E8]+0x28 set takes the matrix
// translation instead), stores the descriptor's +0xC / +0x10 into +0xF1C / +0xF0C / +0xF14, hands out +0xF18 / +0xF08 /
// +0xF10 through EDX and moves the index on (2 -> 1, 1 -> 2, 0 as the listing sets it). Real floats; the two nodes are
// Object3D nodes (class 5, local matrix at class data +0x30); with the re-bind flag the model tree +0xED0 is the gun
// node itself (named "gun", no children), so the re-bind finds it again. Compared: the return, the out pointer as a
// block offset, and the block words +0x5C0, +0xD48, +0xD80..+0xDD4, +0xEA8..+0xEE4, +0xF08..+0xF20.
TEST(native_weapon_compute_secondary_muzzle_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = int(__fastcall*)(std::uint32_t*, std::uint32_t*);
    const F fn[2] = {rt::original<F>(0x0043acf0), reinterpret_cast<F>(&recoil::Weapon_ComputeSecondaryMuzzle)};
    std::mt19937 rng(0x43acf0);
    auto rf = [&]() { return ch::fbits(static_cast<float>(static_cast<int>(rng() % 20001) - 10000) / 64.0f); };
    int compared = 0;
    for (int it = 0; it < 3000; ++it) {
        std::vector<std::uint32_t> blk0(0xf40 / 4), gdata0(0x40), odata0(0x40), desc0(0x40 / 4);
        for (auto& w : blk0) w = rf();
        for (auto& w : gdata0) w = rf();
        for (auto& w : odata0) w = rf();
        for (auto& w : desc0) w = rng();
        desc0[0x28 / 4] = rng() % 2 ? 0 : rng();
        const std::uint32_t mode = rng() % 5 == 0 ? rng() % 8 : rng() % 3, rebind = rng() % 4 == 0 ? 1 : 0;
        const bool no_gun = rng() % 12 == 0, no_other = rng() % 12 == 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            rt::restore_pristine();
            std::vector<std::uint32_t> blk = blk0, gdata = gdata0, odata = odata0, desc = desc0;
            std::vector<std::uint32_t> gun(0xc4 / 4, 0), other(0xc4 / 4, 0);
            std::strcpy(reinterpret_cast<char*>(gun.data()), "gun");
            gun[0x24 / 4] = 1;
            gun[0x34 / 4] = other[0x34 / 4] = 5;
            gun[0x38 / 4] = ch::addr(gdata.data());
            other[0x38 / 4] = ch::addr(odata.data());
            blk[0xed0 / 4] = ch::addr(gun.data());
            blk[0xee0 / 4] = no_gun ? 0u : ch::addr(gun.data());
            blk[0xedc / 4] = no_other ? 0u : ch::addr(other.data());
            blk[0x5c0 / 4] = rebind;
            blk[0x5e8 / 4] = ch::addr(desc.data());
            blk[0xd48 / 4] = mode;
            std::uint32_t veh[2] = {0, ch::addr(blk.data())};
            std::uint32_t out = 0xfeedface;
            ch::wipe_stack();
            snap[side].push_back(static_cast<std::uint32_t>(fn[side](veh, &out)));
            snap[side].push_back(out == 0xfeedface ? 1u : out - ch::addr(blk.data()));
            snap[side].push_back(blk[0x5c0 / 4]);
            snap[side].push_back(blk[0xd48 / 4]);
            for (std::uint32_t off = 0xd80; off < 0xdd8; off += 4) snap[side].push_back(blk[off / 4]);
            for (std::uint32_t off = 0xea8; off < 0xee8; off += 4)
                snap[side].push_back(off == 0xed0 || off == 0xedc || off == 0xee0 ? (blk[off / 4] ? 1u : 0u) : blk[off / 4]);
            for (std::uint32_t off = 0xf08; off < 0xf24; off += 4) snap[side].push_back(blk[off / 4]);
        }
        CHECK_SNAP(snap[0], snap[1]);
        ++compared;
    }
    rt::restore_pristine();
    std::printf("  Weapon_ComputeSecondaryMuzzle calls %d\n", compared);
}

// PlayerEntry_Destruct (0x00438430): when the block [+4] is in mode 2 (+0x60) pops the temporary route nodes
// (AiVeh_PopNegativeRouteNodes: +0xF78 chain through +0xC while the id +0x28 is negative, each freed); clears
// [+0x24]+0x308 when set; frees every mount of the list (+0x14 head, link at +0, unlinking each and keeping +0x10 /
// +0x18 / +0x1C in step), then the block, and clears +8. Entries built with PlayerEntry_Construct and 0..5
// PlayerEntry_AddMount calls of the same side, 0..2 temporary route nodes ending on a stack node. Compared: freed
// pointers by role and the entry's words by role.
TEST(native_weapon_player_entry_destruct_matches_original)
{
    if (!rt::map_original()) { CHECK(false); return; }
    using F = std::uint32_t(__fastcall*)(std::uint32_t*, int);
    const F ctor[2] = {rt::original<F>(0x004383e0), reinterpret_cast<F>(&recoil::PlayerEntry_Construct)};
    const F add[2] = {rt::original<F>(0x004384e0), reinterpret_cast<F>(&recoil::PlayerEntry_AddMount)};
    const F dtor[2] = {rt::original<F>(0x00438430), reinterpret_cast<F>(&recoil::PlayerEntry_Destruct)};
    std::mt19937 rng(0x438430);
    for (int it = 0; it < 500; ++it) {
        const int mounts = static_cast<int>(rng() % 6), temps = static_cast<int>(rng() % 3);
        const std::uint32_t mode = rng() % 3, extra = rng();
        const bool has_obj = rng() % 2 != 0;
        std::vector<std::uint32_t> snap[2];
        for (int side = 0; side < 2; ++side) {
            ch::Roles rl;
            std::uint32_t e[12];
            for (auto& w : e) w = extra;
            ctor[side](e, 0);
            rl.set(e, 1);
            rl.set(e[1], 2);
            for (int m = 0; m < mounts; ++m) rl.set(add[side](e, 0), 0x10 + m);
            std::uint32_t* blk = ch::at(e[1]);
            blk[0x60 / 4] = mode;
            std::uint32_t last[0x30 / 4] = {};
            last[0x28 / 4] = 7;
            rl.set(last, 3);
            std::uint32_t next = ch::addr(last);
            for (int t = 0; t < temps; ++t) {
                auto* n = static_cast<std::uint32_t*>(ch::c_malloc(0x30));
                std::memset(n, 0, 0x30);
                n[0x28 / 4] = 0xffffffffu;
                n[0xc / 4] = next;
                next = ch::addr(n);
                rl.set(next, 0x20 + t);
            }
            blk[0xf78 / 4] = next;
            std::uint32_t obj[0x30c / 4];
            for (auto& w : obj) w = 0x9999;
            e[0x24 / 4] = has_obj ? ch::addr(obj) : 0u;
            ch::freed().clear();
            {
                ch::FreeHook hook;
                dtor[side](e, 0);
            }
            for (std::uint32_t f : ch::freed()) snap[side].push_back(rl(f));
            for (std::uint32_t w : e) snap[side].push_back(rl(w));
            snap[side].push_back(obj[0x308 / 4]);
            // temporary nodes the call kept (mode != 2) are released here
            if (mode != 2)
                for (std::uint32_t p = next; p != ch::addr(last);) { const std::uint32_t q = ch::at(p)[0xc / 4]; ch::c_free(p); p = q; }
        }
        CHECK_SNAP(snap[0], snap[1]);
    }
}
