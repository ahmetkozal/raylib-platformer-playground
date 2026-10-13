#include "raylib.h"
#include <math.h>
#include <stdlib.h>

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define PLAYER_MAX_LIFE         5
#define MAX_PLATFORMS           20
#define GRAVITY                 1000.0f
#define FRICTION                500.0f

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Object
{
    Vector2 position;
    Vector2 prev_position;
    Vector2 size;
    Vector2 velocity;

    bool on_ground;
}Object;

typedef struct Player
{
    Object object;

    Color color;

    float move_speed;
    float max_velocity;
    float jump_power;

    int life;

} Player;

typedef struct Platform
{

    Vector2 position;
    Vector2 size;

    Color color;

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
static bool is_on_ground(const Object *object);
static bool has_landed(Object *object);
static float clamp(float, float min, float max);

static void AddPlatform(float x, float y, float width, float height,Color color);

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
    AddPlatform(platform_count*platform_width+platform_count * gap,ground_y, platform_width,platform_height,BLACK);
    AddPlatform(platform_count*platform_width+platform_count * gap,ground_y,platform_width,platform_height,BLACK);
    AddPlatform(platform_count*platform_width+platform_count * gap,ground_y,platform_width,platform_height,BLACK);
    AddPlatform(platform_count*platform_width+platform_count * gap,ground_y,platform_width,platform_height,BLACK);
    AddPlatform(platform_count*platform_width+platform_count * gap,ground_y,platform_width,platform_height,BLACK);

    AddPlatform(500,ground_y-platform_width,platform_height,platform_width,BLACK);

    player.object.position = (Vector2){20,0};
    player.object.size = (Vector2){10,10};
    player.life = PLAYER_MAX_LIFE;
    player.move_speed = 1000.0f;
    player.jump_power = 500.0f;
    player.max_velocity = 500.0f;

}

static void AddPlatform(float x, float y, float width, float height, Color color)
{
    if (platform_count<MAX_PLATFORMS)
    {
        platforms[platform_count++] = (Platform)
        {
            .position = {x,y},
            .size = {width,height},
            .color = color
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
    player.object.prev_position.y = player.object.position.y;
    if(IsKeyPressed(KEY_SPACE))
        jump();
    if (IsKeyReleased(KEY_SPACE) && player.object.velocity.y < 0)
        player.object.velocity.y *= 0.5f;
    //gravity stuff
    if (!(player.object.on_ground = is_on_ground(&player.object)))
    {
        player.object.velocity.y+=GRAVITY*dt;
        player.object.position.y += player.object.velocity.y * dt;
        has_landed(&player.object);
    }
    //walking stuff
    if(IsKeyDown(KEY_A))
        player.object.velocity.x -= player.move_speed * dt;

    if(IsKeyDown(KEY_D))
        player.object.velocity.x += player.move_speed * dt;

    if(fabs(player.object.velocity.x)>FRICTION * dt)
    {
        if(player.object.velocity.x < 0)
            player.object.velocity.x += FRICTION * dt;
        else if(player.object.velocity.x > 0)
            player.object.velocity.x -= FRICTION * dt;
    }
    else
    {
        player.object.velocity.x = 0;
    }
    
    player.object.velocity.x = clamp(player.object.velocity.x,player.max_velocity*-1,player.max_velocity);
    player.object.position.x += player.object.velocity.x * dt;

}

static void jump()
{
    if (player.object.on_ground)
    {
        player.object.velocity.y -= player.jump_power;
    }


}

static bool is_on_ground(const Object *object)
{
    for(int i = 0;i<platform_count;i++)
    {
        if(object->velocity.y >= 0 && fabsf(object->position.y + object->size.y - platforms[i].position.y) < 0.1f&&
            object->position.x+object->size.x > platforms[i].position.x &&
            object->position.x<platforms[i].position.x + platforms[i].size.x)
            return true;
    }
    return false;
}
static bool has_landed(Object *object)
{
    for(int i = 0;i<platform_count;i++)
    {
        if(object->position.y+object->size.y>=platforms[i].position.y &&
            object->position.x+object->size.x > platforms[i].position.x &&
            object->position.x<platforms[i].position.x + platforms[i].size.x)
        {
            if(object->prev_position.y+object->size.y<platforms[i].position.y && object->position.y+object->size.y>=platforms[i].position.y)
            {
                object->position.y = platforms[i].position.y - object->size.y;
                object->velocity.y = 0;
                return true;
            }
        }
    }
    return false;
}

static float clamp(float move_speed, float min_vel, float max_vel)
{
    if (move_speed<min_vel) return min_vel;
    if (move_speed>max_vel) return max_vel;
    return move_speed;
}