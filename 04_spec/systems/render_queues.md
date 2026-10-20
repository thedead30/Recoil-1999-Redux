

---

# WARNING - these are the DEAD path's queues
Everything above was read from `zRender\zrndr_draw.c`, which **never executes** (see the top
of this file). The **live** D3D renderer in `zvid_ddd3d.c` has its own separate queues with
different capacities, strides and base addresses, and re-uses the name
`MAX_TRANSPARENT_POLYS` for a differently-sized array.

**For anything a remake actually needs, use `04_spec/systems/render_d3d.md`.**

Quick contrast - transparent queue capacity is **350 here** but **256 on the live path**;
overwrite is **350 here** but **384** there, under the distinct name
`ZVID_MAX_OVERWRITE_POLYS`.
