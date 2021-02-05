#include <stdlib.h>
#include <string.h>

/* Since it isn't part of the standard... */

char *strdup(const char *s) {
    size_t size = strlen(s) + 1;
    char *p = malloc(size);

    if (p) {
        memcpy(p, s, size);
    }

    return p;
}
