# Instructor Notes — Lab 2.2 — Processor-Side Interaction and System Validation

    ## Teaching goal

    Validate the generated hardware from the processor/software side and document the full bring-up path.

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

    - student can identify the hardware handoff file
- student can explain BSP role without overclaiming
- student can describe the validation path
- student can reason about stale hardware/software artifacts
- student can explain the complete system from input to output

    ## Suggested oral questions

    1. What is a BSP and why does it matter?
2. What happens if software uses an old hardware handoff file?
3. How can you tell whether a failure is in hardware or software?
4. What does processor-side validation prove?
5. What does it not prove?
6. Why should the bitstream, platform, and application be kept consistent?
7. What would you rebuild after modifying the HLS block?

    ## Common student misunderstandings

    - using an old XSA or hardware platform
- not regenerating BSP after hardware changes
- assuming software rebuild updates hardware automatically
- not checking whether the programmed bitstream matches the expected design
- debugging software while the block design is invalid
- not documenting the complete path from HLS to board behavior

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
