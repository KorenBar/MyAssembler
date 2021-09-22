#include "directive.h"

dir_index get_directive_index(const char *name) {
    dir_index i = 0;
    for (i = 0; i != none_dir; i++)
        if (!strcmp(directives[i].name, name))
            return i; /* found */
    return i; /* none_dir */
}

dir_index get_similar_directive_index(const char *name) {
    dir_index i = 0;
    for (i = 0; i != none_dir; i++)
        if (!strhlfcmp(directives[i].name, name))
            return i; /* found */
    return i; /* none_dir */
}

directive get_directive(const char *name) {
    return directives[get_directive_index(name)];
}

bool is_dir(directive dir, dir_index dir_i) {
    return memcmp(&dir, &directives[dir_i], sizeof(directive)) == 0;
}

directive get_similar_directive(const char *name) {
    return directives[get_similar_directive_index(name)];
}

bool is_directive_name(const char *name) {
    return name != NULL ? *name == '.' : false;
}

bool is_valid_directive_name(string name) { /* a directive name starts with a dot and contains lowercase letters only. */
    string p = name;
    if (!is_directive_name(name)) return false;
    for (p = name + 1; *p != '\0'; p++)
        if (!islower(*p)) return false;
    return true;
}

bool is_dir_equals(directive a, directive b) {
    return memcmp(&a, &b, sizeof(directive));
}

bool is_dir_exists(directive dir) {
    return !is_dir_equals(dir, directives[none_dir]);
}
