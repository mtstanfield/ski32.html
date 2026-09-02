#!/usr/bin/env python3
"""Launch a wine PE, poll /proc/<pid>/mem continuously, print the richest
snapshot seen (IAT slots + stub page when HOME is set). The ski32_inj child
dies in ~300ms, so we sample every 10ms until it's gone.
Usage: iat_snap.py <exe> [timeout_s]
"""
import os, struct, subprocess, sys, time

EXE = sys.argv[1]
TIMEOUT = float(sys.argv[2]) if len(sys.argv) > 2 else 3.0

SLOTS = {
    'i_DeleteObject': 0x40a000, 'i_SelectObject': 0x40a004, 'i_BitBlt': 0x40a00c,
    'i_DeleteDC': 0x40a028, 'i_CreateCompatibleDC': 0x40a034,
    'i_LoadLibraryA': 0x40a068, 'i_GetProcAddress': 0x40a074,
    'i_VirtualAlloc': 0x40a080,
    'i_GetTickCount': 0x40a0c0, 'i_LocalAlloc': 0x40a0c4,
    'i_GetModuleHandleA': 0x40a0c8,
    'i_wsprintfA': 0x40a108, 'i_GetClientRect': 0x40a14c,
    'i_SetWindowTextA': 0x40a164, 'i_GetDC': 0x40a16c, 'i_ReleaseDC': 0x40a170,
}
HOME1, HOME2 = 0x40c284, 0x40c288

def find_child():
    best = 0
    for pid in os.listdir('/proc'):
        if not pid.isdigit():
            continue
        try:
            with open(f'/proc/{pid}/comm') as f:
                if 'ski32' in f.read() and int(pid) > best:
                    best = int(pid)
        except OSError:
            pass
    return best or None

def read_mem(pid):
    """Return dict of values, or None if unreadable (zombie/gone)."""
    try:
        mem = open(f'/proc/{pid}/mem', 'rb')
    except (PermissionError, FileNotFoundError):
        return None
    out = {}
    for name, va in SLOTS.items():
        try:
            mem.seek(va)
            out[name] = struct.unpack('<I', mem.read(4))[0]
        except OSError:
            out[name] = None
    try:
        mem.seek(HOME1)
        h1, h2 = struct.unpack('<II', mem.read(8))
        out['HOME'] = (h1, h2)
        if h1 and h1 == h2:
            for name, off in [('magic', 0), ('dibsec', 0x214), ('localfree', 0x218),
                              ('hdl_msvcrt', 0x220), ('hdl_gdi', 0x224),
                              ('hdl_k32', 0x228), ('o_file', 0x600), ('o_tick', 0x604),
                              ('o_cnt', 0x60c), ('o_stage', 0x610)]:
                try:
                    mem.seek(h1 + off)
                    out[name] = struct.unpack('<I', mem.read(4))[0]
                except OSError:
                    out[name] = None
    except OSError:
        out['HOME'] = None
    # main-thread stack (ski32 main stack sits around 0x51f000-0x520000)
    try:
        mem.seek(0x51c000)
        out['stack'] = mem.read(0x4000)
    except OSError:
        out['stack'] = b''
    mem.close()
    return out

def main():
    env = dict(os.environ, DISPLAY=':99', WINEPREFIX=os.path.expanduser('~/.wine-ski'),
               WINEDEBUG='-all')
    p = subprocess.Popen(['wine', EXE], cwd=os.getcwd(), env=env,
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    t0 = time.time()
    best = None       # richest snapshot (prefers HOME set)
    last = None
    n = 0
    while time.time() - t0 < TIMEOUT and p.poll() is None:
        pid = find_child()
        if pid:
            snap = read_mem(pid)
            if snap:
                n += 1
                last = snap
                if (snap.get('HOME') and not (best and best.get('HOME'))) or best is None:
                    best = snap
        time.sleep(0.01)
    print(f'snapshots={n} rc={p.poll()} t={time.time()-t0:.2f}s')
    for label, snap in [('BEST', best), ('LAST', last)]:
        if not snap:
            print(f'--- {label}: none ---')
            continue
        if snap.get('stack'):
            with open(f'/tmp/ski_stack_{label}.bin', 'wb') as f:
                f.write(snap['stack'])
            snap['stack'] = None
        print(f'--- {label} ---')
        for k, v in snap.items():
            if isinstance(v, tuple):
                print(f'  {k} = {v[0]:#x} {v[1]:#x}')
            elif v is None:
                print(f'  {k} = <unread>')
            else:
                print(f'  {k} = {v:#x}')

if __name__ == '__main__':
    main()
