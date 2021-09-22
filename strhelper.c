#include "strhelper.h"

#define MAX_INT32_STR "2147483647"
#define MIN_INT32_STR "-2147483648"
#define MAX_INT32_LEN 10

bool isinteger(const char *str)
{
    const char *p = str;
    if (!isdigit(*p) && *p != '-' && *p != '+') return false;
    for (p++; *p != '\0'; p++)
        if (!isdigit(*p)) return false;
    return true;
}

range_res check_int32_str(const char *str)
{ /* assume the input string is trimmed */
    bool is_minus;
    string limit_val_str;
    const char *p = str;
    size_t len;

    is_minus = *p == '-';
    limit_val_str = is_minus ? MIN_INT32_STR : MAX_INT32_STR;
    if (is_minus) limit_val_str++; /* skip the '-' */

    /* first make sure it's an integer */
    if (!isinteger(str)) return invalid_num;
    if (!isdigit(*p)) p++; /* the first char is - or + cuz we know it's a valid integer, skip it */

    /* check by length */
    len = strlen(p);
    if (len > MAX_INT32_LEN) return is_minus ? smaller_num : bigger_num;
    if (len < MAX_INT32_LEN) return num_in_range;

    /* same length as the max/min length, check each digit */
    for (; *p != '\0'; p++, limit_val_str++) {
        /* we know these are digits only, so we can check it according to the ascii table */
        if (*p < *limit_val_str) return num_in_range;
        else if (*p > *limit_val_str) return is_minus ? smaller_num : bigger_num;
    }

    return num_in_range; /* the exact max or min value */
}

char *ltrim_pointer(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *ltrim(char *s)
{
    char *lp, *dist;
    lp = ltrim_pointer(s);
    dist = (char *)(lp - s);
    for (; *(lp) != '\0'; lp++) *(char *)(lp - dist) = *lp;
    *(char *)(lp - dist) = '\0';
    return s;
}

char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s)
{
    return ltrim(rtrim(s));
}

int strhlfcmp(const char *str1, const char *str2)
{
    for(; tolower(*str1) == tolower(*str2); ++str1, ++str2)
        if(*str1 == 0) return 0;
    return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}

bool isinlinespace(int c)
{
    return c != '\n' && isspace(c);
}

bool is_char_of(char c, const char *str, bool (*compare)(char c))
{
    const char *p;
    if (str != NULL)
        for (p = str; *p != '\0'; p++)
            if (*p == c) return true;
    return compare != NULL ? compare(c) : false;
}

bool is_int_of(int val, const int *separators, int count, bool (*compare)(int val))
{
    int i;
    if (separators != NULL)
        for (i = 0; i < count; i++)
            if (separators[i] == val) return true;
    return compare != NULL ? compare(val) : false;
}

string get_extension(string str)
{
    string end, p = str;

    while (*p != '\0') p++; /* go to end */
    end = p;

    for (p--; /* start from the last char before the \0 */
         p >= str && *p != '\\' && *p != '/'; /* while in range and not a slash */
         p--) /* move back */
        if (*p == '.') return p;

    return end; /* the real end of that string! */
}

string change_extension(string str, string newExt) {
    size_t i, len; /* the exact required size to allocate */
    string res;
    string ext = get_extension(str);
    len = strlen(newExt);
    len += ext - str;

    res = malloc_or_exit((len + 1) * sizeof(char)); /* +1 for the '\0' */
    if (res == NULL) return NULL;

    /* don't use strcpy or strcat here since str maybe longer than res */
    for (i = 0; str + i < ext; i++) res[i] = str[i];
    res[i] = '\0';

    strcat(res, newExt); /* append the new extension */

    return res; /* we have got a new allocated string in the exact required size and with the new extension */
}

string allocate_string(const char *str) {
    string new;
    if (str == NULL) return NULL;
    new = malloc((strlen(str) + 1) * sizeof(char));
    if (new == NULL) { /* can't allocate memo */
        printf("ERROR: failed to allocate %lu long string.\n", (ulong)strlen(str));
        fprintf(stderr, "ERROR: failed to allocate %lu long string.\n", (ulong)strlen(str));
        exit(1); /* actually we have nothing to do */
        /*return NULL;*/
    }
    strcpy(new, str);
    return new;
}

/*string allocate_string(const char *str, ...)
{
    string new;
    va_list ap;
    va_start(ap, str);
    new = v_allocate_string(str, ap);
    va_end(ap);
    return new;
}

string v_allocate_string(const char *str, va_list ap)
{
    string new;

    // format the source string
    vsprintf(); // a problem here, how can we know the required memory size for the new string?

    if (str == NULL) return NULL;
    new = malloc((strlen(str) + 1) * sizeof(char));
    if (new == NULL) return NULL; // can't allocate memo

    strcpy(new, str);
    return new;
}*/
