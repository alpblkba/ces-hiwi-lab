#include "xparameters.h"
#include "xil_io.h"
#include "sleep.h"

#define SEVEN_SEGMENT_BASEADDR XPAR_SEVEN_SEGMENT_AXI_0_S_AXI_CTRL_BASEADDR
#define DIGIT_REG_OFFSET       0x10

int main(void)
{
    while (1) {
        for (int digit = 0; digit < 10; digit++) {
            Xil_Out32(SEVEN_SEGMENT_BASEADDR + DIGIT_REG_OFFSET, digit);
            sleep(1);
        }
    }

    return 0;
}
