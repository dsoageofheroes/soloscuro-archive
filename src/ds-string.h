#ifndef DS_STRING_H
#define DS_STRING_H
#include <stdlib.h>
#include <stdint.h>

#define DSSTRINGLENGTH (40)
#define DSSTRLEN (20)

typedef struct _s {
    int a;
} s;

typedef char dsl_string_t[DSSTRINGLENGTH+2];
typedef char string20_t[DSSTRLEN];
extern dsl_string_t* dsl_global_strings;
extern dsl_string_t* dsl_local_strings;
extern uint8_t* gTextstring;

#define COMPSTR          (300)
#define MAXGSTRINGS      (10)
#define GSTRINGVARSIZE (MAXGSTRINGS*sizeof(dsl_string_t))
#define MAXLSTRINGS      (10)
#define LSTRINGVARSIZE (MAXLSTRINGS*sizeof(dsl_string_t))
#define TEXTSTRINGSIZE   COMPSTR

char * read_text();
#endif
