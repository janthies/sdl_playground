#ifndef COMPONENTS_H
#define COMPONENTS_H
#include "types.h"

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

typedef struct script
{
    void* data;
    void(*on_create)();
    void(*on_update)(void*);
    void(*on_destroy)();
} script_c;

#define COMPONENT_A_FLAG (1 << 0)
#define COMPONENT_B_FLAG (1 << 1)
#define COMPONENT_C_FLAG (1 << 2)
#define COMPONENT_SCRIPT_FLAG (1 << 3)

#define FLAG_TO_COMPONENT_SIZE(flag) \
	(flag == COMPONENT_A_FLAG ? sizeof(component_1_t) : \
	(flag == COMPONENT_B_FLAG ? sizeof(component_2_t) : \
	(flag == COMPONENT_C_FLAG ? sizeof(component_3_t) : \
	(flag == COMPONENT_SCRIPT_FLAG ? sizeof(script_c) : \
	NULL))))


#endif