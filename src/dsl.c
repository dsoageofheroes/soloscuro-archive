#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dsl.h"
#include "dsl-execute.h"
#include "dsl-object.h"
#include "dsl-narrate.h"
#include "dsl-scmd.h"
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

/*
void dsl_resume() {
    while (!exit_dsl && command_implemented) {
        command_implemented = 1;
        uint8_t command = get_byte();
        //do_dsl_command(command);
        (*dsl_operations[command].func)();
        if (exit_dsl) {
            exit_dsl = (pop_data_ptr() == NULL);
            if (!exit_dsl) {
                debug("returning from subroutine.\n");
            }
        }
        if (is_paused) {
            debug("pausing execution until resumed.");
            return;
        }
    }
    if (!command_implemented) {
        debug("last command needs to be implemented!\n");
    }
    pop_data_ptr();
    //debug("Returning from MAS/DSL %d\n", res_id);
}
*/

void dsl_change_region(const int region_id) {
    dsl_execute_subroutine(region_id, 0, 1);
}


/* SCMD */

static scmd_t* get_script(unsigned char* scmd_entry, const int index) {
    if (scmd_entry == NULL) { return NULL; }
    if (index < 0 || index >= SCMD_MAX_SIZE) {
        error("index for get_script is out of bounds!(%d)\n", index);
        return NULL;
    }
    scmd_t *scmds = (scmd_t*)(scmd_entry + (SCMD_MAX_SIZE * 2));
    uint16_t scmd_idx = *((uint16_t*)scmd_entry + index);
    return scmds + scmd_idx;
}

static void print_scmd(scmd_t *scmd) {
    printf("bmp_idx = %d, delay = %d, flags = 0x%x, xoffset = %d, yoffset = %d, "
        "xoffsethot = %d, yoffsethot = %d, soundidx = %d\n",
        scmd->bmp_idx,
        scmd->delay,
        scmd->flags,
        scmd->xoffset,
        scmd->yoffset,
        scmd->xoffsethot,
        scmd->yoffsethot,
        scmd->soundidx
    );
}

static void print_script(scmd_t *script) {
    if (script == NULL) { return; }
    while((script->flags & SCMD_LAST) == 0) {
        print_scmd(script);
        script++;
    }
    print_scmd(script);
}

scmd_t* dsl_scmd_get(const int gff_file, const int res_id, const int index) {
    unsigned long len;
    char *scmd_entry = gff_get_raw_bytes(gff_file, GT_SCMD, res_id, &len);
    return get_script((unsigned char*) scmd_entry, index);
}

int dsl_scmd_is_default(const scmd_t *scmd, const int scmd_index) {
    if (scmd == NULL) { return 0; }
    if (scmd_index < 0 || scmd_index >= SCMD_MAX_SIZE) {
        error("index for get_script is out of bounds!(%d)\n", scmd_index);
        return 0;
    }
    uint16_t scmd_idx = *((uint16_t*)(((unsigned char *)scmd) + scmd_index));
    return scmd_idx == 0;
}

void dsl_scmd_print(int gff_file, int res_id) {
    unsigned long len;
    char *scmd_entry = gff_get_raw_bytes(gff_file, GT_SCMD, res_id, &len);
    if (scmd_entry == NULL) { return; }
    int script_size = len - (SCMD_MAX_SIZE * 2);
    int script_cmds = script_size / sizeof(scmd_t);
    printf("-------------------------PRINTING SCMD ENTRY----------------------\n");
    printf("len = %lu, script_size = %d, script_cmds = %d\n", len, script_size, script_cmds);
    printf("Default Script (#0):\n");
    print_script(get_script((unsigned char*) scmd_entry, 0));
    uint16_t *scmd_idx = (uint16_t*)scmd_entry;

    int script_count = 1;
    for (int i = 0; i < SCMD_MAX_SIZE; i++) {
        if (scmd_idx[i] > 0) {
            script_count++;
            printf("Script #%d jumps to %d, entry is:\n", i, scmd_idx[i]);
            print_script(get_script((unsigned char*) scmd_entry, i));
        }
    }

    printf("Detected %d scripts\n", script_count);
}
/* END SCMD */


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

