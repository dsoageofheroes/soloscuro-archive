#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../src/gff.h"

static char *new_filename = NULL, *input_file = NULL;
static char *type_to_add = NULL;
static char *data_to_add = NULL;
static char print_info = 0;
static void check_create_file();
static int gff_file = -1;

static void open_input();
static void add_type();
static void add_data();
static void print_infos();

static void print_usage(const char *prg) {
    fprintf(stderr, "Usage: %s\n", prg);
    fprintf(stderr, "    -c  <filename>         : create a gff\n");
    fprintf(stderr, "    -i  <filename>         : input file to manipulate\n");
    fprintf(stderr, "    -a  <type>             : add type\n");
    fprintf(stderr, "    -d  <type>,<id>,<file> : add type\n");
    fprintf(stderr, "    -t                     : print info\n");
}

int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "c:i:a:d:t")) != -1) {
        switch (opt) {
        case 'c':
            new_filename = optarg;
            break;
        case 'i':
            input_file = optarg;
            break;
        case 'a':
            type_to_add = optarg;
            break;
        case 'd':
            data_to_add = optarg;
            break;
        case 't':
            print_info = 1;
            break;
        default:
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    check_create_file();
    open_input();
    add_type();
    add_data();
    print_infos();

    return 0;
}
static void check_create_file() {
    if (!new_filename) { return; }

    if (!(gff_file = gff_create(new_filename))) {
        fprintf(stderr, "error creating '%s'\n", new_filename);
        return;
    }

    printf("'%s' created successfully\n", new_filename);
}

static void open_input() {
    if (!input_file) { return; }

    gff_file = gff_open(input_file);
}

static void input_check() {
    if (gff_file < 0) {
        fprintf(stderr, "Input file needed, please specify with -i <file>\n");
        exit(1);
    }
}

static void add_type() {
    char type[4];
    if (!type_to_add) { return; }
    input_check();

    type[0] = type[1] = type[2] = type[3] = 0x20;
    for (size_t i = 0; i < 4 && i < strlen(type_to_add); i++) {
        type[i] = type_to_add[i];
    }
    gff_add_type(gff_file, *((int*)type));
}

static void* read_data(const char *filename, size_t *len) {
    if (!filename) { return NULL; }
    void *ret = NULL;

    FILE *file = fopen(filename, "rb");
    if (!file) { return NULL; }

    fseek(file, 0L, SEEK_END);
    *len = ftell(file);
    fseek(file, 0L, SEEK_SET);

    ret = malloc(*len);
    if (!ret) { fclose(file); return NULL; }
    if (fread(ret, 1, *len, file) != *len) {
        fprintf(stderr, "Error reading from '%s'\n", filename);
        free(ret);
        fclose(file);
        return NULL;
    }

    fclose(file);

    return ret;
}

static void add_data() {
    char type[4];
    void *data = NULL;
    size_t len = 0;
    if (!data_to_add) { return; }
    input_check();

    char *data_type = strtok(data_to_add, ",");
    char *id = strtok(NULL, ",");
    char *file = strtok(NULL, ",");
    //printf("type = %s, id = %s, file = %s\n", data_type, id, file);

    if (!data_type || !id || !file) {
        fprintf(stderr, "Error: need type,id,file in that order with no spaces.\n");
        return;
    }
    type[0] = type[1] = type[2] = type[3] = 0x20;
    for (size_t i = 0; i < 4 && i < strlen(data_type); i++) {
        type[i] = data_type[i];
    }

    data = read_data(file, &len);
    if (!data) {
        fprintf(stderr, "Unable to read '%s', does it exist?\n", file);
        return;
    }

    if (gff_add_chunk(gff_file, *((int*)type), atoi(id), data, len)) {
        printf("type = %s, id = %s, file = %s added to %s\n", data_type, id, file, open_files[gff_file].filename);
    }
}

static void print_infos() {
    if (!print_info) { return; }
    gff_file_t *gff = open_files + gff_file;

    for (size_t chunk_idx = 0; chunk_idx < gff->num_types; chunk_idx++) {
        char type[5];
        type[4] = '\0';
        *((uint32_t*)type) = (gff->chunks[chunk_idx]->chunk_type & GFFMAXCHUNKMASK);
        printf("%s:\n", type);
        gff_chunk_entry_t *entry = gff->chunks[chunk_idx];
        for (size_t i = 0; i < entry->chunk_count; i++) {
            printf("\t%d: loc = %u, len = %u\n", entry->chunks[i].id,
                 entry->chunks[i].location, entry->chunks[i].length);
        }
    }
}
