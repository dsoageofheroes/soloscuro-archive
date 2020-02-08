#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dsl.h"
#include "dsl-execute.h"
#include "dsl-string.h"
#include "dsl-var.h"

#define DSL_ORDER     (0)
#define DSL_LOS_ORDER (1)

uint8_t command_implemented = 0; // Temporary while I figure out each function.

/* Globals */
void get_parameters(int16_t amt);

/* If command data */
#define MAX_IFDEPTH (32)
int8_t ifptr = 0;
int8_t ifstate[MAX_IFDEPTH+1];
#define YES (1)
#define NO (0)

param_t param;
/* End Globals */

void dsl_change_region(const int region_id) {
    dsl_execute_subroutine(region_id, 0, 1);
}

static void initialize_dsl_stack() {
    ifptr = 0;
    dsl_global_strings = (dsl_string_t*) malloc(GSTRINGVARSIZE);
    memset(dsl_global_strings, 0, GSTRINGVARSIZE);
    dsl_local_strings = (dsl_string_t*) malloc(LSTRINGVARSIZE);
    memset(dsl_local_strings, 0, LSTRINGVARSIZE);
    gTextstring = (uint8_t*)malloc(TEXTSTRINGSIZE);
}

void dsl_init() {
    info("Initalizing DSL.\n");
    initialize_dsl_stack();
    dsl_init_vars();
    dsl_object_init();
    dsl_execute_init();
    info("Running Master DSL #99.\n");
    dsl_execute_subroutine(99, 0, 1);
}

