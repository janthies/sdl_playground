#include "types.h"
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>




typedef struct vector_metadata
{
    u32 size;
    u32 capacity;
} vector_metadata_t;

#define vector(type) type*


// push_back

#define vector_base_to_vec(base) ((void*)&(((vector_metadata_t*)(base))[1]))

#define vector_vec_to_base(vec) (&(((vector_metadata_t*)(vec))[-1]))

#define vector_set_size(vec, _s) vector_vec_to_base(vec)->size = _s

#define vector_set_capacity(vec, _c) vector_vec_to_base(vec)->capacity = _c
    
#define vector_grow(vec_ptr, count) \
    do  \
    {   \
        size_t const size = (count) * sizeof(*(vec_ptr)) + sizeof(vector_metadata_t);   \
        void** vec_addr = (void**)&(vec_ptr); \
        if(*(vec_addr)) \
        {   \
            void* base = vector_vec_to_base(*(vec_addr));   \
            void* ptr = realloc(base, size);    \
            assert(ptr);    \
            *(vec_addr) = vector_base_to_vec(ptr);    \
        }   \
        else     \
        {   \
            void* ptr = malloc(size);   \
            assert(ptr);    \
            *(vec_addr) = vector_base_to_vec(ptr);    \
            vector_set_size(*(vec_addr), 0);    \
        }   \
		printf("setting capacity to %u\n", (count));  \
        vector_set_capacity(*(vec_addr), (count));    \
    } while (0)




#define vector_get_size(vec) vector_vec_to_base(vec)->size
#define vector_get_capacity(vec) vector_vec_to_base(vec)->capacity

#define vector_new_capacity(current_capacity) (2 * (current_capacity))

#define vector_push_back(vec, val)	\
	do	\
	{	\
		if(!vec)	\
		{	\
			vector_grow(vec, 5);	\
		}	\
		u32 const size = vector_get_size(vec);	\
		u32 const capa = vector_get_capacity(vec);	\
		if( size >= capa )	\
		{	\
			printf("growing to %u\n", vector_new_capacity(capa));	\
			vector_grow(vec, vector_new_capacity(capa));	\
		}	\
		(vec)[size] = val;	\
		vector_set_size(vec, size + 1); \
	} while (0)	\
	


#define vector_reserve(vec, count) \
	do	\
	{	\
		u32 const size = vector_get_size(vec);	\
		u32 const capacity = vector_get_capacity(vec);	\
		if(count <= size || count <= capacity) break;	\
		vector_grow(vec, count);	\
	} while (0)	\

#define vector_shrink(vec)	vector_grow(vec, vector_get_size(vec))

	

	

int main()
{
	printf("Hallo Welt\n");
    vector(u32) myvec = NULL;
	
	for(int i = 0; i < 100; i++)
	{
		vector_push_back(myvec, 3);
		printf("%i\n", i);
	}
	
	printf("Values: %u, %u\n", vector_get_size(myvec), vector_get_capacity(myvec));
	vector_shrink(myvec);

	printf("Values: %u, %u\n", vector_get_size(myvec), vector_get_capacity(myvec));
		for(int i = 0; i < 1000; i++)
	{
		vector_push_back(myvec, 3);
		printf("%i\n", i);
	}

	printf("Values: %u, %u\n", vector_get_size(myvec), vector_get_capacity(myvec));

	printf("Tschuess\n");

}