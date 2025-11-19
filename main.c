#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"

int main(int argc, char **argv){
    
    //Terminate if no input provided
    if(argc == 1){
        printf("Please indicate files to assemble.\n");
        exit(1);
    }
    
    //Make sure all input are .sic files
    for(int i = 1; i < argc; i++){
        int len = strlen(argv[i]);

        if(
            len < 4 ||
            strcmp(argv[i] + len - 4, ".sic") != 0
        ){
            printf("Unexpected argument \"%s\" found. Terminating program.\n", argv[i]);
            exit(2);
        }
    }

    //assemble all files
    for(int i = 1; i < argc; i++){
        FILE *file = fopen(argv[i], "r");
        FILE *out = fopen("out.txt", "w");
        
        assemble(file, out);
    }
}