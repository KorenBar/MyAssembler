#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mytypes.h"
#include "strhelper.h"

#ifndef DIRECTIVE_H
#define DIRECTIVE_H

#define WORD_SIZE       4 /*bytes*/
#define HALF_WORD_SIZE  2 /*bytes*/

/* // FIXED: don't print warnings with the -pedantic flag
#define DB_DIR      (directive){ .name = ".db",     .num_of_args = UNLIMITED, .data_size = 1,              .labelable = true  }
#define DW_DIR      (directive){ .name = ".dw",     .num_of_args = UNLIMITED, .data_size = WORD_SIZE,      .labelable = true  }
#define DH_DIR      (directive){ .name = ".dh",     .num_of_args = UNLIMITED, .data_size = HALF_WORD_SIZE, .labelable = true  }
#define ASCIZ_DIR   (directive){ .name = ".asciz",  .num_of_args = 1,         .data_size = UNLIMITED,      .labelable = true  }
#define ENTRY_DIR   (directive){ .name = ".entry",  .num_of_args = 1,         .data_size = 0,              .labelable = false }
#define EXTERN_DIR  (directive){ .name = ".extern", .num_of_args = 1,         .data_size = 0,              .labelable = false }
#define NONE_DIR    (directive){ }
*/
#define DB_DIR      { ".db",     UNLIMITED, 1,              true  }
#define DW_DIR      { ".dw",     UNLIMITED, WORD_SIZE,      true  }
#define DH_DIR      { ".dh",     UNLIMITED, HALF_WORD_SIZE, true  }
#define ASCIZ_DIR   { ".asciz",  1,         UNLIMITED,      true  }
#define ENTRY_DIR   { ".entry",  1,         0,              false }
#define EXTERN_DIR  { ".extern", 1,         0,              false }
#define NONE_DIR    { NULL }


typedef struct {
    const char *name;
    byte num_of_args;
    byte data_size;
    bool labelable;
} directive;

typedef enum                          { db_dir, dw_dir, dh_dir, asciz_dir, entry_dir, extern_dir, none_dir } dir_index;
static const directive directives[] = { DB_DIR, DW_DIR, DH_DIR, ASCIZ_DIR, ENTRY_DIR, EXTERN_DIR, NONE_DIR };

/* get index of given directive name or none_dir if not exists */
dir_index get_directive_index(const char *name);
/* get index of similar name to a given directive name or none_dir if not exists */
dir_index get_similar_directive_index(const char *name);
/* find a directive by a given name */
directive get_directive(const char *name);
/* find a directive with a similar name */
directive get_similar_directive(const char *name);

bool is_directive_name(const char *name);
bool is_valid_directive_name(string name);
bool is_dir_equals(directive, directive);
bool is_dir_exists(directive dir);
bool is_dir(directive dir, dir_index dir_i);

#endif
