#include "input.h"
#include <stdio.h>

extern void sol_key_down(const sol_key_e key) {
    printf("key = %d\n", key);
}
