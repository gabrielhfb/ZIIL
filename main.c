#include "raylib.h"
#include <stdio.h>
#include <math.h>

#define L 1
#define O 2
#define S 3
#define N 4
#define LARGURA_TELA 1200
#define ALTURA_TELA 800
#define QUADRADO 50
#define QUANT_PEDRAS 10
#define QUANT_MONSTROS 5
#define DELAY_ESPADA 0.25
#define COOLDOWN_ESPADA 2

#define PESSEGO \
    (Color) { 255, 204, 197, 255 }
#define MARROM \
    (Color) { 153, 78, 0, 255 }

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
    bool vivo;
} MONSTRO;

int posicao_pedras[QUANT_PEDRAS][2] = {{200, 200}, {400, 400}, {600, 600}, {700, 350}, {100, 600}, {1000, 400}, {300, 450}, {500, 700}, {850, 300}, {950, 100}};
int posicao_monstros[QUANT_MONSTROS][2] = {{250, 200}, {500, 200}, {750, 200}, {1000, 200}, {400, 600}};
int fase = 1;

Rectangle pedras[QUANT_PEDRAS];
MONSTRO monstros[QUANT_MONSTROS];

Texture2D espada_texturas[4];
Texture2D monstro_texturas[4];
Texture2D link_texturas[4];

bool MovimentoEhLegal(POSICAO *pos, Vector2 mov)
{
    // Dada a estrutura POSI��O (passada por refer�ncia) de uma entidade, e um Vector2 correspondente ao movimento a ser realizado,
    // retorna Verdadeiro se o movimento pode ser realizado ou Falso caso contr�rio

    // O movimento � permitido por padr�o,
    bool permitido = true;

    // exceto se a possivel coordenada de destino do movimento acabar fora da �rea jog�vel
    if (pos->atual.x + mov.x >= LARGURA_TELA || pos->atual.x + mov.x < 0 || pos->atual.y + mov.y >= ALTURA_TELA || pos->atual.y + mov.y < 0)
    {
        permitido = false;
    }
    else
    {
        for (int i = 0; i < QUANT_PEDRAS; i++)
        {
            if (CheckCollisionPointRec((Vector2){x : (pos->atual.x + mov.x) + 0.1, y : (pos->atual.y + mov.y) + 0.1}, pedras[i]))
                permitido = false;
        }
    }

    return permitido;
}

void AlteraPosicaoDestino(POSICAO *pos, int *direcao_atual, int direcao_movimento)
{
    // Dada uma POSICAO e a dire��o atual (passados por refer�ncia), bem como a dire��o desejada do movimento de uma entidade,
    // analisa a dire��o do movimento a ser realizado e passa para a fun��o de verifica��o o vetor desse movimento,
    // alterando a coordenada de destino e a dire��o atual se o movimento for legal

    // Se o personagem n�o est� com uma movimenta��o em andamento, ou seja, as posi��es atuais s�o iguais �s de destino
    if ((pos->atual.x == pos->destino.x) && (pos->atual.y == pos->destino.y))
    {
        *direcao_atual = direcao_movimento;
        if (direcao_movimento == L && MovimentoEhLegal(pos, (Vector2){x : QUADRADO, y : 0}))
        {
            pos->destino.x += QUADRADO;
        }
        else if (direcao_movimento == O && MovimentoEhLegal(pos, (Vector2){x : -QUADRADO, y : 0}))
        {
            pos->destino.x -= QUADRADO;
        }
        else if (direcao_movimento == S && MovimentoEhLegal(pos, (Vector2){x : 0, y : QUADRADO}))
        {
            pos->destino.y += QUADRADO;
        }
        else if (direcao_movimento == N && MovimentoEhLegal(pos, (Vector2){x : 0, y : -QUADRADO}))
        {
            pos->destino.y -= QUADRADO;
        }
    }
}

void TraduzInputJogador(POSICAO *pos, int *direcao_atual)
{
    // Dada a estrutura POSI��O e um caractere de dire��o atual (passados por refer�ncia) de um jogador, traduz o aperto de uma determinada tecla
    // para uma a��o do jogo

    // Verifica se alguma tecla de movimenta��o est� pressionada, chama a fun��o de altera��o de destino com os dados do jogador
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        AlteraPosicaoDestino(pos, direcao_atual, L);
    else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        AlteraPosicaoDestino(pos, direcao_atual, O);
    else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        AlteraPosicaoDestino(pos, direcao_atual, S);
    else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        AlteraPosicaoDestino(pos, direcao_atual, N);
}

void MovimentaEntidade(POSICAO *pos, int direcao, float velocidade, float delta)
{
    // Dada a estrutura POSI��O (passada por refer�ncia), um caractere de dire��o atual e um float de velocidade de uma entidade, bem como um
    // float para o delta time (tempo passado desde o �ltimo frame), movimenta essa entidade na tela em dire��o � sua posi��o de destino

    // Se uma das coordenadas da posi��o atual n�o corresponde com a respectiva coordenada da posi��o destino
    if ((pos->atual.x != pos->destino.x) || (pos->atual.y != pos->destino.y))
    {

        // Se a dire��o atual � "X", executa um movimento de 'velocidade' pixels por segundo nessa dire��o at� alcan�ar a posi��o destino
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

void AtaqueEspada(double *cooldownEspada, bool *espada, int direcao, POSICAO pos, float delta, int *quant_monstros)
{
    int posX = pos.atual.x;
    int posY = pos.atual.y;

    if (IsKeyPressed(KEY_SPACE) || *espada == true)
    {
        if (*cooldownEspada <= 0)
        {
            if (direcao == 1)
                posX += 50;
            else if (direcao == 2)
                posX -= 50;
            else if (direcao == 3)
                posY += 50;
            else
                posY -= 50;

            Rectangle espada_hitbox = (Rectangle){posX, posY, 50, 50};
            DrawTexture(espada_texturas[direcao - 1], posX, posY, WHITE);

            for (int i = 0; i < QUANT_MONSTROS; i++)
                if (monstros[i].vivo == true)
                    if (CheckCollisionRecs(espada_hitbox, (Rectangle){monstros[i].pos.atual.x, monstros[i].pos.atual.y, 50, 50}))
                    {
                        monstros[i].vivo = false;
                        *quant_monstros -= 1;
                    }
            if (*quant_monstros == 0)
            {
                fase += 1;
                jogo();
            }

            *espada = true;
        }

        *cooldownEspada -= delta;
    }

    if (*cooldownEspada <= -DELAY_ESPADA)
    {
        *cooldownEspada = COOLDOWN_ESPADA;
        *espada = false;
    }
}

void MonstroSegueJogador(POSICAO *monstro_pos, int *monstro_direcao, POSICAO link_pos, float delta)
{
    int direcaoLink = 0;
    Vector2 newMov = (Vector2){x : 0, y : 0};

    if (abs(monstro_pos->atual.x - link_pos.atual.x) > abs(monstro_pos->atual.y - link_pos.atual.y))
    {
        if (monstro_pos->atual.x > link_pos.atual.x)
        {
            direcaoLink = O;
            newMov.x = -50;
        }
        else if (monstro_pos->atual.x < link_pos.atual.x)
        {
            direcaoLink = L;
            newMov.x = 50;
        }
    }
    else
    {
        if (monstro_pos->atual.y > link_pos.atual.y)
        {
            direcaoLink = N;
            newMov.y = -50;
        }
        else if (monstro_pos->atual.y < link_pos.atual.y)
        {
            direcaoLink = S;
            newMov.y = 50;
        }
    }

    if (MovimentoEhLegal(monstro_pos, newMov))
    {
        AlteraPosicaoDestino(monstro_pos, monstro_direcao, direcaoLink);
    }
    else
    {
        AlteraPosicaoDestino(monstro_pos, monstro_direcao, GetRandomValue(1, 4));
    }
}

void jogo()
{
    int i;
    double cooldownEspada = 0.0, cooldownColisao = 0.0;
    int quant_monstros = QUANT_MONSTROS;

    espada_texturas[0] = LoadTexture("sprites/Attack_right.png");
    espada_texturas[1] = LoadTexture("sprites/Attack_left.png");
    espada_texturas[2] = LoadTexture("sprites/Attack_down.png");
    espada_texturas[3] = LoadTexture("sprites/Attack_up.png");

    monstro_texturas[0] = LoadTexture("sprites/Enemy_right.png");
    monstro_texturas[1] = LoadTexture("sprites/Enemy_left.png");
    monstro_texturas[2] = LoadTexture("sprites/Enemy_front.png");
    monstro_texturas[3] = LoadTexture("sprites/Enemy_back.png");

    link_texturas[0] = LoadTexture("sprites/Link_right.png");
    link_texturas[1] = LoadTexture("sprites/Link_left.png");
    link_texturas[2] = LoadTexture("sprites/Link_front.png");
    link_texturas[3] = LoadTexture("sprites/Link_back.png");

    Texture2D pedra = LoadTexture("sprites/Obstacle.png");

    JOGADOR link = (JOGADOR){
        pos :
            {
                atual : (Vector2){x : 0, y : 0},
                destino : (Vector2){x : 0, y : 0}
            },
        velocidade : 350.0,
        direcao : S,
        nro_vidas : 3,
        espada : false
    };

    for (int i = 0; i < QUANT_PEDRAS; i++)
    {
        pedras[i] = (Rectangle){
            x : posicao_pedras[i][0],
            y : posicao_pedras[i][1],
            width : 50,
            height : 50
        };
    }

    for (i = 0; i < QUANT_MONSTROS; i++)
    {
        monstros[i] = (MONSTRO){
            pos :
                {
                    atual : (Vector2){x : posicao_monstros[i][0], y : posicao_monstros[i][1]},
                    destino : (Vector2){x : posicao_monstros[i][0], y : posicao_monstros[i][1]}
                },
            velocidade : 300.0,
            direcao : S,
            vivo : true,
        };
    }
    // Main game loop
    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();

        BeginDrawing();
        ClearBackground((Color){253, 217, 169, 255});

        TraduzInputJogador(&link.pos, &link.direcao);
        MovimentaEntidade(&link.pos, link.direcao, link.velocidade, delta);
        AtaqueEspada(&cooldownEspada, &link.espada, link.direcao, link.pos, delta, &quant_monstros);
        DrawTexture(link_texturas[link.direcao - 1], link.pos.atual.x, link.pos.atual.y, WHITE);

        Rectangle ret_link = (Rectangle){
            x : link.pos.atual.x,
            y : link.pos.atual.y,
            width : QUADRADO,
            height : QUADRADO
        };

        if (cooldownEspada > 0)
            cooldownEspada -= delta;

        for (i = 0; i < QUANT_MONSTROS; i++)
        {
            if (monstros[i].vivo)
            {
                if (
                    abs(monstros[i].pos.atual.x - link.pos.atual.x) < 800 &&
                    abs(monstros[i].pos.atual.y - link.pos.atual.y) < 800 && cooldownColisao <= 0)
                {
                    MonstroSegueJogador(&monstros[i].pos, &monstros[i].direcao, link.pos, delta);
                }
                else
                {
                    AlteraPosicaoDestino(&monstros[i].pos, &monstros[i].direcao, GetRandomValue(1, 4));
                }

                MovimentaEntidade(&monstros[i].pos, monstros[i].direcao, monstros[i].velocidade, delta);
                DrawTexture(monstro_texturas[monstros[i].direcao - 1], monstros[i].pos.atual.x, monstros[i].pos.atual.y, WHITE);

                if (CheckCollisionRecs(ret_link, (Rectangle){monstros[i].pos.atual.x, monstros[i].pos.atual.y, QUADRADO, QUADRADO}) && cooldownColisao <= 0)
                {
                    link.nro_vidas--;
                    cooldownColisao = 15;

                    DrawText("Colis�o", 400, 550, 30, PURPLE);
                }
                else if (cooldownColisao > 0)
                    cooldownColisao -= delta;
            }
        }

        DrawTexture(link_texturas[link.direcao - 1], link.pos.atual.x, link.pos.atual.y, WHITE);

        for (int i = 0; i < QUANT_PEDRAS; i++)
        {
            DrawTexture(pedra, pedras[i].x, pedras[i].y, WHITE);
        }

        DrawText(TextFormat("Vidas: %d", link.nro_vidas), 100, 50, 20, VIOLET);
        DrawText(TextFormat("FASE: %d", fase), 100, 70, 20, VIOLET);

        if (IsKeyDown(KEY_ENTER))
        {
            for (int i = 0; i < QUANT_PEDRAS; i++)
            {
                pedras[i] = (Rectangle){
                    x : GetRandomValue(0, (LARGURA_TELA - 50) / 50) * 50,
                    y : GetRandomValue(0, (ALTURA_TELA - 50) / 50) * 50,
                    width : 50,
                    height : 50
                };
            }
        }

        EndDrawing();
    }

    CloseWindow();
}

int menu(void)
{
    const int screenWidth = 1200;
    const int screenHeight = 800;
    int escolha = 0, altura_opcao = 40;

    Rectangle retJOGAR = {80, 210, 165, altura_opcao};
    Rectangle retCARREGAR = {80, 310, 275, altura_opcao};
    Rectangle retRECORDES = {80, 410, 275, altura_opcao};
    Rectangle retSAIR = {80, 510, 120, altura_opcao};

    InitWindow(screenWidth, screenHeight, "ZIIL");
    Texture2D hub = LoadTexture("image.png");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(PESSEGO);

        DrawTexture(hub, 0, 0, RAYWHITE);

        if (CheckCollisionPointRec(GetMousePosition(), retJOGAR))
        {
            DrawText("JOGAR", 80, 208, 50, GREEN);
            if (IsMouseButtonPressed(0))
            {
                escolha = 1;
                return escolha;
            }
        }
        else
            DrawText("JOGAR", 80, 208, 50, MARROM);

        if (CheckCollisionPointRec(GetMousePosition(), retCARREGAR))
        {
            DrawText("CARREGAR", 80, 308, 50, GREEN);
            if (IsMouseButtonPressed(0))
            {
                escolha = 2;
                return escolha;
            }
        }
        else
            DrawText("CARREGAR", 80, 308, 50, MARROM);

        if (CheckCollisionPointRec(GetMousePosition(), retRECORDES))
        {
            DrawText("RECORDES", 80, 408, 50, GREEN);
            if (IsMouseButtonPressed(0))
            {
                escolha = 3;
                return escolha;
            }
        }
        else
            DrawText("RECORDES", 80, 408, 50, MARROM);

        if (CheckCollisionPointRec(GetMousePosition(), retSAIR))
        {
            DrawText("SAIR", 80, 508, 50, GREEN);
            if (IsMouseButtonPressed(0))
            {
                escolha = 4;
                return escolha;
            }
        }
        else
            DrawText("SAIR", 80, 508, 50, MARROM);

        DrawText("Made by Grilli & Brandeburski", 640, 492, 29, BLACK);

        EndDrawing();
    }
    CloseWindow();

    return escolha;
}

int main()
{
    int escolha = 0;

    escolha = menu();
    if (escolha == 1)
    {
        jogo();
    }
    if (escolha == 4)
    {
        CloseWindow();
    }
}
