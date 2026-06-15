# Lab 1.1 — Initial Setup and Bring-up

    ## Objective

    Prepare the student environment, verify remote access, and establish the basic Vitis/Vivado workflow before implementing hardware logic.

    ## Background

    This task exists because students often lose significant lab time before the real hardware work begins. They may not know whether they are on the correct machine, whether the Xilinx tools are visible, whether GUI forwarding works, or which project directory should be used. The task makes setup itself explicit and reproducible.

    This task is part of the customized embedded processors lab rewrite. The goal is to create material that future students can follow from a clean start, without relying on undocumented instructor knowledge or fragile GUI state.

    ## What students should learn

    - ssh access to the lab machines
- repository navigation and directory orientation
- toolchain version awareness
- basic Vitis/Vivado visibility checks
- X11/GUI diagnosis without blocking the whole lab
- recording failures in a form that instructors can debug

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

    - short setup report with hostname, tool version, and repository path
- notes on whether GUI forwarding works
- list of commands used for verification
- first observed failure, if any, with exact error message
- confirmation that the student can locate Lab 1.2 material

    ## Suggested workflow

    1. log in to the assigned machine
2. verify hostname, user, shell, path, and working directory
3. clone or update the repository
4. inspect the lab directory layout
5. check whether Vivado, Vitis, Vitis HLS, and XSCT are available
6. record the Xilinx tool version
7. check X11/GUI status if required
8. write a short bring-up note before moving to Lab 1.2

    ## Suggested repository layout

    ```text
    lab1/task1/
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

    - working on the local laptop while thinking they are on the lab machine
- editing .bashrc blindly instead of checking the intended setup flow
- assuming GUI failure means no progress is possible
- not recording the first error message
- mixing different Xilinx versions across generated projects

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
