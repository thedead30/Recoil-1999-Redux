// DecompCheck.java - TODO_STAGE2 P0.3 / KG-12: flag functions whose Ghidra decompile is missing code
// that the instruction listing contains.
// For each non-thunk function in [lo, hi): collect the listing's direct call targets and data
// references (strings are keyed by their start address, pointers to imports are skipped), decompile,
// and report calls absent from the decompile's CALL p-code and data refs whose address or symbol name
// does not appear in the C text. Appends CSV rows:
//   address,name,status,listing_calls,missing_calls,listing_refs,missing_refs,missing_call_list,missing_ref_list
// args: lo hi out_csv   (out path must not contain spaces)
// Missing refs prefixed "r" were only ever loaded into a register (MOV/LEA reg, x): usually an
// argument of a call the decompile shows without arguments.
// Post-processing (IAT filter, summary): 03_re/scripts/decomp_check.py
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.pcode.*;
import ghidra.program.model.address.*;
import ghidra.program.model.mem.*;
import java.util.*;
import java.io.*;

public class DecompCheck extends GhidraScript {
  public void run() throws Exception {
    String[] a = getScriptArgs();
    long lo = Long.decode(a[0]), hi = Long.decode(a[1]);
    String out = a[2];
    DecompInterface d = new DecompInterface();
    d.openProgram(currentProgram);
    Memory mem = currentProgram.getMemory();
    PrintWriter pw = new PrintWriter(new FileWriter(out, true));
    int n = 0, flagged = 0;
    FunctionIterator fi = currentProgram.getFunctionManager().getFunctions(toAddr(lo), true);
    while (fi.hasNext()) {
      Function f = fi.next();
      long ea = f.getEntryPoint().getOffset();
      if (ea >= hi) break;
      if (f.isThunk()) continue;
      Set<Long> lc = new TreeSet<>();
      Map<Long, Address> lr = new TreeMap<>();
      Map<Long, Boolean> regOnly = new HashMap<>();
      InstructionIterator it = currentProgram.getListing().getInstructions(f.getBody(), true);
      while (it.hasNext()) {
        Instruction i = it.next();
        for (Reference r : i.getReferencesFrom()) {
          Address t = r.getToAddress();
          long to = t.getOffset();
          if (!t.isMemoryAddress() || to < 0x400000) continue;
          if (r.getReferenceType().isCall()) {
            // direct calls only: resolved indirect (vtable) calls appear in the decompile as calls through a pointer
            if (r.getReferenceType() == ghidra.program.model.symbol.RefType.UNCONDITIONAL_CALL && getFunctionAt(t) != null) lc.add(to);
            continue;
          }
          if (!r.getReferenceType().isData()) continue;
          MemoryBlock b = mem.getBlock(t);
          if (b == null || b.isExecute()) continue;
          Data dt = getDataContaining(t);
          if (dt != null && dt.isPointer() && dt.getValue() instanceof Address
              && ((Address) dt.getValue()).isExternalAddress()) continue;
          Address key = (dt != null && dt.hasStringValue()) ? dt.getAddress() : t;
          lr.put(key.getOffset(), key);
          // register-passed: MOV/LEA of the address into ECX or EDX (fastcall/thiscall args the decompile often hides)
          String m = i.getMnemonicString(), op0 = i.getNumOperands() > 0 ? i.getDefaultOperandRepresentation(0) : "";
          // loaded into a general register (value or address): usually an argument or operand the decompile
          // folds into a call it shows without arguments. Direct memory use (CMP/MOV [x], FLD [x]...) is not.
          boolean reg = (m.equals("MOV") || m.equals("LEA")) && op0.matches("E[ABCD]X|ESI|EDI|EBP|AX|CX|DX|AL|CL|DL|BL");
          regOnly.put(key.getOffset(), regOnly.getOrDefault(key.getOffset(), true) && reg);
        }
      }
      DecompileResults res = d.decompileFunction(f, 30, monitor);
      String status = "OK", c = "";
      Set<Long> dc = new TreeSet<>();
      if (res == null || !res.decompileCompleted()) status = "FAIL";
      else {
        c = res.getDecompiledFunction().getC();
        Iterator<PcodeOpAST> ops = res.getHighFunction().getPcodeOps();
        while (ops.hasNext()) {
          PcodeOpAST op = ops.next();
          if (op.getOpcode() == PcodeOp.CALL) dc.add(op.getInput(0).getAddress().getOffset());
        }
      }
      List<String> mc = new ArrayList<>(), mr = new ArrayList<>();
      if (status.equals("OK")) {
        String cl = c.toLowerCase();
        for (Long x : lc) {
          if (dc.contains(x)) continue;
          String hx = String.format("%08x", x);
          Function g = getFunctionAt(toAddr(x));
          if (cl.contains(hx) || (g != null && c.contains(g.getName()))) continue; // passed as a pointer
          mc.add(hx);
        }
        for (Map.Entry<Long, Address> e : lr.entrySet()) {
          String hx = String.format("%08x", e.getKey());
          if (cl.contains(hx) || cl.contains(hx.substring(2))) continue;
          Symbol s = getSymbolAt(e.getValue());
          if (s != null && c.contains(s.getName())) continue;
          // non-string globals are often rendered base+offset (DAT_x[i], struct fields): accept when an
          // address up to 0x100 below appears (as hex or as its symbol). Strings keep the exact match.
          Data dd = getDataAt(e.getValue());
          boolean isStr = dd != null && dd.hasStringValue();
          if (!isStr) {
            boolean near = false;
            for (long off = -4; off <= 0x100 && !near; off++) {
              if (off == 0) continue;
              long y = e.getKey() - off;   // off < 0: loop bounds rendered as address+1..+4
              if (cl.contains(String.format("%08x", y)) || cl.contains(String.format("%06x", y))) near = true;
              else { Symbol sy = getSymbolAt(toAddr(y)); if (sy != null && c.contains(sy.getName())) near = true; }
            }
            if (near) continue;
          }
          mr.add(regOnly.getOrDefault(e.getKey(), false) ? "r" + hx : hx);
        }
      }
      if (!mc.isEmpty() || !mr.isEmpty() || !status.equals("OK")) flagged++;
      pw.println(String.format("0x%08x", ea) + "," + f.getName() + "," + status + "," + lc.size() + ","
          + mc.size() + "," + lr.size() + "," + mr.size() + "," + String.join(" ", mc) + ","
          + String.join(" ", mr));
      n++;
    }
    pw.close();
    println("done " + n + " flagged " + flagged);
  }
}
