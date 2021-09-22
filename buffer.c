#include "buffer.h"

dyBuffer *new_buffer(uint unitsize)
{
    dyBuffer* dbuff = malloc(sizeof(dyBuffer));

    if (dbuff == NULL) {
        printf("ERROR: failed to allocate a new buffer.\n");
        return NULL;
    }

    dbuff->data = NULL;
    dbuff->size = 0;
    dbuff->length = 0;
    dbuff->unitsize = unitsize;
    dbuff->exit_on_error = false;

    return dbuff;
}

void clear_buffer(dyBuffer* dbuff)
{
    dbuff->length = 0;
    /* don't free up memory, keep it reusable */
}

void free_buffer(dyBuffer* dbuff)
{
    if (dbuff == NULL) return;
    if (dbuff->data != NULL) free(dbuff->data);
    free(dbuff);
}

string buffer2string(dyBuffer* dbuff)
{
    int n;
    string res = malloc(units_total_size(dbuff) + 1); /* +1 for \0 */

    if (res == NULL) return NULL;

    for (n = 0; n < units_total_size(dbuff); n++)
        res[n] = ((byte*)dbuff->data)[n]; /* as array of byte */
    res[n] = '\0'; /* end with \0 anyway */

    return res;
}

bool add2buffer(dyBuffer* dbuff, const void *data, ulong count)
{
    ulong i, n;

    if (buffer_space_left(dbuff) < count) /* missing space, extend the buffer */
        if (!extend_buffer(dbuff, count))
            return false; /* failed to extend buffer */

    for (n = 0; n < count; n++) /* foreach new item */
        for (i = 0; i < dbuff->unitsize; i++) /* foreach byte of a new item */
            /* add that byte to the buffer's data */
            ((byte*)dbuff->data)[units_total_size(dbuff) + n * dbuff->unitsize + i] = ((byte*)data)[n * dbuff->unitsize + i];

    dbuff->length += count;

    return true;
}

bool put2buffer(dyBuffer* dbuff, const void *c)
{
    return add2buffer(dbuff, c, 1);
}

void *last_in_buff(dyBuffer* dbuff)
{
    if (dbuff == NULL || dbuff->length == 0) return NULL; /* there is no last */
    return ((byte *)dbuff->data) + dbuff->length - 1;
}

ulong buffer_space_left(dyBuffer* dbuff)
{
    ulong free_memo_bytes_left = dbuff->size - units_total_size(dbuff);
    return free_memo_bytes_left / dbuff->unitsize;
}

ulong units_total_size(dyBuffer* dbuff)
{
    return dbuff->length * dbuff->unitsize;
}

ulong units_capacity(dyBuffer* dbuff)
{
    return dbuff->size / dbuff->unitsize;
}

bool resize_buffer(dyBuffer* dbuff, ulong free_necessary)
{
    ulong newSize;
    string tmpData;

    free_necessary -= buffer_space_left(dbuff);

    newSize = (units_capacity(dbuff) + free_necessary) * dbuff->unitsize;
    tmpData = realloc(dbuff->data, newSize);
    if (tmpData == NULL)
    {
        printf("ERROR: failed to reallocate buffer.\n");
        fprintf(stderr, "ERROR: failed to reallocate buffer.\n");
        if (dbuff->exit_on_error) exit(1);
        return false;
    }

    dbuff->data = tmpData;
    dbuff->size = newSize;

    return true;
}

bool extend_buffer(dyBuffer* dbuff, ulong free_necessary)
{
    /* choose the bigger option */
    if (free_necessary * dbuff->unitsize < BUFFER_SIZE_INTERVAL)
        free_necessary = BUFFER_SIZE_INTERVAL / dbuff->unitsize;

    return resize_buffer(dbuff, free_necessary);
}

bool cut_buffer(dyBuffer* dbuff)
{
    return resize_buffer(dbuff, 0);
}
