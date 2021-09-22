#include "asmencoder.h"

asmencoder *new_asmencoder(asmfile *file)
{
    asmencoder *new;

    if (file == NULL) return NULL;

    new = calloc(1, sizeof(asmencoder));
    if (new == NULL) {
        printf("ERROR: failed to allocate a new assembly encoder.\n");
        return NULL;
    }

    new->file = file;
    /* all of these are an option since we've used calloc */
    new->code = NULL;
    new->data = NULL;
    new->symbol_table = NULL;
    new->extern_labels = NULL;
    new->ic = 0;
    new->dc = 0;
    new->sc = 0;
    new->is_encoded = false;

    return new;
}

void free_asmencoder(asmencoder *encoder)
{ /* we'll not close and free up the asmfile here */
    if (encoder == NULL) return;
    if (encoder->code != NULL) free(encoder->code);
    if (encoder->data != NULL) free(encoder->data);
    if (encoder->symbol_table != NULL) free(encoder->symbol_table);
    if (encoder->extern_labels != NULL) free(encoder->extern_labels);
    free(encoder);
}

/*private*/
/* rise the error callback with the relevant arguments */
bool report_error(asmencoder *encoder, const char *msg, ...) {
    va_list ap;

    if (encoder == NULL) return false;

    encoder->file->error_occurred = true; /* flag that */

    if (encoder->file->error_callback != NULL) {
        va_start(ap, msg);
        encoder->file->error_callback(encoder->file, encoder->file, encoder->file->curr_line, 0, false, msg, ap);
        va_end(ap);

        return true;
    }

    return false;
}
/*private*/
/* same as report_error but not flagging that as an error (file->error_occurred)
 * and sets the can_continue argument to true */
bool report_warning(asmencoder *encoder, const char *msg, ...) {
    va_list ap;

    if (encoder->file != NULL && encoder->file->error_callback != NULL) {
        va_start(ap, msg);
        encoder->file->error_callback(encoder->file, encoder->file, encoder->file->curr_line, encoder->file->curr_col, true, msg, ap);
        va_end(ap);

        return true;
    }

    return false;
}

void reset_counters(asmencoder *encoder) {
    if (encoder == NULL) return;
    encoder->ic = 0;
    encoder->dc = 0;
}

bool read2end(asmencoder *encoder, parsing_callbacks pCallbacks, bool use_symbol_table, bool stop_on_error) {
    statement *state;
    parsing_args args = { 0 };
    bool err = false;
    if (encoder == NULL) return false;

    args.sender = encoder;
    args.symbol_table = use_symbol_table ? encoder->symbol_table : NULL;
    args.callbacks = pCallbacks;
    args.error_for_missing_label = use_symbol_table;

    while (!iseof(encoder->file)) {
        state = read_statement(encoder->file); /* if NULL, reading error or EOF */

        args.state = state;
        args.symbol_table_count = encoder->sc; /* set that here cuz it may was changed while reading via the callbacks */

        if (!iseof(encoder->file) || state != NULL)
            /*FIXED: try to parse just if it's not eof or that's the last statement */
            err |= state == NULL || !parse_statement(args);

        free_statement(state);
        if (err && stop_on_error) return false;
    }

    return !err;
}

/* reallocate size to the current IC counter value, returns true when succeeded */
bool realloc_code_array(asmencoder *encoder) {
    byte *tmp = realloc(encoder->code, encoder->ic);
    if (tmp == NULL) {
        report_error(encoder, "failed to allocate code memory."); /* will set the error_occurred flag */
        return false;
    }
    encoder->code = tmp;
    return true;
}
/* reallocate size to the current DC counter value, returns true when succeeded */
bool realloc_data_array(asmencoder *encoder) {
    byte *tmp = realloc(encoder->data, encoder->dc);
    if (tmp == NULL) {
        report_error(encoder, "failed to allocate data memory."); /* will set the error_occurred flag */
        return false;
    }
    encoder->data = tmp;
    return true;
}
/* reallocate the code and data arrays size to the current counters values, returns true when succeeded */
bool realloc_arrays(asmencoder *encoder) {
    return realloc_code_array(encoder) && realloc_data_array(encoder);
}

void update_symbol_table_values(symbol_t *symbol_table, ulong count, symbol_att att2update, ulong diff) {
    symbol_t *sp;
    for (sp = symbol_table; sp < (symbol_table + count); sp++) {
        if (sp->attributes & att2update)
            sp->value += diff;
    }
}

/* the first pass:
 * - reset counters (?)
 * - use the first pass callbacks
 * - read all of the statements and parse each of them (with the right callbacks and WITHOUT a symbol table)
 * - AFTER reading all, if error_occurred: free the dynamic buffer, reset counters and return false here.
 * - reallocate code and data arrays according to the IC and the DC (IC and DC will be the exact size of them)
 * - create a symbol table from the dynamic buffer (set also the SC counter) and free the dynamic buffer
 * - add the final IC to the value of each label in the symbol table
 * - if code or data are too big: report a warning (can continue) (was not done) */

/* the second pass:
 * - use the second pass callbacks
 * - reset IC and DC counters (we can save the real size to a variable if we'll need it)
 *   (it's ok if we'll not know the real size in the second pass, cuz we assume that FILE WAS NOT CHANGED)
 * - read all of the statements and parse each of them (with the right callbacks and WITH a symbol table)
 * - WHILE reading, if error_occurred: STOP! (the caller will have to free that encoder later) */

/* create a symbol table and reallocate code and data memory, will reset the IC and DC counters if succeeded
 * (the first pass) */
bool initialize_encoder(asmencoder *encoder, bool create_symbol_table_anyway) {
    ulong li;
    symbol_t *sp;
    dyBuffer *symbol_buff;
    parsing_callbacks pCallbacks = { NULL };

    if (encoder == NULL || encoder->file == NULL) return false;
    if (encoder->symbol_table != NULL) return false; /* already exists */

    symbol_buff = new_buffer(sizeof(symbol_t));
    if (symbol_buff == NULL) {
        report_error(encoder, "Failed to allocate symbol buffer.");
        return false;
    }

    if (encoder->symbol_table != NULL) free(encoder->symbol_table);
    if (encoder->extern_labels != NULL) free(encoder->extern_labels);
    encoder->sc = 0; /* reset symbols counter */

    /* temporary use the symbol_table pointer for our dynamic buffer
     * so we'll use it on the callback */
    encoder->symbol_table = (symbol_t *)symbol_buff;
    symbol_buff->exit_on_error = true;

    reset_counters(encoder);

    /* redirect reading errors */
    /*encoder->file->error_callback = encoder->error_callback; // do it just after the first pass */

    /* parsing callbacks */
    pCallbacks.error = &error_cb_1;
    pCallbacks.new_label = &new_label_cb_1;
    pCallbacks.data_write = &data_write_cb_1;
    pCallbacks.instruction_created = &instruction_created_cb_1;

    read2end(encoder, pCallbacks, false, false);

    /* after reading */
    encoder->symbol_table = NULL; /* anyway don't leave the buffer pointer in the encoder */

    /* redirect reading errors */
    encoder->file->error_callback = encoder->error_callback;

    /* search for entry labels that was not defined */
    sp = (symbol_t *)symbol_buff->data;
    for(li = 0; li < symbol_buff->length; li++, sp++)
        if (sp->attributes == entry_sa)
            report_error(encoder, "entry label '%s' was not defined in the current file.", sp->name);

    if (!encoder->file->error_occurred || create_symbol_table_anyway) {
        /* create a symbol table and free the dynamic symbol buffer */
        encoder->symbol_table = symbol_buff->data;
        encoder->sc = symbol_buff->length;
        free(symbol_buff); /* free the buffer but not its data (don't use free_buffer here) */

        /* symbol_table created, update the values in it */
        update_symbol_table_values(encoder->symbol_table, encoder->sc, data_sa, encoder->ic);
        /* offsetting */
        update_symbol_table_values(encoder->symbol_table, encoder->sc, data_sa|code_sa, CODE_OFFSET);

        /* init the extern_labels to 0 */
        encoder->extern_labels = calloc_or_exit(encoder->ic, sizeof(symbol_t *));

        if (realloc_arrays(encoder)) /* may will set the error flag */
            reset_counters(encoder); /* reset just after using them (last thing) */
    }
    else free_buffer(symbol_buff); /* free the buffer and its data cuz we didn't use it */

    return !encoder->file->error_occurred;
}

bool asm_encode(asmencoder *encoder) {
    parsing_callbacks pCallbacks = { NULL };

    if (encoder == NULL) return false;
    if (encoder->file == NULL) return false;
    if (encoder->is_encoded) return false; /* was encoded already */

    /* the first pass */
    if (initialize_encoder(encoder, true)) { /* set these callbacks just if succeeded */
        /* it makes sense to just return false here cuz we can't know how correct the errors on the second pass would be,
         * but you asked to print all of the errors at once instead of letting the user fix the errors until now */
        pCallbacks.new_label = &new_label_cb_2;
        pCallbacks.data_write = &data_write_cb_2;
        pCallbacks.instruction_created = &instruction_created_cb_2;
        pCallbacks.extern_label_required = &extern_label_req_cb_2;
    }

    pCallbacks.error = &error_cb_2; /* anyway report for errors on the second pass */

    /* redirect reading errors */
    encoder->file->error_callback = encoder->error_callback;

    /* the second pass */
    gotoline(encoder->file, 1);
    if (!read2end(encoder, pCallbacks, true, false)) return false;

    encoder->is_encoded = true;
    return !encoder->file->error_occurred;
}

bool asm_export_hex_file(asmencoder *encoder) {
    FILE *file;
    string filename;
    ulong i;
    byte *bp;

    if (encoder == NULL) return false;
    if (encoder->data == NULL || encoder->code == NULL) return false;
    if (encoder->file == NULL || encoder->file->filename == NULL) return false; /* where export to */
    if (!encoder->is_encoded) return false; /* was not encoded yet */

    filename = change_extension(encoder->file->filename, HEX_FILE_EXT);
    if (filename == NULL) return false;
    file = fopen(filename, "wb+");
    free(filename);
    if (file == NULL) return false;

    fprintf(file, "     %lu %lu", encoder->ic, encoder->dc);

    /* print code */
    bp = encoder->code;
    for (i = 0; i < encoder->ic; i++, bp++) {
        if (!(i % INS_SIZE)) {
            fprintf(file, "\n");
            fprintf(file, "%04lu ", i + CODE_OFFSET);
        }
        fprintf(file, "%02X ", (ubyte)*bp);
    }

    /* print data */
    bp = encoder->data;
    for (i = 0; i < encoder->dc; i++, bp++) {
        if (!(i % INS_SIZE)) {
            fprintf(file, "\n");
            fprintf(file, "%04lu ", i + encoder->ic + CODE_OFFSET);
        }
        fprintf(file, "%02X ", (ubyte)*bp);
    }

    fclose(file);
    return true;
}

bool asm_export_ent_file(asmencoder *encoder) {
    FILE *file;
    string filename;
    ulong li;
    symbol_t *sp;

    if (encoder == NULL) return false;
    if (encoder->symbol_table == NULL) return false;
    if (encoder->file == NULL || encoder->file->filename == NULL) return false; /* where export to */
    if (!encoder->is_encoded) return false; /* was not encoded yet */

    filename = change_extension(encoder->file->filename, ENTRY_FILE_EXT);
    if (filename == NULL) return false;
    file = fopen(filename, "wb+");
    free(filename);
    if (file == NULL) return false;

    sp = encoder->symbol_table;
    for (li = 0; li < encoder->sc; li++, sp++) {
        if (sp->attributes & entry_sa) /* it's an entry symbol */
            fprintf(file, "%s %04lu\n", sp->name, sp->value); /* note that symbol table is offsetted already */
    }

    fclose(file);
    return true;
}

bool asm_export_ext_file(asmencoder *encoder) {
    FILE *file;
    string filename;
    ulong li;
    symbol_t **sp;

    if (encoder == NULL) return false;
    if (encoder->extern_labels == NULL) return false;
    if (encoder->file == NULL || encoder->file->filename == NULL) return false; /* where export to */
    if (!encoder->is_encoded) return false; /* was not encoded yet */

    filename = change_extension(encoder->file->filename, EXTERN_FILE_EXT);
    if (filename == NULL) return false;
    file = fopen(filename, "wb+");
    free(filename);
    if (file == NULL) return false;

    sp = encoder->extern_labels;
    for (li = 0; li < encoder->ic; li++, sp++)
        if (*sp != NULL)
            fprintf(file, "%s %04lu\n", (*sp)->name, CODE_OFFSET + li);

    fclose(file);
    return true;
}
