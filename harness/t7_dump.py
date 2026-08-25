#!/usr/bin/env python3
"""Task 7 evidence dump: raw const tables + WAVE resource inventory from original/ski32.exe."""
import struct, sys

PE = "original/ski32.exe"
data = open(PE, "rb").read()

def r16(off): return struct.unpack_from("<H", data, off)[0]
def r32(off): return struct.unpack_from("<I", data, off)[0]

# .rdata VMA 0x40a000 -> file 0x0000a000
f_rdata = 0x0000a000
def at(va): return f_rdata + (va - 0x40a000)

print("=== frame_col_table (a1ac, 64 x u16: frame -> column) ===")
tbl = [r16(at(0x40a1ac) + 2 * i) for i in range(64)]
for i in range(0, 64, 8):
    print(f"  frame {i:2d}..{i+7:2d}: " + " ".join(f"{v:3d}" for v in tbl[i:i+8]))

print("=== spawn_frame_table (a22c, 11 x i32: type -> default frame) ===")
sft = [struct.unpack_from("<i", data, at(0x40a22c) + 4 * i)[0] for i in range(11)]
print("  " + " ".join(f"{v}" for v in sft))

print("=== key_action_map1 (a258, 5 x i32) / map2 (a25c, 5 x i32) ===")
m1 = [struct.unpack_from("<i", data, at(0x40a258) + 4 * i)[0] for i in range(5)]
m2 = [struct.unpack_from("<i", data, at(0x40a25c) + 4 * i)[0] for i in range(5)]
print("  map1 (left/0x25/0x64):", m1)
print("  map2 (right/0x27/0x66):", m2)

print("=== activate_sound_table (a308, 10 x (u16 flag, u16 param)) ===")
ast = [(r16(at(0x40a308) + 4 * i), r16(at(0x40a308) + 4 * i + 2)) for i in range(10)]
print("  " + " ".join(f"[{f},{p}]" for f, p in ast))

print("=== activate_anim_table (a434, mixed dwords x N) ===")
a434 = [r32(at(0x40a434) + 4 * i) for i in range(10)]
print("  " + " ".join(f"{v:#x}" for v in a434))

# ---- WAVE resources: parse .rsrc ----
print("=== WAVE resources in .rsrc ===")
pe_off = struct.unpack_from("<I", data, 0x3c)[0]
opt_off = pe_off + 24
num_sections = struct.unpack_from("<H", data, opt_off + 2)[0]
sec_off = opt_off + 224
sections = []
for i in range(num_sections):
    o = sec_off + 40 * i
    name = data[o:o+8].rstrip(b"\x00").decode()
    vsize, vaddr, rsize, raddr = struct.unpack_from("<IIII", data, o + 8)
    sections.append((name, vsize, vaddr, rsize, raddr))
rsrc = [s for s in sections if s[0] == ".rsrc"][0]
rsrc_va, rsrc_off = rsrc[2], rsrc[4]

def rsrc_at(rva): return rsrc_off + (rva - rsrc_va)

def read_res_dir(rva, depth=0, path=""):
    off = rsrc_at(rva)
    num_named, num_id = struct.unpack_from("<HH", data, off + 4)
    entries = []
    for i in range(num_named + num_id):
        e = off + 16 + 8 * i
        name_rva_or_id, data_rva = struct.unpack_from("<II", data, e)
        if name_rva_or_id & 0x80000000:
            name_off = rsrc_at(name_rva_or_id & 0x7fffffff)
            nlen = struct.unpack_from("<H", data, name_off)[0]
            name = data[name_off + 2:name_off + 2 + nlen * 2].decode("utf-16le")
        else:
            name = str(name_rva_or_id)
        if data_rva & 0x80000000:
            entries.append((name, read_res_dir(data_rva & 0x7fffffff, depth + 1, path + "/" + name)))
        else:
            entries.append((name, data_rva))
    return entries

top = read_res_dir(rsrc_va)
def walk(entries, prefix):
    for name, val in entries:
        if isinstance(val, list):
            walk(val, prefix + "/" + name)
        else:
            yield prefix + "/" + name, val

for typ, entries in top:
    if isinstance(entries, list) and any(n == "WAVE" for n, _ in entries):
        for name, rva in entries:
            if name != "WAVE":
                continue
            for leaf, leaf_rva in walk([("x", read_res_dir(rva))], "WAVE"):
                off = rsrc_at(leaf_rva)
                size, code = struct.unpack_from("<II", data, off)
                print(f"  WAVE id={leaf:<4} rva={leaf_rva:#x} size={size} ({code:#x} compressed?)" )
                if code == 0:
                    p = rsrc_at(off + 8)
                    # RIFF header
                    riff = data[p:p+4]
                    fsize = struct.unpack_from("<I", data, p + 4)[0]
                    wave = data[p + 8:p + 12]
                    fmt = data[p + 16:p + 20]
                    if wave == b"WAVE" and fmt == b"fmt ":
                        ch, sr, br, bps, bits = struct.unpack_from("<HHIIH", data, p + 24)
                        # data chunk
                        q = p + 40
                        dlen = 0
                        while q + 8 <= p + fsize and data[q:q+4] != b"data":
                            dl = struct.unpack_from("<I", data, q + 4)[0]
                            q += 8 + dl
                        dlen = struct.unpack_from("<I", data, q + 4)[0]
                        dur = dlen / (sr * ch * bits // 8) if sr else 0
                        print(f"      RIFF ok: {ch}ch {sr}Hz {bits}bit {br}B/s data={dlen}B dur={dur:.2f}s")
                    else:
                        print(f"      header: {riff} {wave} {fmt} (not a standard RIFF/WAVE)")
                else:
                    print(f"      code={code:#x} (RT_RCDATA-like storage)")

# .data sanity: c788 buffer + c790/c794 area
f_data = 0x0000c000
def atd(va): return f_data + (va - 0x40c000)
print("=== .data sanity ===")
print("  c080..c088:", data[atd(0x40c080):atd(0x40c088)].split(b"\x00"))
print("  c0fc..c100 (nosound):", data[atd(0x40c0fc):atd(0x40c104)])
print("  c788 (status init text, 64B):", data[atd(0x40c788):atd(0x40c7c8)])
