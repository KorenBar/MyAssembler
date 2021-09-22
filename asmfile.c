#include "asmfile.h"


asmfile *new_asmfile(string filename)
{
    FILE *file;
    asmfile *new;

    /* as bytes for moving without problem and write permission to promise will not be edited while encoding */
    file = fopen(filename, "rb+");
    if (file == NULL) {
        printf("ERROR: \"%s\" source file was not found or can't be read.\n", filename);
        return NULL;
    }

    new = malloc(sizeof(asmfile));
    if (new == NULL) {
        printf("ERROR: failed to allocate a new assembly file.\n");
        fclose(file);
        return NULL;
    }

    new->filename = allocate_string(filename); /* copy! */
    new->src_file = file;
    new->curr_line = 1;
    new->curr_col = 1;
    new->error_callback = NULL;
    new->buffer = new_buffer(sizeof(char));
    /* if can't allocate memory for the dynamic buffer, we've actually nothing to do */
    new->buffer->exit_on_error = true;
    new->error_occurred = false;

    return new;
}

void free_asmfile(asmfile *asmfile)
{
    if (asmfile == NULL) return;

    if (asmfile->filename != NULL) free(asmfile->filename);
    if (asmfile->src_file != NULL) fclose(asmfile->src_file);
    asmfile->error_callback = NULL;
    if (asmfile->buffer != NULL) free_buffer(asmfile->buffer);

    free(asmfile);
}

void free_symbol_content(symbol_t symbol)
{
    if (symbol.name != NULL) free(symbol.name);
}

/*private*/
/* rise the error callback with the relevant arguments */
static bool report_error(asmfile *file, const char *msg, ...) {
    va_list ap;

    if (file == NULL) return false;

    file->error_occurred = true; /* flag that */

    if (file->error_callback != NULL) {
        va_start(ap, msg);
        file->error_callback(file, file, file->curr_line, file->curr_col, false, msg, ap);
        va_end(ap);

        return true;
    }

    return false;
}
/*private*/
/* same as report_error but not flagging that as an error (file->error_occurred)
 * and sets the can_continue argument to true */
static bool report_warning(asmfile *file, const char *msg, ...) {
    va_list ap;

    if (file != NULL && file->error_callback != NULL) {
        va_start(ap, msg);
        file->error_callback(file, file, file->curr_line, file->curr_col, true, msg, ap);
        va_end(ap);

        return true;
    }

    return false;
}

bool reset_asmfile_buffer(asmfile *file)
{
    if (file->buffer == NULL) /* string buffer was not created yet */
        /* create a string buffer for the first time (it's not a thread-safe way) */
        if ((file->buffer = new_buffer((uint)sizeof(char))) == NULL)
            return false; /* can't allocate a buffer */

    clear_buffer(file->buffer); /* clear it in case of reusing */

    return true;
}

/*private*/
/* returns false if was an error */
bool fill_label_and_name(statement *state, asmfile *file) {
    int c;
    int separators[] = {':', ',', '\n', EOF};

    if (state == NULL || file == NULL) return false;
    if (!reset_asmfile_buffer(file)) return false; /* can't allocate a buffer */

    skip_spaces(file);

    while (!is_int_of(c = agetc(file), separators, 4, &isinlinespace))
    {
        /* collect that char */
        if (!add2buffer(file->buffer, (char*)&c, 1))
        { /* probably failed to extend buffer */
            free_statement(state);
            return false;
        }
    }

    /* check stop reading reason */
    switch (c) {
        case ':': /* it's a label */
            if (file->buffer->length > MAX_LABEL_LEN)
                report_warning(file, "label is longer than %d.", MAX_LABEL_LEN);
            if (state->label == NULL) { /* save that label */
                state->label = buffer2string(file->buffer);
                /* after reading the label do the same to read the statement name */
                return fill_label_and_name(state, file); /* max of one recursive call cuz a label is exists now */
            } else /* a label already exists */
                report_error(file, "two labels was written on a single line.");
            return false;
        case ',': /* illegal comma */
            report_error(file, "illegal comma.");
            return false;
        case '\n': /* end of line */
        case EOF: /* end of file */
        default: /* a space - it's a name */
            state->name = buffer2string(file->buffer);
            return true;
    }
}

/*private*/
/* read escapable string */
/* returns false if string was not ended in line */
/* was not used cuz it's not escaping spacial characters as well */
bool read_str(asmfile *file) {
    int c;
    int separators[] = {'\n', EOF};
    if (file == NULL) return false;
    while (!is_int_of(c = agetc(file), separators, 2, NULL))
        if (c == '\"')
            /* if the last in the buffer is a back-slash replace it and keep collecting */
            if (last_in_buff(file->buffer) != NULL && *((char *)last_in_buff(file->buffer)) == '\\')
                *((char *)last_in_buff(file->buffer)) = (char)c;
            else return true; /* end of string */
        else put2buffer(file->buffer, &c); /* collect that character */
    return false; /* string was not closed in line */
}

/*private*/
/* read a string til end of line.
 * when calling that func the last non-space character in the current line must be ".
 * returns false if that is an invalid string */
bool read_str_line(asmfile *file) {
    int c;
    int separators[] = {'\n', EOF};

    char last_non_space = '\0';
    if (file == NULL) return false;

    /* while line was not ended */
    while (!is_int_of(c = agetc(file), separators, 2, NULL)) {
        put2buffer(file->buffer, &c); /* collect */
        if (!isspace(c)) last_non_space = (char)c;
    }

    if (last_non_space != '\"') return false; /* last non-space char in that line is not " */
    return true;
}

/*private*/
/* returns false if was a syntax error */
bool fill_arguments(statement *state, asmfile *file) {
    int c;
    string str;
    bool in_str;
    bool escape_next;
    int separators[] = {ARG_SEP, '\n', EOF};

    if (state == NULL || file == NULL) return false;
    if (!reset_asmfile_buffer(file)) return false; /* can't allocate a buffer */

    c = ARG_SEP;
    in_str = escape_next = false;

    /* while line was not ended */
    while (c == ARG_SEP) { /* get params */
        /*skip_spaces(file); // don't. that will skip over '\n'. don't worry, arg will be trimmed later */

        /* note that in case of multiple string arguments you have to escape any '"' inside a string with '\'
         * otherwise it will close that string and the real last '"' will open a new string
         * that will not refer ',' as a separator inside it.
         * all of that way is to allow us inserting ',' inside a string arg while supporting multiple string args,
         * so you must follow the escape rules */

        /* while is not a separator, collect characters */
        while (!is_int_of(c = apeekc(file), separators, 3, NULL)
                || ((c == ARG_SEP) && in_str)) /* or it's a separator but inside a string */ {
            c = agetc(file); /* not an active separator, get that char and move on */
            if (!in_str || !escape_next) /* the current char is not escaped */
                if (c == STRING_CHAR) in_str = !in_str;
            escape_next = in_str && !escape_next && (c == ESCAPE_CHAR); /* is unescaped backslash inside a string */
            if (!escape_next) put2buffer(file->buffer, &c);
        }

        /* we've got an arg */
        str = trim(buffer2string(file->buffer));
        if (*str != '\0') /* not empty or white spaces */
            add_param(state, str);
        else report_error(file, "illegal comma"); /* can also ignore and just send a warning */

        agetc(file); /* now skip the separator. if we had done it before, the line counter may was wrong */

        clear_buffer(file->buffer);
    }

    return true;
}

statement *read_statement(asmfile *file)
{
    int c;
    statement *state;

    if (file == NULL) return NULL;

    skip_spaces(file); /* skip spaces and empty lines */
    while ((c = apeekc(file)) == COMMENT_SIGN) { /* while it's a comment line */
        if (!movelines(file, 1)) /* jump to the next line */
            return NULL; /* that was the last line */
        skip_spaces(file); /* skip spaces and empty lines */
    }

    if (c == EOF) return NULL; /* there is no more lines */

    state = new_statement();
    if (state == NULL) return NULL; /* can't allocate a new statement */
    state->line = file->curr_line; /* we have found a statement line (probably) */

    if (!fill_label_and_name(state, file)) {
        /* failed to read label or name */
        free_statement(state);
        return NULL;
    }

    if (state->line != file->curr_line || iseof(file)) return state; /* probably there is no arguments */

    if (!fill_arguments(state, file)) {
        /* failed to read arguments */
        free_statement(state);
        return NULL;
    }

    return state;
}

bool print_line(asmfile *file, long x)
{
    long line_bak, col_bak;
    int c;
    fpos_t pos_bak;
    int separators[] = { '\n', EOF };

    if (file == NULL) return false;
    if (x <= 0) return false; /* invalid line number */

    if (x == 2)
        printf("\n");

    /* backup */
    fgetpos(file->src_file, &pos_bak);
    line_bak = file->curr_line;
    col_bak = file->curr_col;

    if (!gotoline(file, x)) return false; /* line not exists */
    /* use fgetc instead of agetc cuz we'll restore pos */
    while (!is_int_of(c = fgetc(file->src_file), separators, 2, NULL))
        printf("%c", c);
    printf("\n");

    /* restore */
    rewind(file->src_file);
    fsetpos(file->src_file, &pos_bak);
    file->curr_line = line_bak;
    file->curr_col = col_bak;

    return true;
}

bool gotoline(asmfile *file, long x)
{ /* fseek will not replace that func since have to jump lines */
    long i;
    int c;
    fpos_t pos_bak;

    if (file == NULL) return false;
    if (x <= 0) return false; /* invalid line number */

    /* TODO: need a fix here in case of the last line ends with EOF, we can hide it and that will still do the same.
    if (x == file->curr_line) { // the current line, easy case. just move to start of line
        fseek(file->src_file, -(file->curr_col - 1), SEEK_CUR);
        file->curr_col = 1;
        return true;
    }*/

    /* backup current position just in case */
    fgetpos(file->src_file, &pos_bak);

    i = file->curr_line;
    if (x <= file->curr_line) { /* it's behind, go to the start */
        rewind(file->src_file);
        fseek(file->src_file, 0, SEEK_SET); /* but don't update file positions yet */
        i = 1; /* count from start */
    }

    /* skip lines */
    for (; i < x; i++)
        /* use fgetc instead of agetc cuz we may will revert that move */
        while ((c = fgetc(file->src_file)) != '\n')
            if (c == EOF) { /* line x is not exists */
                rewind(file->src_file);
                fsetpos(file->src_file, &pos_bak); /* restore position */
                return false; /* reverted */
            }

    /* update position counters */
    file->curr_line = x;
    file->curr_col = 1;

    return true;
}

bool movelines(asmfile *file, long x)
{
    if (file == NULL) return false;
    return gotoline(file, file->curr_line + x);
}

bool iseof(asmfile *file)
{
    if (file == NULL) return true;
    if (file->src_file == NULL) return true;
    /*return feof(file->src_file);*/
    return apeekc(file) == EOF;
}

int agetc(asmfile *file)
{
    int c;
    if (file == NULL) return EOF;
    c = fgetc(file->src_file);

    if (c == '\n') { /* a new line */
        file->curr_line++;
        file->curr_col = 1;
    } else file->curr_col++;

    if (file->curr_col == MAX_LINE_LEN + 1)
        /* warning: the current line passed the max defined length */
        report_warning(file, "line is longer than %d.", MAX_LINE_LEN);

    return c;
}

int apeekc(asmfile *file)
{
    int c;
    if (file == NULL) return EOF;
    c = fgetc(file->src_file); /* don't use agetc here cuz we gonna go back */
    if (c != EOF) fseek(file->src_file, -1L, SEEK_CUR);
    return c;
}

long skip_spaces(asmfile *file)
{
    int c;
    long counter = 0;
    if (file == NULL) return EOF;
    while (isspace(c = agetc(file))) counter++;
    if (c != EOF) {
        fseek(file->src_file, -1L, SEEK_CUR);
        file->curr_col--; /* if we reached here curr_col must be >1 cuz if the last char was '\n' we would keep moving */
    }
    return counter;
}
