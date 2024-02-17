#include "ecs.h"

typedef struct component_container
{
    u32 count = 0;
    u32 component_size = 0;
    entity_t entity_to_index[MAX_ENTITIES];
    u32 index_to_entity[MAX_ENTITIES];
    void* data; // position_t, velocity_t etc
} component_container_t;