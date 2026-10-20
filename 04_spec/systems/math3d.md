# math3d — general 3-D math

Subsystem `math3d`, **gate OPEN** (17/17 CONFIRMED, membership CLOSED). Pure functions: their
machine code references no `.data` address. Row-vector convention throughout (`p' = p.R + t`),
matching `transform_stack.md`. All addresses `Recoil.exe`.

| addr | operation | evidence |
|---|---|---|
| `0x00474de0` | `atan2(m[6], m[8])` of the matrix at `ECX`; 0 if both 0 | ORACLE, exact |
| `0x00474e10` | Euler extraction -> `(pitch, yaw, roll)` - see `transform_stack.md` | ORACLE 6e-9; roll path body-only |
| `0x00474260` | Euler `(a, b, g)` -> 3 x 3 at `ECX`, translation zeroed; equals `Rz(g).Rx(a).Ry(b)` of the confirmed local rotations | ORACLE 3.7e-8 |
| `0x00474f40` | rotate vector `EDX` about Y into `ECX`: `x' = c.x + s.z`, `z' = c.z - s.x` | ORACLE 1e-9 |
| `0x00474ec0` | rotate vector `EDX` about X into `ECX`: `y' = c.y - s.z`, `z' = c.z + s.y` | **body only** - its one sample had `y = z = 0` |
| `0x00474580` | heading -> direction: `(-sin a, 0, -cos a)` | ORACLE 2.1e-8 |
| `0x004745c0` | `(-v.z, 0, v.x)` - perpendicular in the XZ plane | ORACLE exact |
| `0x00474d10` | look-at from `EDX` to `ECX`: `(atan2(dy, hypot(dx,dz)), atan2(dx, dz), 0)` with `d = ECX - EDX`, `dy = EDX.y - ECX.y` | ORACLE 5.1e-8 |
| `0x00474d90` | the pitch term of the look-at alone | body only |
| `0x00474870` | 8 corners of box `(min.xyz, max.xyz)` at `EDX`, through matrix `ECX`, into 24 floats | ORACLE 9.2e-5 |
| `0x00475070` | triangle normal: `normalise((p1 - p0) x (p2 - p0))` | ORACLE 1e-8 |
| `0x00475130` | 2-D linear solve by Cramer's rule; both outputs 0 when singular | ORACLE 3.2e-10 |
| `0x00475210` | ray/sphere-style quadratic, stable-root form; **uses the fast square root** | body only |
| `0x004744f0` | `out[i] = p[i] + d[i] * t` over an array | body only |
| `0x00475910` | quaternion Hamilton product `(w, x, y, z)` | body only |
| `0x00475a80` | quaternion -> 3 x 3 rotation, standard formula | body only |
| `0x00475b80` | rotation vector `v` -> quaternion `(cos |v|, v sin|v| / |v|)`; `(1,0,0,0)` for `v = 0` | body only |

Called but outside the subsystem: **`0x00402f60`**, normalise in place and return the length -
an **exact** `fsqrt` and reciprocal, not an approximation; its zero test masks the sign bit, so
-0.0 counts as zero. ORACLE 2.1e-6 at length 770. 79k calls in `Recoil18`.

## Remake requirements
- **`0x00475210` must use the fast square root** `((int)x >> 1) + 0x1fc00000`, bit for bit, not
  `sqrt()`; see `math_approximations.md`. Its results differ by up to ~6%.
- Everything else uses the x87 `fsin` / `fcos` / `fpatan` / `fsqrt`, whose results a remake gets
  from ordinary `double` math; the traces agree to float32 rounding.
- "body only" rows never execute in `Recoil18` or `Recoil21`: implement them, but tag the code
  `IMPLEMENTED-UNVERIFIED` until a trace exercises them.

## Function index additions (2026-09-24)
- **Distances.** `Vec3_Distance` `0x004726d0` (sqrt of the squared distance) and
  `Vec3_DistSqXZ` `0x00472730`. Both leave the difference vector in the scratch globals
  `0x00566420..28`.
- **Vector arithmetic.**
  - `Vec3_AddScaled` `0x00472770`.
  - `Vec3_ScaleByReciprocal` `0x004727a0`: an exact `s == 0` test copies the vector unchanged.
  - `Vec3_Reflect` `0x00472860`: `k == 0` exactly gives `-d`.
  - `Vec3_SubNormalize` `0x004729b0` and `Vec3_LerpNormalize` `0x004729f0`.
- **Boxes.** `AABB_ToCorners` `0x00446ed0` expands a box to 8 corners.

