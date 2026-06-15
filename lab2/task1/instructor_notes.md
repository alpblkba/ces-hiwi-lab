# Instructor Notes — Lab 2.1 — HLS IP Integration in a Block Design

    ## Teaching goal

    Integrate an HLS-generated IP block into a Vivado block design and connect it to the surrounding system.

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

    - student can identify the generated IP source
- student can explain every block design connection
- student understands clock/reset requirements
- student can find logs and reports
- student knows whether the failure is HLS, Vivado, or board-related

    ## Suggested oral questions

    1. Why is HLS IP export not the same as a complete FPGA design?
2. How does Vivado find generated IP?
3. What happens if clock or reset is missing?
4. What does block design validation check?
5. What does it not check?
6. Where do synthesis and implementation errors appear?
7. How do constraints affect visible board behavior?

    ## Common student misunderstandings

    - adding the wrong generated directory as IP repository
- not refreshing the IP catalog
- leaving clock or reset unconnected
- ignoring critical warnings
- forgetting address assignment for AXI-connected IP
- using stale output products
- programming a bitstream generated from an older design

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
