#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "verifier.h"

static void
printResult(char *before, char *info, unsigned char res)
{
    switch (res) {
        case V_VALID:
            printf("[SAFE] %s -> %s\n", before, info);
            break;
        case V_INVALID:
            printf("[NOT SAFE] %s: %s\n", before, info);
            break;
        case V_FERROR:
            printf("[INVALID FILE] %s: %s\n", before, info);
    }
}

int
main(int argc, char **argv)
{
    unsigned char i = 0;
    unsigned char res = V_FERROR;
    char *fileInfo = NULL;

    if (argc <= 1) {
        fprintf(stderr, "You must specify at least one file to verify\n");
        return 1;
    }

    for (i = 1; i < argc; ++i) {
        res = prepareFile(argv[i], strlen(argv[i]), &fileInfo);
        printResult(argv[i], fileInfo, res);
        if (res == V_VALID) {
            free(fileInfo);
        }
    }
    return 0;
}

