// punzip.c - parallel unzip using threads
// Usage: ./punzip file1.z [file2.z ...]
//
// Compressed file = entries of 5 bytes (4 byte count + 1 char).
// Entries are split into equal parts, one part per CPU core.
// Every thread unzips its part into its own buffer, then main
// thread prints the buffers in order.
//
// Author: Andrey Stewart
// Sources: OSTEP chapters about threads, man pages mmap, pthread_create

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_THREADS 64

typedef struct {
    char *start;   // first entry of the part
    long n;        // number of entries
    char *out;     // result text
    long outlen;   // result length
} part_t;

// thread function: unzip one part
void *unzip_part(void *arg)
{
    part_t *p = (part_t *)arg;
    int cnt;
    // first find out how big the result is
    long total = 0;
    for (long i = 0; i < p->n; i++) {
        memcpy(&cnt, p->start + i * 5, 4);
        total += cnt;
    }
    p->out = malloc(total + 1);
    long pos = 0;
    for (long i = 0; i < p->n; i++) {
        memcpy(&cnt, p->start + i * 5, 4);   // count
        char c = p->start[i * 5 + 4];        // character
        for (int j = 0; j < cnt; j++) {
            p->out[pos] = c;
            pos++;
        }
    }
    p->outlen = total;
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("punzip: file1 [file2 ...]\n");
        exit(1);
    }
    int nthreads = sysconf(_SC_NPROCESSORS_ONLN);
    if (nthreads < 1)
        nthreads = 1;
    if (nthreads > MAX_THREADS)
        nthreads = MAX_THREADS;
    for (int f = 1; f < argc; f++) {
        int fd = open(argv[f], O_RDONLY);
        if (fd < 0) {
            printf("punzip: cannot open file\n");
            exit(1);
        }
        struct stat st;
        fstat(fd, &st);
        long size = st.st_size;
        if (size == 0) {
            close(fd);
            continue;
        }
        char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data == MAP_FAILED) {
            printf("punzip: cannot open file\n");
            exit(1);
        }
        // split entries into parts (never in the middle of an entry)
        long entries = size / 5;
        long piece = entries / nthreads;
        pthread_t threads[MAX_THREADS];
        part_t parts[MAX_THREADS];
        for (int t = 0; t < nthreads; t++) {
            parts[t].start = data + t * piece * 5;
            parts[t].n = piece;
            if (t == nthreads - 1)
                parts[t].n = entries - t * piece;   // last part gets the rest
            pthread_create(&threads[t], NULL, unzip_part, &parts[t]);
        }
        for (int t = 0; t < nthreads; t++)
            pthread_join(threads[t], NULL);
        // print in correct order
        for (int t = 0; t < nthreads; t++) {
            fwrite(parts[t].out, 1, parts[t].outlen, stdout);
            free(parts[t].out);
        }
        munmap(data, size);
        close(fd);
    }
    return 0;
}
