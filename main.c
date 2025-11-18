#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"

int main(int argc, char **argv){

    if(argc == 1){
        printf("Please indicate files to assemble.\n");
        exit(1);
    }

    for(int i = 1; i < argc; i++){
        int len = strlen(argv[i]);

        if(
            strcmp(argv[i] + len - 4, ".sic") != 0
            || len < 4
        ){
            printf("Unexpected argument \"%s\" found. Terminating program.\n", argv[i]);
            exit(2);
        }
    }

    for(int i = 1; i < argc; i++){
        FILE *file = fopen(argv[i], "r");
        FILE *out = fopen("out.txt", "w");
        
        assemble(file, out);
    }
}