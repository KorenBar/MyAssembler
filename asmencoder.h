#include <stdio.h>
#include "mytypes.h"
#include "strhelper.h"
#include "asmfile.h"
#include "command.h"
#include "directive.h"

#ifndef ASMENCODER_H
#define ASMENCODER_H

#define REGISTER_COUNT  32
#define RAM_SIZE        25 /*bytes*/ /* code size couldn't be bigger */
#define REG_SIZE        4 /*bytes*/
#define INS_SIZE        4 /*bytes*/

#define REG_SYMBOL '$'
#define INVALID_REG -1
#define FIRST_REG 0
#define LAST_REG 31

#define CODE_OFFSET 100

#define EXTERN_FILE_EXT ".ext"
#define ENTRY_FILE_EXT ".ent"
#define HEX_FILE_EXT ".ob"

typedef struct asmencoder asmencoder;
struct asmencoder {
    asmfile *file;
    byte *code;
    byte *data;
    symbol_t *symbol_table;
    symbol_t **extern_labels; /* IC in length, each i value point to the extern symbol used in instruction no. i */
    ulong ic, dc, sc; /* (i)nstruction, (d)ata and (s)ymbol counters */
    bool is_encoded;
    file_error_cb error_callback;
};

asmencoder *new_asmencoder(asmfile *file);
void free_asmencoder(asmencoder *encoder);
/* returns false if failed to encode or was encoded already */
bool asm_encode(asmencoder *encoder);
/* returns false if failed to export or was not encoded yet */
bool asm_export_hex_file(asmencoder *encoder);
/* returns false if failed to export or was not encoded yet */
bool asm_export_ent_file(asmencoder *encoder);
/* returns false if failed to export or was not encoded yet */
bool asm_export_ext_file(asmencoder *encoder);

/* these recursive include must be here in the end of that file (also for better debugging with clion) */
#include "parsing.h"
#include "callbacks.h"
#endif
