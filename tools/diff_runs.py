#!/usr/bin/env python3
"""Differential compare: original (/tmp/orig_r1.log) vs rebuild
(/tmp/rebuild_state) descent streams.

Alignment: first fr=1 sample on each side (descent tick 1 end-state).
Then compare tick-by-tick: summary fields + entity fingerprints
(col, type, fr, x, y, mode, steer, speed, fl, rect[4]).
"""
import sys, re

def parse(path):
    lines = open(path).read().splitlines()
    samples = []
    cur = None
    for l in lines:
        if l.startswith('py='):
            if cur:
                samples.append(cur)
            cur = [l, []]
        elif l.startswith('  #') and cur is not None:
            cur[1].append(l)
    if cur:
        samples.append(cur)
    return samples

def summary(s):
    d = {}
    for kv in s.split()[1:]:
        k, v = kv.split('=', 1)
        d[k] = v
    return d

def ent_fp(line):
    d = {}
    for p in line.split():
        if '=' in p:
            k, v = p.split('=', 1)
            d[k] = v
    return (d.get('col'), d.get('type'), d.get('fr'), d.get('x'),
            d.get('y'), d.get('mode'), d.get('steer'), d.get('speed'),
            d.get('fl'), d.get('rect'))

def first_r(samples, target):
    for i, (s, e) in enumerate(samples):
        if summary(s).get('r') == target:
            return i
    return -1

def first_descent(samples):
    for i, (s, e) in enumerate(samples):
        if summary(s).get('fr') == '1':
            return i
    return -1

# Alignment: first fr=1 sample on each side. Both are the same state
# role: post-keydown, pre-first-descent-tick, r=0x69780dfd, sp/st/py=0
# (the harness hook fires the rebuild's KP_1 at the exact original
# boundary: between menu tick 467 and descent tick 1). The sample
# BEFORE it on each side (A_466, r=0x12e83c69, fr=3) must match fully.
orig = parse('/tmp/orig_r1.log')
reb = parse('/tmp/rebuild_state')
oi = first_descent(orig)
ri = first_descent(reb)
print(f"original: {len(orig)} samples, first fr=1 @ {oi}: {orig[oi][0]}")
print(f"rebuild : {len(reb)} samples, first fr=1 @ {ri}: {reb[ri][0]}")

# pre-boundary check: last fr=3 on each side (A_466) — full match expected
if oi > 0 and ri > 0:
    so, sr = summary(orig[oi-1][0]), summary(reb[ri-1][0])
    bdiffs = [f"{f}: orig={so.get(f)} reb={sr.get(f)}"
              for f in ('py', 'r', 'px', 'st', 'sp', 'md', 'fl', 'fr', 'n', 'cy', 'cx')
              if so.get(f) != sr.get(f)]
    eodiff = ([ent_fp(x) for x in orig[oi-1][1]] != [ent_fp(x) for x in reb[ri-1][1]])
    print(f"pre-boundary A_466 (orig @{oi-1} vs reb @{ri-1}): "
          + ("FULL MATCH" if not bdiffs and not eodiff
             else f"DIFFS: {bdiffs} ents={'DIFF' if eodiff else 'match'}"))

N = int(sys.argv[1]) if len(sys.argv) > 1 else 500
fields = ['py', 't', 'r', 'px', 'st', 'sp', 'md', 'fl', 'fr', 'n', 'cy', 'cx']
mism = 0
first_mism = None
for k in range(N):
    o = orig[oi + k]
    r = reb[ri + k]
    so, sr = summary(o[0]), summary(r[0])
    diffs = []
    for f in fields:
        if f == 't':
            continue  # wall-clock, not comparable
        if so.get(f) != sr.get(f):
            diffs.append(f"{f}: orig={so.get(f)} reb={sr.get(f)}")
    eo, er = [ent_fp(x) for x in o[1]], [ent_fp(x) for x in r[1]]
    if eo != er:
        nd = []
        if len(eo) != len(er):
            nd.append(f"count {len(eo)} vs {len(er)}")
        for j in range(min(len(eo), len(er))):
            if eo[j] != er[j]:
                nd.append(f"e{j}: orig={eo[j]} reb={er[j]}")
        diffs.append("ents: " + "; ".join(nd[:4]))
    if diffs:
        mism += 1
        if first_mism is None:
            first_mism = k
        if mism <= 12:
            print(f"tick+{k}: " + " | ".join(diffs[:6]))
            print(f"   orig: {o[0]}")
            print(f"   reb : {r[0]}")
print(f"\n{mism}/{N} ticks differ; first mismatch at descent-tick+{first_mism}")

# where does each side stop matching r (pure RNG parity)?
kr = 0
for k in range(N):
    if summary(orig[oi+k][0]).get('r') != summary(reb[ri+k][0]).get('r'):
        kr = k
        break
else:
    kr = N
print(f"r (c16c) parity holds through descent-tick+{kr-1}" + ("" if kr == N else f", breaks at +{kr}"))
