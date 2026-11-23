/*
 * reverse.c - prints the lines of a file in reverse order.
 *
 * Usage:
 *   ./reverse                    read from keyboard, print to screen
 *   ./reverse input.txt          read from file, print to screen
 *   ./reverse input.txt out.txt  read from file, write to file
 * Author: Andrey Stewart
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    FILE *in = stdin;    /* by default read from keyboard */
    FILE *out = stdout;  /* by default print to screen */

    /* Error: too many arguments */
    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    /* One or two arguments: open the input file */
    if (argc >= 2) {
        in = fopen(argv[1], "r");
        if (in == NULL) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
    }

    /* Two arguments: open the output file */
    if (argc == 3) {
        /* Error: input and output are the same file */
        if (strcmp(argv[1], argv[2]) == 0) {
            fprintf(stderr, "Input and output file must differ\n");
            exit(1);
        }
        out = fopen(argv[2], "w");
        if (out == NULL) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[2]);
            exit(1);
        }
    }

    /* Array of lines. It grows when it gets full,
       so the file can have any number of lines. */
    char **lines = NULL;
    int count = 0;     /* how many lines we have */
    int capacity = 0;  /* how many lines fit in the array */

    char *line = NULL; /* getline() allocates memory for the line */
    size_t size = 0;

    /* Read the file line by line. getline() works
       with lines of any length. */
    while (getline(&line, &size, in) != -1) {
        /* Array is full: make it two times bigger */
        if (count == capacity) {
            if (capacity == 0)
                capacity = 10;
            else
                capacity = capacity * 2;

            lines = realloc(lines, capacity * sizeof(char *));
            if (lines == NULL) {
                fprintf(stderr, "malloc failed\n");
                exit(1);
            }
        }

        lines[count] = line;  /* save the line */
        count++;

        line = NULL;  /* next getline() call gets new memory */
        size = 0;
    }
    free(line);

    /* Print lines from the last to the first */
    for (int i = count - 1; i >= 0; i--) {
        fprintf(out, "%s", lines[i]);

        /* The last line of a file may have no '\n' at the end */
        if (strchr(lines[i], '\n') == NULL)
            fprintf(out, "\n");

        free(lines[i]);
    }
    free(lines);

    /* Close files */
    if (in != stdin)
        fclose(in);
    if (out != stdout)
        fclose(out);

    return 0;
}
