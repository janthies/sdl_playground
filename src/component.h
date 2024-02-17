#ifndef COMPONENT_H
#define COMPONENT_H

typedef u32 entity_t;
typedef enum component
{
    POSITION,
    VELOCITY,
    SPRITE,
    COUNT
} component_e;


typedef struct component_container component_container_t;

void component_entity_add(component_container_t* container, entity_t e_id);
void component_entity_remove(component_container_t* container, entity_t e_id);

void* component_entity_data_get(component_container_t* container, entity_t e_id);

#endif 