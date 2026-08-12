| variant | configuration | latency (cycles) | II | BRAM | DSP | FF | LUT |
|---------|---------------|------------------|----|------|-----|----|----|
| V00 | baseline, no pragmas | 21025 | 21026 | 0 | 1 | 75 | 198 |
| V01 | PIPELINE on prod (innermost) | 6401 | 6402 | 0 | 1 | 145 | 285 |
| V02 | PIPELINE on col | 2057 | 2058 | 0 | 8 | 373 | 1260 |
| V03 | UNROLL 2, no partition | 10785 | 10786 | 0 | 1 | 99 | 288 |
| V04 | UNROLL 4, no partition | 5665 | 5666 | 0 | 2 | 131 | 405 |
| V05 | UNROLL 8, no partition | 4641 | 4642 | 0 | 4 | 171 | 645 |
| V06 | UNROLL 16 (full), no partition | 3233 | 3234 | 0 | 8 | 688 | 1104 |
| V07 | ARRAY_PARTITION 4 only, no unroll | 21025 | 21026 | 0 | 1 | 81 | 222 |
| V08 | UNROLL 2 + PARTITION 2 | 10785 | 10786 | 0 | 1 | 95 | 240 |
| V09 | UNROLL 4 + PARTITION 4 | 5665 | 5666 | 0 | 2 | 111 | 306 |
| V10 | UNROLL 8 + PARTITION 8 | 3617 | 3618 | 0 | 4 | 158 | 443 |
| V11 | UNROLL 16 + PARTITION complete | 1585 | 1586 | 0 | 8 | 463 | 639 |
| V12 | PIPELINE col + UNROLL 4 + PARTITION 4 | 518 | 519 | 0 | 8 | 331 | 1037 |
