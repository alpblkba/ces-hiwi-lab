#include "seven_segment_axi.h"
#include <iostream>

// ---------------------------------------------------------------------------
// Golden reference, written independently of the design under test.
// ---------------------------------------------------------------------------
static ap_uint<7> golden_pattern(unsigned int digit) {
    switch (digit) {
        case 0: return 0x3F;
        case 1: return 0x06;
        case 2: return 0x5B;
        case 3: return 0x4F;
        case 4: return 0x66;
        case 5: return 0x6D;
        case 6: return 0x7D;
        case 7: return 0x07;
        case 8: return 0x7F;
        case 9: return 0x6F;
        default: return 0x00;
    }
}

static const ap_uint<7> GOLDEN_DASH  = 0x40;
static const ap_uint<7> GOLDEN_BLANK = 0x00;

static ap_uint<8> golden_active_low(ap_uint<7> pattern_active_high) {
    ap_uint<8> out;
    out.range(6, 0) = ~pattern_active_high;
    out[7] = 1;
    return out;
}

static void golden_display(int op1, int op2, unsigned int op_sel,
                           ap_uint<8> expected[SEVEN_SEG_DIGITS]) {
    int result = 0;
    bool error = false;

    switch (op_sel) {
        case OP_ADD: result = op1 + op2; break;
        case OP_SUB: result = op1 - op2; break;
        case OP_MUL: result = op1 * op2; break;
        default:
            if (op2 == 0) error = true;
            else result = op1 / op2;
            break;
    }

    bool negative = (result < 0);
    int magnitude = negative ? -result : result;
    if (magnitude > 9999 || (negative && magnitude > 999)) error = true;

    unsigned int msd_pos;
    if      (magnitude >= 1000) msd_pos = 3;
    else if (magnitude >= 100)  msd_pos = 2;
    else if (magnitude >= 10)   msd_pos = 1;
    else                        msd_pos = 0;

    unsigned int scale = 1;
    for (unsigned int p = 0; p < SEVEN_SEG_DIGITS; p++) {
        if (error) {
            expected[p] = golden_active_low(GOLDEN_DASH);
        } else if (p <= msd_pos) {
            expected[p] = golden_active_low(golden_pattern((magnitude / scale) % 10));
        } else if (negative && p == msd_pos + 1) {
            expected[p] = golden_active_low(GOLDEN_DASH);
        } else {
            expected[p] = golden_active_low(GOLDEN_BLANK);
        }
        scale *= 10;
    }
}

// Runs the design through one complete scan, recording what each digit
// position was driven with while its anode was active.
static bool observe_full_sweep(unsigned int op1, unsigned int op2, unsigned int op_sel,
                               ap_uint<8> seen[SEVEN_SEG_DIGITS]) {
    bool seen_valid[SEVEN_SEG_DIGITS] = {false, false, false, false};

    const unsigned long cycles =
        (unsigned long)SEVEN_SEG_DIGITS * SEVEN_SEG_REFRESH_TICKS + 16;

    for (unsigned long i = 0; i < cycles; i++) {
        ap_uint<8> seg = 0;
        ap_uint<4> an = 0;

        seven_segment_axi(op1, op2, op_sel, &seg, &an);

        int active = -1;
        int active_count = 0;
        for (int p = 0; p < (int)SEVEN_SEG_DIGITS; p++) {
            if (an[p] == 0) {
                active = p;
                active_count++;
            }
        }

        if (active_count != 1) {
            std::cout << "ERROR: " << active_count << " anodes active at once" << std::endl;
            return false;
        }

        seen[active] = seg;
        seen_valid[active] = true;
    }

    for (unsigned int p = 0; p < SEVEN_SEG_DIGITS; p++) {
        if (!seen_valid[p]) {
            std::cout << "ERROR: digit position " << p << " never enabled" << std::endl;
            return false;
        }
    }
    return true;
}

static const char *op_name(unsigned int op_sel) {
    switch (op_sel) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        default:     return "/";
    }
}

static bool check_case(unsigned int op1, unsigned int op2, unsigned int op_sel) {
    ap_uint<8> seen[SEVEN_SEG_DIGITS];
    ap_uint<8> expected[SEVEN_SEG_DIGITS];

    if (!observe_full_sweep(op1, op2, op_sel, seen)) {
        return false;
    }
    golden_display((int)op1, (int)op2, op_sel, expected);

    bool ok = true;
    for (unsigned int p = 0; p < SEVEN_SEG_DIGITS; p++) {
        if (seen[p] != expected[p]) {
            std::cout << op1 << " " << op_name(op_sel) << " " << op2
                      << ": position " << p
                      << " drove 0x" << std::hex << seen[p].to_uint()
                      << ", expected 0x" << expected[p].to_uint() << std::dec
                      << std::endl;
            ok = false;
        }
    }
    return ok;
}

int main() {
    // op1, op2, op_sel
    unsigned int cases[][3] = {
        {0,  0,  OP_ADD},   // 0        smallest result
        {40, 2,  OP_ADD},   // 42       two digits
        {99, 99, OP_ADD},   // 198      three digits, no overflow
        {9,  4,  OP_SUB},   // 5        single digit
        {5,  9,  OP_SUB},   // -4       negative, minus sign
        {0,  99, OP_SUB},   // -99      largest negative
        {99, 99, OP_MUL},   // 9801     uses all four digits
        {12, 0,  OP_MUL},   // 0
        {84, 4,  OP_DIV},   // 21
        {7,  0,  OP_DIV},   // error    division by zero -> "----"
    };

    bool failed = false;
    for (unsigned int i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        if (!check_case(cases[i][0], cases[i][1], cases[i][2])) {
            failed = true;
        }
    }

    if (failed) {
        std::cout << "seven_segment_axi test FAILED" << std::endl;
        return 1;
    }

    std::cout << "seven_segment_axi test passed" << std::endl;
    return 0;
}
