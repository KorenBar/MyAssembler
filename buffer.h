#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "mytypes.h"

#ifndef BUFFER_H
#define BUFFER_H

/* it's not recommended to reallocate memory too many times,
 * choose a reasonable size to grow each time needed.
 * this buffer will not free unused memory by itself
 * so it will be effective to reuse the same buffer
 * instead of creating a new one. */
#define BUFFER_SIZE_INTERVAL 64

typedef struct {
    void *data;
    ulong size; /* capacity */
    ulong length; /* num of units */
    uint unitsize; /* must be positive */
    bool exit_on_error;
} dyBuffer;

/* init a new dynamic buffer */
dyBuffer *new_buffer(uint unitsize);
void clear_buffer(dyBuffer* dbuff);
void free_buffer(dyBuffer* dbuff);
/* add data to the buffer and extend it as needed */
bool add2buffer(dyBuffer* dbuff, const void *items, ulong count);
/* put a single char to a buffer */
bool put2buffer(dyBuffer* dbuff, const void *item);
/* get a pointer to the last unit or NULL if there is no unit */
void *last_in_buff(dyBuffer* dbuff);
/* returns the count of units that can be added until reallocation is required in order to grow */
ulong buffer_space_left(dyBuffer* dbuff);
/* the size of the existing units in bytes */
ulong units_total_size(dyBuffer* dbuff);
/* how many units can a given buffer to hold before extending */
ulong units_capacity(dyBuffer* dbuff);
bool resize_buffer(dyBuffer* dbuff, ulong free_necessary);
bool extend_buffer(dyBuffer* dbuff, ulong free_necessary);
bool cut_buffer(dyBuffer* dbuff);
/* returns a copy of the buffer data as string */
string buffer2string(dyBuffer* dbuff);

#endif
