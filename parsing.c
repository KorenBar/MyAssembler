#include "parsing.h"

/* callers (rises callbacks) */
bool error_callback(error_cb cb, asmencoder *sender, const statement *state, bool can_continue, const char *msg, ...) {
    va_list ap;
    if (cb == NULL) return false;
    va_start(ap, msg);
    cb(sender, state, can_continue, msg, ap);
    va_end(ap);
    return true;
}
bool data_callback(data_write_cb cb, void *sender, const statement *state, const byte *data, ulong data_size) {
    if (cb == NULL) return false;
    cb(sender, state, data, data_size);
    return true;
}
bool instruction_callback(instruction_created_cb cb, void *sender, const statement *state, instruction inst) {
    if (cb == NULL) return false;
    cb(sender, state, inst);
    return true;
}

/* check if a string is a valid register name without checking the number range (input should be trimmed) */
bool is_reg(const char *str) {
    if (*str != REG_SYMBOL) return false; /* invalid symbol */
    if (!isinteger(str+1)) return false;
    return true;
}

/* convert register string name to a number between 0 to 31
 * returns -1 if that name is invalid */
byte get_reg_num(const char *str) {
    myint32_t res;

    if (*str != REG_SYMBOL) return INVALID_REG; /* invalid symbol */
    if (check_int32_str(++str) != num_in_range)
        return INVALID_REG; /* invalid number or out of the int32 range */

    res = atol(str); /* to long cuz we have checked if it's in range of int32 */
    if (res < FIRST_REG || res > LAST_REG) return INVALID_REG; /* out of range (reg not exists) */

    return (byte)res; /* byte should be enough anyway */
}
byte get_reg_num_or_rise_err(const char *str, bool *err_flag, parsing_args args) {
    byte res = get_reg_num(str);
    if (res == INVALID_REG) {
        *err_flag = true;
        error_callback(args.callbacks.error, args.sender, args.state, false,
                       "invalid register '%s'", str);
    }
    return res;
}

ulong get_label_v_or_rise_err(const char *str, symbol_att allow_att, bool *err_flag, parsing_args args) {
    string msg;
    symbol_t *symbol = get_label_by_name(args.symbol_table, args.symbol_table_count, str);

    if (symbol == NULL) { /* not found */
        if (args.error_for_missing_label) {
            *err_flag = true;
            error_callback(args.callbacks.error, args.sender, args.state, false,
                           "label '%s' was not found.", str);
        }
        return 0;
    }

    if (!(allow_att & symbol->attributes)) { /* not allowed attribute */
        *err_flag = true;
        msg = symbol->attributes & external_sa ? "extern "
                : symbol->attributes & data_sa ? "data "
                    : symbol->attributes & code_sa ? "code " : "";
        error_callback(args.callbacks.error, args.sender, args.state, false,
                       "can't use %slabel '%s' for '%s' statement.", msg, str, args.state->name);
        return 0;
    }

    if (symbol->attributes & external_sa)
        if (args.callbacks.extern_label_required != NULL)
            args.callbacks.extern_label_required(args.sender, symbol);

    return symbol->value;
}
long get_label_dist_or_rise_err(const char *str, bool *err_flag, parsing_args args) {
    bool intern_err;
    ulong l_val;
    long sl_val, sic; /* signed values */
    asmencoder *encoder;

    intern_err = false;
    l_val = get_label_v_or_rise_err(str, all_sa&~external_sa/*all but extern*/, &intern_err, args);

    if (intern_err) {
        *err_flag = true;
        return 0;
    }

    if (args.sender == NULL) return 0; /* shouldn't happen */
    encoder = args.sender;

    /* set the sign bit to 0 in case of too big value, so we'll not get a negative value */
    sl_val = (long)l_val;
    SIGN_BIT_OFF(sl_val)

    sic = (long)encoder->ic + CODE_OFFSET;
    SIGN_BIT_OFF(sic)

    return sl_val - sic; /* now it's safe to do it (sign-safe) */
}

myint32_t get_immed_v_or_rise_err(const char *str, bool *err_flag, parsing_args args) {
    myint32_t res;
    range_res rr;
    string msg;

    rr  = check_int32_str(str);
    if (rr != num_in_range) {
        *err_flag = true;

        if (rr == bigger_num) msg = "too big";
        else if (rr == smaller_num) msg = "too small";
        else msg = "invalid";

        error_callback(args.callbacks.error, args.sender, args.state, false,
                       "'%s' is %s integer.", str, msg);
        return 0; /* invalid number or out of the int32 range */
    }

    res = atol(str); /* to long cuz we have checked if it's in range of int32 */
    if (res < MIN_IMMED_VAL || res > MAX_IMMED_VAL) {
        *err_flag = true;
        error_callback(args.callbacks.error, args.sender, args.state, false,
                       "'%s' is %s integer.", str,
                       res < MIN_IMMED_VAL ? "too small" : "too big");
        return 0; /* out of immed range */
    }

    return res;

}

bool validate_label_name(string label_name, parsing_args args) {
    char *c;
    if (label_name == NULL) return false;
    if (*label_name == '\0') { /* label can't be empty */
        error_callback(args.callbacks.error, args.sender, args.state, false, "an invalid empty label.");
        return false;
    }
    if (!isalpha(*label_name)) { /* label must start with a letter */
        error_callback(args.callbacks.error, args.sender, args.state, false,
                       "a label must start with a letter, \"%s\" is invalid label.", label_name);
        return false;
    }
    for (c = label_name; *c != '\0'; c++)
        if (!isalpha(*c) && !isdigit(*c)) { /* label can't contains spacial characters */
            error_callback(args.callbacks.error, args.sender, args.state, false,
                           "a label must contains letters and digits only, \"%s\" is invalid label.", label_name);
            return false;
        }
        else if (label_name - c == MAX_LABEL_LEN) /* label is too long (just a warning) */
            error_callback(args.callbacks.error, args.sender, args.state, true, "label is longer than %d.", MAX_LABEL_LEN);
    if (is_cmd_exists(getcmd(label_name))) {
        error_callback(args.callbacks.error, args.sender, args.state, false,
                       "label can't be a command name \"%s\"", label_name);
        return false;
    }

    return true;
}

symbol_t *get_label_by_name(const symbol_t *symbol_table, ulong symbol_table_count, const char *label_name) {
    ulong i;
    if (symbol_table == NULL) return NULL;
    for (i = 0; i < symbol_table_count; i++)
        if (strcmp(symbol_table[i].name, label_name) == 0)
            return (symbol_t *)symbol_table + i; /* found */
    return NULL; /* not found */
}

bool parse_dir_args(dir_index dir_i, parsing_args args)
{
    /* check each arg while parsing it
     * allocate data, rise callback and then free that data.
     * if it's extern or entry rise the new_label callback
     * if it's not labelable but label exists report warning
     * FIXED: don't rise new_label callback for standard label on entry or extern, the label in that case should be an argument!
     */

    ulong len;
    int i;
    myint32_t val; /* the max possible */
    string tmp_str, s, *sp;
    directive dir;
    byte *data, *bp;
    range_res range_res;
    symbol_t symbol = { NULL };

    /* init */
    dir = directives[dir_i];
    tmp_str = s = NULL;
    sp = NULL;
    data = bp = NULL;

    /* we can assume here that we have the correct number of arguments */
    /* a little bit long but it's easier for me to read all of these in one place */
    switch (dir_i) {
        case db_dir:
        case dw_dir:
        case dh_dir:
            len = args.state->params_count * dir.data_size;
            if (!len) {
                error_callback(args.callbacks.error, args.sender, args.state, true,
                               "passing over a directive data statement with no arguments (ignoring).");
                break; /* just pass over, nothing to add (return true later) */
            }

            /* all of the arguments should be valid integers */
            for (sp = args.state->params; (sp - args.state->params) < args.state->params_count; sp++) {
                range_res = check_int32_str(*sp);
                if (range_res != num_in_range) { /* invalid or out of range */
                    range_res = check_int32_str(*sp);
                    error_callback(args.callbacks.error, args.sender, args.state, false,
                                   range_res == bigger_num
                                        ? "'%s' is too big integer."
                                        : range_res == smaller_num
                                            ? "'%s' is too small integer."
                                            : "'%s' is not an integer.", *sp);
                    return false;
                }
            }

            bp = data = malloc_or_exit(len); /* bp is our moving byte pointer */
            for (sp = args.state->params; (sp - args.state->params) < args.state->params_count; sp++) {
                val = atol(*sp); /* we know that sp is a valid integer(32) string */
                for (i = 0; i < dir.data_size; i++, bp++, val >>= 8) /* and each byte of the current param value */
                    *bp = (byte)val;
            }

            break;
        case asciz_dir:
            tmp_str = args.state->params[0];
            len = strlen(tmp_str);
            if (len < 2 || tmp_str[0] != STRING_CHAR || tmp_str[len - 1] != STRING_CHAR) {
                error_callback(args.callbacks.error, args.sender, args.state, false,
                               "'%s' is invalid string, a string must be written between %c%c.",
                               tmp_str, STRING_CHAR, STRING_CHAR);
                return false;
            }

            /* make sure it's a printable string */
            for (s = tmp_str; *s != '\0'; s++)
                if (!isprint(*s) && !isspace(*s)/*(?) was not defined clearly in the document*/)
                { /* unprintable character */
                    error_callback(args.callbacks.error, args.sender, args.state, false,
                                   "'%s' is unprintable string.", tmp_str);
                    return false;
                }

            /* remove string characters */
            tmp_str[len-1] = '\0';
            tmp_str++;
            data_callback(args.callbacks.data_write, args.sender, args.state, (byte *)tmp_str, len - 2/*string characters*/ + 1/*null-terminate*/);
            /* return them just in case someone will use that source statement later */
            tmp_str--;
            tmp_str[len-1] = STRING_CHAR;

            break;
        case entry_dir:
        case extern_dir:
            tmp_str = args.state->params[0];
            if (!validate_label_name(tmp_str, args)) return false; /* invalid label name (an error was reported) */
            if (args.callbacks.new_label != NULL) { /* new label callback */
                symbol.attributes = dir_i == entry_dir ? entry_sa : external_sa;
                symbol.name = allocate_string(tmp_str);
                args.callbacks.new_label(args.sender, args.state, symbol);
            }
            break;
        default:
            error_callback(args.callbacks.error, args.sender, args.state, false,
                           "undefined directive (no. %d) while parsing, make sure all of the defined directives are handled in that assembler!", dir_i);
            return false;
    }

    if (data != NULL) { /* rise data_write callback and then free that data */
        data_callback(args.callbacks.data_write, args.sender, args.state, data, len);
        free(data);
    }

    return true;
}

bool directive_statement(parsing_args args)
{
    string msg;
    directive dir;
    dir_index dir_i;

    dir_i = get_directive_index(args.state->name);

    if (dir_i == none_dir) { /* was not found */
        dir_i = get_similar_directive_index(args.state->name);
        dir = directives[dir_i];

        if (is_valid_directive_name(args.state->name)) /* unknown */
            msg = dir_i != none_dir /* similar was found */
                  ? "Unknown directive name \"%s\", did you mean to \"%s\"?"
                  : "Unknown directive name \"%s\""; /* currently we'll not even check for similar in case of valid */
        else /* invalid */
            msg = dir_i != none_dir /* similar was found */
                  ? "Invalid directive name \"%s\", did you mean to \"%s\"?"
                  : "Invalid directive name \"%s\"";

        error_callback(args.callbacks.error, args.sender, args.state, false, msg, args.state->name, dir.name);
        return false;
    }

    dir = directives[dir_i];

    /* check num of arguments */
    if (dir.num_of_args != UNLIMITED && dir.num_of_args != args.state->params_count) { /* wrong number of arguments */
        if (args.state->params_count < dir.num_of_args)
            error_callback(args.callbacks.error, args.sender, args.state, false,
                           "missing arguments for directive '%s'. expected %d, actual %d.",
                           dir.name, dir.num_of_args, args.state->params_count);
        else
            error_callback(args.callbacks.error, args.sender, args.state, false,
                           "too many arguments for directive '%s'. expected %d, actual %d.",
                           dir.name, dir.num_of_args, args.state->params_count);
        return false;
    }

    return parse_dir_args(dir_i, args);
}

bool parse_cmd_args(cmd_index cmd_idx, parsing_args args)
{
    cmd cmd;
    instruction inst;
    bool err;

    cmd_r *cr;
    cmd_i *ci;
    cmd_j *cj;

    err = false;
    cmd = commands[cmd_idx];
    inst = cmd2instruction(cmd);

    /* must these pointer to set the fields properly */
    cr = (cmd_r *)(ci = (cmd_i *)(cj = (cmd_j *)&inst));

    /* label => label_value - current_ic */
    /* assume that we have the correct number of arguments */

    /* cases of operands */
    switch (cmd_idx) {
        case add_cmd:
        case sub_cmd:
        case and_cmd:
        case or_cmd:
        case nor_cmd:
            /* R -> rs,rt,rd */
            cr->fields.rs = (myuint32_t)get_reg_num_or_rise_err(args.state->params[0], &err, args);
            cr->fields.rt = (myuint32_t)get_reg_num_or_rise_err(args.state->params[1], &err, args);
            cr->fields.rd = (myuint32_t)get_reg_num_or_rise_err(args.state->params[2], &err, args);
            break;
        case move_cmd:
        case mvhi_cmd:
        case mvlo_cmd:
            /* R -> rd,rs */
            /* FIXED: wrong definition for these commands in the document. swapped between rd and rs */
            cr->fields.rd = (myuint32_t)get_reg_num_or_rise_err(args.state->params[1], &err, args);
            cr->fields.rs = (myuint32_t)get_reg_num_or_rise_err(args.state->params[0], &err, args);
            break;
        case addi_cmd:
        case subi_cmd:
        case andi_cmd:
        case ori_cmd:
        case nori_cmd:
        case lb_cmd:
        case sb_cmd:
        case lw_cmd:
        case sw_cmd:
        case lh_cmd:
        case sh_cmd:
            /* I -> rs,immed,rt */
            ci->fields.rs    = (myuint32_t)get_reg_num_or_rise_err(args.state->params[0], &err, args);
            ci->fields.immed = (myuint32_t)get_immed_v_or_rise_err(args.state->params[1], &err, args);
            ci->fields.rt    = (myuint32_t)get_reg_num_or_rise_err(args.state->params[2], &err, args);
            break;
        case bne_cmd:
        case beq_cmd:
        case blt_cmd:
        case bgt_cmd:
            /* I -> rs,rt,label */
            ci->fields.rs    = (myuint32_t)get_reg_num_or_rise_err(args.state->params[0], &err, args);
            ci->fields.rt    = (myuint32_t)get_reg_num_or_rise_err(args.state->params[1], &err, args);
            ci->fields.immed = (myuint32_t)get_label_dist_or_rise_err(args.state->params[2], &err, args);
            break;
        case jmp_cmd:
            /* J -> label/reg */
            if (is_reg(args.state->params[0])) { /* that's a register */
                cj->fields.reg = 1;
                cj->fields.address = (myuint32_t) get_reg_num_or_rise_err(args.state->params[0], &err, args);
            } else /* not a register, probably a label */
                cj->fields.address = (myuint32_t)get_label_v_or_rise_err(args.state->params[0], all_sa/*code_sa|external_sa*/, &err, args);
            break;
        case la_cmd:
        case call_cmd:
            /* J -> label */
            cj->fields.address = (myuint32_t)get_label_v_or_rise_err(args.state->params[0], all_sa/*code_sa|external_sa*/, &err, args);
            break;
        case stop_cmd: /* J -> no operands */
            break; /* nothing to add */
        default:
            error_callback(args.callbacks.error, args.sender, args.state, false,
                           "undefined command (no. %d) while parsing, make sure all of the defined commands are handled in that assembler!", cmd_idx);
            err = true;
    }

    /* reach here anyway even if was an error (cuz of that I'm using an error flag) */
    instruction_callback(args.callbacks.instruction_created, args.sender, args.state, inst);

    return !err;
}

bool command_statement(parsing_args args)
{
    string msg;
    cmd cmd;
    cmd_index cmd_idx;

    cmd_idx = get_cmd_index(args.state->name);

    if (cmd_idx == none_cmd) { /* was not found */
        cmd_idx = get_similar_cmd_index(args.state->name);
        cmd = commands[cmd_idx];

        if (is_valid_cmd_name(args.state->name)) /* unknown */
            msg = cmd_idx != none_cmd /* similar was found */
                  ? "Unknown command name \"%s\", did you mean to \"%s\"?"
                  : "Unknown command name \"%s\""; /* currently we'll not even check for similar in case of valid */
        else /* invalid */
            msg = cmd_idx != none_cmd /* similar was found */
                  ? "Invalid command name \"%s\", did you mean to \"%s\"?"
                  : "Invalid command name \"%s\"";

        error_callback(args.callbacks.error, args.sender, args.state, false, msg, args.state->name, cmd.name);
        return false;
    }

    cmd = commands[cmd_idx];

    /* check num of arguments */
    if (cmd.num_of_args != UNLIMITED && cmd.num_of_args != args.state->params_count) { /* wrong number of arguments */
        if (args.state->params_count < cmd.num_of_args)
            error_callback(args.callbacks.error, args.sender, args.state, false,
                           "missing arguments for command '%s'. expected %d, actual %d.",
                           cmd.name, cmd.num_of_args, args.state->params_count);
        else
            error_callback(args.callbacks.error, args.sender, args.state, false,
                           "too many arguments for command '%s'. expected %d, actual %d.",
                           cmd.name, cmd.num_of_args, args.state->params_count);
        return false;
    }

    return parse_cmd_args(cmd_idx, args);
}

symbol_att get_label_att(statement state)
{
    dir_index dir_i = get_directive_index(state.name);
    if (dir_i != none_dir) { /* it's a directive */
        if (dir_i == extern_dir)    return external_sa;
        if (dir_i == entry_dir)     return entry_sa;
        return data_sa; /* if not entry or extern, it's some data */
    }
    return code_sa; /* if not a directive assume it's code */
}

bool label_handling(parsing_args args)
{
    symbol_att sa;
    symbol_t symbol = { NULL };

    if (args.state == NULL) return false; /* no statement to parse */

    if (args.state->label != NULL) { /* label exists */
        if (!validate_label_name(args.state->label, args)) return false; /* invalid label name (an error was reported) */
        sa = get_label_att(*(args.state));
        if (sa & (external_sa | entry_sa)) /* it's entry or extern, isn't labelable, report a warning */
            error_callback(args.callbacks.error, args.sender, args.state, true,
                           "label \"%s\" for statement \"%s\" was ignored.", args.state->label, args.state->name);
        else if (args.callbacks.new_label != NULL) { /* new label callback */
            symbol.name = allocate_string(args.state->label);
            symbol.attributes = sa;
            args.callbacks.new_label(args.sender, args.state, symbol);
        }
    }

    return true;
}

bool parse_statement(parsing_args args)
{
    if (args.state == NULL) return false; /* no statement to parse */

    if (!label_handling(args)) return false; /* invalid label (an error was reported) */

    if (is_directive_name(args.state->name)) /* directive */
        return directive_statement(args);
    else return command_statement(args); /* command */
}
