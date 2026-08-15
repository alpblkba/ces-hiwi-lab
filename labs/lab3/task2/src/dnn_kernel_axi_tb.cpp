// ---------------------------------------------------------------------------
// Testbench for the AXI-wrapped DNN kernel.
//
// The behaviour of the layer itself is already covered by the Task 3.1 and 3.2
// testbenches. What this one checks is everything the wrapper adds:
//
//   * the on-chip operand generator agrees with the plain-C one that the ARM
//     application uses -- if these two ever disagree, the board reports a
//     failure that has nothing to do with the pragmas
//   * reps = 0 behaves as reps = 1
//   * reps = 1 gives the checksum the board tests expect
//   * reps > 1 accumulates rather than repeating the same value, which is what
//     stops the tool from hoisting the loop and reading zero work in the
//     timing mode
//   * variant_id reports the build that was actually compiled
//
// Run it before building a bitstream. A wrong checksum found here costs
// seconds; found on the board it costs a synthesis run.
// ---------------------------------------------------------------------------

#include "dnn_kernel_axi.h"
#include <cstdio>

// --- golden model, plain int ------------------------------------------------
// Deliberately a separate implementation. This is also the exact code that
// dnn_app.c runs on the ARM core -- keep the three in step.
static void golden_gen(unsigned seed, int x[N][N], int W[N][N], int b[N]) {
    for (int i = 0; i < N; i++) {
        b[i] = (int)((seed + 31u * (unsigned)i) % 401u) - 200;
        for (int j = 0; j < N; j++) {
            unsigned a = (seed + 7u * (unsigned)i + 3u * (unsigned)j) & 0xFFu;
            unsigned w = (seed * 2u + 5u * (unsigned)i + 11u * (unsigned)j) & 0xFFu;
            x[i][j] = (a > 127u) ? (int)a - 256 : (int)a;
            W[i][j] = (w > 127u) ? (int)w - 256 : (int)w;
        }
    }
}

static int golden_layer(const int x[N][N], const int W[N][N], const int b[N]) {
    int s = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            int a = b[j];
            for (int k = 0; k < N; k++) a += x[i][k] * W[k][j];
            s += (a > 0) ? a : 0;
        }
    return s;
}

static unsigned golden_checksum(unsigned seed, unsigned reps) {
    int x[N][N], W[N][N], b[N];
    golden_gen(seed, x, W, b);
    if (reps == 0) reps = 1;
    const int b0 = b[0];
    int total = 0;
    for (unsigned r = 0; r < reps; r++) {
        b[0] = b0 + (int)r;
        total += golden_layer(x, W, b);
    }
    return (unsigned)total;
}

// --- helpers ----------------------------------------------------------------
static int check(const char *what, unsigned got, unsigned want) {
    if (got == want) {
        printf("  [pass] %-46s 0x%08X\n", what, got);
        return 0;
    }
    printf("  [FAIL] %-46s got 0x%08X, expected 0x%08X\n", what, got, want);
    return 1;
}

static unsigned call(unsigned seed, unsigned reps, unsigned *vid) {
    ap_uint<32> c = 0, v = 0;
    dnn_kernel_axi(seed, reps, &c, &v);
    if (vid) *vid = (unsigned)v;
    return (unsigned)c;
}

int main() {
    unsigned vid = 0;
    int err = 0;
    char label[96];

    call(1, 1, &vid);
    printf("dnn_kernel_axi testbench, built as VARIANT %u\n", vid);
    err += check("variant_id reports the compiled variant", vid, (unsigned)VARIANT);

    // The seeds the board tests use. 1000 clamps several outputs, so ReLU and
    // the bias are exercised rather than being dead code.
    static const unsigned seeds[] = { 1, 42, 1000, 7, 255 };
    printf("\n  functional, reps = 1 -- these are the values dnn_app.c expects\n");
    for (unsigned s = 0; s < sizeof(seeds) / sizeof(seeds[0]); s++) {
        snprintf(label, sizeof(label), "seed %-6u", seeds[s]);
        err += check(label, call(seeds[s], 1, 0), golden_checksum(seeds[s], 1));
    }

    printf("\n  wrapper behaviour\n");
    err += check("reps = 0 is treated as reps = 1",
                 call(1, 0, 0), golden_checksum(1, 1));

    // If the tool ever hoists the layer out of the rep loop, this collapses to
    // the reps = 1 value and the test says so.
    for (unsigned r = 2; r <= 8; r *= 2) {
        snprintf(label, sizeof(label), "reps = %-3u accumulates", r);
        err += check(label, call(1, r, 0), golden_checksum(1, r));
    }
    if (call(1, 4, 0) == call(1, 1, 0)) {
        printf("  [FAIL] %-46s reps has no effect on the result\n", "rep loop is real");
        err++;
    } else {
        printf("  [pass] %-46s\n", "rep loop is real, not hoisted");
    }

    if (err) {
        printf("\nFAILED (%d)\n", err);
        return 1;
    }
    printf("\ndnn_kernel_axi passed\n");
    return 0;
}
