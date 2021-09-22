#include "memohelper.h"
#include "mytypes.h"

void alloc_error(size_t sizeReq) {
    printf("ERROR: Failed to allocate memory! required memory size: %lu. exiting..\n", (ulong)sizeReq);
    fprintf(stderr, "ERROR: Failed to allocate memory! required memory size: %lu. exiting..\n", (ulong)sizeReq);
    exit(1);
}

void *malloc_or_exit(size_t size)
{
    void *res = malloc(size);
    if (res == NULL) alloc_error(size);
    return res;
}
void *realloc_or_exit(void *memory, size_t newSize)
{
    void *res = realloc(memory, newSize);
    if (res == NULL) alloc_error(newSize);
    return res;
}
void *calloc_or_exit(size_t numOfElements, size_t sizeOfElements)
{
    void *res = calloc(numOfElements, sizeOfElements);
    if (res == NULL) alloc_error(numOfElements * sizeOfElements);
    return res;
}