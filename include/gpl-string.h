#ifndef GPL_STRING_H
#define GPL_STRING_H

#include <stdlib.h>
#include <stdint.h>
#include "status.h"

#define DSSTRINGLENGTH (40)
#define DSSTRLEN (20)

typedef char sol_gpl_string_t[DSSTRINGLENGTH+2];

extern sol_gpl_string_t* sol_gpl_global_strings;
extern sol_gpl_string_t* sol_gpl_local_strings;
extern uint8_t*          sol_gpl_global_string;

#define COMPSTR          (300)
#define MAXGSTRINGS      (10)
#define GSTRINGVARSIZE (MAXGSTRINGS*sizeof(sol_gpl_string_t))
#define MAXLSTRINGS      (10)
#define LSTRINGVARSIZE (MAXLSTRINGS*sizeof(sol_gpl_string_t))
#define TEXTSTRINGSIZE   COMPSTR

extern sol_status_t sol_gpl_read_text(char **ret);
#endif
