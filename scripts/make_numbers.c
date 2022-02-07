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

int* read_urandom(int size, int lo, int hi){
    FILE *fp;

    fp = fopen("/dev/urandom", "r");
    if (fp == NULL)
    {
        // something went wrong
        printf("ERROR: failed to read file\n");
        return NULL;
    }
    else
    {
        // printf("copying\n");
        int byte_count = 64;
        char data[64];
        fread(&data, 1, byte_count, fp);
        
        fclose(fp);
        fp = NULL;

        // printf("data: %s\n", data);

        // for (int i=0; i<size; i++){
        //     printf("rand data: %d\n", data[i]);
        // }
        // printf("\n");

        // printf("lo=%d, hi=%d\n\n", lo,hi);
        // Min + (int)(Math.random() * ((Max - Min) + 1))
        
        int *retval = malloc(size);
        for (int i=0; i<size; i++){
            // printf("rand data: %d\n", data[i]);

            int inter = (data[i] % ((hi-lo)+1));
            // printf("bef inter: %d\n", inter);
            if (inter<0) {
                // printf("adjust\n");
                inter += ((hi-lo)+1);
                }    
            // printf("inter: %d\n", inter);

            retval[i] = lo + inter;
            // printf("rand num: %d\n", retval[i]);
            // printf("\n");
        }
        return retval;
    }
}


// argc: argument count
// argv: argument vectors
int main(int argc, char **argv){
    if (argc != 4) {
        printf("Usage: need 3 parameters: n, lo, hi.\n");
        return 0; 
        }

    int n, lo, hi;
    if (isNumber(argv[1]) && isNumber(argv[2]) && isNumber(argv[3])){
        n  = atoi(argv[1]);
        lo = atoi(argv[2]);
        hi = atoi(argv[3]);
    }
    else{
        // printf("INPUT ERROR: parameters not int data type.\n");
        return 1;   // input error retval
    }

    if (n<0 || n>INT_MAX || lo<INT_MIN || lo>INT_MAX || hi<INT_MIN || hi>INT_MAX){
        // printf("INPUT ERROR: parameter value exceeded bounds.\n");
        return 1;   // input error retval
    }

    // printf("n=%d, lo=%d, hi=%d\n",n,lo,hi);

    // printf("now working on a0 details.\n");
    // printf("here");
    if (lo>hi) {
        // printf("here2");
        printf("lo>hi, nothing written\n");
        return 1;
    }

    FILE *fp;

    fp = fopen("./log.txt", "w");               // overwrite existing data
    if (fp==NULL){
        // printf("Failed to open file\n");
        return 0;
    }

    fprintf(fp, "%d\n", n);

    if (lo==hi){
        // printf("hi==lo\n");
        for (int i=0; i<n; i++){
            fprintf(fp, "\n%d", lo);
        }

        if (fp) fclose(fp);
        fp = NULL;

        return 0;
    }

    // else lo < hi, generate random numbers
    if (lo<hi){
        int *rand_num = read_urandom(n, lo, hi);
        if (rand_num == NULL){
            // failed
            return 0;
        }
        else{
            for (int i=0; i<n; i++){
                // printf("rand num: %d\n", rand_num[i]);
            }
            
            for (int i=0; i<n; i++){
                fprintf(fp, "%d\n", rand_num[i]);
            }

            free(rand_num);
            if (fp) fclose(fp);
            fp = NULL;
            return 0;
        }
    }
}