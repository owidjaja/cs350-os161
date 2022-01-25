#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

// https://stackoverflow.com/questions/1787996/c-library-function-to-perform-sort
int compare_function(const void *a,const void *b) {
    int *x = (int *) a;
    int *y = (int *) b;
    return *x - *y;
}

int main(){

    FILE *fpr, *fpw;

    fpr = fopen("./log.txt", "r");
    if (fpr==NULL){
        printf("Failed to open file\n");
        return 0;
    }

    //https://stackoverflow.com/questions/14001907/read-data-from-file-till-end-of-line-in-c-c

    char buffer[10000];
    char *pbuff;
    int value;
    
    if (!fgets(buffer, sizeof buffer, fpr)) return 0;
    pbuff = buffer;
    int n = strtol(pbuff, &pbuff, 10);

    int numbers[n];
    for (int i=0; i<n; i++){
        if (!fgets(buffer, sizeof buffer, fpr)) break;
        pbuff = buffer;
        numbers[i] = strtol(pbuff, &pbuff, 10);
    }
    
    // for (int i=0; i<n; i++){
    //     printf("numbers[%d] = %d\n", i,numbers[i]);
    // }

    fclose(fpr);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    // https://stackoverflow.com/questions/1787996/c-library-function-to-perform-sort
    // qsort(<arrayname>,<size>,sizeof(<elementsize>),compare_function);
    qsort(numbers, n, sizeof(*numbers), compare_function);

    gettimeofday(&end, NULL);
    
    // for (int i=0; i<n; i++){
    //     printf("numbers[%d] = %d\n", i,numbers[i]);
    // }
    
    printf("Time taken for sort: %ld micro seconds\n",
    ((end.tv_sec * 1000000 + end.tv_usec) -
    (start.tv_sec * 1000000 + start.tv_usec)));

    fpw = fopen("./sorted.txt", "w");
    if (fpr==NULL){
        printf("Failed to open file\n");
        return 0;
    }

    fprintf(fpw, "%d\n", n);
    for (int i=0; i<n; i++){
        fprintf(fpw, "%d\n", numbers[i]);
    }

    fclose(fpw);
    fpw = NULL;
    
    return 0;
}