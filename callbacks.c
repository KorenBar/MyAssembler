#include "callbacks.h"

/* first pass callbacks:
 * - add new labels to a dynamic buffer, and set the value for each label (will be the current IC or DC)
 * - if a label is already exists, report an error
 * X print errors without stop reading, but flag it so we'll now about it in end of reading
 * - count instruction and data size without writing the data it-self to the encoder's arrays */
void error_cb_1(void *sender, const statement *state, bool can_continue, const char *msg, va_list ap)
{
    /* since we printing errors on the second pass anyway, we'll do it just there to prevent double printing */
}
void new_label_cb_1(void *sender, const statement *state, symbol_t symbol) {
    /* the sub-content of the symbol (strings and allocated memo in it) will not be freed up,
     * so we can add it to our buffer as is */
    dyBuffer *dbuff;
    symbol_t *sp;
    asmencoder *encoder = sender;

    if (encoder == NULL) { /* shouldn't happened */
        free_symbol_content(symbol);
        return;
    }

    /* the symbol_table pointer is currently a pointer to our dynamic buffer, add that symbol to it */
    dbuff = (dyBuffer *)encoder->symbol_table;

    /* set symbol value */
    switch (symbol.attributes) {
        case code_sa:
            symbol.value = encoder->ic;
            break;
        case data_sa:
            symbol.value = encoder->dc;
            break;
        default: /* entry or extern */
            symbol.value = 0;
            break;
    }

    sp = get_label_by_name(dbuff->data, dbuff->length, symbol.name);

    /* can merge only when just one of them is just entry and the other one is not extern
     * (in that case we can also merge the values cuz entry value is 0) */
    if (sp == NULL) /* not exists */
        put2buffer(dbuff, &symbol); /* we pass a pointer to it but the content will be copied */
    else if (((sp->attributes == entry_sa) != (symbol.attributes == entry_sa)) /* exact one of them is just an entry */
            && !((sp->attributes | symbol.attributes) & external_sa) /* the other one is not an extern */
            && !(sp->attributes & symbol.attributes & entry_sa)) /* FIXED: not already an entry (just for printing a warning) */
    {
        /* exists but can merge */
        sp->value |= symbol.value;
        sp->attributes |= symbol.attributes;
    }
    else /* label already exists and can't merge */
        /* using the error_callback func in order to convert to va_list */
        error_callback(&error_cb_2, sender, state,
                       /* if it's another definition for entry or extern we can continue */
                       (symbol.attributes & (entry_sa | external_sa)) && (symbol.attributes & sp->attributes),
                       "multiple definition for the same label name \"%s\"", symbol.name);
}
void data_write_cb_1(void *sender, const statement *state, const byte *data, ulong dataSize)
{ /* if data is a string, the size will be strlen + 1 for the null-terminated '\0' char */
    asmencoder *encoder = sender;
    if (encoder == NULL) return;
    encoder->dc += dataSize;
}
void instruction_created_cb_1(void *sender, const statement *state, instruction inst)
{ /* instruction size is fixed */
    asmencoder *encoder = sender;
    if (encoder == NULL) return;
    encoder->ic += INS_SIZE;
}

/* second pass callbacks: (WE ASSUMING THAT SOURCE FILE WAS NOT CHANGED SINCE THE FIRST PASS!)
 * - if a new LABEL was found, report an error (we'll STOP in that case)
 * - if an error was occurred, report an error (we'll STOP in that case)
 * - write data and move the DC
 * - write instructions and move the IC (x4 bytes for each instruction)
 * - if an extern label required, add it to the extern_labels array */
void error_cb_2(void *sender, const statement *state, bool can_continue, const char *msg, va_list ap) {
    long line;
    asmencoder *encoder = sender;
    if (encoder == NULL) return;
    if (encoder->file == NULL) return;
    if (!can_continue) encoder->file->error_occurred = true; /* flag that we had an error */
    line = encoder->file->curr_line;
    if (!iseof(encoder->file)) line--;
    if (state != NULL) line = (long)state->line; /*fix*/
    SIGN_BIT_OFF(line)
    if (encoder->error_callback != NULL) /* it's parsing error, that means we're already in the nex line */
        encoder->error_callback(encoder, encoder->file, line, 0/*unknown*/, can_continue, msg, ap);
}
void new_label_cb_2(void *sender, const statement *state, symbol_t symbol) {
    asmencoder *encoder = sender;
    if (encoder == NULL) return;
    if (encoder->symbol_table != NULL)
        if (get_label_by_name(encoder->symbol_table, encoder->sc, symbol.name) == NULL)
            error_cb_2(sender, state, false,
                       "a new label was found on the second pass. something goes wrong, was the file changed while encoding?", NULL);
}
void data_write_cb_2(void *sender, const statement *state, const byte *data, ulong dataSize) {
    /* we have to copy the data, the caller will free it after returning */
    int i;
    asmencoder *encoder = sender;
    if (encoder == NULL) return;
    if (encoder->data == NULL) return;
    for (i = 0; i < dataSize; i++) /* write each byte (copy) */
        encoder->data[encoder->dc + i] = data[i];
    encoder->dc += dataSize; /* update counter */
}
void instruction_created_cb_2(void *sender, const statement *state, instruction inst) {
    int i;
    asmencoder *encoder = sender;
    if (encoder == NULL) return;
    if (encoder->code == NULL) return;
    for (i = 0; i < INS_SIZE; i++) /* add each byte of the instruction */
        encoder->code[encoder->ic + i] = (byte) (inst >> i*8);
    encoder->ic += INS_SIZE; /* update counter */
}
void extern_label_req_cb_2(void *sender, symbol_t *sp) {
    asmencoder *encoder = sender;
    if (encoder == NULL) return;
    if (encoder->extern_labels == NULL) return;
    encoder->extern_labels[encoder->ic] = sp;
}
