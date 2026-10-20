// ghidra_GlobRefs.java - dump every data reference from a function to a non-executable
// address in 0x004cc000..0x00800000 as (global, func, kind R/W/A).
// Run through GhidraMCP run_script_inline (paste the body) or the Script Manager; output is
// written to the path below, then copied to 03_re/ledger/global_refs.csv.
// Consumer: 03_re/scripts/gen_globals.py
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.mem.*;
import java.io.*;
public class GlobRefs extends GhidraScript {
  public void run() throws Exception {
    PrintWriter pw=new PrintWriter(new FileWriter(askString("out", "output csv path")));
    pw.println("global,func,kind");
    Memory mem=currentProgram.getMemory();
    FunctionIterator fi=currentProgram.getFunctionManager().getFunctions(true);
    while(fi.hasNext()){ Function f=fi.next();
      InstructionIterator it=currentProgram.getListing().getInstructions(f.getBody(),true);
      while(it.hasNext()){ Instruction i=it.next();
        for(Reference r:i.getReferencesFrom()){ if(!r.getReferenceType().isData()) continue;
          long a=r.getToAddress().getOffset(); if(a<0x4cc000||a>0x800000) continue;
          MemoryBlock b=mem.getBlock(r.getToAddress()); if(b==null||b.isExecute()) continue;
          String k=r.getReferenceType().isWrite()?"W":(r.getReferenceType().isRead()?"R":"A");
          pw.println(String.format("0x%08x",a)+","+String.format("0x%08x",f.getEntryPoint().getOffset())+","+k); } } }
    pw.close();
  }
}
