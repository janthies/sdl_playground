#include "types.h"
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef  DEFAULT_CAPACITY
#define DEFAULT_CAPACITY 10
#endif

typedef struct vector_metadata
{
    u32 size;
    u32 capacity;
} vector_metadata_t;

// Private Utilities
#define _vector_base_to_vec(base) ((void*)&(((vector_metadata_t*)(base))[1]))
#define _vector_vec_to_base(vec) (&(((vector_metadata_t*)(vec))[-1]))
#define _vector_set_size(vec, _s) _vector_vec_to_base(vec)->size = _s
#define _vector_set_capacity(vec, _c) _vector_vec_to_base(vec)->capacity = _c

#define _vector_grow(vec_ptr, count) \
    do  \
    {   \
        size_t const size = (count) * sizeof(*(vec_ptr)) + sizeof(vector_metadata_t);   \
        void** vec_addr = (void**)&(vec_ptr); \
		assert(*(vec_addr));	\
		void* base = _vector_vec_to_base(*(vec_addr));   \
		void* ptr = realloc(base, size);    \
		assert(ptr);    \
		*(vec_addr) = _vector_base_to_vec(ptr);    \
		printf("setting capacity to %u\n", (count));  \
        _vector_set_capacity(*(vec_addr), (count));    \
    } while (0)


// Public Interface
#define vector_get_size(vec) _vector_vec_to_base(vec)->size

#define vector_get_capacity(vec) _vector_vec_to_base(vec)->capacity

#define vector_new_capacity(current_capacity) (2 * (current_capacity))

#define vector_push_back(vec, val)	\
	do	\
	{	\
		if(!vec)	\
		{	\
			_vector_grow(vec, 5);	\
		}	\
		u32 const size = vector_get_size(vec);	\
		u32 const capa = vector_get_capacity(vec);	\
		if( size >= capa )	\
		{	\
			printf("growing to %u\n", vector_new_capacity(capa));	\
			_vector_grow(vec, vector_new_capacity(capa));	\
		}	\
		(vec)[size] = val;	\
		_vector_set_size(vec, size + 1); \
	} while (0)	

#define vector_reserve(vec, count) \
	do	\
	{	\
		u32 const size = vector_get_size(vec);	\
		u32 const capacity = vector_get_capacity(vec);	\
		if(count <= size || count <= capacity) break;	\
		_vector_grow(vec, count);	\
	} while (0)	

#define vector_shrink(vec)	_vector_grow(vec, vector_get_size(vec))

#define vector_get(vec, pos) \
    ({ \
        assert(pos < vector_get_size(vec)); \
        &vec[pos];  \
    })

// pos+1 can be out of bounds when deleting the last element
// however, in this case count will be zero, thus, memmove won't be called
#define vector_remove(vec, pos)	\
	do	\
	{	\
		assert(pos < vector_get_size(vec));	\
		void* dst = (void*)(&(vec)[pos]);	\
		void* src = (void*)(&(vec)[pos + 1]);	\
		u32 const count = vector_get_size(vec) - pos - 1;	\
		memmove(dst, src, count * sizeof(*vec));	\
	} while (0)	

// pos+1 can be out of bounds when deleting the last element
// however, in this case count will be zero, thus, memmove won't be called
#define vector_insert(vec, pos, elem)	\
	do	\
	{	\
		assert(pos < vector_get_size(vec));	\
		if(vector_get_size(vec) >= vector_get_capacity(vec))	\
		{	\
			_vector_grow(vec, vector_new_capacity(vector_get_capacity(vec)));	\
		}	\
		void* dst = (void*)(&(vec)[pos + 1]);	\
		void* src = (void*)(&(vec)[pos]);	\
		u32 const count = vector_get_size(vec) - pos;	\
		memmove(dst, src, count * sizeof(*vec));	\
		(vec)[pos] = elem;	\
	} while (0)	
	
	
#define vector(type) type*

#define vector_initialize(vec)	\
	do	\
	{	\
		assert(!vec);	\
		size_t const size = (DEFAULT_CAPACITY) * sizeof(*(vec)) + sizeof(vector_metadata_t);	\
		void** vec_addr = (void**)&(vec); \
		void* ptr = malloc(size);   \
		assert(ptr);    \
		*(vec_addr) = _vector_base_to_vec(ptr);    \
		_vector_set_size(*(vec_addr), 0);    \
		_vector_set_capacity(*(vec_addr), DEFAULT_CAPACITY); \
	} while (0)
	

#define vector_for_each(vec, func)	\
	do	\
	{	\
		u32 const size = vector_get_size(vec);	\
		for(int i = 0; i < size; i++)	\
		{	\
			func(&(vec)[i]);	\
		}	\
	} while (0)	


