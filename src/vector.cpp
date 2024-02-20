#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


#include "vector.h"
#include "uthash.h"


#define MAX_ENTITIES 1200
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
	assert(ca->entity_to_index[e] != INVALID_ENTITY);	// check if entity is registered
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
	}

	return &archetype->component_arrays[index].ca;
}

void archetype_entity_add(archetype_t* archetype, entity e) // add entity_id to every component_array
{
	assert(archetype);
	printf("Adding entity %u\n", e);
	for(int i = 0; i < vector_get_size(archetype->component_arrays); i++)
	{
		component_array_wrapper_t* caw = (component_array_wrapper_t*) vector_get(archetype->component_arrays, i);
		component_array_add(&caw->ca, e);
	}
}


// Utility function that combines the retrieval of a component_array and the entities component inside it
void* archetype_entity_component_get(archetype_t* archetype, entity e, u32 component_flag)
{
	assert(archetype);
	component_array_t* ca = archetype_component_get(archetype, component_flag);
	return component_array_get(ca, e);
}

void archetype_entity_remove(archetype_t* archetype, entity e)
{
	assert(archetype);
	printf("removing entity %u\n", e);
	for(int i = 0; i < vector_get_size(archetype->component_arrays); i++)
	{
		component_array_wrapper_t* caw = (component_array_wrapper_t*) vector_get(archetype->component_arrays, i);
		component_array_remove(&caw->ca, e);
	}
}

void archetype_deinitialize(archetype_t* archetype)
{
	assert(archetype);
	for(int i = 0; i < vector_get_size(archetype->component_arrays); i++)
	{
		component_array_deinitialize(&(archetype->component_arrays[i].ca));
	}
	archetype->component_mask = 0; // doesn't contain any component arrays
	vector_destroy(archetype->component_arrays);
}

u32 archetype_entity_count(archetype_t* archetype)
{
	assert(archetype);
	if(! archetype->component_arrays) { return 0; } // will be false if there are no component arrays
	return archetype->component_arrays[0].ca.count; // count will be the same for every component array belonging to this archetype
}

typedef struct
{
	u32 mask;	// key
	archetype_t archetype;
	UT_hash_handle hh;
} archetype_wrapper_t;


// entities acts as a freelist
// if start points to an index, the data at that place isn't considered to be a component mask
// instead, the data is considered to be a pointer to the next free index holding a dead entity
typedef struct ecs
{
	size_t start;
	size_t available;
	entity entities[MAX_ENTITIES]; // Entity freelist

	archetype_wrapper_t* mask_to_archetype; // hm mask <--> archetype
} ecs_t;

#define ECS_STOP_INDEX UINT32_MAX
void ecs_initialize(ecs_t* ecs)
{
	assert(ecs);
	ecs->start = 0;
	ecs->available = MAX_ENTITIES;
	for(int i = 0; i < MAX_ENTITIES; i++)
	{
		ecs->entities[i] = i + 1;
	}

	ecs->mask_to_archetype = NULL;
}


entity ecs_entity_create(ecs_t* ecs)
{
	assert(ecs);
	assert(ecs->available);

	entity e = ecs->start;
	ecs->start = ecs->entities[ecs->start];
	ecs->available--;

	ecs->entities[e] = 0; // no components yet
	printf("new entity: %u\n", e);

	return e;
}

void ecs_entity_destroy(ecs_t* ecs, entity e)
{
	assert(ecs);
	// can't make sure that the entity is indeed being used

	ecs->entities[e] = ecs->start;
	ecs->start = e;

	ecs->available++;
	printf("destroyed entity %u\n", e);
}

void ecs_entity_add_component(ecs_t* ecs, entity e, u32 component_flag)
{
	assert(ecs);

	u32 const current_mask = ecs->entities[e];
	u32 const new_mask = current_mask | component_flag;


	archetype_wrapper_t* old_archetype;
	HASH_FIND_INT(ecs->mask_to_archetype, &current_mask, old_archetype);

	archetype_wrapper_t* new_archetype;
	HASH_FIND_INT(ecs->mask_to_archetype, &new_mask, new_archetype);

	printf("Old mask: %x\t New mask: %x\t - found old: %i\t found new: %i\n", current_mask, new_mask, old_archetype != NULL, new_archetype != NULL);

	// Create new archetype if it doesn't yet exist
	if(!new_archetype)
	{
		new_archetype = (archetype_wrapper_t*)malloc(sizeof(archetype_wrapper_t));
		new_archetype->mask = new_mask;
		archetype_initialize(&new_archetype->archetype, new_mask);
		HASH_ADD_INT(ecs->mask_to_archetype, mask, new_archetype);
	}

	archetype_entity_add(&new_archetype->archetype, e);
	ecs->entities[e] = new_mask; // entity now belongs to new archetype
		
	// In case the old archetype exists, copy over the data
	if(old_archetype)
	{
		u32 mask = current_mask;
		u32 rightmost_one = (mask & (-mask));
		while(mask)
		{
			component_array_t* ca_old = (component_array_t*)archetype_component_get(&old_archetype->archetype, rightmost_one);
			component_array_t* ca_new = (component_array_t*)archetype_component_get(&new_archetype->archetype, rightmost_one);

			memcpy(component_array_get(ca_new, e), component_array_get(ca_old, e), ca_new->element_size);
			
			mask ^= rightmost_one;
			rightmost_one = (mask & (-mask));
		}
		
		archetype_entity_remove(&old_archetype->archetype, e);
	}
}

/*
ecs_entity_remove_component(ecs, entity, flag)
{
	
}

// should be more efficient than repeated component_add or component_remove calls
ecs_entity_component_set(ecs, entity, flag)
{
	
}

mask ecs_entity_component_mask_get(ecs, entity); // needed for removing multiple components at once | old mask can be & ~newMask in order to selectively disable components



vector(archetype) ecs_archetypes_get(ecs_t* ecs, u32 mask)
{
	// return a vector of all archetypes, that have mask component_arrays as subgroup
}
// This function can be called by a system
RandomSystemFunction()
{
	vector(archetype) archetypes = ecs_archetypes_get(ecs, COMPONENT_X | COMPONENT_Y | COMPONENT_Z);

	for every archetype
	{
		component_arrayX = archetype_component_get(archetype, COMPONENT_X);
		component_arrayY = archetype_component_get(archetype, COMPONENT_Y);
		component_arrayZ = archetype_component_get(archetype, COMPONENT_Z);

		for(int i = 0; i < archetype_entity_count(); i++)
		{
			// do calculations
			component_array_X[i] += component_array_Y[i];
		}
	}

}

*/





/*
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
*/

#define COMPONENT_CAST(type, ptr) (*(type*)ptr)

void print_list(ecs_t* ecs)
{
	printf("List: \n");
	for(int i = 0; i < 8; i++)
	{
		printf("E: %u, Ptr: %u\n", i, ecs->entities[i]);
	}
	printf("\n\n");
}

void shuffle(u32* unique_indices)
{
	for(int i = 0; i < 3000; i++)
	{
		u32 const left = rand() % MAX_ENTITIES;
		u32 const right = rand() % MAX_ENTITIES;

		u32 const a = unique_indices[left];
		unique_indices[left] = unique_indices[right];
		unique_indices[right] = a;
	}
}

typedef struct 
{
	int key;
	u32 val;
	UT_hash_handle hh;
} Test;






int main()
{

	Test* test = NULL;

	Test t1 = {.key = 42, .val = 69};
	Test t2 = {.key = 17, .val = 24};
	Test t3 = {.key = 100, .val = 200};

	//HASH_ADD_INT(test, key, &t1);
	//HASH_ADD_INT(test, key, &t2);
	//HASH_ADD_INT(test, key, &t3);

	Test* res;
	int search_id = 16;
	HASH_FIND_INT(test, &search_id , res);

	if(res)
	{
		printf("Found val: %u\n", res->val);
	}
	else
	{
		printf("not found\n");
	}

	ecs_t ecs;
	ecs_initialize(&ecs);


	entity e = ecs_entity_create(&ecs);
	ecs_entity_add_component(&ecs, e, COMPONENT_A_FLAG);
	ecs_entity_add_component(&ecs, e, COMPONENT_B_FLAG);
	
	entity e2 = ecs_entity_create(&ecs);
	ecs_entity_add_component(&ecs, e2, COMPONENT_A_FLAG | COMPONENT_B_FLAG);

	entity e3 = ecs_entity_create(&ecs);
	ecs_entity_add_component(&ecs, e3, COMPONENT_A_FLAG);
	ecs_entity_add_component(&ecs, e3, COMPONENT_B_FLAG);

	/*
	printf("Hallo Welt\n");

    
	archetype_t at;
	archetype_initialize(&at, COMPONENT_A_FLAG | COMPONENT_B_FLAG);

	for(int i = 0; i < 10; i++)
	{
		archetype_entity_add(&at, i);
		COMPONENT_CAST(component_1_t, archetype_entity_component_get(&at, i, COMPONENT_A_FLAG)).val = i;
	}

	COMPONENT_CAST(component_2_t, archetype_entity_component_get(&at, 3, COMPONENT_B_FLAG)).val = 69.42f;

	printf("%f\n", COMPONENT_CAST(component_2_t, archetype_entity_component_get(&at, 3, COMPONENT_B_FLAG)).val);

	archetype_entity_remove(&at, 2);

	archetype_deinitialize(&at);

	printf("Tschuess\n");
	
	*/

}


