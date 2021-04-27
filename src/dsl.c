#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dsl.h"
#include "wizard.h"
#include "dsl-manager.h"
#include "ssi-item.h"
#include "region-manager.h"
#include "ds-state.h"
#include "ds-string.h"
#include "dsl-var.h"
//#include "gameloop.h"
#include "player.h"
#include "replay.h"
#include "trigger.h"

#define DSL_ORDER     (0)
#define DSL_LOS_ORDER (1)

uint8_t command_implemented = 0; // Temporary while I figure out each function.
uint8_t quiet = 0;

/* Globals */
void get_parameters(int16_t amt);

param_t param;
/* End Globals */

void dsl_change_region(const int region_id) {
    //dsl_execute_subroutine(region_id, 0, 1);
    replay_print("rep.change_region(%lld)\n", region_id);
    dsl_lua_execute_script(region_id, 0, 1);
}

static void initialize_dsl_stack() {
    dsl_global_strings = (dsl_string_t*) malloc(GSTRINGVARSIZE);
    memset(dsl_global_strings, 0, GSTRINGVARSIZE);
    dsl_local_strings = (dsl_string_t*) malloc(LSTRINGVARSIZE);
    memset(dsl_local_strings, 0, LSTRINGVARSIZE);
    gTextstring = (uint8_t*)malloc(TEXTSTRINGSIZE);
}

void dsl_init() {
    info("Initalizing DSL.\n");
    initialize_dsl_stack();
    powers_init();
    dsl_init_vars();
    sol_player_init();
    ssi_item_init();
    trigger_init();
    dsl_manager_init();
    region_manager_init();
    info("Running Master DSL #99.\n");
    //dsl_execute_subroutine(99, 0, 1);
    dsl_lua_execute_script(99, 0, 1);
}

void dsl_set_quiet(const int val) {
    quiet = val;
}

void dsl_debug(const char *file, const int line_num, const char *pretty, const char *str, ...) {
    if (quiet) { return; }
    va_list argp;
    va_start(argp, str);
    printf("[%s:%d] %s: ", file, line_num, pretty);
    vprintf(str, argp);
}

void dsl_cleanup() {
    powers_cleanup();
    free(gTextstring);
    free(dsl_local_strings);
    free(dsl_global_strings);
    dsl_cleanup_vars();
    dsl_manager_cleanup();
    ssi_item_close();
    trigger_cleanup();
    region_manager_cleanup();
    player_cleanup();
}
