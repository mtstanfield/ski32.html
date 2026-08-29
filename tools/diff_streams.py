#!/usr/bin/env python3
"""Aligned tick-by-tick diff of the original (/tmp/orig_r2.log) and
rebuild (/tmp/rebuild_state) differential streams.

Alignment: first fr=1 sample on each side (descent start). Then compare
tick-by-tick: summary line (py, r, px, st, sp, md, fl, fr, n, cy, cx) and
in-list entity fingerprints (col, type, fr, x, y, mode, steer, speed, fl,
rect). t= (wall clock), pointer fields (addr/gn/pt/desc) and the
observation-phase 0x04 bit are excluded (see NOTES T11-residual section:
/proc poller snapshots at a variable sub-tick phase; 1-sample 0x04 skew
self-converges; c16c may sit 1-9 LCG steps ahead on the same orbit).

Exit 0 = parity through the end of the shorter stream.
"""
import re, sys

def parse(path):
    samples, cur = [], None
    for l in open(path).read().splitlines():
        if l.startswith('py='):
            if cur: samples.append(cur)
            cur = {'s': l, 'e': []}
        elif l.startswith('  #') and cur is not None:
            cur['e'].append(l)
    if cur: samples.append(cur)
    return samples

def kv(line):
    return {k: v for p in line.split()[1:] if '=' in p
            for k, v in [p.split('=', 1)]}

def fp(e):
    d = kv(e)
    return (d.get('col'), d.get('type'), d.get('fr'), d.get('x'),
            d.get('y'), d.get('mode'), d.get('steer'), d.get('speed'),
            d.get('rect'))

SUM_KEYS = ('py', 'r', 'px', 'st', 'sp', 'md', 'fl', 'fr', 'n', 'cy', 'cx')

def main():
    o = parse(sys.argv[1]); r = parse(sys.argv[2])
    def first_fr1(samples):
        for i, s in enumerate(samples):
            if kv(s['s']).get('fr') == '1':
                return i
        return None
    io, ir = first_fr1(o), first_fr1(r)
    if io is None or ir is None:
        print(f"no fr=1 found (orig={io}, rebuild={ir})"); return 2
    print(f"align: orig sample {io}, rebuild sample {ir}")
    rmatch = c16c_eq = 0
    first_diff = None
    for i in range(min(len(o) - io, len(r) - ir)):
        so, sr = o[io + i], r[ir + i]
        ko, kr = kv(so['s']), kv(sr['s'])
        sdiff = {k: (ko[k], kr[k]) for k in SUM_KEYS if ko.get(k) != kr.get(k)}
        eoff = []
        no, nr = so['e'], sr['e']
        for j in range(max(len(no), len(nr))):
            if j < len(no) and j < len(nr):
                fo, fr_ = fp(no[j]), fp(nr[j])
                if fo != fr_:
                    eoff.append((j, no[j].strip(), nr[j].strip()))
            else:
                eoff.append((j, no[j].strip() if j < len(no) else '<none>',
                             nr[j].strip() if j < len(nr) else '<none>'))
        if i < 400 and (ko.get('r') == kr.get('r')):
            c16c_eq += 1
        if not sdiff and not eoff:
            rmatch += 1
            continue
        if first_diff is None:
            first_diff = (i, sdiff, eoff[:4], so['s'].strip(), sr['s'].strip())
            print(f"\nFIRST DIFF at descent tick +{i}:")
            for k, (a, b) in sdiff.items():
                print(f"  {k}: orig={a} rebuild={b}")
            for j, a, b in eoff[:4]:
                print(f"  ent[{j}] orig:    {a}")
                print(f"  ent[{j}] rebuild: {b}")
    print(f"\nparity: {rmatch} identical ticks; c16c equal (i<400): {c16c_eq}")
    lo = len(o) - io; lr = len(r) - ir
    print(f"stream lengths after align: orig={lo} rebuild={lr}")
    if first_diff is None:
        print("RESULT: FULL PARITY through end of shorter stream"); return 0
    print("RESULT: divergence (see above)"); return 1

if __name__ == '__main__':
    sys.exit(main())
