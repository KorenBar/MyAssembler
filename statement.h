#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "mytypes.h"
#include "buffer.h"
#include "strhelper.h"

#ifndef STATEMENT_H
#define STATEMENT_H

typedef struct {
    string  label;
    string  name;
    string *params;
    ushort  params_count;
    ulong   line;
} statement;

/* init a new statement */
statement* new_statement();
void free_statement(statement* state);
/* copy a string and add it as param of a given statement */
bool add_param(statement* state, char *str);
/* use it for debugging */
void print_statement(statement* state);

#endif