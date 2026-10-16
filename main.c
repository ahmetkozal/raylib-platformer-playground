#include "raylib.h"
#include <math.h>
#include <stdlib.h>

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define PLAYER_MAX_LIFE         4
#define MAX_PLATFORMS           20
#define MAX_ENEMIES             20
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
    float damage_timer;

    int life;

    bool can_get_damage;
    bool on_ground;
    bool dead;
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

} Player;

typedef struct Enemy
{
    float move_speed;
    int direction;

    Object object;

    Color color;
}Enemy;

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

static Enemy enemies[MAX_ENEMIES];
static int enemy_count = 0;
static float enemy_size = 30;

static Platform platforms[MAX_PLATFORMS];
static int platform_count = 0;
static float platform_main_friction = 3000.0f;

static float ground_y = 420;
static float platform_width = 100;
static float platform_height = 20;

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
static int check_enemy_touching(Object *object);

static void AddPlatform(float x, float y, float width, float height,Color color, float friction, float coef_max_vel);
static void AddEnemy(float x, float y, float width, float height, Color color);
static void enemy_move(float dt);

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
    platform_count = 0;
    enemy_count = 0;
    
    //PLATFORM DRAWING

    //LEVEL PLATFORMS
    AddPlatform(0,screenHeight-platform_height,screenWidth,platform_height,YELLOW,platform_main_friction,1.0f);
    AddPlatform(0,0,platform_height,screenWidth,YELLOW,platform_main_friction,1.0f);
    AddPlatform(screenWidth-platform_height,0,platform_height,screenWidth,YELLOW,platform_main_friction,1.0f);

    //AddPlatform(80,380,platform_width,platform_height,BLACK,platform_main_friction,1.0f);
    AddPlatform(80+60*0+(platform_width*0),360,platform_width,platform_height,BLACK,platform_main_friction,1.0f);
    AddPlatform(80+60*1+(platform_width*1),320,platform_width,platform_height,BLACK,platform_main_friction,1.0f);
    AddPlatform(80+60*2+(platform_width*2),300,platform_width,platform_height,BLACK,platform_main_friction,1.0f);
    AddPlatform(80+60*3+(platform_width*3),320,platform_width,platform_height,BLACK,platform_main_friction,1.0f);

    //player setup
    player.object.position = (Vector2){20,0};
    player.object.size = (Vector2){20,20};
    player.object.life = PLAYER_MAX_LIFE;
    player.move_speed = 8000.0f;
    player.jump_power = 400.0f;
    player.default_max_vel = 250.0f;
    player.max_velocity = player.default_max_vel;
    player.object.friction = player.move_speed /2;
    player.object.can_get_damage = true;
    player.object.damage_timer = 0.0f;

    //enemies setup
    

    AddEnemy(80,screenHeight-platform_height-enemy_size,enemy_size,enemy_size,PURPLE);
    enemies[0].direction = 1;
    enemies[0].move_speed = 300.0f;
    enemies[0].object.dead = false;

    AddEnemy(80,screenHeight-platform_height-enemy_size,enemy_size,enemy_size,BROWN);
    enemies[1].direction = 1;
    enemies[1].move_speed = 350.0f;
    enemies[1].object.dead = false;

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

static void AddEnemy(float x, float y, float width, float height, Color color)
{
    if(enemy_count<MAX_ENEMIES)
    {
        enemies[enemy_count++] = (Enemy)
        {
            .object.position = {x,y},
            .object.size = {width,height},
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
        for(int i = 0; i < enemy_count; i++)
        {
            //if(enemies[i].object.position.y > screenHeight+100000)
                DrawRectangleV(enemies[i].object.position,enemies[i].object.size,enemies[i].color);
        }
        
        DrawRectangleV(player.object.position,player.object.size,RED);
        
        for(int i = 0; i<player.object.life;i++)
        {
            DrawCircle(20+i*20,20,10,GREEN);
        }
        if(game_over)
        {
            DrawText("GAME OVER!",screenWidth/2,screenHeight/2,40,RED);
        }
        

    EndDrawing();

}

// Update game (one frame)
static void UpdateGame(void)
{
    float dt = GetFrameTime();
    if(!game_over)
    {
        player_move(dt);
        if(player.object.damage_timer>0)
            player.object.damage_timer-=dt;
        if(player.object.damage_timer <= 0)
        {
            player.object.can_get_damage = true;
        }
        if(player.object.life <= 0)
        {
            game_over = true;
            
        }
        enemy_move(dt);
    }
    else
    {
        
        if(IsKeyPressed(KEY_SPACE))
        {
            game_over = false;
            InitGame();

        }
    }
}

// Update and Draw (one frame)
static void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}

static void enemy_move(float dt)
{
    for(int i = 0; i < enemy_count; i++)
    {
        Enemy *enemy = &enemies[i];
        enemy->object.prev_position = enemy->object.position;

        if(!enemy->object.dead)
        {
            enemy->object.velocity.x = enemy->move_speed * enemy->direction;
            enemy->object.position.x += enemy->object.velocity.x *dt;

            if (touching_which_platform(&enemy->object) > -1)
            {
                enemy->direction *= -1;
            }
        }
        else
        {
            enemies[i].object.size.y = enemy_size/2;
            enemies[i].object.position.y += 100.0f * dt;
        }

    }
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
    check_enemy_touching(&player.object);

    

    
    
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

        if ((object_right >= platform_left && object_left <= platform_left && object_bottom > platform_up && object_up < platform_bottom)||
            (object_left <= platform_right && object_right >= platform_right && object_bottom > platform_up && object_up < platform_bottom))
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

static int check_enemy_touching(Object *object)
{
    float upper,lower,left,right,prev_upper,prev_lower,prev_left,prev_right;
    float enemy_upper,enemy_lower,enemy_left,enemy_right;
    for(int i = 0; i<enemy_count;i++)
    {
        upper = object->position.y;
        lower = upper + object->size.y;
        left = object->position.x;
        right = left + object->size.x;

        prev_upper = object->prev_position.y;
        prev_lower = prev_upper + object->size.y;
        prev_left = object->prev_position.x;
        prev_right = prev_left + object->size.x;

        Enemy enemy = enemies[i];

        enemy_upper = enemy.object.position.y;
        enemy_lower = enemy_upper + enemy.object.size.y;
        enemy_left = enemy.object.position.x;
        enemy_right = enemy_left + enemy.object.size.x;

        if(right>=enemy_left && left <= enemy_left && upper<enemy_lower && lower > enemy_upper ||
            left <= enemy_right && right >= enemy_right && upper<enemy_lower && lower > enemy_upper)
        {  
            if(prev_lower <= enemy_upper)
            {
                object->position.y = enemy_upper - object->size.y;
                object->velocity.y = 0.0f;
                object->velocity.y -= 300.0f;
                
                enemies[i].object.dead = true;
            }
            else{
                if(object->can_get_damage && !enemy.object.dead)
                {
                    object->life--;
                    object->can_get_damage = false;
                    object->damage_timer = 1.0f;
                }

            }
        return i;
        }
    }
    return -1;
}