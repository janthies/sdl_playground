#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>



#include "uthash.h"
#include "vector.h"

#include "components.h"


#define MAX_ENTITIES 1200
#define COMPONENT_ARRAY_DEFAULT_CAPACITY 16
#define INVALID_ENTITY UINT32_MAX


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
	// This check can be flawed if nonsense data is supplied to this function
	// This could happen as a result of an out of bounds read, after querying
	// a list of archetypes from the ecs
	assert(archetype);

	// check if archetype has the requested component_array
	assert(archetype->component_mask & flag);

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

u32 ecs_entity_get_mask(ecs_t* ecs, entity e)
{
	return ecs->entities[e];
}

void* ecs_entity_component_get(ecs_t* ecs, entity e, u32 component_flag)
{
	assert(ecs);
	u32 const entity_mask = ecs->entities[e];

	archetype_wrapper_t* aw = NULL;
	HASH_FIND_INT(ecs->mask_to_archetype, &entity_mask, aw);
	assert(aw);

	void* component = archetype_entity_component_get(&aw->archetype, e, component_flag);
	assert(component);
	return component;
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

vector(archetype_t*) ecs_archetypes_get(ecs_t* ecs, u32 mask)
{
	archetype_wrapper_t* aw;
	vector(archetype_t*) archetypes = NULL;
	vector_initialize(archetypes);
	
	for(aw = ecs->mask_to_archetype; aw != NULL; aw = (archetype_wrapper_t*)aw->hh.next)
	{
		if((aw->mask & mask) == mask)
		{
			vector_push_back(archetypes, &aw->archetype);
		}
	}

	// Can be empty if no archetypes match the mask
	return archetypes;
}

#define COMPONENT_CAST(type, ptr) (*(type*)ptr)

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

#define COMPONENT_ARRAY_CAST(type, compoennt_array) (*(type*)component_array)

void SystemAB(ecs_t* ecs)
{
	printf("System AB\n");
	vector(archetype*) archetypes = ecs_archetypes_get(ecs, COMPONENT_A_FLAG | COMPONENT_B_FLAG);
	for(int i = 0; i < vector_get_size(archetypes); i++)
	{
		archetype_t* at = (archetype_t*)*vector_get(archetypes, i);
		component_array_t* component_a = archetype_component_get(at, COMPONENT_A_FLAG);
		component_array_t* component_b = archetype_component_get(at, COMPONENT_B_FLAG);
		
		for(int j = 0; j < component_a->count; j++)
		{
			printf("%u\t%f\n", 	((component_1_t*)(component_a->components))[j].val, ((component_2_t*)(component_b->components))[j].val);
		}
	}
}

void SystemB(ecs_t* ecs)
{
	printf("System B\n");
	vector(archetype*) archetypes = ecs_archetypes_get(ecs, COMPONENT_B_FLAG);
	for(int i = 0; i < vector_get_size(archetypes); i++)
	{
		archetype_t* at = (archetype_t*)*vector_get(archetypes, i);
		component_array_t* component_b = archetype_component_get(at, COMPONENT_B_FLAG);
		
		for(int j = 0; j < component_b->count; j++)
		{
			printf("%f\n", ((component_2_t*)(component_b->components))[j].val);
		}
	}
}

void script_system(ecs_t* ecs, float ts)
{
	vector(archetype*) archetypes = ecs_archetypes_get(ecs, COMPONENT_SCRIPT_FLAG);
	for(int i = 0; i < vector_get_size(archetypes); i++)
	{
		archetype_t* at = (archetype_t*)*vector_get(archetypes, i);
		component_array_t* scripts = archetype_component_get(at, COMPONENT_SCRIPT_FLAG);
		
		for(int j = 0; j < scripts->count; j++)
		{
			script_c* s = &((script_c*)(scripts->components))[j];
			entity eid = scripts->index_to_entity[j];

			script_data_t data = {.e = eid, .timestep = ts, .user_data = s->data};

			if(!s->running && s->on_create)
			{
				s->on_create(&data);
				s->running = true;
			}
			
			s->on_update(&data);
		}
	}
}

typedef struct playerdata
{
	u32 num;
	float runtime;
} playerdata_t;


void create_player(script_data_t* data)
{
	printf("I just got created!\n");
}

void update_player(script_data_t* data)
{
	printf("Hello target\t");

	playerdata_t* pd = (playerdata_t*)data->user_data;

	printf("I am entity: %u\t", pd->num);
	printf("Running since: %f\t", pd->runtime);

	pd->runtime += data->timestep;


	if(pd->runtime > 1.f)
	{
		//ecs_entity_remove_component(); NOT IMPLEMENTED ... components should probably be tagged and then the vector should be compacted at the end of the run
	}

	

}

void update_button(script_data_t* data)
{
	printf("Hello Button\t");
	printf("I am entity: %u\n", data->e);
}


void ecs_entity_add_script(ecs_t* ecs, entity e, script_c script)
{
	COMPONENT_CAST(script_c, ecs_entity_component_get(ecs, e, COMPONENT_SCRIPT_FLAG)) = script;
	COMPONENT_CAST(script_c, ecs_entity_component_get(ecs, e, COMPONENT_SCRIPT_FLAG)).running = false;
}



int main()
{
	ecs_t ecs;
	ecs_initialize(&ecs);

	for(int i = 0; i < 5; i++)
	{
		entity e = ecs_entity_create(&ecs);
		ecs_entity_add_component(&ecs, e, COMPONENT_A_FLAG | COMPONENT_B_FLAG);
		COMPONENT_CAST(component_1_t, ecs_entity_component_get(&ecs, e, COMPONENT_A_FLAG)).val = rand() % 10;
		COMPONENT_CAST(component_2_t, ecs_entity_component_get(&ecs, e, COMPONENT_B_FLAG)).val = (rand() % 100000) / (float)10000;
	}

	for(int i = 0; i < 5; i++)
	{
		entity e = ecs_entity_create(&ecs);
		ecs_entity_add_component(&ecs, e, COMPONENT_B_FLAG);
		COMPONENT_CAST(component_2_t, ecs_entity_component_get(&ecs, e, COMPONENT_B_FLAG)).val = (rand() % 100) / (float)10;
	}

	entity player = ecs_entity_create(&ecs);
	ecs_entity_add_component(&ecs, player, COMPONENT_SCRIPT_FLAG);

	entity button = ecs_entity_create(&ecs);
	ecs_entity_add_component(&ecs, button, COMPONENT_SCRIPT_FLAG);

	entity button2 = ecs_entity_create(&ecs);
	ecs_entity_add_component(&ecs, button2, COMPONENT_SCRIPT_FLAG);
	
	playerdata_t pd = {.num = 42, .runtime = 0.f};

	ecs_entity_add_script(&ecs, player, {
		.data =  (void*)&pd,  
		.on_create = create_player,
		.on_update = update_player
	});

	ecs_entity_add_script(&ecs, button, {
		.on_update = update_button
	});

	ecs_entity_add_script(&ecs, button2, {
		.on_update = update_button
	});



	SystemAB(&ecs);
	SystemB(&ecs);

	script_system(&ecs, 0.2f);
	script_system(&ecs, 0.2f);
	script_system(&ecs, 0.2f);
	script_system(&ecs, 0.2f);
	script_system(&ecs, 0.2f);
	script_system(&ecs, 0.2f);
}


