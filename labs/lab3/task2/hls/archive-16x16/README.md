# Archive: the 16x16 plain matmul (superseded)

These are C-synthesis results for an earlier version of the Lab 3 kernel: a
plain 16x16 matrix multiply with no bias and no ReLU.

**This design is not used any more.** The lab now uses a 4x4 dense layer with
bias and ReLU, so that the kernel is a real neural-network layer rather than a
bare matrix product, and so that the whole pragma study fits comfortably on the
board.

The numbers are kept for one reason: they are the only measurement we have of
**when ARRAY_PARTITION actually starts to pay off**, which cannot be observed at
4x4.

| size | configuration | unroll alone | + matching partition | gain |
|------|---------------|--------------|----------------------|------|
| 4x4  | unroll 4 (full)  | 129   | 129   | none |
| 8x8  | unroll 8 (full)  | 625   | 625   | none |
| 16x16| unroll 8         | 4641  | 3617  | 1.28x |
| 16x16| unroll 16 (full) | 3233  | 1585  | 2.04x |

Partitioning only helps once the unrolled loop asks for more concurrent memory
accesses than the block RAM can supply. Below that point it changes the
generated hardware and changes performance not at all — which is exactly the
lesson Task 3.2 is built around.

Not part of the assignment, not shown in the slides. Kept for the record and to
support the note in the Task 3.2 README.
