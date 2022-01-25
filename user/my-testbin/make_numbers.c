#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

bool isNumber(char number[])
{
    int i = 0;

    //checking for negative numbers
    if (number[0] == '-')
        i = 1;
    for (; number[i] != 0; i++)
    {
        //if (number[i] > '9' || number[i] < '0')
        if (!isdigit(number[i]))
            return false;
    }
    return true;
}

// argc: argument count
// argv: argument vectors
int main(int argc, char **argv){
    if (argc != 4) {
        printf("Usage: need 3 parameters: n, lo, hi.\n");
        return 0; 
        }

    int n, lo, hi;

    if (isNumber(argv[1])){
        printf("Works\n");
    }
    
    if (isNumber(argv[1]) && isNumber(argv[2]) && isNumber(argv[3])){
        n  = atoi(argv[1]);
        lo = atoi(argv[2]);
        hi = atoi(argv[3]);
    }
    else{
        printf("INPUT ERROR: parameters not int data type.\n");
        return 1;   // input error retval
    }

    if (n<0 || n>INT_MAX || lo<INT_MIN || lo>INT_MAX || hi<INT_MIN || hi>INT_MAX){
        printf("INPUT ERROR: parameter value exceeded bounds.\n");
        return 1;   // input error retval
    }

    printf("n=%d, lo=%d, hi=%d\n",n,lo,hi);

    printf("now working on a0 details.\n");

    if (hi<lo) {
        printf("hi<lo, nothing written\n");
        return 0;
    }

    FILE *fp;

    fp = fopen("./log.txt", "w");               // overwrite existing data
    if (fp==NULL){
        printf("Failed to open file\n");
        return 0;
    }

    fprintf(fp, "%d", n);

    if (hi==lo){
        printf("hi==lo\n");
        for (int i=0; i<n; i++){
            fprintf(fp, "\n%d", lo);
        }

        if (fp) fclose(fp);
        fp = NULL;

        return 0;
    }

    // else lo < hi, generate random numbers

    

    return 0;
}


ssize_t get_random(int size){
    int randomData = open("/dev/urandom", 0);
    if (randomData < 0)
    {
        // something went wrong
    }
    else
    {
        char myRandomData[size];
        ssize_t result = read(randomData, myRandomData, sizeof myRandomData);
        if (result < 0)
        {
            // something went wrong
        }
    }
}