
#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "asmencoder.h"
#include "parsing.h"

void error_cb_1(void *sender, const statement *state, bool can_continue, const char *msg, va_list ap);
void new_label_cb_1(void *sender, const statement *state, symbol_t symbol);
void data_write_cb_1(void *sender, const statement *state, const byte *data, ulong dataSize);
void instruction_created_cb_1(void *sender, const statement *state, instruction inst);
void error_cb_2(void *sender, const statement *state, bool can_continue, const char *msg, va_list ap);
void new_label_cb_2(void *sender, const statement *state, symbol_t symbol);
void data_write_cb_2(void *sender, const statement *state, const byte *data, ulong dataSize);
void instruction_created_cb_2(void *sender, const statement *state, instruction inst);
void extern_label_req_cb_2(void *sender, symbol_t *sp);

#endif
