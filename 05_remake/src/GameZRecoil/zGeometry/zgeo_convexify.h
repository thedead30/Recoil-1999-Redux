// SUBSYSTEM: geometry
// Declarations for src/GameZRecoil/zGeometry/zgeo_convexify.cpp (original file GameZRecoil\\zGeometry\\zgeo_convexify.cpp,
// ledger orig_file). Spec: 04_spec/systems/geometry.md
#pragma once

namespace recoil {

// 0x0046ced0 Triangulate_SplitPolygon
int __fastcall Triangulate_SplitPolygon(int ecx, int edx, int arg1, int arg2, int arg3);
// 0x0046cb50 Triangulate_PolygonRecursive
int __fastcall Triangulate_PolygonRecursive(int ecx, int edx, int arg1, int arg2);
// 0x0046c760 Convexify
int __fastcall Convexify(int ecx, int edx, int arg1);

}  // namespace recoil
