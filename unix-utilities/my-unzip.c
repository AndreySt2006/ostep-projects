/*
 * my-unzip.c - decompresses files made by my-zip.
 *
 * Usage: ./my-unzip file1 [file2 ...]
 *
 * The file consists of 5-byte entries: a 4-byte integer (count)
 * and one character. Each entry is printed as count characters.
 *
 * Author: Andrey Stewart
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    /* No files given */
    if (argc < 2) {
        printf("my-unzip: file1 [file2 ...]\n");
        exit(1);
    }
    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("my-unzip: cannot open file\n");
            exit(1);
        }
        int count;
        char c;
        /* Read one entry at a time: first the count, then the character.
           fread() returns 1 if it managed to read one item. */
        while (fread(&count, sizeof(int), 1, fp) == 1 &&
               fread(&c, sizeof(char), 1, fp) == 1) {
            for (int j = 0; j < count; j++) {
                printf("%c", c);
            }
        }
        fclose(fp);
    }
    return 0;
}
