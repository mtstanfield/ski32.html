// Ghidra headless postScript: decompile every function, dump C + function list.
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.address.Address;
import ghidra.util.task.TaskMonitor;
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;

public class DumpDecompiled extends ghidra.app.script.GhidraScript {
    @Override
    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        DecompInterface dec = new DecompInterface();
        dec.openProgram(currentProgram);
        TaskMonitor monitor = getMonitor();
        String outDir = System.getenv("GH_DUMP_DIR");
        if (outDir == null) outDir = "decompile/ghidra";
        File dir = new File(outDir);
        dir.mkdirs();
        PrintWriter json = new PrintWriter(new FileWriter(dir + "/function-map.json"));
        json.print("[\n");
        boolean first = true;
        for (Function f : fm.getFunctions(true)) {
            Address entry = f.getEntryPoint();
            String tag = String.format("%08x", entry.getOffset());
            DecompileResults res = dec.decompileFunction(f, 60, monitor);
            String c = (res != null && res.decompileCompleted())
                    ? res.getDecompiledFunction().getC() : "// DECOMPILE FAILED";
            PrintWriter w = new PrintWriter(new FileWriter(dir + "/FUN_" + tag + ".c"));
            w.println("/* " + f.getName() + " @ 0x" + entry +
                      " size=" + f.getBody().getNumAddresses() + " */");
            w.print(c);
            w.close();
            if (!first) json.print(",\n");
            first = false;
            json.printf("{\"name\":\"%s\",\"addr\":%d,\"size\":%d,\"file\":\"FUN_%s.c\"}",
                    f.getName().replace("\"", "'"), entry.getOffset(),
                    f.getBody().getNumAddresses(), tag);
            println("decompiled " + f.getName() + " @0x" + entry);
        }
        json.print("\n]\n");
        json.close();
        dec.dispose();
        println("DONE: " + dir + "/function-map.json");
    }
}
