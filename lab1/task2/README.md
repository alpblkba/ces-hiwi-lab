# Lab 1.2 — Seven-Segment Display with HLS

    ## Objective

    Introduce high-level synthesis by implementing a seven-segment display controller as an HLS-generated IP block.

    ## Background

    The seven-segment display is intentionally small and concrete. It allows students to focus on the meaning of hardware generation from C/C++ without being overwhelmed by a complex algorithm. The important learning outcome is not the display decoder itself, but the ability to explain how a top-level C/C++ function becomes hardware.

    This task is part of the customized embedded processors lab rewrite. The goal is to create material that future students can follow from a clean start, without relying on undocumented instructor knowledge or fragile GUI state.

    ## What students should learn

    - C/C++ to hardware thinking
- top-level HLS function definition
- switch input to seven-segment output mapping
- single-digit mandatory implementation
- two-digit optional extension
- active-high versus active-low segment behavior
- student explanation of generated hardware

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

    - short design explanation
- description of input and output signals
- truth table or mapping for seven-segment encoding
- HLS project notes and generated reports
- explanation of single-digit behavior
- optional notes for two-digit extension
- reflection answers about HLS and hardware/software boundaries

    ## Suggested workflow

    1. read the expected input and output behavior
2. define the top-level function interface
3. write the smallest single-digit mapping first
4. simulate or reason through all relevant input cases
5. run HLS synthesis
6. inspect interface, latency, and resource reports
7. export the generated IP if required by the next task
8. document how the generated hardware should be connected later

    ## Suggested repository layout

    ```text
    lab1/task2/
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

    - believing HLS simply runs C++ on the FPGA
- not defining a stable top-level function
- forgetting active-low display behavior
- testing only one input value
- not checking generated interface reports
- exporting stale IP after modifying the source

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
