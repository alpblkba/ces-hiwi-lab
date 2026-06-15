# Lab 2.2 — Processor-Side Interaction and System Validation

    ## Objective

    Validate the generated hardware from the processor/software side and document the full bring-up path.

    ## Background

    A lab based on customized embedded processors should not stop at isolated hardware generation. Students should understand how a hardware block becomes part of a system, how software can interact with it, and why the hardware handoff must match the actual bitstream and platform configuration.

    This task is part of the customized embedded processors lab rewrite. The goal is to create material that future students can follow from a clean start, without relying on undocumented instructor knowledge or fragile GUI state.

    ## What students should learn

    - Vitis application project structure
- BSP awareness
- hardware handoff files
- software-controlled hardware access
- system-level validation
- distinguishing hardware bugs from software/platform bugs
- student-facing explanation and oral defense questions

    ## Expected starting point

    Students are expected to have:

    - access to the assigned lab machine or remote development node
    - a cloned copy of this repository
    - basic familiarity with Linux shell commands
    - basic understanding of C/C++ functions
    - basic awareness of FPGA design flow terminology
    - no requirement to already know HLS in depth

    ## Expected outcome

    At the end of this task, students should be able to explain:

    - what they built or verified
    - which tool generated which artifact
    - where the hardware/software boundary is
    - how inputs and outputs are represented
    - how the design was built, exported, integrated, or tested
    - what common failure modes they encountered
    - what they would check first when something fails

    ## Student deliverables

    - description of hardware handoff artifact used
- software project or application structure notes
- BSP/platform generation notes
- validation plan
- observed board or runtime behavior
- debug notes for mismatches between hardware and software
- final short explanation of the complete system

    ## Suggested workflow

    1. identify the exported hardware handoff file
2. create or update the Vitis platform project
3. regenerate BSP if hardware changed
4. create or update the application project
5. build the software side
6. program or connect to the target as required
7. run the validation steps
8. document whether the observed behavior matches the expected hardware design

    ## Suggested repository layout

    ```text
    lab2/task2/
    ├── README.md
    ├── commands.md
    ├── instructor_notes.md
    ├── questions.md
    └── expected_observations.md
    ```

    ## Implementation notes

    This documentation intentionally avoids providing final C/C++ implementation code. The lab should require students to reason about the design and produce the implementation themselves.

    A good solution should still be structured around:

    - one clearly identified design entry point
    - simple and explainable input/output behavior
    - minimal hidden state
    - deterministic behavior for every relevant case
    - build steps that can be reproduced from a clean checkout
    - logs and reports that can be inspected after failure

    ## Common risks

    - using an old XSA or hardware platform
- not regenerating BSP after hardware changes
- assuming software rebuild updates hardware automatically
- not checking whether the programmed bitstream matches the expected design
- debugging software while the block design is invalid
- not documenting the complete path from HLS to board behavior

    ## Reflection questions

    1. What is the input of this task?
    2. What is the output?
    3. Which part is generated automatically by the tool?
    4. Which part is written or configured by the student?
    5. What does the FPGA implement physically?
    6. What would change if the design were extended?
    7. Which build artifact is needed by the next stage?
    8. What would you check first if the design does not work?

    ## Current status

    - task structure prepared
    - student-facing explanation drafted
    - build/run path documented separately
    - troubleshooting collected in the shared guide
    - implementation intentionally left for the lab development phase
