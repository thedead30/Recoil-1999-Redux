// Writes the instruction listings of the functions named in the script argument (comma-separated addresses; output
// file after a '>') in the format tools/asm_port reads: "### <addr> <name>" then "<addr> <instruction>" per line.
// Example argument: 0044eed0,00475fa0>C:/path/03_re/listings/out.txt
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import java.io.*;

public class DumpListings extends GhidraScript {
    public void run() throws Exception {
        String arg = getScriptArgs().length > 0 ? getScriptArgs()[0] : "";
        String[] parts = arg.split(">");
        PrintWriter w = new PrintWriter(new FileWriter(parts[1]));
        for (String a : parts[0].split(",")) {
            Function f = getFunctionAt(toAddr(Long.parseLong(a.trim(), 16)));
            if (f == null) { println("no function at " + a); continue; }
            w.println(String.format("### %08x %s", f.getEntryPoint().getOffset(), f.getName()));
            InstructionIterator it = currentProgram.getListing().getInstructions(f.getBody(), true);
            while (it.hasNext()) {
                Instruction i = it.next();
                w.println(String.format("%x %s", i.getAddress().getOffset(), i.toString()));
            }
            w.println();
        }
        w.close();
        println("listings written: " + parts[1]);
    }
}
