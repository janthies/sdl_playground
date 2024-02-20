#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


#include "vector.h"


#define MAX_ENTITIES 1024 * 4
#define COMPONENT_ARRAY_DEFAULT_CAPACITY 16
#define INVALID_ENTITY UINT32_MAX
#define entity u32

typedef struct component_array
{
	u32* entity_to_index;
	u32* index_to_entity;
	u32 count;
	u32 capacity;
	u32 element_size;
	void* components;
} component_array_t;

void component_array_initialize(component_array_t* ca, u32 element_size)
{
	// Initialize primitive members
	ca->count = 0;
	ca->capacity = COMPONENT_ARRAY_DEFAULT_CAPACITY;
	ca->element_size = element_size;

	// Initialize sparse array
	const u32 ARRAY_SIZE = MAX_ENTITIES * sizeof(u32);
	void* ptr = malloc(ARRAY_SIZE);
	assert(ptr);
	memset(ptr, INVALID_ENTITY, ARRAY_SIZE);
	ca->entity_to_index = (u32*) ptr;

	// Initialize packed array
	ptr = malloc(ARRAY_SIZE);
	assert(ptr);
	ca->index_to_entity = (u32*) ptr;

	// Initialize component data array
	const u32 COMPONENT_DATA_SIZE = COMPONENT_ARRAY_DEFAULT_CAPACITY * element_size;
	ptr = malloc(COMPONENT_DATA_SIZE);
	assert(ptr);
	ca->components = ptr;
}

void component_array_deinitialize(component_array_t* ca)
{
	ca->capacity = 0;
	ca->count = 0;
	ca->element_size = 0;
	free(ca->entity_to_index);
	free(ca->index_to_entity);
	free(ca->components);
}

void _component_array_grow(component_array_t* ca, u32 count)
{
	assert(count >= ca->count); // make sure no components are being lost
	
	u32 const NEW_SIZE = count * ca->element_size;
	void* ptr = realloc(ca->components, NEW_SIZE);
	assert(ptr);

	ca->components = ptr;
	ca->capacity = count;
}

void* component_array_add(component_array_t* ca, entity e)
{
	assert(ca);	
	assert(e < MAX_ENTITIES);
	assert(ca->entity_to_index[e] == INVALID_ENTITY);	// entity isn't yet registered

	if(ca->count >= ca->capacity)
	{
		_component_array_grow(ca, 2 * ca->count);
	}

	u32 const NEW_ENTITY_INDEX = ca->count;

	// TODO Subscript muss händisch mit elem_size berechnet werden!!!!!

	void* dst = (void*)((char*)(ca->components) + (NEW_ENTITY_INDEX * ca->element_size));
	memset(dst, 0, ca->element_size); // set entity component data to zero -> unnecessary
	ca->entity_to_index[e] = NEW_ENTITY_INDEX;
	ca->index_to_entity[NEW_ENTITY_INDEX] = e;
	
	ca->count++;
	return dst;
}


void component_array_remove(component_array_t* ca, entity e)
{
	assert(ca);	
	assert(e < MAX_ENTITIES);
	assert(ca->entity_to_index[e] != INVALID_ENTITY);	// entity is registered
	assert(ca->count > 0); // there is at least 1 entity -> duplicate because in that case e should be invalid?

	// move last entity in index to empty spot
	u32 const ENTITY_REMOVE_INDEX = ca->entity_to_index[e];
	u32 const LAST_ENTITY_INDEX = ca->count - 1;
	u32 const LAST_ENTITY_ID = ca->index_to_entity[LAST_ENTITY_INDEX];

	// Move component data
	void* src = (void*)((char*)(ca->components) + (LAST_ENTITY_INDEX * ca->element_size));
	void* dst = (void*)((char*)(ca->components) + (ENTITY_REMOVE_INDEX * ca->element_size));
	memcpy(dst, src, ca->element_size);

	// adjust packed array
	ca->index_to_entity[ENTITY_REMOVE_INDEX] = LAST_ENTITY_ID;
	ca->index_to_entity[LAST_ENTITY_INDEX] = INVALID_ENTITY;

	// adjust sparse arrayLAST_ENTITY_INDEX
	ca->entity_to_index[LAST_ENTITY_ID] = ENTITY_REMOVE_INDEX;
	ca->entity_to_index[e] = INVALID_ENTITY;

	ca->count--;
}

void* component_array_get(component_array_t* ca, entity e)
{
	assert(ca);	
	assert(e < MAX_ENTITIES);
	assert(ca->entity_to_index[e] != INVALID_ENTITY); // entity is registered
	
	u32 const INDEX = ca->entity_to_index[e];
	return (void*)(((char*)ca->components) + (INDEX * ca->element_size));
}






typedef struct component_1
{
	u32 val;
} component_1_t;

typedef struct component_2
{
	float val;
} component_2_t;

typedef struct component_3
{
	
} component_3_t;

#define COMPONENT_A_FLAG (1 << 0)
#define COMPONENT_B_FLAG (1 << 1)
#define COMPONENT_C_FLAG (1 << 2)

#define FLAG_TO_COMPONENT_SIZE(flag) \
	(flag == COMPONENT_A_FLAG ? sizeof(component_1_t) : \
	(flag == COMPONENT_B_FLAG ? sizeof(component_2_t) : \
	(flag == COMPONENT_C_FLAG ? sizeof(component_3_t) : \
	NULL)))





typedef struct component_array_wrapper
{
	u32 flag;
	component_array_t ca;
} component_array_wrapper_t;

typedef struct archetype
{
	u32 component_mask;
	vector(component_array_wrapper_t) component_arrays;
} archetype_t;


void archetype_initialize(archetype_t* archetype, u32 mask)
{
	archetype->component_mask = mask;
	archetype->component_arrays = NULL;
	vector_initialize(archetype->component_arrays);
	while(mask)
	{
		printf("Hello\n");
		u32 const flag = (mask & (-mask)); // rightmost one
		mask ^= flag; // turn rightmost one to zero

		component_array_t ca;
		component_array_initialize(&ca, FLAG_TO_COMPONENT_SIZE(flag));

		component_array_wrapper_t wrapper = {.flag = flag, .ca = ca};

		vector_push_back(archetype->component_arrays, wrapper);
	}
}
component_array_t* archetype_component_get(archetype_t* archetype, u32 flag)
{
	assert(archetype);
	assert(archetype->component_mask & flag); // check if archetype has the requested component_array

	// this loop iterates over all 1s that are part of mask from right to left
	// it then checks whether the current rightmost one is equal to the 1
	// requested by flag
	// if so, we know the index of the component that the flag represents
	int index = 0;
	u32 mask = archetype->component_mask;
	while(! ((mask & (-mask)) & flag))
	{
		mask ^= (mask & (-mask)); // disable rightmost one
		index++;
		printf("k\n");
	}

	return &archetype->component_arrays[index].ca;
}

void archetype_entity_add(archetype_t* archetype, entity e) // add entity_id to every component_array
{
	for(int i = 0; i < vector_get_size(archetype->component_arrays); i++)
	{
		component_array_wrapper_t* caw = (component_array_wrapper_t*) vector_get(archetype->component_arrays, i);
		component_array_add(&caw->ca, e);
	}
}


// Utility function that combines the retrieval of a component_array and the entities component inside it
void* archetype_entity_component_get(archetype_t* archetype, entity e, u32 component_flag)
{
	component_array_t* ca = archetype_component_get(archetype, component_flag);
	return component_array_get(ca, e);
}

/*
void archetype_deinitialize(archetype* archetype)
{
	u32 mask = archetype->component_mask;
	while(mask)
	{
		u32 const flag = (mask & (-mask)); // rightmost one
		mask ^= flag; // turn rightmost one to zero

		component_array_deinitialize((component_array_t*)vector_back(archetype->component_arrays));
		//vector_pop(archetype->component_arrays);
	}

}

void archetype_entity_remove(archetype, entity_id)




// this is some more abstract code found in ecs idk
void* entity_component_add(entity, component)
{
	mask = ecs_entity_mask_get()
	
	masknew = mask | component

	archetype_old = ecs_archetype_get(mask)
	archetype_new = ecs_archetype_get(masknew) // create new archetype if doesn't exist

	for(every 1 in mask)
	{
		c_o = archetype_component_get(archetype_old, current 1)
		c_n = archetype_componeng_get(archetype_new, current 1)

		component_array_add(ca, entity) = component_array_get(c_o, entity);
	}

	c_n = archetype_componeng_get(archetype_new, component)
	return component_array_add(c_n, entity) // return 
}
void entity_component_remove

*/

typedef struct c1
{
	u32 val1;
	u32 val2;
	float val3;
	int val4;
} c1_t;



#define COMPONENT_CAST(type, ptr) (*(type*)ptr)


int main()
{
	printf("Hallo Welt\n");
    

	archetype_t at;
	archetype_initialize(&at, COMPONENT_A_FLAG | COMPONENT_B_FLAG);

	for(int i = 0; i < 10; i++)
	{
		archetype_entity_add(&at, i);
		COMPONENT_CAST(component_1_t, archetype_entity_component_get(&at, i, COMPONENT_A_FLAG)).val = i;
	}


	printf("%u\n", COMPONENT_CAST(component_1_t, archetype_entity_component_get(&at, 3, COMPONENT_A_FLAG)).val);
	

	printf("Tschuess\n");
}
