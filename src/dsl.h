// DarkSun Scripting Language (DSL)
#ifndef DSL_H
#define DSL_H

#include "gff.h"
#include "gfftypes.h"
#include "dsl-scmd.h"

void dsl_init();
void dsl_check_for_updates();
void dsl_scmd_print(int gff_file, int res_id);
scmd_t* dsl_scmd_get(const int gff_file, const int res_id, const int index);

#endif
