// ---------------------------------------------------------------------------
// Testbench for the DNN kernel, Task 3.2.
//
// Identical to the Task 3.1 testbench except for the variant banner: the
// pragmas must not change a single output value, so the same cases apply
// unchanged to all five builds.
//
// This checks BEHAVIOUR, not synthesis. Every case builds a 4x4 input by hand,
// runs the kernel, and compares against a golden model written in plain int C
// with no HLS types. If the two disagree the kernel is wrong, whatever the
// synthesis report says.
//
// The cases are chosen to be the ones that actually break: the ReLU boundary,
// the sign of an 8-bit operand, the width of the accumulator, and whether the
// two matrix indices got swapped. A random input finds none of these reliably.
//
// Usage (plain g++):
//     ./dnn_test                  every case
//     ./dnn_test list             show the case names
//     ./dnn_test relu-edge        one case (any number of names)
//     ./dnn_test random 42        pseudo-random input with that seed
//
// Usage (Vitis HLS): pass the same words through csim_design, e.g.
//     csim_design -argv {relu-edge int8-extremes}
// ---------------------------------------------------------------------------

#include "dnn_task2.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

// --- golden model -----------------------------------------------------------
// Deliberately written in plain int, with no ap_int and no shared code with the
// kernel. Two implementations of the same formula that were written twice.
static void golden(const int x[N][N], const int W[N][N], const int b[N],
                   int y[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int acc = b[j];
            for (int k = 0; k < N; k++) {
                acc += x[i][k] * W[k][j];
            }
            y[i][j] = (acc > 0) ? acc : 0;
        }
    }
}

// --- case builders ----------------------------------------------------------
// Each one fills x, W and bias. Operands must fit in 8 bits; the runner checks
// that, so a case that is itself wrong is reported as a bad case and not as a
// kernel failure.

static void fill(int m[N][N], int v) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) m[i][j] = v;
}

// everything zero: the output must be exactly zero, not "small"
static void c_zeros(int x[N][N], int W[N][N], int b[N]) {
    fill(x, 0); fill(W, 0);
    for (int j = 0; j < N; j++) b[j] = 0;
}

// x = 0, so every product vanishes and y is ReLU(bias) alone. Catches a kernel
// that initialises the accumulator to zero and adds the bias afterwards, or
// forgets the bias entirely.
static void c_bias_only(int x[N][N], int W[N][N], int b[N]) {
    fill(x, 0); fill(W, 0);
    b[0] = -5; b[1] = 0; b[2] = 7; b[3] = -1;
}

// every accumulator lands far below zero: the whole output must be clamped
static void c_relu_clamp(int x[N][N], int W[N][N], int b[N]) {
    fill(x, 100); fill(W, -100);
    for (int j = 0; j < N; j++) b[j] = -1000;
}

// nothing is clamped: ReLU must be transparent here
static void c_relu_pass(int x[N][N], int W[N][N], int b[N]) {
    fill(x, 100); fill(W, 100);
    for (int j = 0; j < N; j++) b[j] = 1000;
}

// the boundary itself. Only x[i][0] and W[0][j] survive, both zero, so the
// accumulator for neuron j is exactly b[j]: -1, 0, +1 and far negative.
// Note that `acc > 0` and `acc >= 0` are the same function here -- ReLU(0) is
// 0 either way -- so this case does not, and cannot, distinguish them. What it
// does catch is a clamp that lets a small negative value through, and a
// truncated accumulator that turns -1 into something positive.
static void c_relu_edge(int x[N][N], int W[N][N], int b[N]) {
    fill(x, 0); fill(W, 0);
    for (int i = 0; i < N; i++) x[i][0] = 1;
    b[0] = -1; b[1] = 0; b[2] = 1; b[3] = -1000;
}

// W = identity, bias = 0, so y = ReLU(x) element by element. A quick sanity
// check that says nothing about index order -- the identity is symmetric.
static void c_identity(int x[N][N], int W[N][N], int b[N]) {
    fill(W, 0);
    for (int k = 0; k < N; k++) W[k][k] = 1;
    for (int j = 0; j < N; j++) b[j] = 0;
    int v[N][N] = { { 5, -5, 120, -120 },
                    { -1, 1, 0, 64 },
                    { 127, -128, 7, -7 },
                    { 33, -33, 99, -99 } };
    memcpy(x, v, sizeof(v));
}

// W is a shift matrix, which is NOT symmetric, so it fails if x[i][k]*W[k][j]
// was written as x[k][i]*W[j][k] or the two dimensions were swapped anywhere.
// Expected: neuron 0 sees nothing, neuron j sees feature j-1.
static void c_index_order(int x[N][N], int W[N][N], int b[N]) {
    fill(W, 0);
    for (int k = 0; k + 1 < N; k++) W[k][k + 1] = 1;
    for (int j = 0; j < N; j++) b[j] = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) x[i][j] = 10 * (j + 1);
}

// the corners of the 8-bit range. -128 * -128 = +16384, and four of those plus
// the bias is the largest magnitude the accumulator ever has to hold. Catches
// an operand that was declared unsigned and a narrowed accumulator at once.
static void c_int8_extremes(int x[N][N], int W[N][N], int b[N]) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            x[i][j] = ((i + j) & 1) ? 127 : -128;
            W[i][j] = ((i + j) & 1) ? -128 : 127;
        }
    b[0] = 0; b[1] = 32767; b[2] = -32768; b[3] = 1;
}

// a bias that does not fit in 8 bits. If bias or y was declared data_t instead
// of acc_t this truncates and the case fails.
static void c_wide_bias(int x[N][N], int W[N][N], int b[N]) {
    fill(x, 1); fill(W, 1);
    b[0] = 100000; b[1] = -100000; b[2] = 65536; b[3] = -1;
}

// every sample row is a different constant and W is the identity, so row i of
// the output depends only on row i of the input. Fails if unrolling or array
// partitioning ever lets one sample read another's data.
static void c_row_independence(int x[N][N], int W[N][N], int b[N]) {
    fill(W, 0);
    for (int k = 0; k < N; k++) W[k][k] = 1;
    for (int j = 0; j < N; j++) b[j] = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) x[i][j] = 10 * (i + 1);
}

// every neuron has a different weight column, so each output column is a
// different value. The mirror image of the case above, for the j index.
static void c_neuron_independence(int x[N][N], int W[N][N], int b[N]) {
    fill(x, 1);
    for (int k = 0; k < N; k++)
        for (int j = 0; j < N; j++) W[k][j] = j + 1;
    for (int j = 0; j < N; j++) b[j] = 0;
}

// --- pseudo-random case -----------------------------------------------------
// Kept because it exercises combinations no hand-written case covers. It is
// the weakest test here, not the strongest.
static unsigned rnd_seed = 1;
static int rnd(int lo, int hi) {
    rnd_seed = rnd_seed * 1103515245u + 12345u;
    return lo + (int)((rnd_seed >> 16) % (unsigned)(hi - lo + 1));
}
static void c_random(int x[N][N], int W[N][N], int b[N]) {
    for (int i = 0; i < N; i++) {
        b[i] = rnd(-1000, 1000);
        for (int j = 0; j < N; j++) {
            x[i][j] = rnd(-128, 127);
            W[i][j] = rnd(-128, 127);
        }
    }
}

// --- runner -----------------------------------------------------------------

typedef void (*builder)(int x[N][N], int W[N][N], int b[N]);

struct testcase {
    const char *name;
    const char *what;
    builder     make;
};

static const testcase CASES[] = {
    { "zeros",                "everything zero, output must be exactly zero",   c_zeros },
    { "bias-only",            "x = 0, so y is ReLU(bias) alone",                c_bias_only },
    { "relu-clamp",           "every accumulator negative, all outputs clamped", c_relu_clamp },
    { "relu-pass",            "every accumulator positive, ReLU transparent",   c_relu_pass },
    { "relu-edge",            "accumulators of exactly -1, 0 and +1",           c_relu_edge },
    { "identity",             "W = I, so y = ReLU(x)",                          c_identity },
    { "index-order",          "non-symmetric W, catches swapped indices",       c_index_order },
    { "int8-extremes",        "-128 and +127 operands, widest accumulator",     c_int8_extremes },
    { "wide-bias",            "bias far outside 8 bits",                        c_wide_bias },
    { "row-independence",     "each sample row distinct",                       c_row_independence },
    { "neuron-independence",  "each weight column distinct",                    c_neuron_independence },
    { "random",              "pseudo-random operands over the full range",     c_random },
};
static const int NCASES = (int)(sizeof(CASES) / sizeof(CASES[0]));

static int operands_in_range(const int m[N][N]) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (m[i][j] < -128 || m[i][j] > 127) return 0;
    return 1;
}

static void dump(const char *label, const int m[N][N]) {
    printf("      %s\n", label);
    for (int i = 0; i < N; i++) {
        printf("       ");
        for (int j = 0; j < N; j++) printf(" %7d", m[i][j]);
        printf("\n");
    }
}

// returns 0 on pass
static int run_case(const testcase &tc) {
    int xi[N][N], wi[N][N], bi[N], yg[N][N];
    tc.make(xi, wi, bi);

    if (!operands_in_range(xi) || !operands_in_range(wi)) {
        printf("  [BAD CASE] %-20s operands do not fit in 8 bits\n", tc.name);
        return 1;
    }

    data_t X[N][N], Wq[N][N];
    acc_t  B[N], Y[N][N];
    for (int i = 0; i < N; i++) {
        B[i] = (acc_t)bi[i];
        for (int j = 0; j < N; j++) {
            X[i][j]  = (data_t)xi[i][j];
            Wq[i][j] = (data_t)wi[i][j];
        }
    }

    dnn_kernel(X, Wq, B, Y);
    golden(xi, wi, bi, yg);

    int bad = 0, clamped = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            if (yg[i][j] == 0) clamped++;
            if ((int)Y[i][j] != yg[i][j]) bad++;
        }

    if (bad == 0) {
        printf("  [pass] %-20s %s  (%d/%d clamped by ReLU)\n",
               tc.name, tc.what, clamped, N * N);
        return 0;
    }

    printf("  [FAIL] %-20s %s  (%d of %d outputs wrong)\n",
           tc.name, tc.what, bad, N * N);
    dump("x", xi);
    dump("W", wi);
    printf("      bias\n       ");
    for (int j = 0; j < N; j++) printf(" %7d", bi[j]);
    printf("\n      got / expected\n");
    for (int i = 0; i < N; i++) {
        printf("       ");
        for (int j = 0; j < N; j++) {
            printf(" %7d%s", (int)Y[i][j],
                   ((int)Y[i][j] == yg[i][j]) ? " " : "*");
        }
        printf("   |");
        for (int j = 0; j < N; j++) printf(" %7d", yg[i][j]);
        printf("\n");
    }
    return 1;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "list") == 0) {
        for (int i = 0; i < NCASES; i++)
            printf("  %-20s %s\n", CASES[i].name, CASES[i].what);
        return 0;
    }

    // The whole claim of Task 3.2 is that pragmas do not change the result.
    // Printing the variant makes it obvious which build produced the output.
    printf("dnn_kernel behaviour test, N = %d, variant %s\n", N, variant_name());

    int failures = 0, ran = 0;

    if (argc > 1) {
        for (int a = 1; a < argc; a++) {
            const char *name = argv[a];
            // "random 42" is two words: consume the seed here so it is not
            // mistaken for a case name on the next pass.
            if (strcmp(name, "random") == 0 && a + 1 < argc) {
                rnd_seed = (unsigned)atoi(argv[a + 1]);
                a++;
                printf("  (random seed %u)\n", rnd_seed);
            }
            int found = 0;
            for (int i = 0; i < NCASES; i++) {
                if (strcmp(name, CASES[i].name) == 0) {
                    failures += run_case(CASES[i]);
                    ran++;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                printf("  [BAD CASE] no case named '%s' -- try 'list'\n", name);
                failures++;
            }
        }
    } else {
        for (int i = 0; i < NCASES; i++) {
            failures += run_case(CASES[i]);
            ran++;
        }
    }

    if (failures) {
        printf("FAILED: %d of %d case(s)\n", failures, ran);
        return 1;
    }
    printf("all %d case(s) passed\n", ran);
    return 0;
}
