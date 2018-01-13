#include "syscall.h"
int A[10] = {1,2,3,4,5,6,7,8,9,0};
int B[10];
int summation;
main()
    {
        int    n;
        for (n=20;n>5;n--){
            PrintInt(n);
        }
        for (n=0; n<10; n++){
            PrintInt(n);
        }
        
        for (n=0; n<10; n++){
            B[n] = n*n;
        }

        summation = 0;
        for (n=0; n<10; n++){
            summation += B[n];
        }
        PrintInt(summation);
    }
