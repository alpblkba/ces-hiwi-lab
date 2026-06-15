# Development Log — Week 1

Date: 2026-06-15

## Goal

Prepare the initial structure and documentation for the customized embedded processors lab rewrite.

The main priority for this week is to create a reviewable repository skeleton that can be discussed with supervisors before the final implementation details are fixed.

## Work completed

### Repository structure

Created or prepared the following lab structure:

- Lab 1.1 — initial setup and bring-up
- Lab 1.2 — seven-segment display with HLS
- Lab 2.1 — HLS IP integration in a block design
- Lab 2.2 — processor-side interaction and system validation

Each task now has a student-facing README, command notes, instructor notes, questions, and expected observations.

### Shared documentation

Prepared shared documentation for:

- initial setup and bring-up
- toolchain verification
- X11/GUI diagnosis
- reproducible workflow expectations
- troubleshooting across setup, HLS, Vivado, and Vitis stages

### Teaching focus

The documentation emphasizes:

- reproducibility
- clear hardware/software boundary
- generated versus authored files
- student explanation quality
- log-based debugging
- tool version awareness

### Current implementation status

No final C/C++ implementation was added in this documentation pass.

This is intentional. The current focus is to establish the lab structure and student-facing explanation before locking down final task implementations.

## Open questions

- Which Xilinx version should be standardized for the lab: 2022.2 or 2023.1?
- Is GUI usage required, or should the lab support a command-line-first workflow?
- Which machines/nodes will students officially use?
- What is the final board and constraint setup?
- What is the expected final submission format?
- Should two-digit seven-segment support be mandatory or optional?
- How much processor-side software should be required in Lab 2?

## Risks

- X11 access may be unreliable on some student machines.
- Tool versions may differ across machines.
- Generated Vivado/Vitis projects may not be portable.
- Students may confuse Vitis, Vivado, and HLS roles.
- If setup documentation is weak, lab time may be lost before students reach the actual design task.

## Next steps

1. Review documentation with supervisors.
2. Confirm official tool version.
3. Confirm expected remote access method.
4. Implement the smallest working HLS seven-segment example.
5. Validate IP export and Vivado integration.
6. Add screenshots or logs after the flow is confirmed.
7. Convert instructor notes into final teaching material.
