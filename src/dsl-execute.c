#include "dsl.h"
#include "dsl-object.h"
#include "dsl-narrate.h"
#include "dsl-scmd.h"
#include "dsl-string.h"
#include "dsl-var.h"

static int is_paused = 0;

void dsl_execute_subroutine(const int file, const int addr, const int is_mas) {
    if (is_paused) {
        error("dsl_print called while dsl execution is paused!  You must resume first!");
        return;
    }

    this_gpl_type = is_mas ? MASFILE : DSLFILE;
    unsigned long len;
    unsigned char *dsl = (unsigned char*)gff_get_raw_bytes(DSLDATA_GFF_INDEX, is_mas ? GT_MAS : GT_GPL, file, &len);

    command_implemented = 1;

    debug("Executing %s file #%d @ %d\n", is_mas ? "MAS" : "DSL", file, addr);

    push_data_ptr(dsl);
    set_data_ptr(dsl, dsl + addr);
    uint8_t command = get_byte();
    while (do_dsl_command(command) && command_implemented) {
        command = get_byte();
    }
    pop_data_ptr();

    if (!command_implemented) {
        error("last command needs to be implemented!\n");
    } else {
        debug("returing from subroutine.\n");
    }
    debug("---------------------Ending Execution----------------------\n");
    debug("%lu of %lu\n", get_data_ptr() - get_data_start_ptr(), len);
    //if (len != (get_data_ptr() - get_data_start_ptr())) {exit(1);}
}
