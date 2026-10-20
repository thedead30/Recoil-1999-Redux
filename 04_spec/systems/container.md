# Container (`0x0048c8e0`–`0x0048cbd0`) — the engine's generic list

**Status:** nine functions `CONFIRMED-BINARY`. This is not a subsystem in the ledger sense —
it is a general service used across the engine (9, 7, 6, 4, 3, 2 callers respectively). It is
specified here because the sound stream-cue machine, and probably much else, is unreadable
without it.

Reached while closing sound's callee coverage: 9 of sound's 19 uncovered callees were this
one family.

## Layout — `CONFIRMED-BINARY`

**Header, 0x14 bytes**, from `Container_CreateList` (`0x0048c950`): `malloc(0x14)`, all five
dwords zeroed.

| offset | meaning |
|---|---|
| `+0x00` | element count |
| `+0x04`, `+0x08`, `+0x0C` | zeroed at construction; no confirmed use in these nine |
| `+0x10` | **cursor** — the current node, and the anchor for every walk |

**Node**, allocated by `Container_AllocNodeWithPayload` (`0x0048ca10`):

| offset | meaning |
|---|---|
| `+0x00` | payload pointer |
| `+0x04` | next |
| `+0x08` | prev |

The list is **circular and doubly linked**: a one-element list has `[n+4] == [n+8] == n`, and
every walk terminates by comparing back against the cursor rather than against null. There is
no sentinel node.

The node block size is not established — only the three fields above, which the link and walk
functions pin exactly. See "Nodes are recycled, never freed" below.

## Nodes are recycled, never freed — `CONFIRMED-BINARY`

`Container_AllocNodeWithPayload` and `Container_FreeNode` do **not** reach `malloc`/`free`.
They call a pair that maintains an **engine-wide free list of nodes**, whose header
`[0x0056ae70]` is itself a Container header — the recycler is built out of the same primitive
it serves.

| address | name | behaviour |
|---|---|---|
| `0x0048c8e0` | `Container_NodeAlloc_FromFreeList` | pops a node from `[0x0056ae70]`; if empty, returns 0 when ECX == 0, else refills via `0x0048c800` and bumps `[0x0056ae7c]` |
| `0x0048c820` | `Container_NodeFree_ToFreeList` | pushes the node back onto `[0x0056ae70]`, creating it on first use |

`[0x0056ae78]` tracks live free-node count; `[0x0056ae7c]` counts refills. Both pop and push
are hand-inlined rather than calling `ListPopCursor`/`InsertAfterCursor`, though the pointer
surgery is identical (the push does reuse `Container_LinkNodePair`).

**Consequences for a remake:**
- Container nodes are never returned to the CRT heap during a run. A node freed by the sound
  cue machine can be handed straight to any other subsystem.
- **Node memory is not zeroed on reuse.** Only `[n+0]` is written by
  `AllocNodeWithPayload`; `next`/`prev` carry whatever the previous owner left until the
  insert path overwrites them. Code that reads a node's links before linking it would see
  stale pointers in the original too.
- `ECX == 0` at `0x0048c8e0` means "do not refill" — a caller can probe for an available
  node without growing the pool.

The bulk allocator `0x0048c800` is **UNREAD**; it is what would establish the node block size
and the growth chunk.

## The operations

| address | name | args | returns |
|---|---|---|---|
| `0x0048c950` | `Container_CreateList` | — | the header |
| `0x0048c9c0` | `Container_ListInsertAfterCursor` | hdr ECX | new count, or −1 |
| `0x0048c9a0` | `Container_LinkNodePair` | ECX, EDX, 1 stack | — |
| `0x0048ca10` | `Container_AllocNodeWithPayload` | payload ECX | node |
| `0x0048ca70` | `Container_ListRemoveMatching` | hdr ECX, key EDX | new count, or −1 |
| `0x0048cb00` | `Container_FindNodeByPayload` | hdr ECX, key EDX | **node**, or 0 |
| `0x0048cae0` | `Container_FreeNode` | node ECX | **the payload** |
| `0x0048cb70` | `Container_ListPopCursor` | hdr ECX | **the payload**, or 0 |
| `0x0048cbd0` | `Container_FindFirstByPredicate` | hdr ECX, fn EDX, 1 stack | payload, or 0 |

Everything is `__fastcall`-shaped with the header in ECX. Note the asymmetry worth
preserving: `FindNodeByPayload` returns a **node**, while `FreeNode`, `ListPopCursor` and
`FindFirstByPredicate` return a **payload**.

### The cursor moves on almost every operation
Insert makes the new node the cursor. Remove advances the cursor when it is the node being
removed. Pop always advances it. Every walk *starts* from it. So iteration order depends on
the mutation history — this is not a stable head-anchored list, and a remake using a
`std::list` with a fixed `begin()` will visit elements in a different order.

### Removal hands the payload back
`Container_FreeNode` reads `[n+0]` **before** freeing the node and returns it. That is how a
caller disposes of the stored object after the list has forgotten it — the list owns nodes,
never payloads.

## `Container_FindFirstByPredicate` is used as a for-each — `CONFIRMED-BINARY`

```
0048cbea  mov ecx,dword ptr [esi]     ; ECX = node payload
0048cbe8  mov edx,ebp                 ; EDX = the stack argument
0048cbec  call edi                    ; predicate
0048cbee  test eax,eax
0048cbf0  je  <return this payload>   ; ZERO means "found"
```

It stops on a **zero** return. `Sound_ServiceActiveStreamCues` (`0x004a5050`) passes
`0` as the user data and `Sound_StreamCue_TickDispatch` (`0x004a4c40`) as the predicate — and
**every arm of that dispatcher returns 1**, so the search never matches and the walk visits
every element before returning 0.

A find-first primitive is being deliberately reused as an iterator, and the behaviour depends
entirely on the tick function's return value. **A remake whose per-element tick returns 0 or
`void` would silently service only the first cue and then stop.** This is the kind of coupling
that produces a bug with no obvious cause, so it is recorded here rather than only in the
sound spec.

## Not established
- The node allocator `0x0048c8e0` and free `0x0048c820` — whether they are plain
  `malloc`/`free` wrappers or a pool. Until read, node size is unknown.
- Header fields `+0x04`, `+0x08`, `+0x0C`. Zeroed at construction and untouched by these
  nine, so another family of functions must own them.
- Whether any caller relies on the cursor's position across calls (it is observable, and the
  sound code does not appear to).
