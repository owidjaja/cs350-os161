#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

int main(){
    int size = 5;

    FILE *fp;

    fp = fopen("/dev/random", "r");
    // fp = fopen("./log", "r");
    if (fp == NULL)
    {
        // something went wrong
        printf("ERROR: failed to read file\n");
        return -1;
    }
    else
    {
        printf("copying\n");
        int byte_count = 64;
        char data[64];
        fread(&data, 1, byte_count, fp);
        fclose(fp);

        printf("data:%s\n", data);
    }
}