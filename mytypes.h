
#ifndef MYTYPES_H
#define MYTYPES_H

#define UNLIMITED -1

typedef enum { false, true } bool;

typedef signed char byte;
typedef unsigned char ubyte;
typedef char *string;
/* minimum long size is 32bit, I will use it cuz I can't use the stdint.h */
typedef long int myint32_t;
typedef unsigned long int myuint32_t;
/* minimum int size is 16bit, I will use it cuz I can't use the stdint.h */
typedef int myint16_t;
typedef unsigned int myuint16_t;

typedef unsigned short ushort;
typedef unsigned long int ulong;
typedef unsigned int uint;

#endif
