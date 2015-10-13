#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* This C short program preprocesses the file DR005893.F01
 * to remove all 2-Byte headers peculiar to IBM 360 data
 * representation. Recall that IBM 360 organise data into
 * blocks of variable size. Each header contains the block
 * size */


int main(int argc, char **argv) {
    assert(sizeof(unsigned short)==2);

    unsigned short blocksize, j, prevblocksize=20004u;

    if(argc<2)  { perror("File name expected"); exit(-1); }

    FILE* f = fopen(argv[1], "r");
    if(!f)  { perror("unable to open file"); exit(-1); }

    FILE* f2 = fopen("without_header", "w");
    if(!f2)  { perror("unable to create new file"); exit(-1); }


    while(!feof(f)) {
        fread(&blocksize, 2, 1, f);
        blocksize = ntohs(blocksize);
        printf("At %X: Block size: %hu\n", ftell(f), blocksize);
        j = 2;
        while(!feof(f) && j< (prevblocksize==20004u? 8u:4u)) {
            printf("%X, ", fgetc(f));
            j++;
        } printf(" removed\n");

        while(!feof(f) && j<blocksize) {
            fputc(fgetc(f), f2);
            j++;
        }

        printf("'%X' removed\n", !feof(f)? fgetc(f) : 0);   // remove 0A
        prevblocksize = blocksize;
    }
    fclose(f); fclose(f2);
    printf("Preprocessing ended. Output file DR005893.F02 created.\n");
    return EXIT_SUCCESS;
    }
