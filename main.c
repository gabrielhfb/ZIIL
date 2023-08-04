#include "raylib.h"

#define FPS 60
#define VELOC 90
#define V VELOC/FPS

typedef struct posicao
{
    Vector2 atual;
    Vector2 destino;
} POSICAO;


typedef struct jogador
    {
        POSICAO pos;
        int nro_vidas;
    } JOGADOR;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 600;

    JOGADOR link = {{0, 0}, {0, 0}, 3};

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second



    
    

    //--------------------------------------------------------------------------------------
    float currentX = 0, targetX = 0;
    

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {

        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawRectangle(currentX, 0, 50, 50, RED);

        EndDrawing();

        

        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}