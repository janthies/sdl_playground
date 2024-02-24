#ifndef COMPONENTS_H
#define COMPONENTS_H
#include "types.h"
#include "util.h"

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

#define MAX_SCRIPT_DATA 64

typedef struct script_data
{
	entity e;
	float timestep; // seconds
	void* user_data;
} script_data_t;

typedef struct script
{
	bool running;
    void* data;
    void(*on_create)(script_data_t*);
    void(*on_update)(script_data_t*);
} script_c;

#define COMPONENT_A_FLAG (1 << 1)
#define COMPONENT_B_FLAG (1 << 2)
#define COMPONENT_C_FLAG (1 << 3)
#define COMPONENT_SCRIPT_FLAG (1 << 4)

#define FLAG_TO_COMPONENT_SIZE(flag) \
	(flag == COMPONENT_A_FLAG ? sizeof(component_1_t) : \
	(flag == COMPONENT_B_FLAG ? sizeof(component_2_t) : \
	(flag == COMPONENT_C_FLAG ? sizeof(component_3_t) : \
	(flag == COMPONENT_SCRIPT_FLAG ? sizeof(script_c) : \
	0))))

// wrapping T in () leads to compile time error
#define COMPONENT_TO_FLAG(T) _Generic(T, \
	component_1_t : COMPONENT_A_FLAG, \
	component_2_t : COMPONENT_B_FLAG, \
	component_3_t : COMPONENT_C_FLAG, \
	script_c : COMPONENT_SCRIPT_FLAG, \
	default : -1)



#define REC0(FIRST, ...) 0
#define REC1(FIRST, ...) COMPONENT_TO_FLAG(FIRST)
#define REC2(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC1(__VA_ARGS__)
#define REC3(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC2(__VA_ARGS__)
#define REC4(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC3(__VA_ARGS__)
#define REC5(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC4(__VA_ARGS__)
#define REC6(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC5(__VA_ARGS__)
#define REC7(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC6(__VA_ARGS__)
#define REC8(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC7(__VA_ARGS__)
#define REC9(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC8(__VA_ARGS__)
#define REC10(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC9(__VA_ARGS__)
#define REC11(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC10(__VA_ARGS__)
#define REC12(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC11(__VA_ARGS__)
#define REC13(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC12(__VA_ARGS__)
#define REC14(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC13(__VA_ARGS__)
#define REC15(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC14(__VA_ARGS__)
#define REC16(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC15(__VA_ARGS__)
#define REC17(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC16(__VA_ARGS__)
#define REC18(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC17(__VA_ARGS__)
#define REC19(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC18(__VA_ARGS__)
#define REC20(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC19(__VA_ARGS__)
#define REC21(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC20(__VA_ARGS__)
#define REC22(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC21(__VA_ARGS__)
#define REC23(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC22(__VA_ARGS__)
#define REC24(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC23(__VA_ARGS__)
#define REC25(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC24(__VA_ARGS__)
#define REC26(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC25(__VA_ARGS__)
#define REC27(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC26(__VA_ARGS__)
#define REC28(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC27(__VA_ARGS__)
#define REC29(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC28(__VA_ARGS__)
#define REC30(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC29(__VA_ARGS__)
#define REC31(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC30(__VA_ARGS__)
#define REC32(FIRST, ...) COMPONENT_TO_FLAG(FIRST) | REC31(__VA_ARGS__)
#define RECN(n, ...) REC##n(__VA_ARGS__)
#define REC_INDIRECT(n, ...) RECN(n, __VA_ARGS__)
#define GET_MASK_FROM_COMPONENT_TYPES(...) REC_INDIRECT(VAR_COUNT(__VA_ARGS__), __VA_ARGS__)

#endif