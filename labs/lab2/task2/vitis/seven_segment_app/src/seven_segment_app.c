#include "xparameters.h"
#include "xil_io.h"
#include <stdio.h>

// ---------------------------------------------------------------------------
// Lab 2.2 - processor side of the HLS calculator.
//
// The HLS block is free-running (ap_ctrl_none): it continuously refreshes the
// seven-segment display on its own. The processor therefore does not start it
// and does not wait for it to finish - it only writes the three operand
// registers, and the display follows on the next clock cycle.
//
// Register map, taken from the generated xseven_segment_axi_hw.h:
//     0x00 .. 0x0c   reserved (no ap_start: the block is free-running)
//     0x10           op1     (7 bits, 0..99)
//     0x18           op2     (7 bits, 0..99)
//     0x20           op_sel  (2 bits, operation)
// ---------------------------------------------------------------------------

#define CALC_BASEADDR    XPAR_SEVEN_SEGMENT_AXI_0_S_AXI_CTRL_BASEADDR

#define OP1_REG_OFFSET    0x10
#define OP2_REG_OFFSET    0x18
#define OPSEL_REG_OFFSET  0x20

#define OP_ADD 0
#define OP_SUB 1
#define OP_MUL 2
#define OP_DIV 3

// Reads one operand, rejecting anything outside the range the hardware
// accepts. The 7-bit register would silently truncate a larger number.
static unsigned int read_operand(const char *label)
{
    unsigned int value;

    for (;;) {
        printf("%s (0-99): ", label);

        if (scanf("%u", &value) != 1) {
            printf("\r\nnot a number, try again\r\n");
            while (getchar() != '\n') { }   // discard the rest of the line
            continue;
        }

        if (value <= 99) {
            return value;
        }

        printf("\r\nvalue must be 0..99, try again\r\n");
    }
}

static unsigned int read_operation(void)
{
    unsigned int op_sel;

    for (;;) {
        printf("Operation (0=add, 1=subtract, 2=multiply, 3=divide): ");

        if (scanf("%u", &op_sel) != 1) {
            printf("\r\nnot a number, try again\r\n");
            while (getchar() != '\n') { }
            continue;
        }

        if (op_sel <= OP_DIV) {
            return op_sel;
        }

        printf("\r\nunknown operation, try again\r\n");
    }
}

int main(void)
{
    printf("\r\n=== HLS seven-segment calculator ===\r\n");
    printf("The result appears on the four-digit display.\r\n");
    printf("Negative results show a leading minus sign;\r\n");
    printf("division by zero shows '----'.\r\n\r\n");

    while (1) {
        unsigned int op1    = read_operand("First number");
        unsigned int op2    = read_operand("Second number");
        unsigned int op_sel = read_operation();

        // Writing these three registers is the entire hardware interaction.
        // The HLS block picks the new values up on its next clock cycle and
        // keeps scanning the display without any further software help.
        Xil_Out32(CALC_BASEADDR + OP1_REG_OFFSET,   op1);
        Xil_Out32(CALC_BASEADDR + OP2_REG_OFFSET,   op2);
        Xil_Out32(CALC_BASEADDR + OPSEL_REG_OFFSET, op_sel);

        if (op_sel == OP_DIV && op2 == 0) {
            printf("\r\ndivision by zero - display shows '----'\r\n\r\n");
        } else {
            printf("\r\nresult is now on the display\r\n\r\n");
        }
    }

    return 0;
}
