#include "raylib.h"
#include <math.h>
#include <stdlib.h>

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define PLAYER_MAX_LIFE         5
#define MAX_PLATFORMS           20
#define GRAVITY                 1000.0f

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Object
{
    Vector2 position;
    Vector2 prev_position;
    Vector2 size;
    Vector2 velocity;

    float friction;

    bool on_ground;
}Object;

typedef struct Player
{
    Object object;

    Color color;

    float default_max_vel;
    float move_speed;
    float max_velocity;
    float air_max_vel;
    float jump_power;

    int life;

} Player;

typedef struct Platform
{

    Vector2 position;
    Vector2 size;

    Color color;

    float friction;
    float coef_max_vel;

} Platform;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------

static const int screenWidth = 800;
static const int screenHeight = 450;

static bool game_over = false;

static Player player = {0};


static Platform platforms[MAX_PLATFORMS];
static int platform_count = 0;
static float platform_main_friction = 2000.0f;

static float ground_y = 420;
static float platform_width = 100;
static float platform_height = 20;
static float gap = 50;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

static void jump();
static void player_move(float dt);
static int on_which_ground(const Object *object);
static int has_landed_on(Object *object);
static int touching_which_platform(Object *object);
static float clamp(float, float min, float max);

static void AddPlatform(float x, float y, float width, float height,Color color, float friction, float coef_max_vel);

int main()
{

    SetTargetFPS(60);

    InitWindow(screenWidth,screenHeight,"TEST");

    InitGame();

    while(!WindowShouldClose())
    {
        UpdateDrawFrame();

    }
    CloseWindow();
}

static void InitGame(void)
{
    //PLATFORM DRAWING

    //LEVEL PLATFORMS
    AddPlatform(platform_count*platform_width+platform_count * gap,ground_y, platform_width,platform_height,BLACK,platform_main_friction,1.0f);
    AddPlatform(platform_count*platform_width+platform_count * gap,ground_y,platform_width,platform_height,GREEN,platform_main_friction,1.5f);
    AddPlatform(platform_count*platform_width+platform_count * gap,ground_y,platform_width,platform_height,BLACK,platform_main_friction,1.0f);
    AddPlatform(platform_count*platform_width+platform_count * gap,ground_y,platform_width,platform_height,BLACK,platform_main_friction,1.0f);
    AddPlatform(platform_count*platform_width+platform_count * gap,ground_y,platform_width,platform_height,BLACK,platform_main_friction,1.0f);

    AddPlatform(500,ground_y-platform_width,platform_height,platform_width,BLACK,platform_main_friction,1.0f);

    //player setup
    player.object.position = (Vector2){20,0};
    player.object.size = (Vector2){10,10};
    player.life = PLAYER_MAX_LIFE;
    player.move_speed = 2500.0f;
    player.jump_power = 350.0f;
    player.default_max_vel = 250.0f;
    player.max_velocity = player.default_max_vel;
    player.object.friction = player.move_speed /2;

}

static void AddPlatform(float x, float y, float width, float height, Color color, float friction,float coef_max_vel)
{
    if (platform_count<MAX_PLATFORMS)
    {
        platforms[platform_count++] = (Platform)
        {
            .position = {x,y},
            .size = {width,height},
            .color = color,
            .friction = friction,
            .coef_max_vel = coef_max_vel
        };
    }

}

static void DrawGame(void)
{
    BeginDrawing();
        ClearBackground(RAYWHITE);
        for(int i = 0; i < platform_count;i++)
        {
            DrawRectangleV(platforms[i].position,platforms[i].size,platforms[i].color);
        }
        DrawRectangleV(player.object.position,player.object.size,RED);
    EndDrawing();

}

// Update game (one frame)
static void UpdateGame(void)
{
    float dt = GetFrameTime();
    if(!game_over)
    {
        player_move(dt);
    }
}

// Update and Draw (one frame)
static void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}

static void player_move(float dt)
{
    //Jump stuff
    int ground_platform, landed_on, touching_platform;
    player.object.prev_position.y = player.object.position.y;
    player.object.prev_position.x = player.object.position.x;
    if(IsKeyPressed(KEY_SPACE))
        jump();
    if (IsKeyReleased(KEY_SPACE) && player.object.velocity.y < 0)
        player.object.velocity.y *= 0.5f;
    //gravity stuff
    if ((ground_platform = on_which_ground(&player.object)) == -1)//on air
    {
        player.object.on_ground = false;
        player.object.velocity.y+=GRAVITY*dt;
        player.object.position.y += player.object.velocity.y * dt;
        player.object.friction = player.move_speed *0.2f;
        player.max_velocity = player.air_max_vel;
        if((landed_on = has_landed_on(&player.object)) > -1)
        {
            player.object.on_ground = true;
        }
        
    }
    else{ //on ground
        player.object.friction = platforms[ground_platform].friction;
        player.max_velocity = player.default_max_vel*platforms[ground_platform].coef_max_vel;
        player.air_max_vel = player.max_velocity *1.05f;
        player.object.on_ground = true;
    }
    //walking stuff
    if(IsKeyDown(KEY_A))
        player.object.velocity.x -= player.move_speed * dt;

    if(IsKeyDown(KEY_D))
        player.object.velocity.x += player.move_speed * dt;

    if(fabs(player.object.velocity.x)>player.object.friction * dt)
    {
        if(player.object.velocity.x < 0)
            player.object.velocity.x += player.object.friction * dt;
        else if(player.object.velocity.x > 0)
            player.object.velocity.x -= player.object.friction * dt;
    }
    else
    {
        player.object.velocity.x = 0;
    }
    player.object.velocity.x = clamp(player.object.velocity.x,player.max_velocity*-1,player.max_velocity);
    player.object.position.x += player.object.velocity.x * dt;

    //checks obstacles for walking
    touching_platform = touching_which_platform(&player.object);

    

    
    
}

static void jump()
{
    if (player.object.on_ground)
    {
        player.object.velocity.y -= player.jump_power;
    }


}

static int on_which_ground(const Object *object)
{
    for(int i = 0;i<platform_count;i++)
    {
        if(object->velocity.y >= 0 && 
            fabsf(object->position.y + object->size.y - platforms[i].position.y) < 0.1f&&
            object->position.x+object->size.x > platforms[i].position.x &&
            object->position.x<platforms[i].position.x + platforms[i].size.x)
            return i;
    }
    return -1;
}
static int has_landed_on(Object *object)
{
    for(int i = 0;i<platform_count;i++)
    {
        if(object->position.y+object->size.y>=platforms[i].position.y &&
            object->position.x+object->size.x > platforms[i].position.x &&
            object->position.x<platforms[i].position.x + platforms[i].size.x)
        {
            if(object->prev_position.y+object->size.y<platforms[i].position.y && 
                object->position.y+object->size.y>=platforms[i].position.y)
            {
                object->position.y = platforms[i].position.y - object->size.y;
                object->velocity.y = 0;
                return i;
            }
        }
    }
    return -1;
}

static int touching_which_platform(Object *object)
{
    float object_up, object_bottom, object_right, object_left, platform_up, platform_bottom, platform_left, platform_right,prev_up, prev_bottom, prev_left, prev_right;
    for (int i = 0; i<platform_count;i++)
    {
        object_up = object->position.y;
        object_bottom = object->position.y + object->size.y;
        object_left = object->position.x;
        object_right = object->position.x + object->size.x;

        prev_bottom = object->prev_position.y + object->size.y;
        prev_up = object->prev_position.y;
        prev_left = object->prev_position.x;
        prev_right = object->prev_position.x + object->size.x; 

        platform_up = platforms[i].position.y;
        platform_bottom = platforms[i].position.y + platforms[i].size.y;
        platform_left = platforms[i].position.x;
        platform_right = platforms[i].position.x + platforms[i].size.x;

        if ((object_right >= platform_left && object_left <= platform_left && object_bottom >= platform_up && object_up <= platform_bottom)||
            (object_left <= platform_right && object_right >= platform_right && object_bottom >= platform_up && object_up <= platform_bottom))
        {
            if (prev_left >= platform_right && object->velocity.x < 0)
            {
                object->position.x = platform_right;
                object->velocity.x = 0;
                return i;
            }
            else if(prev_right <= platform_left && object->velocity.x > 0)
            {
                object->position.x = platform_left - object->size.x;
                object->velocity.x = 0;
                return i;
            }
        }
    }
    return -1;
}

static float clamp(float move_speed, float min_vel, float max_vel)
{
    if (move_speed<min_vel) return min_vel;
    if (move_speed>max_vel) return max_vel;
    return move_speed;
}