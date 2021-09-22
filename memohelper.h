#include <stdio.h>
#include <stdlib.h>

#ifndef MYASSEMBLER_MEMOHELPER_H
#define MYASSEMBLER_MEMOHELPER_H

void *malloc_or_exit(size_t size);
void *realloc_or_exit(void *memory, size_t newSize);
void *calloc_or_exit(size_t numOfElements, size_t sizeOfElements);

#endif
