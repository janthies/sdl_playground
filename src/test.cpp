#include "types.h"
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

typedef struct vector
{
    u32 elem_size;
    u32 count;
    u32 capacity;
    void* data;
} vector_t;

void vector_initialize(vector_t* vec, u32 elem_size)
{
    vec->elem_size = elem_size;
    vec->count = 0;
    vec->capacity = 5;

    void* ptr = malloc(vec->elem_size * vec->capacity);
    assert(ptr);
    vec->data = ptr;
}

void vector_deinitialize(vector_t* vec)
{
    vec->elem_size = 0;
    vec->count = 0;
    vec->capacity = 0;
    free(vec->data);
    vec->data = NULL;
}

void _vector_resize(vector_t* vec)
{
    float const RESIZE_FACTOR = 1.5f;

    u32 const new_capacity = (u32)(RESIZE_FACTOR * vec->capacity);
    u32 const new_size = new_capacity * vec->elem_size;
    
    void* ptr = realloc(vec->data, new_size);
    assert(ptr);

    vec->capacity = new_capacity;
    vec->data = ptr;
}

 
// Returns address to insert new elem into
void* vector_add(vector_t* vec)
{
    if(vec->count == vec->capacity)
    {
        _vector_resize(vec);
    }

    
    u32 const pos = vec->count;
    vec->count++;
    return (((char*)vec->data) + (vec->elem_size * pos));
}

void vector_remove(vector_t* vec, u32 pos)
{
    assert(pos < vec->count);

    char* dst = ((char*)vec->data) + vec->elem_size * pos;
    char* src = ((char*)dst) + vec->elem_size;
    u32 size = (vec->count - pos - 1) * vec->elem_size;

    vec->count--;
    memmove(dst, src, size);
}

void* vector_get(vector_t* vec, u32 pos)
{
    assert(pos < vec->count);
    return ((char*)vec->data) + vec->elem_size * pos;
}

#define VECTOR_INIT(vec, type) vector_initialize(&vec, sizeof(type))

#define VECTOR_DEINIT(vec) vector_deinitialize(&vec)

#define VECTOR_ADD(vec, type, val) \
    do  \
    {   \
        *(type*)vector_add(&vec) = val; \
    } while(0) \

#define VECTOR_REMOVE(vec, pos) vector_remove(&vec, pos)

#define VECTOR_GET(vec, type, pos) *(type*)vector_get(&vec, pos)

    
    

int main()
{
    printf("Hello World\n");
    vector_t vec;

    VECTOR_INIT(vec, u32);
    for(int i = 0; i < 32; i++)
    {
        VECTOR_ADD(vec, u32, (u32)i);
    }

    VECTOR_GET(vec, u32, 3) = -1;

    u32 val = VECTOR_GET(vec, u32, 3);
    printf("Gotten %u\n", val);



    printf("{");
    for(int i = 0; i < vec.count; i++)
    {
        u32 val = *(u32*)vector_get(&vec, i);
        printf("%u, ", val);
    }
    printf("}\n");
    sleep(4);
    printf("Ende\n");
    
    return 0;
}