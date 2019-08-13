#include <stdio.h>
#include "gff.h"

extern int gff_create(const char *pathName);
extern int gff_update(const char *path, int id);
int main (int argc, char *argv[]) {
    int idx;
    gff_init();
    printf("Attempting to open '%s'\n", argv[1]);
    idx = gff_open(argv[1]);

    gff_print(idx, stdout);

    printf("Done, cleaning up\n");
    gff_close(idx);
    return 0;
}
