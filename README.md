# CES HiWi Lab Workspace

This repository contains the working material for the customized embedded processors lab rewrite at the KIT Chair of Embedded Systems.

The current focus is to prepare a student-facing lab structure around HLS-based accelerator design, Vitis/Vivado workflows, initial setup, bring-up, and reproducible documentation.

## Current scope

The repository currently covers:

- Lab 1.1 — initial setup and bring-up
- Lab 1.2 — seven-segment display with HLS
- Lab 2.1 — HLS IP integration in a block design
- Lab 2.2 — processor-side interaction and system validation
- shared setup documentation
- troubleshooting documentation
- development log
- instructor-facing notes

## Design philosophy

The lab material should be:

- reproducible from a clean checkout
- understandable by students with limited HLS background
- explicit about tool roles
- explicit about generated versus hand-written files
- useful for both students and instructors
- robust against common setup problems

## Repository structure

```text
.
├── docs/
│   ├── initial-setup-and-bringup.md
│   └── troubleshooting.md
├── devlog/
│   └── week1.md
├── lab1/
│   ├── task1/
│   └── task2/
└── lab2/
    ├── task1/
    └── task2/
```

## Lab overview

### Lab 1.1

Initial setup and bring-up. Students learn how to access the environment, verify tools, understand the repository, and diagnose setup issues.

### Lab 1.2

Seven-segment display with HLS. Students implement a small hardware-oriented function and use it as the entry point into HLS thinking.

### Lab 2.1

HLS IP integration. Students move from isolated HLS design toward system integration in Vivado.

### Lab 2.2

Processor-side interaction and validation. Students connect the hardware artifact to software-side validation and explain the full system.

## Current status

This repository is in active lab development. Documentation and structure are being prepared before the final implementation is fixed.

The current emphasis is on creating a reliable skeleton that can be reviewed, extended, and turned into the final student material.
