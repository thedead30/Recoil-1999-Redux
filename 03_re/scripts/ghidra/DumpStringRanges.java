// Exports every string / char array Ghidra defined in .rdata and .data to 03_re/ledger/data_string_ranges.csv;
// tools/asm_port/gen_data_image.py never relocates words inside them (Recoil.exe has no relocation table, so
// pointers are recognised by value, and text can read as an image address). Run on Recoil.exe in the Ghidra project.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import java.io.*;

public class DumpStringRanges extends GhidraScript {
    public void run() throws Exception {
        PrintWriter w = new PrintWriter(new FileWriter(new java.io.File(getSourceFile().getParentFile().getParentFile().getParentFile(), "ledger/data_string_ranges.csv")));
        w.println("start,end,type");
        int n = 0;
        for (String bn : new String[] {".rdata", ".data"}) {
            MemoryBlock b = currentProgram.getMemory().getBlock(bn);
            DataIterator it = currentProgram.getListing().getDefinedData(new ghidra.program.model.address.AddressSet(b.getStart(), b.getEnd()), true);
            while (it.hasNext()) {
                Data d = it.next();
                DataType t = d.getBaseDataType();
                boolean str = d.hasStringValue();
                if (!str && t instanceof Array) str = ((Array) t).getDataType() instanceof CharDataType;
                if (!str) continue;
                w.println(String.format("0x%08x,0x%08x,%s", d.getAddress().getOffset(), d.getMaxAddress().getOffset() + 1, t.getName().replace(",", ";")));
                n++;
            }
        }
        w.close();
        println("string ranges: " + n);
    }
}
