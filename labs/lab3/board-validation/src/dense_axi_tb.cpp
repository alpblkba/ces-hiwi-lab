#include "dense_axi.h"
#include <cstdio>
static unsigned golden(unsigned seed) {
    int x[N][N], W[N][N], b[N];
    for (int i = 0; i < N; i++) {
        b[i] = (int)((seed + 31u*i) % 401u) - 200;
        for (int j = 0; j < N; j++) {
            int a=(int)((seed+7u*i+3u*j)&0xFF), w=(int)((seed*2u+5u*i+11u*j)&0xFF);
            x[i][j]=(a>127)?a-256:a;  W[i][j]=(w>127)?w-256:w;
        }
    }
    int s=0;
    for (int i=0;i<N;i++) for (int j=0;j<N;j++){
        int a=b[j]; for(int k=0;k<N;k++) a+=x[i][k]*W[k][j];
        s += (a>0)?a:0;
    }
    return (unsigned)s;
}
int main(){
    unsigned seeds[]={1,42,7,255,1000}; int err=0;
    for(unsigned s=0;s<5;s++){
        unsigned g=golden(seeds[s]); printf("seed %-5u golden=0x%08X  ",seeds[s],g);
        for(int v=0;v<5;v++){
            ap_uint<32> c=0; dense_axi(seeds[s],v,&c);
            printf("v%d=0x%08X ",v,(unsigned)c);
            if((unsigned)c!=g){printf("<WRONG> ");err++;}
        }
        printf("\n");
    }
    if(err){printf("FAILED (%d)\n",err);return 1;}
    printf("dense_axi passed: all 5 variants match the golden model\n");return 0;
}
