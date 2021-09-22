#include <stdio.h>
#include "mytypes.h"
#include "buffer.h"
#include "statement.h"

#ifndef ASMFILE_H
#define ASMFILE_H

#define ASM_EXTENSION ".as"

#define COMMENT_SIGN ';'
#define ARG_SEP ','
#define ESCAPE_CHAR '\\'
#define STRING_CHAR '\"'

#define MAX_LINE_LEN 80
#define MAX_LABEL_LEN 30

/* each bit is a flag so we can combine few values together */
typedef enum { all_sa = ~0, none_sa = 0, code_sa = 1, data_sa = 1 << 1, entry_sa = 1 << 2, external_sa = 1 << 3 } symbol_att;

typedef struct {
    string name;
    ulong value; /* address */
    symbol_att attributes;
} symbol_t;

typedef struct asmfile asmfile;
typedef void (*file_error_cb)(void *sender, asmfile *file, long line, long col, bool can_continue, const char *msg, va_list al);

struct asmfile {
    string filename;
    FILE *src_file;
    long curr_line;
    long curr_col;
    file_error_cb error_callback;
    bool error_occurred;
    dyBuffer *buffer;
};

/* open an assembly file, returns NULL if was failed to open */
asmfile *new_asmfile(string filename);
void free_asmfile(asmfile *asmfile);
void free_symbol_content(symbol_t symbol);
/* returns the next statement line (directive or command), ignoring empty and comment lines
 * returns NULL on reading error or if there are no more lines */
statement* read_statement(asmfile *file);
/* clear the buffer of an assembly file
 * returns false if can't allocate a buffer when needed */
bool reset_asmfile_buffer(asmfile *file);
bool print_line(asmfile *file, long x);
/* move to start of line x, returns false if can't move to that line */
bool gotoline(asmfile *file, long x);
/* move x line, returns false if can't move to that line */
bool movelines(asmfile *file, long x);
bool iseof(asmfile *file);
/* get a char and update the line counter if that char is a new line. DON'T USE fgetc DIRECTLY! */
int agetc(asmfile *file);
/* peek the current char without moving on */
int apeekc(asmfile *file);
/* skip over spaces and get the count of spaces was skipped */
long skip_spaces(asmfile *file);

#endif
