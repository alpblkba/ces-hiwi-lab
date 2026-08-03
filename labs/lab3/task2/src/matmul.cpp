#include "matmul.h"

// ---------------------------------------------------------------------------
// Baseline dense layer: C = A x B
//
// This is the Task 1 reference. It contains NO optimisation pragmas at all -
// no PIPELINE, no UNROLL, no ARRAY_PARTITION. Every figure produced in Task 2
// is compared against the numbers this version reports.
//
// The three loops are labelled so that pragmas can be attached to a specific
// loop later on. Labels have no effect on the generated hardware; they only
// give the loops a name in the reports and in the directives file.
// ---------------------------------------------------------------------------
void matmul(const data_t A[N][N], const data_t B[N][N], acc_t C[N][N]) {
row:
    for (int i = 0; i < N; i++) {
    col:
        for (int j = 0; j < N; j++) {
            acc_t acc = 0;
        prod:
            for (int k = 0; k < N; k++) {
                acc += (acc_t)A[i][k] * (acc_t)B[k][j];
            }
            C[i][j] = acc;
        }
    }
}
