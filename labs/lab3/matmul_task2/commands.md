# Commands — Lab 3.2 — Pragma exploration

The kernel is the same as Task 3.1. Only pragmas change, and the project setup
is identical, so most of `task1/commands.md` still applies.

## 1. Environment and working directory

```bash
source /Software/xilinx/2022.2/Vitis_HLS/2022.2/settings64.sh
mkdir -p ~/vitis/lab3_task2
cd ~/vitis/lab3_task2
cp <repo>/labs/lab3/task2/src/matmul.*  .
```

## 2. The one setting that makes the comparison honest

Vitis HLS **optimises the code even when there is no pragma at all**. With the
default settings it automatically pipelines the `col` loop and completely
unrolls `prod`. You can see it in the synthesis log:

```text
INFO: [XFORM 203-510] Pipelining loop 'col' ... automatically.
INFO: [HLS 200-489] Unrolling loop 'prod' ... completely with a factor of 16.
```

If you measure against that, your pragmas appear to do nothing — or to make
things worse — because you are not adding optimisation, you are overriding the
tool's own. To see what each pragma contributes, switch the automatic behaviour
off in every run:

```tcl
config_compile -pipeline_loops 0
```

Put this line after `create_clock` and before `csynth_design`.

Check it worked: the log must no longer contain "automatically".

```bash
grep -i "automatically" synth.log     # expect no output
```

## 3. Synthesizing one variant

```bash
cat > run.tcl <<'EOF'
open_project -reset prj
set_top matmul
add_files matmul.cpp
open_solution -reset "sol" -flow_target vivado
set_part {xc7z007sclg400-1}
create_clock -period 10 -name default
config_compile -pipeline_loops 0
csynth_design
exit
EOF

vitis_hls -f run.tcl
```

Then edit `matmul.cpp`, add the pragma for the next variant, and run it again.

## 4. Where each pragma goes

`PIPELINE` and `UNROLL` attach to a loop and go **immediately inside** it:

```cpp
    prod:
        for (int k = 0; k < N; k++) {
        #pragma HLS UNROLL factor=4
            acc += (acc_t)A[i][k] * (acc_t)B[k][j];
        }
```

`ARRAY_PARTITION` attaches to an array and goes at the **top of the function**:

```cpp
void matmul(const data_t A[N][N], const data_t B[N][N], acc_t C[N][N]) {
#pragma HLS ARRAY_PARTITION variable=A dim=2 cyclic factor=4
#pragma HLS ARRAY_PARTITION variable=B dim=1 cyclic factor=4
```

The dimensions are not arbitrary. The unrolled loop variable is `k`, and `k`
indexes `A[i][k]` (second dimension) and `B[k][j]` (first dimension). Those are
the dimensions that must be split so that several `k` values can be read at
once. `C` is not indexed by `k`, so partitioning it achieves nothing.

The partition factor should match the unroll factor. Unrolling by 4 while
partitioning by 2 leaves the memory unable to feed the arithmetic.

## 5. Reading the numbers

```bash
REPORT=prj/sol/syn/report/matmul_csynth.rpt

grep -A6 "Latency (cycles)" $REPORT | head -8
sed -n '/Utilization Estimates/,/Interface/p' $REPORT | grep -E "^\|(Total|Available|Utilization)"
```

Record latency, II, BRAM, DSP, FF, LUT for every variant into one table. The
table is the deliverable — a single fastest configuration is not.

## 6. Running the whole sweep automatically

Repeating this by hand for a dozen variants is slow and easy to get wrong.
`src/sweep.py` generates each variant, synthesizes it, and writes the results
table:

```bash
cp <repo>/labs/lab3/task2/src/sweep.py .
mkdir -p src && cp matmul.h matmul.cpp src/
python3 sweep.py              # all variants
python3 sweep.py V00 V09      # only the named ones
cat results.md
```

To reproduce the tool-default comparison instead, set `KEEP_AUTO=1`:

```bash
KEEP_AUTO=1 RUNDIR=runs_auto RESULTS=results_auto.md python3 sweep.py
```

## 7. Keeping the results

```bash
mkdir -p <repo>/labs/lab3/task2/hls
cp results.md <repo>/labs/lab3/task2/hls/
cp runs/V00/prj/sol/syn/report/matmul_csynth.rpt <repo>/labs/lab3/task2/hls/V00.rpt
```

Copy reports and the results table, never the build trees.

## Troubleshooting

**A pragma seems to do nothing**
Check the synthesis log. Vitis HLS reports when it ignores or cannot apply a
directive, and it says why.

```bash
grep -iE "WARNING|ignored|unable|cannot" synth.log | head
```

**Unrolling made the design slower**
Expected, if the arrays were not partitioned. See the Task 3.2 README — this is
the main result of the lab, not a mistake.

**DSP usage above 66**
The design does not fit on this device. Vitis HLS still reports an estimate; it
does not stop you. Record the number and note that the configuration is not
implementable on the Blackboard.

**Latency reported as `?`**
A loop bound stopped being a compile-time constant. Check `matmul.h`.
