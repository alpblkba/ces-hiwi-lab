#!/usr/bin/env python3
"""
Lab 3 Task 2 - pragma design space exploration.

Generates one source variant per pragma configuration, runs C synthesis on
each, and collects the real latency / interval / resource figures into a
markdown table. The student-facing kernel in src/ stays free of macro tricks:
this script inserts the pragma lines into a copy.

    python3 sweep.py            # run every variant
    python3 sweep.py V00 V09    # run only the named variants
"""

import os
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
SRC = HERE / "src"
WORK = HERE / (os.environ.get("RUNDIR") or "runs")
PART = "xc7z007sclg400-1"
PERIOD = 10          # ns -> 100 MHz
HLS_SETTINGS = "/Software/xilinx/2022.2/Vitis_HLS/2022.2/settings64.sh"

# name -> (description, pipeline_target, unroll_factor, partition_factor)
#   pipeline_target : None | "prod" | "col"
#   unroll_factor   : None | int   (16 == full unroll of the k loop)
#   partition_factor: None | int | "complete"
VARIANTS = [
    ("V00", "baseline, no pragmas",                       None,   None, None),
    ("V01", "PIPELINE on prod (innermost)",               "prod", None, None),
    ("V02", "PIPELINE on col",                            "col",  None, None),
    ("V03", "UNROLL 2, no partition",                     None,   2,    None),
    ("V04", "UNROLL 4, no partition",                     None,   4,    None),
    ("V05", "UNROLL 8, no partition",                     None,   8,    None),
    ("V06", "UNROLL 16 (full), no partition",             None,   16,   None),
    ("V07", "ARRAY_PARTITION 4 only, no unroll",          None,   None, 4),
    ("V08", "UNROLL 2 + PARTITION 2",                     None,   2,    2),
    ("V09", "UNROLL 4 + PARTITION 4",                     None,   4,    4),
    ("V10", "UNROLL 8 + PARTITION 8",                     None,   8,    8),
    ("V11", "UNROLL 16 + PARTITION complete",             None,   16,   "complete"),
    ("V12", "PIPELINE col + UNROLL 4 + PARTITION 4",      "col",  4,    4),
]


def make_source(pipeline, unroll, partition):
    """Inserts the requested pragmas into the baseline kernel."""
    lines = (SRC / "matmul.cpp").read_text().split("\n")
    out, pending_label = [], None

    for line in lines:
        out.append(line)
        stripped = line.strip()

        # Partition pragmas go right after the function opening brace.
        # The k index selects A's 2nd dimension and B's 1st, so those are the
        # dimensions that must be split to feed a partially unrolled k loop.
        if stripped.startswith("void matmul(") or (
                stripped == "{" and out[-2].strip().startswith("void matmul(")):
            if partition and stripped.endswith("{"):
                if partition == "complete":
                    out.append("#pragma HLS ARRAY_PARTITION variable=A dim=2 complete")
                    out.append("#pragma HLS ARRAY_PARTITION variable=B dim=1 complete")
                else:
                    out.append(f"#pragma HLS ARRAY_PARTITION variable=A dim=2 cyclic factor={partition}")
                    out.append(f"#pragma HLS ARRAY_PARTITION variable=B dim=1 cyclic factor={partition}")

        if stripped in ("row:", "col:", "prod:"):
            pending_label = stripped[:-1]
            continue

        # The line after a label is the for-statement; attach loop pragmas here.
        if pending_label and stripped.startswith("for "):
            if pipeline == pending_label:
                out.append("#pragma HLS PIPELINE II=1")
            if pending_label == "prod" and unroll:
                if unroll >= 16:
                    out.append("#pragma HLS UNROLL")
                else:
                    out.append(f"#pragma HLS UNROLL factor={unroll}")
            pending_label = None

    return "\n".join(out)


def parse_report(rpt):
    """Pulls latency, II and resource use out of the csynth report."""
    if not rpt.exists():
        return None
    t = rpt.read_text()
    res = {}

    m = re.search(r"\|\s*(\d+|\?)\|\s*(\d+|\?)\|[^|]*\|[^|]*\|\s*(\d+|\?)\|\s*(\d+|\?)\|", t)
    if m:
        res["lat_min"], res["lat_max"] = m.group(1), m.group(2)
        res["ii_min"], res["ii_max"] = m.group(3), m.group(4)

    block = re.search(r"\|Total\s*\|([^\n]*)\n", t)
    if block:
        nums = re.findall(r"(\d+)", block.group(1))
        if len(nums) >= 5:
            res["bram"], res["dsp"], res["ff"], res["lut"] = (
                nums[0], nums[1], nums[2], nums[3])

    m = re.search(r"\+\s*Timing.*?\|\s*ap_clk\s*\|\s*([\d.]+) ns\|\s*([\d.]+) ns", t, re.S)
    if m:
        res["target_ns"], res["est_ns"] = m.group(1), m.group(2)
    return res


def run_variant(name, desc, pipeline, unroll, partition):
    d = WORK / name
    d.mkdir(parents=True, exist_ok=True)
    (d / "matmul.h").write_text((SRC / "matmul.h").read_text())
    (d / "matmul.cpp").write_text(make_source(pipeline, unroll, partition))

    auto = ("# auto loop pipelining left at the tool default"
            if os.environ.get("KEEP_AUTO") else
            "config_compile -pipeline_loops 0")
    AUTO = auto
    (d / "run.tcl").write_text(f"""
open_project -reset prj
set_top matmul
add_files matmul.cpp
open_solution -reset "sol" -flow_target vivado
set_part {{{PART}}}
create_clock -period {PERIOD} -name default
{AUTO}
csynth_design
exit
""")
    subprocess.run(
        f"source {HLS_SETTINGS} && vitis_hls -f run.tcl > synth.log 2>&1",
        shell=True, cwd=d, executable="/bin/bash")

    r = parse_report(d / "prj/sol/syn/report/matmul_csynth.rpt")
    if r is None:
        print(f"  {name}: SYNTHESIS FAILED (see {d}/synth.log)")
    return r


def main():
    wanted = [a for a in sys.argv[1:] if a.startswith("V")]
    rows = []
    for name, desc, pipe, unr, part in VARIANTS:
        if wanted and name not in wanted:
            continue
        print(f"running {name}: {desc} ...", flush=True)
        r = run_variant(name, desc, pipe, unr, part)
        if r:
            print(f"  latency={r.get('lat_max','?')} II={r.get('ii_max','?')} "
                  f"DSP={r.get('dsp','?')} LUT={r.get('lut','?')} "
                  f"FF={r.get('ff','?')} BRAM={r.get('bram','?')}", flush=True)
            rows.append((name, desc, r))

    hdr = ("| variant | configuration | latency (cycles) | II | BRAM | DSP | FF | LUT |\n"
           "|---------|---------------|------------------|----|------|-----|----|----|\n")
    body = "".join(
        f"| {n} | {d} | {r.get('lat_max','?')} | {r.get('ii_max','?')} | "
        f"{r.get('bram','?')} | {r.get('dsp','?')} | {r.get('ff','?')} | {r.get('lut','?')} |\n"
        for n, d, r in rows)
    (HERE / (os.environ.get("RESULTS") or "results.md")).write_text(hdr + body)
    print("\n" + hdr + body)


if __name__ == "__main__":
    main()
