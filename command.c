#include "command.h"

cmd_index get_cmd_index(const char *name) {
    cmd_index i = 0;
    for (i = 0; i != none_cmd; i++)
        if (!strcmp(commands[i].name, name))
            return i; /* found */
    return i; /* none_dir */
}

cmd_index get_similar_cmd_index(const char *name) {
    cmd_index i = 0;
    for (i = 0; i != none_cmd; i++)
        if (!strhlfcmp(commands[i].name, name))
            return i; /* found */
    return i; /* none_dir */
}

cmd getcmd(const char *name) {
    return commands[get_cmd_index(name)];
}

bool is_valid_cmd_name(const char *name) { /* a cmd name contains lowercase letters only. */
    const char *p;
    for (p = name; *p != '\0'; p++)
        if (!islower(*p)) return false;
    return true;
}

cmd get_similar_cmd(const char *name) {
    return commands[get_similar_cmd_index(name)];
}

myuint32_t cmd2instruction(cmd cmd) {
    cmd_r cr;
    cmd_i ci;
    cmd_j cj;

    cr.bits = ci.bits = cj.bits = 0;

    switch (cmd.type) {
        case R_CMD:
            cr.fields.opcode = cmd.opcode;
            cr.fields.funct = cmd.funct;
            return cr.bits;
        case I_CMD:
            ci.fields.opcode = cmd.opcode;
            return ci.bits;
        case J_CMD:
            cj.fields.opcode = cmd.opcode;
            return cj.bits;
        default:
            return 0;
    }

    /*// FIXED to the -pedantic flag
    switch (cmd.type) {
        case R_CMD: return (cmd_r){ .fields.opcode = cmd.opcode, .fields.funct = cmd.funct }.bits;
        case I_CMD: return (cmd_i){ .fields.opcode = cmd.opcode }.bits;
        case J_CMD: return (cmd_j){ .fields.opcode = cmd.opcode }.bits;
        default: return 0;
    }*/
}

bool is_cmd_equals(cmd a, cmd b) {
    return memcmp(&a, &b, sizeof(cmd)) == 0;
}

bool is_cmd_exists(cmd cmd) {
    return !is_cmd_equals(cmd, commands[none_cmd]);
}
