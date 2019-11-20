#ifndef DSL_STRING_H
#define DSL_STRING_H
#include <stdlib.h>
#include <stdint.h>

#define DSLSTRINGLENGTH (40)
#define DSLSTRLEN (20)

typedef struct _s {
    int a;
} s;

typedef char dsl_string_t[DSLSTRINGLENGTH+2];
typedef char string20_t[DSLSTRLEN];
extern dsl_string_t* gGstringvar;
extern dsl_string_t* gLstringvar;
extern uint8_t* gTextstring;

#define COMPSTR          (300)
#define MAXGSTRINGS      (10)
#define GSTRINGVARSIZE (MAXGSTRINGS*sizeof(dsl_string_t))
#define TEXTSTRINGSIZE   COMPSTR

char * read_text();
#endif
