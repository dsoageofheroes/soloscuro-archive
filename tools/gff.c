#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "../src/gff.h"

static char *new_filename = NULL;
static void check_create_file();

static void print_usage(const char *prg) {
    fprintf(stderr, "Usage: %s\n", prg);
    fprintf(stderr, "    -c  <filename> : create a gff\n");
}

int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
        case 'c':
            new_filename = optarg;
            break;
        default:
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    check_create_file();

    return 0;
}

static void check_create_file() {
    if (!new_filename) { return; }

    if (!gff_create(new_filename)) {
        fprintf(stderr, "error creating '%s'\n", new_filename);
        return;
    }

    printf("'%s' created successfully\n", new_filename);
}
