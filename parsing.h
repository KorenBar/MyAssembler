#include <stdarg.h>
#include "mytypes.h"
#include "strhelper.h"
#include "command.h"
#include "directive.h"
#include "memohelper.h"

#ifndef PARSING_H
#define PARSING_H

#include "asmencoder.h"

#define SIGN_BIT_ON(v) v |= (1UL << (sizeof(v)*8-1));
#define SIGN_BIT_OFF(v) v &= ~(1UL << (sizeof(v)*8-1));

typedef enum { none_state, invalid_state, empty_state, comment_state, command_state, directive_state } statement_type;

typedef void (*error_cb)(void *sender, const statement *state, bool can_continue, const char *msg, va_list ap);
typedef void (*data_write_cb)(void *sender, const statement *state, const byte *data, ulong dataSize);
typedef void (*instruction_created_cb)(void *sender, const statement *state, instruction inst);
typedef void (*new_label_cb)(void *sender, const statement *state, symbol_t symbol);
typedef void (*extern_label_req_cb)(void *sender, symbol_t *sp);

/* why doing it that way?
 * cuz we have to do the same parsing twice,
 * so on each time we'll use different callbacks for our needs */
typedef struct {
    error_cb error;
    data_write_cb data_write;
    instruction_created_cb instruction_created;
    new_label_cb new_label;
    extern_label_req_cb extern_label_required;
} parsing_callbacks;

typedef struct {
    void *sender;
    const statement *state;
    const symbol_t *symbol_table;
    ulong symbol_table_count;
    parsing_callbacks callbacks;
    bool error_for_missing_label;
} parsing_args;

/* parse a raw statement to an statement_t structure
 * if the symbol_table is NULL no error will be reported for missing label
 * returns false if error was occurred */
bool parse_statement(parsing_args args);
symbol_t *get_label_by_name(const symbol_t *symbol_table, ulong symbol_table_count, const char *label_name);
bool error_callback(error_cb cb, asmencoder *sender, const statement *state, bool can_continue, const char *msg, ...);

#endif
