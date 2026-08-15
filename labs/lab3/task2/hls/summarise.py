#!/usr/bin/env python3
"""Turn the per-variant C-synthesis reports into one comparison table.

    python3 summarise.py                  # V0*_csynth.rpt in this directory
    python3 summarise.py V00.rpt V04.rpt  # named files, in that order

Writes results.md and prints the same table. Reads only the summary sections,
so it works on any Vitis HLS 2022.x report for any kernel.

The numbers here are ESTIMATES. On the 4x4 kernel the estimate was out by 2x on
DSP and by nearly 3x on LUT compared with the post-place-and-route figures, in
opposite directions. Use this table to compare variants against each other, and
the implementation report for anything you intend to claim about the device.
"""

import glob
import os
import re
import sys

VARIANTS = {
    "0": "baseline, no pragmas",
    "1": "PIPELINE on the neuron loop",
    "2": "UNROLL the product loop",
    "3": "ARRAY_PARTITION alone",
    "4": "all three together",
}


def cells(line):
    """Split one report table row into its cells."""
    return [c.strip() for c in line.strip().strip("|").split("|")]


def parse(path):
    """Pull latency, interval and the resource totals out of one report."""
    with open(path, errors="replace") as fh:
        lines = fh.readlines()

    latency = interval = None
    total = avail = None

    for i, line in enumerate(lines):
        # The latency summary is the first data row after the "Latency (cycles)"
        # header. Columns: min, max, abs min, abs max, II min, II max, type.
        if latency is None and "Latency (cycles)" in line and "Iteration" not in line:
            for row in lines[i + 1 : i + 8]:
                c = cells(row)
                if len(c) >= 7 and c[0].isdigit():
                    latency, interval = c[1], c[5]
                    break

        # Columns: name, BRAM_18K, DSP, FF, LUT, URAM
        if line.startswith("|Total "):
            total = cells(line)[1:]
        elif line.startswith("|Available ") and total is not None and avail is None:
            avail = cells(line)[1:]

    if latency is None or total is None:
        raise SystemExit(f"{path}: could not find the summary tables")

    return {
        "latency": latency,
        "interval": interval,
        "bram": total[0],
        "dsp": total[1],
        "ff": total[2],
        "lut": total[3],
        "avail": avail,
    }


def variant_of(path):
    # V00..V04 for this lab. The archived 16x16 reports go up to V12, so take
    # the whole number rather than a single digit and let those fall through to
    # the filename as their label.
    m = re.search(r"V(\d+)", os.path.basename(path))
    return str(int(m.group(1))) if m else "?"


def main():
    paths = sys.argv[1:] or sorted(glob.glob("V0*_csynth.rpt"))
    if not paths:
        raise SystemExit("no V0*_csynth.rpt here - run run_hls.tcl first")

    rows = []
    base = None
    for path in paths:
        r = parse(path)
        v = variant_of(path)
        r["v"] = v
        r["what"] = VARIANTS.get(v, os.path.basename(path))
        if base is None and v == "0":
            base = int(r["latency"])
        rows.append(r)

    out = [
        "| variant | configuration | latency (cycles) | II | BRAM | DSP | FF | LUT |",
        "|---------|---------------|------------------|----|------|-----|----|-----|",
    ]
    for r in rows:
        out.append(
            "| V{v} | {what} | {latency} | {interval} | {bram} | {dsp} | {ff} | {lut} |".format(**r)
        )

    if base:
        out.append("")
        out.append("Speed-up against V0 ({} cycles):".format(base))
        out.append("")
        out.append("| variant | latency | speed-up |")
        out.append("|---------|---------|----------|")
        for r in rows:
            lat = int(r["latency"])
            out.append("| V{} | {} | {:.2f}x |".format(r["v"], lat, base / lat))

    if rows and rows[0]["avail"]:
        a = rows[0]["avail"]
        out.append("")
        out.append(
            "Device budget: BRAM {}, DSP {}, FF {}, LUT {}. "
            "These are C-synthesis estimates, not implementation figures.".format(
                a[0], a[1], a[2], a[3]
            )
        )

    text = "\n".join(out) + "\n"
    with open("results.md", "w") as fh:
        fh.write(text)
    print(text)
    print("written to results.md")


if __name__ == "__main__":
    main()
