// Ghidra headless postScript: create functions at explicit addresses,
// decompile each, and write decompile/ghidra/FUN_<addr>.c
// in the same format as DumpDecompiled.java.
//
// Env vars:
//   ADD_FN_ADDRS  whitespace-separated addresses (0x-prefixed hex or decimal)
//   GH_DUMP_DIR   output directory (default "decompile/ghidra")
//
// Functions that already exist at the given addresses are skipped (no overwrite).
// The project must already be fully analyzed (it is: Task 4); Ghidra 12.1.3
// exposes no script-friendly local-analysis API, so no re-analysis is
// performed — createFunction operates on the existing disassembly.
//
// Body computation: [addr, nextFnEntry - 1] with trailing 0x90 nop padding
// trimmed off. Requires the address to be in a gap between existing functions
// (all four current targets are).
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSet;
import ghidra.program.model.symbol.SourceType;
import ghidra.program.model.symbol.Symbol;
import ghidra.util.task.TaskMonitor;
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.StringTokenizer;

public class AddFunctions extends ghidra.app.script.GhidraScript {
    @Override
    public void run() throws Exception {
        String spec = System.getenv("ADD_FN_ADDRS");
        if (spec == null || spec.trim().isEmpty()) {
            println("ERROR: ADD_FN_ADDRS env var is not set");
            return;
        }
        FunctionManager fm = currentProgram.getFunctionManager();
        DecompInterface dec = new DecompInterface();
        dec.openProgram(currentProgram);
        TaskMonitor monitor = getMonitor();
        String outDir = System.getenv("GH_DUMP_DIR");
        if (outDir == null) outDir = "decompile/ghidra";
        new File(outDir).mkdirs();
        StringTokenizer st = new StringTokenizer(spec);
        while (st.hasMoreTokens()) {
            String tok = st.nextToken().trim();
            long val = Long.decode(tok);
            Address a = currentProgram.getAddressFactory()
                    .getDefaultAddressSpace().getAddress(val);
            if (fm.getFunctionAt(a) != null) {
                println("skip (already a function): 0x" + Long.toHexString(val));
                continue;
            }
            // natural end: entry of the next existing function after a
            Address next = null;
            for (Function g : fm.getFunctions(true)) {
                Address e = g.getEntryPoint();
                if (e.compareTo(a) > 0 && (next == null || e.compareTo(next) < 0)) {
                    next = e;
                }
            }
            if (next == null) {
                println("skip (no function boundary after): 0x" + Long.toHexString(val));
                continue;
            }
            Address end = next;
            while (end.subtract(1).compareTo(a) >= 0
                    && currentProgram.getMemory().getByte(end.subtract(1)) == (byte) 0x90) {
                end = end.subtract(1);
            }
            end = end.subtract(1);
            if (end.compareTo(a) < 0) end = a;
            Symbol sym = currentProgram.getSymbolTable().getSymbol(a.getOffset());
            String name = (sym != null && !sym.getName().isEmpty())
                    ? sym.getName() : String.format("FUN_%08x", a.getOffset());
            AddressSet body = new AddressSet();
            body.add(a, end);
            Function f = fm.createFunction(name, a, body, SourceType.USER_DEFINED);
            DecompileResults res = dec.decompileFunction(f, 60, monitor);
            String c = (res != null && res.decompileCompleted())
                    ? res.getDecompiledFunction().getC() : "// DECOMPILE FAILED";
            String tag = String.format("%08x", a.getOffset());
            PrintWriter w = new PrintWriter(new FileWriter(outDir + "/FUN_" + tag + ".c"));
            w.println("/* " + f.getName() + " @ 0x" + a +
                      " size=" + f.getBody().getNumAddresses() + " */");
            w.print(c);
            w.close();
            println("created+decompiled " + f.getName() + " @0x" + a +
                    " size=" + f.getBody().getNumAddresses() +
                    " -> FUN_" + tag + ".c");
        }
        dec.dispose();
        println("DONE");
    }
}
