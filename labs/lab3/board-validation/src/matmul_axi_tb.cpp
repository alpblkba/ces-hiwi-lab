#include "matmul_axi.h"
#include <cstdio>

// Independent golden model in plain int arithmetic.
static unsigned golden(unsigned seed) {
    int A[N][N], B[N][N];
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            int a = (int)((seed + 7u * i + 3u * j) & 0xFF);
            int b = (int)((seed * 2u + 5u * i + 11u * j) & 0xFF);
            A[i][j] = (a > 127) ? a - 256 : a;      // interpret as signed 8-bit
            B[i][j] = (b > 127) ? b - 256 : b;
        }
    int sum = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            int acc = 0;
            for (int k = 0; k < N; k++) acc += A[i][k] * B[k][j];
            sum += acc;
        }
    return (unsigned)sum;
}

int main() {
    unsigned seeds[] = {1, 42, 7, 255, 1000};
    int errors = 0;

    for (unsigned s = 0; s < sizeof(seeds) / sizeof(seeds[0]); s++) {
        unsigned seed = seeds[s];
        ap_uint<32> c0 = 0, c1 = 0;
        matmul_axi(seed, 0, &c0);
        matmul_axi(seed, 1, &c1);
        unsigned g = golden(seed);

        printf("seed %-5u  baseline=0x%08X  optimised=0x%08X  golden=0x%08X",
               seed, (unsigned)c0, (unsigned)c1, g);

        if (c0 != g)  { printf("  <-- baseline WRONG");  errors++; }
        if (c1 != g)  { printf("  <-- optimised WRONG"); errors++; }
        if (c0 != c1) { printf("  <-- VARIANTS DIFFER"); errors++; }
        printf("\n");
    }

    if (errors) { printf("matmul_axi test FAILED (%d)\n", errors); return 1; }
    printf("matmul_axi test passed: both variants match the golden model\n");
    return 0;
}
