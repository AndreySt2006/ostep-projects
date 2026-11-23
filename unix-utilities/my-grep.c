/*
 * my-grep.c - prints the lines that contain a search term.
 *
 * Usage: ./my-grep searchterm [file ...]
 * If no file is given, reads from standard input.
 *
 * Author: Andrey Stewart
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Reads fp line by line and prints lines that contain term. */
void search(FILE *fp, char *term)
{
    char *line = NULL;  /* getline() allocates memory for the line */
    size_t size = 0;
    /* getline() works with lines of any length */
    while (getline(&line, &size, fp) != -1) {
        /* strstr() returns NULL if term is not in the line.
           It is case sensitive: "foo" does not match "Foo". */
        if (strstr(line, term) != NULL) {
            printf("%s", line);
        }
    }
    free(line);
}

int main(int argc, char *argv[])
{
    /* No search term given */
    if (argc < 2) {
        printf("my-grep: searchterm [file ...]\n");
        exit(1);
    }
    char *term = argv[1];
    /* Only search term given: read from standard input */
    if (argc == 2) {
        search(stdin, term);
        return 0;
    }
    /* Search every file in order */
    for (int i = 2; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("my-grep: cannot open file\n");
            exit(1);
        }
        search(fp, term);
        fclose(fp);
    }
    return 0;
}
