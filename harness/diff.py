#!/usr/bin/env python3
"""Frame-stream pixel diff for the SkiFree rebuild-vs-original verification.

Usage: diff.py ORIG_DIR REBUILD_DIR [options]

Frames: frame_%06d_main.ppm (P6 top-down 24bpp), index = game tick (first
dumped tick = 1). Both streams must be tick-aligned (tick-boundary dumps;
no wall-clock resync needed).

Scene region = full client frame (760x734) minus the status-panel mask.
Default mask 620,0,760,60 covers the Time/Dist/Speed/Style panel (top-right),
where the Speed digit legitimately jitters under Wine (commit b6facf5) and
the Time/Dist/Style values are display-only. The 0-px requirement applies to
the scene region.

Options:
  --mask X0,Y0,X1,Y1   panel mask rect (x1,y1 exclusive); default 620,0,760,60
  --tol N              allowed differing scene pixels per frame (default 0)
  --min-tick T / --max-tick T   compare window (default: full overlap)
  --shift S            compare rebuild tick t vs original tick t+S (default 0)
  --viz DIR            for each failing frame, write frame_%06d_diff.png
                       (rebuild frame with differing scene pixels in red)
  --report FILE        append a one-line summary for evidence/ records
Exit code: 0 = all frames within tolerance, 1 = at least one failure,
2 = usage/stream error.
"""
import argparse
import pathlib
import sys

import numpy as np


def load_ppm(path: str) -> np.ndarray:
    """P6 top-down 24bpp -> HxWx3 uint8."""
    with open(path, "rb") as f:
        data = f.read()
    # header: P6\nW H\n255\n  (whitespace-separated, no comments produced)
    if data[:2] != b"P6":
        raise ValueError(f"{path}: not P6")
    parts = []
    i = 2
    while len(parts) < 3:
        while i < len(data) and data[i:i + 1].isspace():
            i += 1
        if data[i:i + 1] == b"#":
            while data[i:i + 1] not in (b"\n", b""):
                i += 1
            continue
        j = i
        while not data[j:j + 1].isspace():
            j += 1
        parts.append(int(data[i:j]))
        i = j
    w, h, maxv = parts
    if maxv != 255:
        raise ValueError(f"{path}: maxval {maxv}")
    i += 1  # the single WS separating maxval from the raster
    px = data[i:i + w * h * 3]
    if len(px) != w * h * 3:
        raise ValueError(f"{path}: short pixel data {len(px)} != {w*h*3}")
    return np.frombuffer(px, dtype=np.uint8).reshape(h, w, 3).copy()


def frame_path(d: str, tick: int):
    return pathlib.Path(d) / f"frame_{tick:06d}_main.ppm"


def list_frames(d: str):
    p = pathlib.Path(d)
    if not p.is_dir():
        return []
    ticks = []
    for f in p.iterdir():
        if f.name.startswith("frame_") and f.name.endswith("_main.ppm"):
            try:
                ticks.append(int(f.name[6:12]))
            except ValueError:
                pass
    return sorted(ticks)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("orig")
    ap.add_argument("rebuild")
    ap.add_argument("--mask", default="620,0,760,60")
    ap.add_argument("--tol", type=int, default=0)
    ap.add_argument("--min-tick", type=int, default=None)
    ap.add_argument("--max-tick", type=int, default=None)
    ap.add_argument("--shift", type=int, default=0)
    ap.add_argument("--viz", default=None)
    ap.add_argument("--report", default=None)
    a = ap.parse_args()

    x0, y0, x1, y1 = (int(v) for v in a.mask.split(","))

    oticks = list_frames(a.orig)
    rticks = list_frames(a.rebuild)
    if not oticks or not rticks:
        print(f"error: no frames in {a.orig!r} (orig) or {a.rebuild!r} (rebuild)",
              file=sys.stderr)
        return 2
    # rebuild tick t  <->  original tick t + shift
    lo = a.min_tick if a.min_tick is not None else max(oticks[0] - a.shift, rticks[0])
    hi = a.max_tick if a.max_tick is not None else min(oticks[-1] - a.shift, rticks[-1])
    if lo > hi:
        print(f"error: empty compare window lo={lo} hi={hi} "
              f"(orig {oticks[0]}..{oticks[-1]}, rebuild {rticks[0]}..{rticks[-1]}, "
              f"shift {a.shift})", file=sys.stderr)
        return 2

    vizdir = pathlib.Path(a.viz) if a.viz else None
    if vizdir:
        vizdir.mkdir(parents=True, exist_ok=True)

    n_frames = 0
    n_fail = 0
    worst = 0
    for t in range(lo, hi + 1):
        ro, rr = frame_path(a.orig, t + a.shift), frame_path(a.rebuild, t)
        if not ro.exists() or not rr.exists():
            print(f"tick {t}: missing frame (orig {ro.name} rebuild {rr.name})")
            n_fail += 1
            continue
        im_o = load_ppm(str(ro))
        im_r = load_ppm(str(rr))
        if im_o.shape != im_r.shape:
            print(f"tick {t}: size mismatch {im_o.shape} vs {im_r.shape}")
            n_fail += 1
            continue
        H, W, _ = im_r.shape
        diff = im_o != im_r  # HxWx3
        # scene region = everything except the mask rect
        mask = np.zeros((H, W), dtype=bool)
        mask[max(y0, 0):min(y1, H), max(x0, 0):min(x1, W)] = True
        scene = diff.any(axis=2) & ~mask
        npx = int(scene.sum())
        n_frames += 1
        if npx > worst:
            worst = npx
        if npx > a.tol:
            n_fail += 1
            md = int(np.abs(im_o.astype(np.int16) - im_r.astype(np.int16)).max())
            print(f"tick {t}: {npx} differing scene px (max channel delta {md})")
            if vizdir:
                out = im_r.copy()
                out[scene] = (255, 0, 255)
                with open(vizdir / f"frame_{t:06d}_diff.png", "wb") as f:
                    from PIL import Image
                    Image.fromarray(out, "RGB").save(f, "PNG")
    ok = n_fail == 0
    print(f"{n_frames} frames, {n_fail} failing (tol={a.tol}), "
          f"worst={worst}px, scene={W}x{H} mask=({x0},{y0})-({x1},{y1}), "
          f"shift={a.shift} -> {'PASS' if ok else 'FAIL'}")
    if a.report:
        with open(a.report, "a") as f:
            f.write(f"diff {a.orig} vs {a.rebuild}: {n_frames} frames, "
                    f"{n_fail} failing, worst {worst}px, "
                    f"mask {a.mask}, shift {a.shift} -> {'PASS' if ok else 'FAIL'}\n")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
