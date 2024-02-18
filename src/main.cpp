#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <SDL.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "types.h"
#include "vector.h"


#define MAX_ENTITIES 55000


typedef enum component
{
    POSITION,
    VELOCITY,
    SPRITE,
    COUNT
} component_e;

typedef u32 entity_t;
typedef u32 component_mask_t;

typedef struct position_t
{
    float posX;
    float posY;
    float vV;
    float vH;

} position_t;

typedef struct velocity
{
    float v;
} velocity_t;

typedef struct sprite
{
    u32 width;
    u32 height;
    SDL_Color color;
} sprite_t;


typedef struct component_container
{
    u32 count = 0;
    u32 component_size = 0;
    entity_t entity_to_index[MAX_ENTITIES];
    u32 index_to_entity[MAX_ENTITIES];
    void* data; // position_t, velocity_t etc
} component_container_t;


typedef struct entity_component_system
{
    u32 count = 0;
    entity_t entities_alive[MAX_ENTITIES] {0}; // e7, e1, e15, e9, e3
    entity_t entities_dead[MAX_ENTITIES] = {0}; // ... kind of a freelist? 
    component_mask_t entity_component_masks[MAX_ENTITIES] = {0}; // ecm[entity_id] = mask

    component_container_t components[COUNT];
} ecs_t;

void _ecs_initialize_component(component_container_t* container, u32 component_size)
{
    container->component_size = component_size;
    container->data = calloc(MAX_ENTITIES, component_size);
}

void ecs_initialize(ecs_t* ecs)
{
    _ecs_initialize_component(&ecs->components[POSITION], sizeof(position_t));
    _ecs_initialize_component(&ecs->components[VELOCITY], sizeof(velocity_t));
    _ecs_initialize_component(&ecs->components[SPRITE], sizeof(sprite_t));

    for(int i = 0; i < MAX_ENTITIES; i++)
    {
        ecs->entities_dead[MAX_ENTITIES - 1 - i] = i;
    }
}

void ecs_uninitialize(ecs_t* ecs)
{
    assert(false);
}

component_container_t* ecs_component_get(ecs_t* ecs, component_e component)
{
    return &ecs->components[component];
}

void* ecs_component_entity_data_get(component_container_t* container, entity_t e_id)
{
    const u32 index = container->entity_to_index[e_id];
    const u32 offset = index * container->component_size;

    return container->data + offset;
}

entity_t ecs_entity_create(ecs_t* ecs)
{
    assert(ecs->count < MAX_ENTITIES);
    entity_t new_entity = ecs->entities_dead[MAX_ENTITIES - 1 - ecs->count];
    ecs->entities_alive[ecs->count++] = new_entity;

    //printf("Hallo entity: %u\n", new_entity);
    return new_entity;
}

void ecs_entity_destroy(ecs_t* ecs, entity_t e_id)
{
    assert(false);
}

void ecs_component_entity_add(ecs_t* ecs, component_e component, entity_t e_id)
{
    assert(e_id < MAX_ENTITIES);
    
    component_container_t* container = &ecs->components[component];
    assert(container->count < MAX_ENTITIES);
    
    container->entity_to_index[e_id] = container->count;
    container->index_to_entity[container->count] = e_id;
    container->count++;
}

void _component_entity_remove(component_container_t* container, entity_t e_id)
{
    assert(e_id < MAX_ENTITIES);
    assert(container->count > 0);
    assert(container->entity_to_index[e_id] == (entity_t)-1);

    const u32 back_index = container->count - 1;
    
    // Put data of last element into element that is about to be removed
    const u32 move_to_offset = container->component_size * container->entity_to_index[e_id];
    const u32 move_from_offset = container->component_size * back_index;
    memcpy( container->data + move_to_offset, 
            container->data + move_from_offset, 
            container->component_size);

    //container->data[container->entity_to_index[e_id]] = container->data[container->count - 1]; -> not possibble because of missing type information

    // Put index of last entity into slot of entity that is being removed
    container->index_to_entity[container->entity_to_index[e_id]] = container->entity_to_index[back_index];

    // Update entity_to_index mapping to match the previous data swap
    container->entity_to_index[back_index] = container->entity_to_index[e_id];

    // invalidate removed entity
    container->entity_to_index[e_id] = (entity_t)-1;
}

void ecs_component_entity_remove(ecs_t* ecs, component_e component, entity_t e_id)
{
    _component_entity_remove(&ecs->components[component], e_id);
}

void ecs_system_update_position(ecs_t* ecs, float dt)
{
    component_container_t* position_c = &ecs->components[POSITION];
    position_t* positions = (position_t*)position_c->data;

    for(int i = 0; i < position_c->count; i++)
    {
        position_t* data = &positions[i];

        data->posX += data->vH * dt;
        data->posY += data->vV * dt;

        if(data->posX < 0 || data->posX > 1820) { data->vH *= -1; }
        if(data->posY < 0 || data->posY > 980) { data->vV *= -1; }
    }
}

void ecs_system_update_velocity(ecs_t* ecs)
{
    component_container_t* velocity_c = &ecs->components[VELOCITY];
    velocity_t* velocities = (velocity_t*)velocity_c->data;

    for(int i = 0; i < velocity_c->count; i++)
    {
        ImGui::Text("e_id: %u, velocity: %f", velocity_c->index_to_entity[i], velocities[i].v);
        
    }
}

void ecs_system_update_sprite(ecs_t* ecs)
{
}

void create_sprite_entities(ecs_t* ecs)
{
    entity_t e_id = ecs_entity_create(ecs);
    ecs_component_entity_add(ecs, POSITION, e_id);
    ecs_component_entity_add(ecs, SPRITE, e_id);

    position_t pos = 
    {
        .posX = rand() % 500,
        .posY = rand() % 500,
        .vV = (rand() % 100),
        .vH = (rand() % 100)
    };

    position_t* e_pos = (position_t*) ecs_component_entity_data_get(ecs_component_get(ecs, POSITION), e_id);
    *e_pos = pos;

}

uint8_t color_float_to_uint(float color)
{
    return (uint8_t)(color * 255);
}

int new_entities = 0;
int rectangle_sidelength = 1;



typedef struct player {
    float posX = 0;
    float posY = 0;
    float const speed = 250.f;
} player_t;

player_t player;

///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////ECS////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

typedef struct component_array
{
    u32 count;
    u32 element_size;
    u32* entity_to_index;
    u32* index_to_entity;
    vector(void) v;
} component_array_t; 

void component_array_init(component_array_t* ca, u32 element_size)
{
    ca->count = 0;
    ca->element_size = element_size;

    void* ptr = calloc(MAX_ENTITIES, sizeof(u32));
    assert(ptr);
    ca->entity_to_index = (u32*) ptr;

    ptr = calloc(MAX_ENTITIES, sizeof(u32));
    assert(ptr);
    ca->index_to_entity = (u32*) ptr;
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#undef main
int main()
{
    srand(1);
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        return -1;
    }

    SDL_Window* window_ptr = SDL_CreateWindow(
        "window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1920,
        1080,
        SDL_WINDOW_SHOWN
    );

    if(!window_ptr)
    {
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer_ptr = SDL_CreateRenderer(window_ptr, -1, 0);

    if(!renderer_ptr)
    {
        SDL_DestroyWindow(window_ptr);
        SDL_Quit();
        return -1;
    }


    SDL_Texture* texture = SDL_CreateTexture(
        renderer_ptr,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        300,
        200
    );

    SDL_SetRenderTarget(renderer_ptr, texture);
    SDL_SetRenderDrawColor(renderer_ptr, 255, 0, 0, 255);
    SDL_RenderClear(renderer_ptr);
    SDL_SetRenderTarget(renderer_ptr, NULL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplSDL2_InitForSDLRenderer(window_ptr, renderer_ptr);
    ImGui_ImplSDLRenderer2_Init(renderer_ptr);



    position_t pos = {
        .posX = 10,
        .posY = 10
    };

    ecs_t my_ecs;
    ecs_initialize(&my_ecs);
    
    bool running = true;
    while(running)
    {
        static u32 millisNow = SDL_GetTicks();
        u32 millisPast = millisNow;
        millisNow = SDL_GetTicks();
        float const ds = (float)(millisNow - millisPast) / (float)1000; 
        
        


        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
            {
                running = false;
            }
            if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
            {
                running = false;
            }
            if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_0)
            {
            }
            if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_9)
            {
            }
            ImGui_ImplSDL2_ProcessEvent(&e);
        }

        
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame(window_ptr);
        ImGui::NewFrame();


        ImGui::Begin("color");
        float colors[3];
        ImGui::ColorPicker3("color", colors, 0);

        ImGui::InputInt("new entity count", &new_entities);
        if(ImGui::Button("SpawnEntities"))
        {
            for(int i = 0; i < new_entities; i++)
            {
                create_sprite_entities(&my_ecs);
            }
        }
        ImGui::SliderInt("rectangle size", &rectangle_sidelength, 1, 15);
        ImGui::End();

        const Uint8* keystates = SDL_GetKeyboardState(NULL);

        // Check if 'A' key is pressed
        if (keystates[SDL_SCANCODE_A]) {
            player.posX -= player.speed * ds;
        }
        if (keystates[SDL_SCANCODE_D]) {
            player.posX += player.speed * ds;
        }
        if (keystates[SDL_SCANCODE_W]) {
            player.posY -= player.speed * ds;
        }
        if (keystates[SDL_SCANCODE_S]) {
            player.posY += player.speed * ds;
        }

        ImGui::Begin("Entities - Position");
        ecs_system_update_position(&my_ecs, ds);
        ImGui::End();
        
        
        SDL_SetRenderDrawColor(renderer_ptr, 255, 255, 255, 255);
        SDL_RenderClear(renderer_ptr);

        


        // Draw all entities with sprite component
        component_container_t* sprite_c = ecs_component_get(&my_ecs, SPRITE);
        component_container_t* position_c = ecs_component_get(&my_ecs, POSITION);
        for(int i = 0; i < sprite_c->count; i++)
        {
            entity_t e_id = sprite_c->index_to_entity[i];
            sprite_t* entity_sprite_data = (sprite_t*) ecs_component_entity_data_get(sprite_c, e_id);
            position_t* entity_position_data = (position_t*) ecs_component_entity_data_get(position_c, e_id);

            SDL_Rect rectangle = {
                .x = entity_position_data->posX,
                .y = entity_position_data->posY,
                .w = rectangle_sidelength,
                .h = rectangle_sidelength

            };

            SDL_SetRenderDrawColor( renderer_ptr, 
                                    120, 120, 120, 255);

            SDL_RenderFillRect(renderer_ptr, &rectangle);

        }


        SDL_Rect player_sprite = {
            .x = (u32) player.posX,
            .y = (u32) player.posY,
            .w = 10,
            .h = 10
        };

        SDL_SetRenderDrawColor(renderer_ptr,
                               color_float_to_uint(colors[0]), 
                               color_float_to_uint(colors[1]),
                               color_float_to_uint(colors[2]),
                               255);
        SDL_RenderFillRect(renderer_ptr, &player_sprite);

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

        SDL_RenderPresent(renderer_ptr);
    }


    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();


    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer_ptr);
    SDL_DestroyWindow(window_ptr);
    SDL_Quit();


    
    return 0;
}