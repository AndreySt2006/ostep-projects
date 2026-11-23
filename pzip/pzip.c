// pzip.c - parallel zip (RLE) using threads
// Usage: ./pzip file1 [file2 ...] > out.z
//
// Each file is split into equal parts, one part per CPU core.
// Every thread compresses its own part, then main thread writes
// the results in order (and joins runs between parts).
//
// Author: Andrey Stewart
// Sources: OSTEP chapters about threads, man pages mmap, pthread_create

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_THREADS 64

// one part of the file for one thread
typedef struct {
    char *start;   // where the part starts
    long len;      // length of the part
    int *counts;   // result: run lengths
    char *chars;   // result: run characters
    int n;         // number of runs
} part_t;

// thread function: compress one part
void *zip_part(void *arg)
{
    part_t *p = (part_t *)arg;
    p->counts = malloc(p->len * sizeof(int) + 1);
    p->chars = malloc(p->len + 1);
    p->n = 0;
    long i = 0;
    while (i < p->len) {
        char c = p->start[i];
        int cnt = 0;
        while (i < p->len && p->start[i] == c) {
            cnt++;
            i++;
        }
        p->counts[p->n] = cnt;
        p->chars[p->n] = c;
        p->n++;
    }
    return NULL;
}

// current run, kept here because next part/file can continue it
int count = 0;
char prev = 0;
void add_run(int cnt, char c)
{
    if (count > 0 && c == prev) {
        count += cnt;   // same char -> join runs
        return;
    }
    if (count > 0) {
        fwrite(&count, sizeof(int), 1, stdout);
        fwrite(&prev, sizeof(char), 1, stdout);
    }
    prev = c;
    count = cnt;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("pzip: file1 [file2 ...]\n");
        exit(1);
    }
    // number of threads = number of cpu cores
    int nthreads = sysconf(_SC_NPROCESSORS_ONLN);
    if (nthreads < 1)
        nthreads = 1;
    if (nthreads > MAX_THREADS)
        nthreads = MAX_THREADS;
    for (int f = 1; f < argc; f++) {
        int fd = open(argv[f], O_RDONLY);
        if (fd < 0) {
            printf("pzip: cannot open file\n");
            exit(1);
        }
        struct stat st;
        fstat(fd, &st);
        long size = st.st_size;
        if (size == 0) {   // empty file, nothing to do
            close(fd);
            continue;
        }
        // map the whole file to memory
        char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data == MAP_FAILED) {
            printf("pzip: cannot open file\n");
            exit(1);
        }
        // split file into parts
        pthread_t threads[MAX_THREADS];
        part_t parts[MAX_THREADS];
        long piece = size / nthreads;
        for (int t = 0; t < nthreads; t++) {
            parts[t].start = data + t * piece;
            parts[t].len = piece;
            if (t == nthreads - 1)
                parts[t].len = size - t * piece;   // last part gets the rest
            pthread_create(&threads[t], NULL, zip_part, &parts[t]);
        }
        // wait for all threads
        for (int t = 0; t < nthreads; t++)
            pthread_join(threads[t], NULL);
        // write results in correct order
        for (int t = 0; t < nthreads; t++) {
            for (int k = 0; k < parts[t].n; k++)
                add_run(parts[t].counts[k], parts[t].chars[k]);
            free(parts[t].counts);
            free(parts[t].chars);
        }
        munmap(data, size);
        close(fd);
    }

    // write the last run
    if (count > 0) {
        fwrite(&count, sizeof(int), 1, stdout);
        fwrite(&prev, sizeof(char), 1, stdout);
    }
    return 0;
}
