| variant | configuration | latency (cycles) | II | BRAM | DSP | FF | LUT |
|---------|---------------|------------------|----|------|-----|----|----|
| V00 | baseline, no pragmas | 2057 | 2058 | 0 | 8 | 376 | 1264 |
| V01 | PIPELINE on prod (innermost) | 6401 | 6402 | 0 | 1 | 145 | 285 |
| V02 | PIPELINE on col | 2057 | 2058 | 0 | 8 | 373 | 1260 |
| V03 | UNROLL 2, no partition | 4353 | 4354 | 0 | 1 | 170 | 338 |
| V04 | UNROLL 4, no partition | 4609 | 4610 | 0 | 2 | 157 | 519 |
| V05 | UNROLL 8, no partition | 4865 | 4866 | 0 | 4 | 262 | 744 |
| V06 | UNROLL 16 (full), no partition | 269 | 270 | 0 | 128 | 7895 | 10710 |
| V07 | ARRAY_PARTITION 4 only, no unroll | 518 | 519 | 0 | 8 | 338 | 1041 |
| V08 | UNROLL 2 + PARTITION 2 | 4353 | 4354 | 0 | 1 | 177 | 318 |
| V09 | UNROLL 4 + PARTITION 4 | 3329 | 3330 | 0 | 2 | 223 | 378 |
| V10 | UNROLL 8 + PARTITION 8 | 2817 | 2818 | 0 | 4 | 256 | 510 |
| V11 | UNROLL 16 + PARTITION complete | 143 | 144 | 0 | 128 | 9092 | 10146 |
| V12 | PIPELINE col + UNROLL 4 + PARTITION 4 | 518 | 519 | 0 | 8 | 331 | 1037 |
