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

void _component_array_initialize(component_array_t* ca, u32 capacity, u32 element_size)
{
	// Initialize primitive members
	ca->count = 0;
	ca->capacity = capacity;
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
	const u32 COMPONENT_DATA_SIZE = capacity * element_size;
	ptr = malloc(COMPONENT_DATA_SIZE);
	assert(ptr);
	ca->components = ptr;
}

#define component_array_initialize(component_array_ptr, component_type) _component_array_initialize(component_array_ptr, COMPONENT_ARRAY_DEFAULT_CAPACITY, sizeof(component_type))

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


typedef struct c1
{
	u32 val1;
	u32 val2;
	float val3;
	int val4;
} c1_t;


int main()
{
	printf("Hallo Welt\n");
    

	component_array_t ca;

	component_array_initialize(&ca, c1_t);
	


	for(int i = 0; i < 24; i++)
	{
		

		component_array_add(&ca, i);
		c1_t* comp = (c1_t*)component_array_get(&ca, i);

		comp->val1 = rand() % 200;
		comp->val2 = rand() % 10;
		comp->val3 = 42.42f;
		comp->val4 = -1;

	}
	
	printf("ID\t\tValue\n");
	for(int i = 0; i < 24; i++)
	{
		printf("%u\t\t%u\t%u\t%f\t%d\n", ca.index_to_entity[i], ((c1_t*)ca.components)[i].val1,((c1_t*)ca.components)[i].val2,((c1_t*)ca.components)[i].val3,((c1_t*)ca.components)[i].val4);
	}
	
	component_array_deinitialize(&ca);
	

	printf("Tschuess\n");
}
