# Instructor Notes — Lab 1.2 — Seven-Segment Display with HLS

    ## Teaching goal

    Introduce high-level synthesis by implementing a seven-segment display controller as an HLS-generated IP block.

    The main goal is to help students understand the flow, not only to make them click through tools. Students should be able to explain each stage of the design process in their own words.

    ## What to look for

    A good student solution should show:

    - a clean understanding of the task objective
    - a reproducible workflow
    - clear separation between generated files and authored files
    - awareness of tool version dependencies
    - ability to debug from logs rather than guessing
    - ability to explain the hardware/software boundary

    ## Task-specific instructor checks

    - student can explain every input-output case
- student understands the top function as a hardware boundary
- student can locate HLS reports
- student can explain what artifact is passed to Vivado
- student can describe how the design could be extended

    ## Suggested oral questions

    1. What does the HLS top function represent physically?
2. Is the C/C++ function executed by a CPU after synthesis?
3. What are the input and output ports?
4. How do you handle invalid switch values?
5. What is the difference between simulation correctness and board correctness?
6. Why might seven-segment encodings differ between boards?
7. What changes for a two-digit extension?

    ## Common student misunderstandings

    - believing HLS simply runs C++ on the FPGA
- not defining a stable top-level function
- forgetting active-low display behavior
- testing only one input value
- not checking generated interface reports
- exporting stale IP after modifying the source

    ## Grading guidance

    Suggested dimensions:

    - correctness of design intent
    - reproducibility of workflow
    - clarity of explanation
    - quality of debugging notes
    - ability to answer conceptual questions
    - minimal but meaningful documentation

    ## Instructor TODO

    - verify the final Xilinx version used for the lab
    - confirm machine names and access instructions
    - confirm whether GUI access is required or optional
    - confirm whether students use Vitis HLS standalone or Vitis unified flow
    - confirm board-specific constraints
    - confirm final submission format
