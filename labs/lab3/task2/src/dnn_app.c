// ---------------------------------------------------------------------------
// Lab 3.3 - driving the DNN kernel from the ARM core.
//
// The processor writes a seed, starts the accelerator, reads the checksum
// back, and then computes the same answer itself in plain C. The comparison
// happens ON THE BOARD and the board prints PASS or FAIL. No host tool, no
// script, nothing to trust except what is on the terminal.
//
// One bitstream carries one pragma variant. Program a different bitstream, run
// this same application unchanged, type the same seeds: the checksums must not
// move. That is the whole claim of Lab 3, demonstrated on silicon.
//
// Everything the accelerator exposes is an AXI4-Lite register. There is no
// display and no LED in this design - the terminal is the instrument.
// ---------------------------------------------------------------------------

#include <stdio.h>
#include "xparameters.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xtime_l.h"

// ---------------------------------------------------------------------------
// Where the accelerator lives.
//
// xparameters.h defines this from the block design. The fallback is only so
// the file still compiles if the IP was given a different instance name -
// check the Vivado address editor, or the build log line
//     ADDR_SEG: .../SEG_dnn_kernel_axi_0_Reg -> 0x40000000
// ---------------------------------------------------------------------------
#ifdef XPAR_DNN_KERNEL_AXI_0_S_AXI_CTRL_BASEADDR
#define KERNEL_BASE  XPAR_DNN_KERNEL_AXI_0_S_AXI_CTRL_BASEADDR
#else
#warning "XPAR_DNN_KERNEL_AXI_0_S_AXI_CTRL_BASEADDR not found, using 0x40000000"
#define KERNEL_BASE  0x40000000U
#endif

// ---------------------------------------------------------------------------
// Register map. Confirm against the generated xdnn_kernel_axi_hw.h in the HLS
// project - Vitis HLS assigns these offsets from the argument order, so adding
// or reordering an argument moves them.
//
//   0x00  ap_ctrl    bit0 ap_start (W), bit1 ap_done (R, clear-on-read),
//                    bit2 ap_idle (R)
//   0x10  seed       32-bit R/W
//   0x18  reps       32-bit R/W
//   0x20  checksum   32-bit R
//   0x28  variant_id 32-bit R
// ---------------------------------------------------------------------------
#define R_AP_CTRL     0x00
#define R_SEED        0x10
#define R_REPS        0x18
#define R_CHECKSUM    0x20
#define R_VARIANT     0x28

#define AP_START      0x1U
#define AP_DONE       0x2U

// The PL clock, used to turn a measured time into a cycle count. It is set by
// PCW_FPGA_FCLK0 in the block design.
#define PL_CLK_MHZ    100U

// Enough polls to cover a large reps value; a block that never answers is a
// hardware problem, not something to wait out.
#define POLL_LIMIT    20000000U

#define N 4

// ---------------------------------------------------------------------------
// Golden model.
//
// The same formula the fabric uses, in plain int C. These two implementations
// have to agree bit for bit; if the kernel's gen() is ever edited, edit this
// too, in the same commit.
// ---------------------------------------------------------------------------
static void golden_gen(u32 seed, int x[N][N], int W[N][N], int b[N])
{
    int i, j;
    for (i = 0; i < N; i++) {
        b[i] = (int)((seed + 31u * (u32)i) % 401u) - 200;
        for (j = 0; j < N; j++) {
            u32 a = (seed + 7u * (u32)i + 3u * (u32)j) & 0xFFu;
            u32 w = (seed * 2u + 5u * (u32)i + 11u * (u32)j) & 0xFFu;
            x[i][j] = (a > 127u) ? (int)a - 256 : (int)a;   // signed 8-bit
            W[i][j] = (w > 127u) ? (int)w - 256 : (int)w;
        }
    }
}

// Fills y as well as returning the checksum, so the menu can show the layer's
// actual output and not only the summary number.
static int golden_layer(const int x[N][N], const int W[N][N], const int b[N],
                        int y[N][N])
{
    int i, j, k, s = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            int acc = b[j];
            for (k = 0; k < N; k++) acc += x[i][k] * W[k][j];
            y[i][j] = (acc > 0) ? acc : 0;      // ReLU
            s += y[i][j];
        }
    }
    return s;
}

static u32 golden_checksum(u32 seed, u32 reps)
{
    int x[N][N], W[N][N], b[N], y[N][N];
    int b0, total = 0;
    u32 r;

    golden_gen(seed, x, W, b);
    if (reps == 0u) reps = 1u;
    b0 = b[0];
    for (r = 0; r < reps; r++) {
        b[0] = b0 + (int)r;                     // matches the kernel's rep loop
        total += golden_layer(x, W, b, y);
    }
    return (u32)total;
}

// ---------------------------------------------------------------------------
// Hardware access
// ---------------------------------------------------------------------------
static int kernel_run(u32 seed, u32 reps, u32 *checksum)
{
    u32 guard = 0;

    Xil_Out32(KERNEL_BASE + R_SEED, seed);
    Xil_Out32(KERNEL_BASE + R_REPS, reps);
    Xil_Out32(KERNEL_BASE + R_AP_CTRL, AP_START);

    // ap_done is clear-on-read, so the first read that sees it set is the one
    // that consumes it. Poll bit 1 only: ap_idle is still high in the first
    // cycles after the start and would end the loop before any work happened.
    while ((Xil_In32(KERNEL_BASE + R_AP_CTRL) & AP_DONE) == 0u) {
        if (++guard > POLL_LIMIT) return -1;
    }

    *checksum = Xil_In32(KERNEL_BASE + R_CHECKSUM);
    return 0;
}

static const char *variant_name(u32 id)
{
    switch (id) {
    case 0: return "baseline, no pragmas";
    case 1: return "PIPELINE on the neuron loop";
    case 2: return "UNROLL the product loop";
    case 3: return "ARRAY_PARTITION alone";
    case 4: return "all three together";
    default: return "unknown - is this bitstream from this lab?";
    }
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------
static u32 read_u32(const char *prompt, u32 fallback)
{
    unsigned int v;
    int c;

    printf("%s [%u]: ", prompt, fallback);
    if (scanf("%u", &v) != 1) {
        while ((c = getchar()) != '\n' && c != EOF) { }   // drop the bad line
        printf("\r\nnot a number, using %u\r\n", fallback);
        return fallback;
    }
    return (u32)v;
}

// ---------------------------------------------------------------------------
// Menu actions
// ---------------------------------------------------------------------------

// The seeds the board tests have always used, with the checksums recorded the
// first time this design ran on hardware. Printing all three columns means a
// drift between the fabric and the software model is visible immediately, and
// so is a drift between today's software model and the recorded history.
static const u32 REF_SEED[]     = { 1u, 42u, 1000u, 7u };
static const u32 REF_CHECKSUM[] = { 0x00006328u, 0x00050AB8u, 0x00003E6Bu, 0x0000CC88u };
#define REF_COUNT  (sizeof(REF_SEED) / sizeof(REF_SEED[0]))

static void act_selftest(void)
{
    unsigned i;
    int failures = 0;

    printf("\r\n  seed     hardware     on-board model   recorded     result\r\n");
    printf("  ------------------------------------------------------------\r\n");

    for (i = 0; i < REF_COUNT; i++) {
        u32 hw = 0;
        u32 sw = golden_checksum(REF_SEED[i], 1u);
        int ok;

        if (kernel_run(REF_SEED[i], 1u, &hw) != 0) {
            printf("  %-8u  TIMED OUT - the accelerator never raised ap_done\r\n",
                   (unsigned)REF_SEED[i]);
            failures++;
            continue;
        }

        ok = (hw == sw) && (hw == REF_CHECKSUM[i]);
        printf("  %-8u 0x%08X   0x%08X       0x%08X   %s\r\n",
               (unsigned)REF_SEED[i], (unsigned)hw, (unsigned)sw,
               (unsigned)REF_CHECKSUM[i], ok ? "PASS" : "FAIL");
        if (!ok) failures++;
    }

    // A block that latched one answer and returns it for ever would pass every
    // line above only if every line expected the same value. They do not, so
    // this is already covered - but say so explicitly, because it is the
    // question a reviewer asks first.
    printf("\r\n  %u seed(s), %u failure(s). The four expected values differ from\r\n",
           (unsigned)REF_COUNT, (unsigned)failures);
    printf("  each other, so a block returning a stale register cannot pass.\r\n");
    printf("\r\n  %s\r\n", failures ? "*** SELF-TEST FAILED ***" : "*** SELF-TEST PASSED ***");
}

static void act_one_seed(void)
{
    u32 seed = read_u32("seed", 1u);
    u32 hw = 0;
    u32 sw = golden_checksum(seed, 1u);

    if (kernel_run(seed, 1u, &hw) != 0) {
        printf("\r\n  TIMED OUT - the accelerator never raised ap_done.\r\n");
        printf("  Is the PL programmed with a Lab 3 bitstream?\r\n");
        return;
    }
    printf("\r\n  hardware       0x%08X\r\n", (unsigned)hw);
    printf("  ARM golden     0x%08X\r\n", (unsigned)sw);
    printf("  %s\r\n", (hw == sw) ? "PASS" : "FAIL - the fabric and the software disagree");
}

static void act_show_operands(void)
{
    int x[N][N], W[N][N], b[N], y[N][N];
    u32 seed = read_u32("seed", 1u);
    u32 hw = 0;
    int sw, i, j;

    golden_gen(seed, x, W, b);
    sw = golden_layer(x, W, b, y);

    printf("\r\n  The accelerator builds these on chip from the seed. The same\r\n");
    printf("  numbers are what the ARM used for its own answer.\r\n\r\n");

    printf("  x (samples x features)          W (inputs x neurons)\r\n");
    for (i = 0; i < N; i++) {
        printf("   ");
        for (j = 0; j < N; j++) printf(" %5d", x[i][j]);
        printf("        ");
        for (j = 0; j < N; j++) printf(" %5d", W[i][j]);
        printf("\r\n");
    }

    printf("\r\n  bias   ");
    for (j = 0; j < N; j++) printf(" %7d", b[j]);

    printf("\r\n\r\n  y = ReLU(x*W + bias)\r\n");
    for (i = 0; i < N; i++) {
        printf("   ");
        for (j = 0; j < N; j++) printf(" %8d", y[i][j]);
        printf("\r\n");
    }

    printf("\r\n  sum of y  0x%08X   <- this is the checksum\r\n", (unsigned)sw);

    if (kernel_run(seed, 1u, &hw) == 0) {
        printf("  hardware  0x%08X   %s\r\n", (unsigned)hw,
               (hw == (u32)sw) ? "PASS" : "FAIL");
    } else {
        printf("  hardware  TIMED OUT\r\n");
    }
}

static void act_timing(void)
{
    u32 seed = read_u32("seed", 1u);
    u32 reps = read_u32("repetitions (large enough that AXI stops dominating)", 100000u);
    u32 checksum = 0;
    XTime t0, t1;
    u64 dt, ns, cyc_x100;

    if (reps == 0u) reps = 1u;

    XTime_GetTime(&t0);
    if (kernel_run(seed, reps, &checksum) != 0) {
        printf("\r\n  TIMED OUT.\r\n");
        return;
    }
    XTime_GetTime(&t1);

    dt = (u64)(t1 - t0);
    ns = (dt * 1000000000ULL) / COUNTS_PER_SECOND;

    // total PL cycles = ns * MHz / 1000; keep two decimals for the per-call
    // figure by scaling before the division.
    cyc_x100 = (ns * (u64)PL_CLK_MHZ) / (10ULL * (u64)reps);

    printf("\r\n  %u repetitions in %u us\r\n",
           (unsigned)reps, (unsigned)(ns / 1000ULL));
    printf("  %u.%02u PL cycles per layer evaluation at %u MHz\r\n",
           (unsigned)(cyc_x100 / 100ULL), (unsigned)(cyc_x100 % 100ULL),
           (unsigned)PL_CLK_MHZ);
    printf("\r\n  This includes the AXI handshake once, spread over %u calls,\r\n",
           (unsigned)reps);
    printf("  so the figure approaches the loop latency as reps grows. Compare\r\n");
    printf("  it against the latency in the C-synthesis report, and against the\r\n");
    printf("  same measurement on a bitstream built with different pragmas.\r\n");
}

static void act_axi_alive(void)
{
    const u32 pattern = 0x12345678u;
    u32 back;

    // seed is a plain read/write register, so this exercises the AXI path and
    // the block's clock without involving the layer at all. If it fails, every
    // checksum is meaningless and the cause is upstream: no PL clock, the
    // wrong base address, or an unprogrammed fabric.
    Xil_Out32(KERNEL_BASE + R_SEED, pattern);
    back = Xil_In32(KERNEL_BASE + R_SEED);

    printf("\r\n  base address   0x%08X\r\n", (unsigned)KERNEL_BASE);
    printf("  wrote          0x%08X\r\n", (unsigned)pattern);
    printf("  read back      0x%08X   %s\r\n", (unsigned)back,
           (back == pattern) ? "PASS" : "FAIL");

    if (back != pattern) {
        printf("\r\n  Nothing below this point can be trusted. Check that the\r\n");
        printf("  bitstream is programmed, that ps7_init ran, and that the base\r\n");
        printf("  address matches the Vivado address editor.\r\n");
    }
}

int main(void)
{
    u32 vid;
    char choice;
    int c;

    printf("\r\n\r\n=== Lab 3.3 - DNN kernel on the PL ===\r\n");

    vid = Xil_In32(KERNEL_BASE + R_VARIANT);
    printf("bitstream variant : %u  (%s)\r\n", (unsigned)vid, variant_name(vid));
    printf("base address      : 0x%08X\r\n", (unsigned)KERNEL_BASE);
    printf("kernel            : y = ReLU(x*W + bias), %dx%d, int8 in, int32 acc\r\n",
           N, N);

    for (;;) {
        printf("\r\n"
               "  1  self-test, the four reference seeds\r\n"
               "  2  run one seed you choose\r\n"
               "  3  show the operands and the expected output\r\n"
               "  4  timing, cycles per layer evaluation\r\n"
               "  5  AXI alive check\r\n"
               "  q  quit\r\n"
               "> ");

        if (scanf(" %c", &choice) != 1) {
            while ((c = getchar()) != '\n' && c != EOF) { }
            continue;
        }

        switch (choice) {
        case '1': act_selftest();       break;
        case '2': act_one_seed();       break;
        case '3': act_show_operands();  break;
        case '4': act_timing();         break;
        case '5': act_axi_alive();      break;
        case 'q': case 'Q':
            printf("\r\nbye\r\n");
            return 0;
        default:
            printf("\r\nunknown choice\r\n");
            break;
        }
    }
}
