# ai_net - AI (ai_net) (spec)

Path-network AI: net_%02d.zrd graph (v105), per-vehicle state machine at +0xf84 (0 patrol, 1 combat, 2-5 steer/route/return), combat sub-mode +0xff0, fire decision dot>0.75.

All rows CONFIRMED-BINARY at the cited address (bytes read; method in ledger).

| addr | name | notes |
|---|---|---|
| 0x00401060 | AiVeh_StateTick | [+0xec0..]=player pos +0x410; switch [+0xf84]: 0 -> 0x00401180 then TryDetect; 1 -> AiVeh_CombatTick; 2 -> SteerTowardPlayerA, TryDetect, on hit PushReturnNode; 3 -> SteerTowardPlayerB, TryDetect; 4 -> AiVeh_RouteTick, T |
| 0x00401180 | AiVeh_PatrolTick | state 0 patrol: target = link [+0xf80] of head; if AiVeh_ProbeAhead(): AdvanceToLinkedNode, [+0xf8c]=[+0xf84], [+0xf84]=5, [+0x1018]=1, normalize dir (0x004727f0), zero controls. Else steer to node: ahead -> throttle max |
| 0x00401420 | AiVeh_ProbeAhead | if [+0x4dc] or [+0x4e4]: [+0xffc]++, return 1. v=vel +0xa4; L=len(v) (0x00402f60, called twice when >=1.0, else 1.0); probe=pos+v*(L*0.5-[[ECX+8]+4+0x114]); 0x00423b10(2,&{-1,-1}) sweep; return 1 if list counts [+0x4a0]  |
| 0x00401580 | AiVeh_AdvanceToLinkedNode | EDX=&cur; next=cur.link[+0xf80]; [+0xf78]=next; if cur id<0 (temp node): NetNode_Free(cur), cur=next, pick link via NetNode_PickRandomLink(-1) (0 if next id<0), mode [[+0xf74]+0x18]==1 -> [+0xf84]=2; else cur=next, find  |
| 0x004016a0 | NetNode_PickRandomLink | count nonnull links (3 at node+0xc); 1 -> out=0; 2 -> out=0; else out=rand()%n; if out==avoid -> (out+1)%n; returns 1; ret 8 |
| 0x00401710 | AiVeh_CombatTick | if [+0xfd0]<=t and [+0xff0]!=3: AiVeh_PushReturnNode. d=player-self (xz), pitch=dy//d/; fwd=dot(fwd,d), cross; if [+0xffc]>6 -> [+0xff0]=5. switch [+0xff0]: 0 KeepRangeDrive (fwd kept), 1 CircleOrAim, 2 FollowOrAim, 3 0x |
| 0x00401970 | AiVeh_KeepRangeDrive | thiscall(fwd,cross,dist) ret 0xc: fwd<=0 -> throttle 0, steer sign(cross); else steer=cross, throttle 1.0 if dist>[[+0xf74]+0x34], -1.0 if dist<[[+0xf74]+0x30], else 0; copy to +0x7c/+0x80 |
| 0x00401a40 | AiVeh_CircleOrAim | if 0x00401420()==0 -> AiVeh_CircleStrafeTarget; else Vehicle_ComputeAimYawToPoint, zero +0x68/+0x6c/+0x7c/+0x80, [+0xff4]=[+0xff0], [+0xff0]=6; ret 0xc |
| 0x00401ab0 | AiVeh_FollowOrAim | if 0x00401420()==0 -> AiVeh_FollowOffsetTarget(arg3); else aim as 0x00401a40 and [+0xff0]=6; ret 0xc |
| 0x00401b20 | AiVeh_TryDetectPlayer | if [0x004f36ac]==0 and [+0xf90]<t: if 0x00472670() < [+0xfb0] and Collision_TraceWithPlayerPos(1)!=0: 0x00401c60, [+0x448]=1, if [[+0xf74]+0x48] then AiList_ResetNonIdle, [+0xffc]=0, return 1; else [+0x448]=0. return 0 |
| 0x00401c00 | AiList_ResetNonIdle | if [0x004f36ac]==0: for each node in ring (+0xc) with veh[+0xf84]!=1: 0x00401c60, [+0xfdc]=1, [+0xfe4]=[0x0056b428]+10.0 |
| 0x00401c60 | AiVeh_EnterState1 | if [0x004f36ac]==0: prev [+0xf88]=[+0xf84]; [+0xfa8]=t, [+0xfac]=t+[+0xf94]; [+0xf84]=1; [+0xfb8..]=pos of link [+0xf80] of head; if [+0xff0]==2: [+0xfc4]=self-player (y=0), 0x00402f60 |
| 0x00401d50 | Collision_TraceWithPlayerPos | (dir,vec EDX): player=[[0x004f3a88]+4]; Collision_GridDDATraversal between player +0x410 and EDX (endpoint order by dir==1); returns 0 iff traversal==0 and local flag set, else 1 |
| 0x00401e50 | Collision_TraceWithCameraPos | as Collision_TraceWithPlayerPos but endpoint is Camera_GetWorldPosition; (dir,vec EDX); returns 0 iff traversal==0 and local flag set, else 1; ret 4 |
| 0x00401f60 | AiVeh_PushReturnNode | if 0x00472670() >= 400.0: node=malloc(0x30) zeroed at self pos, id=-1, [+0xc]=EDX, edge malloc(0x3c) NetEdge_Init(node, head, 10.0); push as new head [+0xf78], [+0xf80]=0, [+0xfd0]=t+1.0 |
| 0x00402080 | AiVeh_CopyF88ToF84 | [[ECX+4]+0xf84]=[[ECX+4]+0xf88]; ret |
| 0x00402090 | AiVeh_SteerTowardPlayerA | d=player[+0x3ec]-self[+0x3ec] (y=0), 0x00402f60(d); with fwd self+0x380 (x,z at +0/+8): cross=fz*dx-fx*dz; if dot<=0 cross=sign(+-1); +0x6c=cross, +0x68=0, +0x7c=0, +0x80=+0x6c |
| 0x00402170 | AiVeh_SteerTowardPlayerB | byte-identical to 0x00402090 except store order (+0x80 before +0x68/+0x7c); separate call sites |
| 0x00402250 | AiVeh_FireDecision | thiscall(dist,dot) ret 8; w=[self+0x5e4]; burst window: if [+0xfac]<t: [+0xfa8]=t+[+0xf98], [+0xfac]=[+0xfa8]+[+0xf94]. Not-firing: if w[0x10]<t, [+0xfa8]<t, [+0x18]==0, dot>0.75, w[0x12]<dist<w[0x13], Collision_TraceWit |
| 0x004024a0 | AiVeh_LeadTargetIntercept | EDX target; s=1/[[self+0x5e4]+0x24] (projectile speed); solve intercept time from rel pos +0x3ec and rel vel +0xa4 (fast sqrt 0x1fc00000); out=target+0x410 + t*relvel; if no solution out=target pos; out.y += (rand/32767- |
| 0x004026d0 | AiVeh_CircleStrafeTarget | EDX target; r=[[self+0xf74]+0x30]; d=self-target (xz); aim=target+r*rot(d) with c=[0x004da0e4]=0.9659 s=[0x004da0e8]=0.2588 (initial values, 15 deg; .data globals may change); steer toward aim: ahead -> throttle max(1-/c |
| 0x004028c0 | AiVeh_FollowOffsetTarget | thiscall(dist) EDX target; r=[[+0xf74]+0x30], k=[[+0xf74]+0x34]; aim=target+r*dir(+0xfc4) + lateral k*clamp((2r-dist)/r,0,1), negated when [+0xb8]>0 and dist<2r (reverse); steer: ahead or dist>=10.0 -> throttle max(1-/cr |
| 0x00402b70 | AiVeh_RouteTick | q=[ECX+4]; if [q+0xfa4] < global time 0x004f3760: head [q+0xf78]==target [q+0xf7c] -> 0x00402be0; elif head.next(+0xc)==target and head[+0x28]!=-1 -> 0x00402d60; else 0x00401180. Always sets [q+0xfdc]=1 |
| 0x00402be0 | AiVeh_DriveToNextNode | target=[head+0xc] pos; d=target-self+0x3ec (y=0); dist=0x00402f60; if dist<5.0: head=next, zero +0x68/+0x6c/+0x7c/+0x80, +0xfa4=time+4.0; elif behind: turn sign; else throttle=max(1-/cross/,0.25) forward, steer=cross |
| 0x00402d60 | AiVeh_ReverseToNextNode | as 0x00402be0 with forward vector negated, throttle negative, arrival wait time+14.0 |
| 0x00402f10 | AiNet_ResetPlayerVehicles | For each entry of player list [0x004f3a7c]: if vehicle [e+4] type +0x60 == 2 and +0xf84 == 1 calls 0x00402080 on the entry. Sets [0x004f36ac] = 1. |
| 0x00402f60 | FUN_00402f60 | Normalise vector at ECX in place, return length in ST0; exact fsqrt and 1/len (not an approximation); zero guard tests the length bits with 0x7fffffff so -0.0 counts as zero. Recoil18: err 2.1e-6, length 769.650. 79028 R |
| 0x00402fd0 | NetGraph_LoadAllForMission | Calls NetGraph_LoadFromZrdByIndex with ECX = 1..99. |
| 0x00402ff0 | NetGraph_AllocAndAppend | malloc(0x58) zeroed; append to list head 0x004e5c58 tail 0x004e5c5c via +0x54; returns it |
| 0x00403040 | NetGraph_LoadFromZrdByIndex | sprintf net_%02d -> %s.zrd, open via Settings_OpenDetailPresetFile; version must be 0x69 (105) else error ai_net.cpp line 0x8c; mode byte +0x18 0..3; keys path_width->+0x1c (default 10.0), activate_rad->+0x20, attack_rad |
| 0x00403510 | NetGraph_LookupByIndex | Walks net-graph list [0x004e5c58] (next +0x54) returning the graph whose [0] == ECX, else 0. |
| 0x00403530 | NetNode_FindById | EDX list head; walk next +0x2c until [+0x28]==ECX; returns node or 0 |
| 0x00403550 | NetGraph_LinkNodes | for each node (next +0x2c): 3 link ids at +0xc; id<0 -> clear link and edge; else link=NetNode_FindById, edge=malloc(0x3c) zeroed, NetEdge_Init(node pos, link pos, arg) |
| 0x00403620 | NetEdge_Init | thiscall(p0 xyz, p1 xyz, r): e.dir=p1-p0; e[3]=max(r*0.5, /xz/-r); normalize xz; then 0x004745c0, 0x00474f40(45.0f), 0x00474f40(-45.0f); e[0xd]=r; ret 0x1c |
| 0x004036f0 | NetGraph_FindNearestNode | ECX=point, EDX=node list (next +0x2c): returns the node with the smallest distance (0x00472670) from the point, starting from -1.0 = none. |
| 0x00403750 | NetGraph_LinkVehiclesOnSamePath | For every pair of player entries (list [0x004f3a7c]) sharing net path index v+0xf70 where the later vehicle's state v+0x60 != 4 and its group link [3] points to itself: splices it into the earlier entry's circular group  |
| 0x004037c0 | NetNode_Free | free 3 ptrs at +0x18..+0x20 if nonnull, then free(node) |
| 0x00403800 | NetGraph_Free | ECX graph: free each node in list +0x50 (next +0x2c) via 0x004037c0, then free(graph) |
| 0x00403830 | AiVeh_PopNegativeRouteNodes | q=[ECX+4]; while head [q+0xf78] has [+0x28]<0: head=[+0xc], NetNode_Free(old) |
| 0x00403870 | NetGraph_FreeAll | Pops graphs off [0x004e5c58] (next +0x54) freeing each via 0x00403800 until empty. |

## Function index additions (2026-09-24)
- **`Pos_SnapToGround` `0x0042ba50`:** collision ray from 500.0 down, then
  `Terrain_FindGroundHeight(y, 2.0, ...)`.
- **`AIVehicle_OnDeath` `0x0043c0c0`:** creates the death effect `+0xf6c` and stops damage over
  time; if EDX is set it also calls `0x0045e0d0(self)`; then `0x00412620`.

