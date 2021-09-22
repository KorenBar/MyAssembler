#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mytypes.h"
#include "strhelper.h"

#ifndef COMMAND_H
#define COMMAND_H

#define CMD_SIZE 32/*bits*/
#define MAX_IMMED_VAL 32768 /* 2^15 */
#define MIN_IMMED_VAL -32769 /* -(2^15 + 1) */

/* FIXED: don't print warnings with the -pedantic flag.
 * instead of:
 * #define ADD_CMD  (cmd){ .name = "add",  .opcode = 0, .funct = 1, .type = R_CMD, .num_of_args = 3 }
 * write:
 * #define ADD_CMD  { "add", 0, 1, R_CMD, 3 }
*/

/* R Commands */
#define ADD_CMD  { "add",  0,  1, R_CMD, 3 }
#define SUB_CMD  { "sub",  0,  2, R_CMD, 3 }
#define AND_CMD  { "and",  0,  3, R_CMD, 3 }
#define OR_CMD   { "or",   0,  4, R_CMD, 3 }
#define NOR_CMD  { "nor",  0,  5, R_CMD, 3 }
#define MOVE_CMD { "move", 1,  1, R_CMD, 2 }
#define MVHI_CMD { "mvhi", 1,  2, R_CMD, 2 }
#define MVLO_CMD { "mvlo", 1,  3, R_CMD, 2 }
/* I Commands */
#define ADDI_CMD { "addi", 10, 0, I_CMD, 3 }
#define SUBI_CMD { "subi", 11, 0, I_CMD, 3 }
#define ANDI_CMD { "andi", 12, 0, I_CMD, 3 }
#define ORI_CMD  { "ori",  13, 0, I_CMD, 3 }
#define NORI_CMD { "nori", 14, 0, I_CMD, 3 }
#define BNE_CMD  { "bne",  15, 0, I_CMD, 3 }
#define BEQ_CMD  { "beq",  16, 0, I_CMD, 3 }
#define BLT_CMD  { "blt",  17, 0, I_CMD, 3 }
#define BGT_CMD  { "bgt",  18, 0, I_CMD, 3 }
#define LB_CMD   { "lb",   19, 0, I_CMD, 3 }
#define SB_CMD   { "sb",   20, 0, I_CMD, 3 }
#define LW_CMD   { "lw",   21, 0, I_CMD, 3 }
#define SW_CMD   { "sw",   22, 0, I_CMD, 3 }
#define LH_CMD   { "lh",   23, 0, I_CMD, 3 }
#define SH_CMD   { "sh",   24, 0, I_CMD, 3 }
/* J Commands */
#define JMP_CMD  { "jmp",  30, 0, J_CMD, 1 }
#define LA_CMD   { "la",   31, 0, J_CMD, 1 }
#define CALL_CMD { "call", 32, 0, J_CMD, 1 }
#define STOP_CMD { "stop", 63, 0, J_CMD, 0 }

#define NONE_CMD { NULL }

typedef enum { R_CMD, I_CMD, J_CMD } cmd_type;

typedef struct {
    const char *name; /* that will be ok to copy that pointer since the names are always exists in the commands array */
    byte opcode;
    byte funct;
    cmd_type type;
    byte num_of_args;
} cmd;

/* we'll use it to search for a cmd by its name */
static const cmd commands[] =  { ADD_CMD, SUB_CMD, AND_CMD, OR_CMD, NOR_CMD, MOVE_CMD, MVHI_CMD, MVLO_CMD,
                                 ADDI_CMD, SUBI_CMD, ANDI_CMD, ORI_CMD, NORI_CMD, BNE_CMD, BEQ_CMD,
                                 BLT_CMD, BGT_CMD, LB_CMD, SB_CMD, LW_CMD, SW_CMD, LH_CMD, SH_CMD,
                                 JMP_CMD, LA_CMD, CALL_CMD, STOP_CMD, NONE_CMD };
typedef enum                   { add_cmd, sub_cmd, and_cmd, or_cmd, nor_cmd, move_cmd, mvhi_cmd, mvlo_cmd,
                                 addi_cmd, subi_cmd, andi_cmd, ori_cmd, nori_cmd, bne_cmd, beq_cmd,
                                 blt_cmd, bgt_cmd, lb_cmd, sb_cmd, lw_cmd, sw_cmd, lh_cmd, sh_cmd,
                                 jmp_cmd, la_cmd, call_cmd, stop_cmd, none_cmd } cmd_index;

/* Instructions */
typedef myuint32_t instruction;

typedef union {
    struct __attribute__((__packed__)) {
        /* can't use int32(long) here with the -pedantic flag, but that will be ok with int16(int) */
        myuint16_t unused : 6,
                funct: 5,
                rd: 5,
                rt: 5,
                rs: 5,
                opcode: 6;
    } fields;
    myuint32_t bits;
} cmd_r;

typedef union {
    struct __attribute__((__packed__)) {
        /* can't use int32(long) here with the -pedantic flag, but that will be ok with int16(int) */
        myuint16_t immed : 16,
                rt : 5,
                rs : 5,
                opcode : 6;
    } fields;
    myuint32_t bits;
} cmd_i;

typedef union {
    struct __attribute__((__packed__)) {
        /* can't use int32(long) here with the -pedantic flag, but that will be ok with int16(int) */
        myuint16_t address : 25,
                reg : 1,
                opcode : 6;
    } fields;
    myuint32_t bits;
} cmd_j;


/* get index of given command name or none_cmd if not exists */
cmd_index get_cmd_index(const char *name);
/* get index of similar name to a given command name or none_cmd if not exists */
cmd_index get_similar_cmd_index(const char *name);
/* find a command by a given name */
cmd getcmd(const char *name);
bool is_valid_cmd_name(const char *name);
/* find a command with a similar name */
cmd get_similar_cmd(const char *name);
/* get an initial instruction for a given command info */
myuint32_t cmd2instruction(cmd cmd);
bool is_cmd_equals(cmd a, cmd b);
bool is_cmd_exists(cmd cmd);

#endif
