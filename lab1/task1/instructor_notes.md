# Instructor Notes — Lab 1.1 — Initial Setup and Bring-up

    ## Teaching goal

    Prepare the student environment, verify remote access, and establish the basic Vitis/Vivado workflow before implementing hardware logic.

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

    - student can show hostname and repository path
- student can identify tool availability
- student can explain whether GUI works and why this matters
- student can continue with a command-line-first workflow if GUI is unavailable
- student reports issues in a reproducible format

    ## Suggested oral questions

    1. Which machine are you working on, and how do you know?
2. Which Xilinx tool version is visible in your shell?
3. What is the difference between a missing tool and a missing PATH entry?
4. What does DISPLAY mean in the context of GUI forwarding?
5. Why should setup problems be documented before changing shell files?
6. What is the minimum successful state before starting HLS work?

    ## Common student misunderstandings

    - working on the local laptop while thinking they are on the lab machine
- editing .bashrc blindly instead of checking the intended setup flow
- assuming GUI failure means no progress is possible
- not recording the first error message
- mixing different Xilinx versions across generated projects

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
