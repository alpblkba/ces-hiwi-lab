# Initial Setup and Bring-up Guide

## Purpose

This document describes the initial setup and bring-up workflow for the customized embedded processors lab.

The goal is to make the first student interaction with the lab environment predictable. Before students write hardware code, they should be able to access the machine, locate the repository, understand the toolchain, and verify that the basic development environment works.

## Expected environment

Students may work on a remote lab machine, a university workstation, or another assigned development node.

The exact environment may vary, but the following should be clarified before the lab starts:

- machine hostname
- login method
- required VPN or network access
- Xilinx tool version
- whether GUI access is expected
- whether command-line flow is supported
- board type
- repository URL
- submission format

## First login checklist

After logging in, students should run:

```bash
hostname
whoami
pwd
echo "$SHELL"
echo "$PATH"
```

They should confirm that they are on the expected machine and not accidentally working on their local laptop.

## Repository checkout

Students should clone or update the repository:

```bash
git clone <repository-url>
cd <repository-name>
git status
```

If the repository already exists:

```bash
cd <repository-name>
git status
git pull
```

## Directory orientation

Students should inspect the repository:

```bash
find . -maxdepth 3 -type f | sort
tree -L 3 .
```

Expected top-level directories:

- docs/
- lab1/
- lab2/
- devlog/

## Toolchain verification

Students should check whether the required tools are available:

```bash
which vivado || true
which vitis || true
which vitis_hls || true
which xsct || true
```

If a command is missing, students should not immediately modify shell startup files. They should first document the issue and ask whether the tool must be loaded through a module system or setup script.

## Tool version check

The lab should use one consistent Xilinx version. Candidate versions discussed during preparation include 2022.2 and 2023.1.

Students should record the version they use:

```bash
vivado -version
vitis -version
```

If versions differ between students or machines, this must be documented because generated projects may not be fully portable across versions.

## GUI / X11 bring-up

If GUI access is required, students should check:

```bash
echo "$DISPLAY"
xdpyinfo >/dev/null && echo "x11 works" || echo "x11 failed"
```

Optional GUI smoke test:

```bash
xclock
```

If DISPLAY is empty or xclock cannot open the display, GUI forwarding is not working.

Possible causes:

- no X server running locally
- ssh was started without X forwarding
- XQuartz or equivalent is not configured
- remote server does not allow forwarding
- the session is not intended to support GUI usage

## Command-line-first mindset

Where possible, students should prefer reproducible command-line steps over undocumented GUI state.

GUI tools are useful, but the lab material should still explain:

- what project was created
- what file was added
- what top-level module/function was selected
- what artifact was exported
- where logs are located

## Bring-up stages

The recommended bring-up path is:

1. login works
2. repository is available
3. toolchain commands are visible
4. tool versions are known
5. GUI status is known
6. simple project can be opened or created
7. logs can be found
8. build failure can be diagnosed
9. generated artifact can be identified
10. next lab task can consume the artifact

## What to record

Students should record:

- machine name
- tool version
- whether GUI works
- exact command used
- first failing command, if any
- relevant log file
- whether the issue is reproducible

## Minimal successful setup

A setup is considered minimally successful when the student can:

- access the assigned machine
- navigate the repository
- identify the correct lab task
- run basic diagnostic commands
- identify whether Xilinx tools are available
- explain the next step in the lab flow

Full GUI functionality is useful but should not be the only definition of progress.
