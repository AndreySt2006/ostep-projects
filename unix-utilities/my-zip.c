/*
 * my-zip.c - compresses files with run-length encoding (RLE).
 *
 * Usage: ./my-zip file1 [file2 ...] > output.z
 *
 * Every run of the same character is written as a 4-byte integer
 * (how many times) and the character itself, so 5 bytes per run.
 * Example: "aaaaaaaaaabbbb" -> 10 'a' 4 'b'
 * Several files are compressed into one stream.
 *
 * Author: Andrey Stewart
 */
#include <stdio.h>
#include <stdlib.h>

/* Writes one run: 4-byte count in binary + 1 character */
void write_run(int count, char c)
{
    fwrite(&count, sizeof(int), 1, stdout);
    fwrite(&c, sizeof(char), 1, stdout);
}

int main(int argc, char *argv[])
{
    /* No files given */
    if (argc < 2) {
        printf("my-zip: file1 [file2 ...]\n");
        exit(1);
    }
    int count = 0;   /* length of the current run */
    char prev = 0;   /* character of the current run */
    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("my-zip: cannot open file\n");
            exit(1);
        }
        int c;
        /* Read the file one character at a time */
        while ((c = fgetc(fp)) != EOF) {
            if (count > 0 && c == prev) {
                /* Same character as before: run continues */
                count++;
            } else {
                /* New character: write the old run first */
                if (count > 0)
                    write_run(count, prev);
                prev = c;
                count = 1;
            }
        }
        fclose(fp);
        /* We do not write the run here, because the next file
           may start with the same character. */
    }
    /* Write the last run */
    if (count > 0)
        write_run(count, prev);
    return 0;
}
