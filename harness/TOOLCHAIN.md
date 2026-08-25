# Toolchain pins

Verified on Ubuntu 24.04 x64 (2026-08-24). Task 2 of the SkiFree decompile+WASM port plan.

| Tool | Pinned version |
|---|---|
| Ghidra | 12.1.3 (`ghidra_12.1.3_PUBLIC`, build 20260817) — installed at `~/tools/ghidra_12.1.3_PUBLIC`, headless entry point `~/tools/ghidra_12.1.3_PUBLIC/support/analyzeHeadless` |
| Java | OpenJDK 21.0.11 (21.0.11+10-1-24.04.2-Ubuntu) — satisfies Ghidra 12.1.3's Java 21 64-bit requirement |
| emcc (emsdk) | 6.0.6 (clang 24.0.0git, commit ce75e06884093bcefb86a6b8fd56a5d62a4cc245, target wasm32-unknown-emscripten) — emsdk at `~/.emsdk`, `emcc` at `~/.emsdk/upstream/emscripten/emcc` |
| wine | 9.0 (`wine` + `wine32`, both 9.0~repack-4build3; Ubuntu 24.04, i386 multiarch enabled for 32-bit PE support) |
| mingw-w64 | `i686-w64-mingw32-gcc` (GCC) 13-win32 |
| node | v22.23.1 |
| python venv (`harness/.venv`) | Python 3.12.3, Pillow 12.3.0 |
| xvfb | 2:21.1.12-1ubuntu1.6 |
| xdotool | 1:3.20160805.1-5build1 |
| x11-apps | 7.7+11build3 |
| imagemagick | 8:6.9.12.98+dfsg1-5.2build2 |

## Ghidra install notes

- The plan's pinned URL (`ghidra_11.4_public_20250718.zip`) 404s — no such asset exists (the 11.4 release's real asset is `ghidra_11.4_PUBLIC_20250620.zip`). Per the Task 2 fallback rule, the latest release asset was installed instead:
  `ghidra_12.1.3_PUBLIC_20260817.zip` from https://github.com/NationalSecurityAgency/ghidra/releases/latest/
- SHA-256: `93a5d11a9ad510622acaaf908c556a7b9b764d338e78a7567f3689bf5081fd54` (verified against the release notes).
- Release root is `~/tools/ghidra_12.1.3_PUBLIC/`; note `ghidraRun` is a *file* (GUI launcher) — the headless script is `support/analyzeHeadless`, not `ghidraRun/analyzeHeadless`.
- Smoke test: import of `original/ski32.exe` completed cleanly into throwaway project `~/tools/ghidra-project/ski32-smoke`.
- **12.1.3 headless invocation (verified in Task 4):** `$GHIDRA analyzeHeadless <projectDir> <projectName> -import <file> -processor "x86:LE:32:default" [-scriptPath <dir> -postScript <name>.java] [-overwrite]` — no `analyzeHeadless` keyword, no `-postScriptDir` (it is `-scriptPath`, and the script name MUST include the `.java` extension).
- **12.1.3 GhidraScript API (verified in Task 4):** class is `ghidra.app.script.GhidraScript`; results class is `ghidra.app.decompiler.DecompileResults`; the decompile call is `DecompInterface.decompileFunction(f, timeout, getMonitor())`; use `getMonitor()` (FlatProgramAPI) instead of `new TaskMonitor() {}`. Reference working script: `harness/ghidra/DumpDecompiled.java`.
- Processor spec format changed in Ghidra 12.0 to `<arch>:<endianness>:<bitness>:<variant>`: use `-processor "x86:LE:32:default"` for this 32-bit little-endian x86 target (both the old `x86:IA32:default,little` and the 11.x-style `x86:IA32:default:little` are rejected as unsupported languages in 12.1.3). Later Ghidra tasks (Task 4+) must use this form.
