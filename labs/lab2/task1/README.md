# Lab 2.1 — HLS IP Integration in a Block Design

    ## Objective

    Integrate an HLS-generated IP block into a Vivado block design and connect it to the surrounding system.

    ## Background

    After students generate an IP block with HLS, they need to understand that the block does not automatically become a complete FPGA system. It must be imported, connected, clocked, reset, constrained, and validated in the context of a board-level design.

    This task is part of the customized embedded processors lab rewrite. The goal is to create material that future students can follow from a clean start, without relying on undocumented instructor knowledge or fragile GUI state.

    ## What students should learn

    - IP export and IP repository handling
- Vivado block design workflow
- clock and reset connectivity
- board I/O and constraint awareness
- address assignment if AXI interfaces are used
- output product generation and bitstream build
- debugging synthesis and implementation issues

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

    - block design screenshot or textual description
- list of connected interfaces and signals
- notes on clock/reset connections
- Vivado validation result
- synthesis/implementation status
- bitstream generation note, if reached
- known issues and debug notes

    ## Suggested workflow

    1. locate the exported HLS IP
2. add the IP repository path to Vivado
3. refresh the IP catalog
4. instantiate the generated IP in the block design
5. connect clock, reset, and data/control interfaces
6. validate the block design
7. generate output products
8. run synthesis and implementation
9. generate bitstream if the design is complete

    ## Suggested repository layout

    ```text
    lab2/task1/
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

    - adding the wrong generated directory as IP repository
- not refreshing the IP catalog
- leaving clock or reset unconnected
- ignoring critical warnings
- forgetting address assignment for AXI-connected IP
- using stale output products
- programming a bitstream generated from an older design

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
