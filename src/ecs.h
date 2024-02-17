#ifndef ECS_H
#define ECS_H

#include "types.h"
#include "component.h"


#define MAX_ENTITIES 10000;
typedef struct entity_component_system ecs_t;



void ecs_initialize(ecs_t* ecs);
void ecs_uninitialize(ecs_t* ecs);

entity_t ecs_entity_create(ecs_t* ecs)
void ecs_entity_destroy(ecs_t* ecs, entity_t e_id)

void ecs_component_entity_add(ecs_t* ecs, component_e component, entity_t e_id);
void ecs_component_entity_remove(ecs_t* ecs, component_e component, entity_t e_id);

component_container_t* ecs_component_get(ecs_t* ecs, component_e component);



#endif