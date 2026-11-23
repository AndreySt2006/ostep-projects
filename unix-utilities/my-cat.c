/*
 * my-cat.c - prints the contents of one or more files.
 *
 * Usage: ./my-cat file1 [file2 ...]
 *
 * Author: Andrey Stewart
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char buffer[1024];
    /* Go through all files given on the command line.
       If there are no files, the loop does nothing and we return 0. */
    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("my-cat: cannot open file\n");
            exit(1);
        }

        /* fgets reads at most 1023 characters at a time.
           A longer line is just read in several parts,
           so printing the parts gives the whole line. */
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
        }
        fclose(fp);
    }
    return 0;
}
