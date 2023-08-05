#include "raylib.h"
#include <stdio.h>

#define LARGURA_TELA 800
#define ALTURA_TELA 600


typedef struct posicao
{
    Vector2 atual;
    Vector2 destino;
} POSICAO;


typedef struct jogador
    {
        POSICAO pos;
        char direcao;
        float velocidade;
        int nro_vidas;
    } JOGADOR;


bool MovimentoEhLegal(POSICAO *pos, Vector2 mov){
    // Dada a estrutura POSIÇÃO (passada por referência) de uma entidade, e um Vector2 correspondente ao movimento a ser realizado, 
    // retorna Verdadeiro se o movimento pode ser realizado ou Falso caso contrário

    // O movimento é permitido por padrão,
    bool permitido = true;

    // exceto se a possivel coordenada de destino do movimento acabar fora da área jogável 
    if (pos->atual.x + mov.x >= LARGURA_TELA || pos->atual.x + mov.x < 0 || pos->atual.y + mov.y >= ALTURA_TELA || pos->atual.y + mov.y < 0 )
        permitido = false;
    
    return permitido;
}

void TraduzInputPlayer(POSICAO *pos, char *direcao){
    // Dada a estrutura POSIÇÃO e um caractere de direção atual (passados por referência) de um jogador, traduz o aperto de uma determinada tecla  
    // para uma ação do jogo
        
    // Se o personagem não está com uma movimentação em andamento, ou seja, as posições atuais são iguais às de destino
    if((pos->atual.x == pos->destino.x) && (pos->atual.y == pos->destino.y)){

        // Verifica se alguma tecla de movimentação está pressionada, passa para a função de verificação o vetor do movimento a ser realizado,
        // e altera a coordenada da posição destino correspondente se o movimento for legal
        if(IsKeyDown(KEY_RIGHT) && MovimentoEhLegal(pos, (Vector2){x: 50, y: 0})){
            pos->destino.x += 50;
            *direcao = 'D';
        }
        else if(IsKeyDown(KEY_LEFT) && MovimentoEhLegal(pos, (Vector2){x: -50, y: 0})){
            pos->destino.x -= 50;
            *direcao = 'E';
        }
        else if(IsKeyDown(KEY_DOWN) && MovimentoEhLegal(pos, (Vector2){x: 0, y: 50})){
            pos->destino.y += 50;
            *direcao = 'B';
        }
        else if(IsKeyDown(KEY_UP) && MovimentoEhLegal(pos, (Vector2){x: 0, y: -50})){
            pos->destino.y -= 50;
            *direcao = 'C';
        }
    }
        
    
 
}

void MovimentaEntidade(POSICAO *pos, char direcao, float velocidade, float delta){
    // Dada a estrutura POSIÇÂO (passada por referência), um caractere de direção atual e um float de velocidade de uma entidade, bem como um 
    // float para o delta time (tempo passado desde o último frame), movimenta essa entidade na tela em direção à sua posição de destino

    // Se uma das coordenadas da posição atual não corresponde com a respectiva coordenada da posição destino
    if((pos->atual.x != pos->destino.x) || (pos->atual.y != pos->destino.y)){

        // Se a direção atual é "X", executa um movimento de 'velocidade' pixels por segundo nessa direção até alcançar a posição destino
        if(direcao == 'D'){
            pos->atual.x += velocidade * delta; 
            if(pos->atual.x >= pos->destino.x)
                pos->atual.x = pos->destino.x;
        }

        else if(direcao == 'E'){
            pos->atual.x -= velocidade * delta;
            if(pos->atual.x <= pos->destino.x)
                pos->atual.x = pos->destino.x;
        }

        else if(direcao == 'B'){
            pos->atual.y += velocidade * delta;
            if(pos->atual.y >= pos->destino.y)
                pos->atual.y = pos->destino.y;
        }

        else if(direcao == 'C'){
            pos->atual.y -= velocidade * delta;
            if(pos->atual.y <= pos->destino.y)
                pos->atual.y = pos->destino.y;
        }
    }

}


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(LARGURA_TELA, ALTURA_TELA, "ZIIL");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    JOGADOR link = (JOGADOR)
    {
        pos: 
        {
            atual: (Vector2){x: 0, y: 0}, 
            destino: (Vector2){x:0, y: 0}
        },
        velocidade: 175.0,
        direcao: 'B',
        nro_vidas: 3
    };

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key

    
    {
        float delta = GetFrameTime();
        

        BeginDrawing();

        ClearBackground(RAYWHITE);

        TraduzInputPlayer(&link.pos, &link.direcao);
        MovimentaEntidade(&link.pos, link.direcao, link.velocidade, delta);


        DrawRectangle(link.pos.atual.x, link.pos.atual.y, 50, 50, RED);

        char xat[10];
        char yat[10];
        char xdes[10];
        char ydes[10];
        sprintf(xat, "%0.5lf", link.pos.atual.x);
        sprintf(yat, "%0.5lf", link.pos.atual.y);
        sprintf(xdes, "%0.5lf", link.pos.destino.x);
        sprintf(ydes, "%0.5lf", link.pos.destino.y);

        DrawText(xat, 100, 100, 20, BLUE);
        DrawText(yat, 300, 100, 20, BLUE);

        DrawText(xdes, 100, 200, 20, GREEN);
        DrawText(ydes, 300, 200, 20, GREEN);


        if(IsKeyDown(KEY_RIGHT))
            DrawText(">>", 450, 500, 30, ORANGE);
        if(IsKeyDown(KEY_LEFT))
            DrawText("<<", 350, 500, 30, ORANGE);
        if(IsKeyDown(KEY_UP))
            DrawText("^^", 400, 450, 40, ORANGE);
        if(IsKeyDown(KEY_DOWN))
            DrawText("vv", 400, 500, 30, ORANGE);


        EndDrawing();

        

        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}