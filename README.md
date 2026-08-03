# CES HiWi Lab Workspace

This repository contains the working material for the customized embedded processors lab rewrite at the KIT Chair of Embedded Systems.

The current focus is to prepare a student-facing lab structure around HLS-based accelerator design, DNN/CNN pragma exploration, Vitis/Vivado workflows, initial setup, bring-up, and reproducible documentation.

## Current scope

The repository currently covers:

- Lab 1 — initial setup, bring-up, and a first HLS exercise (seven-segment display)
- Lab 2 — HLS IP integration into a Vivado block design, with processor-side validation
- Lab 3 — DNN kernel synthesis and pragma-based design-space exploration in Vitis HLS
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
├── lab2/
│   ├── task1/
│   └── task2/
└── lab3/
    ├── task1/
    └── task2/
```

## Lab overview

### Lab 1

Initial setup and bring-up, followed by a first small HLS exercise. Task 1 covers accessing the environment, verifying tools, and diagnosing setup issues. Task 2 has students implement a seven-segment display with HLS as an entry point into HLS thinking.

### Lab 2

Moving from isolated HLS design to system integration. Task 1 covers HLS IP integration into a Vivado block design. Task 2 connects the hardware artifact to processor-side validation, so students see and explain the full system.

### Lab 3

Pragma-based design-space exploration on a DNN kernel. Task 1 has students find, synthesize, and baseline a small DNN kernel in Vitis HLS with no optimization. Task 2 applies PIPELINE, UNROLL, and ARRAY_PARTITION — alone and combined — and has students explain what each pragma actually changes in the hardware.

## Current status

This repository is in active lab development. Documentation and structure are being prepared before the final implementation is fixed.

The current emphasis is on creating a reliable skeleton that can be reviewed, extended, and turned into the final student material.
