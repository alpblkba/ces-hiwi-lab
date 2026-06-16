# Commands — Lab 1.2 — Seven-Segment Display with HLS

This file collects commands useful during setup, bring-up, and debugging for seven-segment display with HLS.

The exact commands may differ depending on the assigned machine, installed Xilinx version, and whether the student uses a GUI or command-line workflow.

## Basic repository checks

```bash
pwd
ls
find . -maxdepth 3 -type f | sort
git status
```

## Environment checks

```bash
hostname
whoami
echo "$SHELL"
echo "$PATH"
echo "$DISPLAY"
```

## Tool availability checks

```bash
which vivado || true
which vitis || true
which vitis_hls || true
which xsct || true
```

## Tool version checks

```bash
vivado -version
vitis -version
```

## X11 / GUI checks

```bash
echo "$DISPLAY"
xdpyinfo >/dev/null && echo "x11 works" || echo "x11 failed"
xclock
```

If these fail, continue with the command-line workflow where possible and document the issue.

## Recommended diagnostic commands

```bash
tree -L 3 .
find . -name "*.log" -o -name "*.jou" -o -name "*.rpt"
grep -R "ERROR:" . || true
grep -R "CRITICAL WARNING:" . || true
```

## Build log habit

Students should save terminal output when something fails:

```bash
mkdir -p logs
script -a logs/session.log
```

Exit the logging session with:

```bash
exit
```

## Clean-up checks before asking for help

```bash
pwd
git status
find . -maxdepth 3 -type f | sort
```

## Help request template

```text
Task:
Machine:
Tool version:
Exact failing command:
First error message:
Relevant log file:
What I expected:
What happened:
What I already checked:
```

## Notes

Do not blindly delete generated project folders unless the task explicitly says so. Generated folders often contain useful logs that explain the failure.
