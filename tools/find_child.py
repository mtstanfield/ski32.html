#!/usr/bin/env python3
"""Find the 32-bit ski32 child process (not the 64-bit wine wrapper).

A child is identified by having the PE image mapped at 0x400000 in /proc/PID/maps.
Prints the PID, or nothing.
"""
import os, sys

def is_child(pid):
    try:
        with open(f"/proc/{pid}/maps") as f:
            for line in f:
                start = int(line.split("-")[0], 16)
                if start == 0x400000:
                    return True
    except (OSError, ValueError):
        return False
    return False

def main():
    pats = sys.argv[1:] or ["ski32"]
    found = []
    for pid in os.listdir("/proc"):
        if not pid.isdigit():
            continue
        try:
            with open(f"/proc/{pid}/comm") as f:
                comm = f.read().strip()
        except OSError:
            continue
        if not any(p in comm for p in pats):
            continue
        if is_child(pid):
            found.append(int(pid))
    if found:
        print(max(found))

if __name__ == "__main__":
    main()
