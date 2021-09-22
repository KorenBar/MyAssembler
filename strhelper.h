#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "mytypes.h"
#include "memohelper.h"

#ifndef STRHELPER_H
#define STRHELPER_H

typedef enum { num_in_range, bigger_num, smaller_num, invalid_num } range_res;

/* left trim - returns a pointer to the first non-space char
 * note that returned pointer is pointing to an existing memory so don't free it */
char *ltrim_pointer(char *s);
/* left trim - moving all characters form the first non-space to the begin
 * returns the same pointer param as is */
char *ltrim(char *s);
/* right trim - ending the string with \0 before the white spaces at end
 * returns the same pointer param as is */
char *rtrim(char *s);
/* trim a string from both sides - returns a pointer to the first non-space char
 * note that returned pointer is pointing to an existing memory so don't free it */
char *trim(char *s);
/* compare between two strings without reference to lower/upper case */
int strhlfcmp(const char *str1, const char *str2);

/* is a space but not a new line */
bool isinlinespace(int c);
bool is_char_of(char c, const char *str, bool (*compare)(char c));
bool is_int_of(int val, const int *separators, int count, bool (*compare)(int val));

bool isinteger(const char *str);
range_res check_int32_str(const char *str);
void uint32_to_hex(myuint32_t value, char *buffer);

string get_extension(string str);
string change_extension(string str, string new_ext);

string allocate_string(const char *str);

#endif
