#ifndef GPL_STRING_H
#define GPL_STRING_H
#include <stdlib.h>
#include <stdint.h>

#define DSSTRINGLENGTH (40)
#define DSSTRLEN (20)

typedef char gpl_string_t[DSSTRINGLENGTH+2];
typedef char string20_t[DSSTRLEN];

extern gpl_string_t* gpl_global_strings;
extern gpl_string_t* gpl_local_strings;
extern uint8_t*      gpl_global_string;

#define COMPSTR          (300)
#define MAXGSTRINGS      (10)
#define GSTRINGVARSIZE (MAXGSTRINGS*sizeof(gpl_string_t))
#define MAXLSTRINGS      (10)
#define LSTRINGVARSIZE (MAXLSTRINGS*sizeof(gpl_string_t))
#define TEXTSTRINGSIZE   COMPSTR

extern char*         gpl_read_text();
#endif
