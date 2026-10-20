// DumpIndirectEdges.java - recover the call edges Ghidra's static graph misses.
//
// WHY THIS EXISTS: 504 of Recoil.exe's 3,114 engine functions have no static caller
// because the engine is C++ with vtable dispatch plus init-time function-pointer
// registration. A forward walk over direct edges alone under-counts the map-1
// reachable set, which would open the Stage 2 gate with whole subsystems unread
// (Vehicle_MainUpdateTick and Weapon_ProjectileUpdateTick are both orphans).
//
// For each orphan it classifies every reference:
//   INSTALLER  - the ref sits inside a function, which therefore installs the pointer
//                (e.g. 0x426390 <- 0x420061 in Player_InitPhysicsGlobalsAndVehicleClasses)
//   VTABLE     - the ref sits in data. Walk back to the vtable base (a contiguous run
//                of pointers into code), then report who references that base - the
//                constructor. That constructor is the real proxy caller.
//   THUNK      - an UNCONDITIONAL_CALL from an immediately preceding address.
//
// Output: 03_re/ledger/_indirect_edges.tsv
//   orphan  kind  via_address  attributed_function
//
// Ambiguity is emitted, never resolved silently: an orphan whose vtable base has many
// referencing functions produces one row per candidate, and the consumer treats them
// all as possible callers. Over-inclusion is the safe error for a gate.

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class DumpIndirectEdges extends GhidraScript {

    private boolean pointsToCode(Address a) {
        try {
            if (a == null) return false;
            long v = currentProgram.getMemory().getInt(a) & 0xFFFFFFFFL;
            if (v < 0x00401000L || v > 0x004d0000L) return false;
            Address t = toAddr(v);
            return currentProgram.getFunctionManager().getFunctionAt(t) != null;
        } catch (Exception e) { return false; }
    }

    // Walk backwards in 4-byte steps while each slot still holds a code pointer.
    private Address vtableBase(Address slot) {
        Address cur = slot, base = slot;
        for (int i = 0; i < 512; i++) {
            Address prev;
            try { prev = cur.subtract(4); } catch (Exception e) { break; }
            if (!pointsToCode(prev)) break;
            base = prev; cur = prev;
        }
        return base;
    }

    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        ReferenceManager rm = currentProgram.getReferenceManager();

        File out = new File(getScriptArgs().length > 0 ? getScriptArgs()[0]
            : new java.io.File(getSourceFile().getParentFile().getParentFile().getParentFile(), "ledger/_indirect_edges.tsv").getPath());
        PrintWriter pw = new PrintWriter(new FileWriter(out));
        pw.println("orphan\tkind\tvia_address\tattributed_function");

        // Collect orphans: engine functions with no calling function.
        List<Function> orphans = new ArrayList<>();
        for (Function f : fm.getFunctions(true)) {
            if (f.isThunk() || f.isExternal()) continue;
            if (f.getCallingFunctions(monitor).isEmpty()) orphans.add(f);
        }
        println("orphans: " + orphans.size());

        int rows = 0;
        for (Function f : orphans) {
            Address fa = f.getEntryPoint();
            ReferenceIterator it = rm.getReferencesTo(fa);
            boolean any = false;
            while (it.hasNext()) {
                Reference r = it.next();
                Address from = r.getFromAddress();
                any = true;
                Function host = fm.getFunctionContaining(from);
                if (host != null) {
                    String kind = r.getReferenceType().isCall() ? "THUNK" : "INSTALLER";
                    pw.println(fa + "\t" + kind + "\t" + from + "\t" + host.getName());
                    rows++;
                } else {
                    // data slot - resolve the vtable and report its referrers
                    Address base = vtableBase(from);
                    ReferenceIterator vit = rm.getReferencesTo(base);
                    boolean found = false;
                    while (vit.hasNext()) {
                        Function ctor = fm.getFunctionContaining(vit.next().getFromAddress());
                        if (ctor != null) {
                            pw.println(fa + "\tVTABLE\t" + base + "\t" + ctor.getName());
                            rows++; found = true;
                        }
                    }
                    if (!found) {
                        pw.println(fa + "\tVTABLE_UNRESOLVED\t" + base + "\t-");
                        rows++;
                    }
                }
            }
            if (!any) { pw.println(fa + "\tNO_REFS\t-\t-"); rows++; }
        }
        pw.close();
        println("wrote " + rows + " rows to " + out.getAbsolutePath());
    }
}
