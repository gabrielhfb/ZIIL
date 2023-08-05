#include "raylib.h"
#include <stdio.h>

#define L 1
#define O 2
#define S 3
#define N 4

#define LARGURA_TELA 1200
#define ALTURA_TELA 800

typedef struct posicao
{
    Vector2 atual;
    Vector2 destino;
} POSICAO;

typedef struct jogador
{
    POSICAO pos;
    int direcao;
    float velocidade;
    int nro_vidas;
    int pontos;
    bool espada;
} JOGADOR;

typedef struct monstro
{
    POSICAO pos;
    int direcao;
    float velocidade;
} MONSTRO;

Rectangle g_pedra = (Rectangle){
            x: 200,
            y: 200,
            width: 50,
            height: 50
        };

bool MovimentoEhLegal(POSICAO *pos, Vector2 mov)
{
    // Dada a estrutura POSIÇÃO (passada por referência) de uma entidade, e um Vector2 correspondente ao movimento a ser realizado,
    // retorna Verdadeiro se o movimento pode ser realizado ou Falso caso contrário

    // O movimento é permitido por padrão,
    bool permitido = true;

    // exceto se a possivel coordenada de destino do movimento acabar fora da área jogável
    if (pos->atual.x + mov.x >= LARGURA_TELA || pos->atual.x + mov.x < 0 || pos->atual.y + mov.y >= ALTURA_TELA || pos->atual.y + mov.y < 0)
        permitido = false;

    if (CheckCollisionPointRec((Vector2){x:(pos->atual.x + mov.x)+0.1, y: (pos->atual.y + mov.y)+0.1}, g_pedra))
        permitido = false;

    return permitido;
}

void AlteraPosicaoDestino(POSICAO *pos, int *direcao_atual, int direcao_movimento)
{
    // Dada uma POSICAO e a direção atual (passados por referência), bem como a direção desejada do movimento de uma entidade,
    // analisa a direção do movimento a ser realizado e passa para a função de verificação o vetor desse movimento,
    // alterando a coordenada de destino e a direção atual se o movimento for legal

    // Se o personagem não está com uma movimentação em andamento, ou seja, as posições atuais são iguais às de destino
    if ((pos->atual.x == pos->destino.x) && (pos->atual.y == pos->destino.y))
    {
        if (direcao_movimento == L && MovimentoEhLegal(pos, (Vector2){x : 50, y : 0}))
        {
            pos->destino.x += 50;
            *direcao_atual = L;
        }
        else if (direcao_movimento == O && MovimentoEhLegal(pos, (Vector2){x : -50, y : 0}))
        {
            pos->destino.x -= 50;
            *direcao_atual = O;
        }
        else if (direcao_movimento == S && MovimentoEhLegal(pos, (Vector2){x : 0, y : 50}))
        {
            pos->destino.y += 50;
            *direcao_atual = S;
        }
        else if (direcao_movimento == N && MovimentoEhLegal(pos, (Vector2){x : 0, y : -50}))
        {
            pos->destino.y -= 50;
            *direcao_atual = N;
        }
    }
}

void TraduzInputJogador(POSICAO *pos, int *direcao_atual)
{
    // Dada a estrutura POSIÇÃO e um caractere de direção atual (passados por referência) de um jogador, traduz o aperto de uma determinada tecla
    // para uma ação do jogo

    // Verifica se alguma tecla de movimentação está pressionada, chama a função de alteração de destino com os dados do jogador
    if (IsKeyDown(KEY_RIGHT))
        AlteraPosicaoDestino(pos, direcao_atual, L);

    else if (IsKeyDown(KEY_LEFT))
        AlteraPosicaoDestino(pos, direcao_atual, O);

    else if (IsKeyDown(KEY_DOWN))
        AlteraPosicaoDestino(pos, direcao_atual, S);

    else if (IsKeyDown(KEY_UP))
        AlteraPosicaoDestino(pos, direcao_atual, N);
}

void MovimentaEntidade(POSICAO *pos, int direcao, float velocidade, float delta)
{
    // Dada a estrutura POSIÇÂO (passada por referência), um caractere de direção atual e um float de velocidade de uma entidade, bem como um
    // float para o delta time (tempo passado desde o último frame), movimenta essa entidade na tela em direção à sua posição de destino

    // Se uma das coordenadas da posição atual não corresponde com a respectiva coordenada da posição destino
    if ((pos->atual.x != pos->destino.x) || (pos->atual.y != pos->destino.y))
    {

        // Se a direção atual é "X", executa um movimento de 'velocidade' pixels por segundo nessa direção até alcançar a posição destino
        if (direcao == L)
        {
            pos->atual.x += velocidade * delta;
            if (pos->atual.x >= pos->destino.x)
                pos->atual.x = pos->destino.x;
        }

        else if (direcao == O)
        {
            pos->atual.x -= velocidade * delta;
            if (pos->atual.x <= pos->destino.x)
                pos->atual.x = pos->destino.x;
        }

        else if (direcao == S)
        {
            pos->atual.y += velocidade * delta;
            if (pos->atual.y >= pos->destino.y)
                pos->atual.y = pos->destino.y;
        }

        else if (direcao == N)
        {
            pos->atual.y -= velocidade * delta;
            if (pos->atual.y <= pos->destino.y)
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

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    JOGADOR link = (JOGADOR){
        pos :
            {
                atual : (Vector2){x : 0, y : 0},
                destino : (Vector2){x : 0, y : 0}
            },
        velocidade : 175.0,
        direcao : S,
        nro_vidas : 3
    };

    MONSTRO monstro1 = (MONSTRO)
    {
    pos :
            {
                atual : (Vector2){x : 250, y : 200},
                destino : (Vector2){x : 250, y : 200}
            },
        velocidade : 80.0,
        direcao : S,
    };

    // Main game loop
    while (!WindowShouldClose())
    {

        float delta = GetFrameTime();

        BeginDrawing();

        if (IsKeyPressed(KEY_F11))
        {
            ToggleFullscreen();
        }

        ClearBackground(RAYWHITE);

        TraduzInputJogador(&link.pos, &link.direcao);
        MovimentaEntidade(&link.pos, link.direcao, link.velocidade, delta);

        AlteraPosicaoDestino(&monstro1.pos, &monstro1.direcao, GetRandomValue(1,4));
        MovimentaEntidade(&monstro1.pos, monstro1.direcao, monstro1.velocidade, delta);

        Rectangle ret_link = (Rectangle){
            x: link.pos.atual.x,
            y: link.pos.atual.y,
            width: 50,
            height: 50
        };

        Rectangle ret_monstroi = (Rectangle){
            x: monstro1.pos.atual.x,
            y: monstro1.pos.atual.y,
            width: 50,
            height: 50
        };


        DrawRectangleRec(ret_link, GREEN);
        DrawRectangleRec(ret_monstroi, PURPLE);
        DrawRectangleRec(g_pedra, BROWN);

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

        if (IsKeyDown(KEY_RIGHT))
            DrawText(">>", 450, 500, 30, ORANGE);
        if (IsKeyDown(KEY_LEFT))
            DrawText("<<", 350, 500, 30, ORANGE);
        if (IsKeyDown(KEY_UP))
            DrawText("^^", 400, 450, 40, ORANGE);
        if (IsKeyDown(KEY_DOWN))
            DrawText("vv", 400, 500, 30, ORANGE);

        if (CheckCollisionRecs(ret_link, ret_monstroi))
            DrawText("Colisão", 400, 550, 30, PURPLE);

        EndDrawing();

        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}