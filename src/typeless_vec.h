#ifndef TYPELESS_VEC_H
#define TYPELESS_VEC_H
#include "types.h"

typedef struct typeless_vector_header
{
    u32 size;
    u32 capacity;
    u32 element_size;
} typeless_vector_header_t;




#endif