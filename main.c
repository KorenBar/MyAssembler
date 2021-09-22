#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "asmfile.h"
#include "asmencoder.h"

/*read about colors here https://www.theurbanpenguin.com/4184-2/*/
/* TODO: write a class for color changes with color enum (0,30,31,..) and print functions */
#define BLACK_COLOR     "\033[0;30m"
#define RED_COLOR       "\033[0;31m"
#define GREEN_COLOR     "\033[0;32m"
#define YELLOW_COLOR    "\033[0;33m"
#define BLUE_COLOR      "\033[0;34m"
#define PURPLE_COLOR    "\033[0;35m"
#define CYAN_COLOR      "\033[0;36m"
#define WHITE_COLOR     "\033[0;37m"
#define DEFAULT_COLOR   "\033[0;0m"

#define WARNING_LABEL   "\033[0;33m[WARNING]\033[0;0m"
#define ERROR_LABEL     "\033[0;31m[ERROR]  \033[0;0m"
#define DONE_LABEL      "\033[0;32mDONE!\033[0;0m"

void error_callback_func(void *sender, asmfile *file, long line, long col, bool can_continue, const char *msg, va_list al)
{
    long i;

    /*printf("    %s line: %ld:%ld => ", can_continue ? "[WARNING]" : "[ERROR]  ", line, col);*/
    printf("    %s %s% 4ld:%-4ld%s =>\t",
           can_continue ? WARNING_LABEL : ERROR_LABEL, BLUE_COLOR, line, col, DEFAULT_COLOR);
    vprintf(msg, al);
    printf("\n");

    /* just for fun */
    printf(RED_COLOR);
    printf("%26c", '\t'); /* line padding */
    if (print_line(file, line) && col) {
        printf("%26c", '\t'); /* cursor padding */
        for (i = 0; i < col - 1; i++) printf(" ");
        printf("^"); /* the cursor */
    }
    printf(DEFAULT_COLOR);
    printf("\n");

}

/* as you can see I've used the boolean type as return type a lot to know if succeeded or failed
 * that because in C we don't actually have the exceptions way */

bool encode_filename(string filename)
{
    asmfile *file;
    asmencoder *encoder;

    printf("%s:\n", filename);

    if (strcmp(get_extension(filename), ASM_EXTENSION) != 0) {
        printf("    %s invalid source file!\n", ERROR_LABEL);
        return false;
    }

    file = new_asmfile(filename);
    if (file == NULL) {
        printf("    %s failed to open assembly source file!\n", ERROR_LABEL);
        return false;
    }

    encoder = new_asmencoder(file);
    if (encoder == NULL) {
        /* probably an error was printed */
        free_asmfile(file);
        return false;
    }

    encoder->error_callback = error_callback_func;
    if (asm_encode(encoder)) { /* encoded successfully */
        if (asm_export_hex_file(encoder)
            & asm_export_ent_file(encoder)
            & asm_export_ext_file(encoder))
            printf("    %s\n", DONE_LABEL);
        else printf("    %s failed to export!\n", ERROR_LABEL);
    }

    free_asmencoder(encoder);
    free_asmfile(file);

    return true;
}

int main(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++) {
        encode_filename(argv[i]);
        printf("\n");
    }

    return 0;
}
