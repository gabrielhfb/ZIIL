#include "raylib.h"
#include <stdio.h>

#define L 1
#define O 2
#define S 3
#define N 4

#define LARGURA_TELA 1200
#define ALTURA_TELA 800
#define QUADRADO 50
#define n_monstros 5
#define PESSEGO \
    (Color) { 255, 204, 197, 255 }
#define MARROM \
    (Color) { 153, 78, 0, 255 }
#define DELAY_ESPADA 0.25
#define COOLDOWN_ESPADA 2

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

int cord_pedras[20] = {200, 200, 400, 400, 600, 600, 700, 350, 100, 600, 1000, 400, 300, 450, 500, 700, 850, 300, 950, 100};
MONSTRO monstros[n_monstros];
Texture2D espada_texturas[4];
Texture2D monstro_texturas[4];
Texture2D link_texturas[4];

bool MovimentoEhLegal(POSICAO *pos, Vector2 mov)
{
    // Dada a estrutura POSIÇÃO (passada por referência) de uma entidade, e um Vector2 correspondente ao movimento a ser realizado,
    // retorna Verdadeiro se o movimento pode ser realizado ou Falso caso contrário

    // O movimento é permitido por padrão,
    bool permitido = true;
    int i;

    // exceto se a possivel coordenada de destino do movimento acabar fora da área jogável
    if (pos->atual.x + mov.x >= LARGURA_TELA || pos->atual.x + mov.x < 0 || pos->atual.y + mov.y >= ALTURA_TELA || pos->atual.y + mov.y < 0)
        permitido = false;

    for (i = 0; i < 20; i += 2)
    {
        if (CheckCollisionPointRec((Vector2){x : (pos->atual.x + mov.x) + 0.1, y : (pos->atual.y + mov.y) + 0.1}, (Rectangle){x : cord_pedras[i],
                                                                                                                              y : cord_pedras[i + 1],
                                                                                                                              width : QUADRADO,
                                                                                                                              height : QUADRADO}))

            permitido = false;
        if (permitido == false)
            break;
    }

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

void AtaqueEspada(double *cooldownEspada, bool *espada, int direcao, POSICAO pos, float delta)
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

            for (int i = 0; i < n_monstros; i++)
                if (CheckCollisionRecs(espada_hitbox, (Rectangle){monstros[i].pos.atual.x, monstros[i].pos.atual.y, 50, 50}))
                    monstros[i].vivo = false;

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

void jogo(void)
{
    int i;
    double cooldownEspada = 0.0, cooldownColisao = 0.0;
    int posicao_monstros[n_monstros][2] = {{250, 200}, {500, 200}, {750, 200}, {1000, 200}, {400, 600}};
    // Array com x e y dos monstros
    Rectangle ret_monstros[n_monstros];

    InitWindow(LARGURA_TELA, ALTURA_TELA, "ZIIL");

    SetTargetFPS(60);

    JOGADOR link = (JOGADOR){
        pos :
            {
                atual : (Vector2){x : 0, y : 0},
                destino : (Vector2){x : 0, y : 0}
            },
        velocidade : 375.0,
        direcao : S,
        nro_vidas : 3,
        espada : false
    };

    for (i = 0; i < n_monstros; i++)
    {
        monstros[i] = (MONSTRO){
            pos :
                {
                    atual : (Vector2){x : posicao_monstros[i][0], y : posicao_monstros[i][1]},
                    destino : (Vector2){x : posicao_monstros[i][0], y : posicao_monstros[i][1]}
                },
            velocidade : 200,
            direcao : S,
            vivo : true,
        };
    }
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

    // Main game loop
    while (!WindowShouldClose())
    {

        float delta = GetFrameTime();

        BeginDrawing();

        if (IsKeyPressed(KEY_F11))
        {
            ToggleFullscreen();
        }

        ClearBackground((Color){253, 217, 169, 255});

        TraduzInputJogador(&link.pos, &link.direcao);
        MovimentaEntidade(&link.pos, link.direcao, link.velocidade, delta);
        AtaqueEspada(&cooldownEspada, &link.espada, link.direcao, link.pos, delta);
        DrawTexture(link_texturas[link.direcao - 1], link.pos.atual.x, link.pos.atual.y, WHITE);

        Rectangle ret_link = (Rectangle){
            x : link.pos.atual.x,
            y : link.pos.atual.y,
            width : QUADRADO,
            height : QUADRADO
        };

        if (cooldownEspada > 0)
            cooldownEspada -= delta;

        for (i = 0; i < n_monstros; i++)
        {
            if (monstros[i].vivo)
            {
                if (CheckCollisionRecs(ret_link, ret_monstros[i]))
                    DrawText("Colisão", 400, 550, 30, PURPLE);

                AlteraPosicaoDestino(&monstros[i].pos, &monstros[i].direcao, GetRandomValue(1, 4));
                MovimentaEntidade(&monstros[i].pos, monstros[i].direcao, monstros[i].velocidade, delta);

                ret_monstros[i] = (Rectangle){
                    x : monstros[i].pos.atual.x,
                    y : monstros[i].pos.atual.y,
                    width : QUADRADO,
                    height : QUADRADO
                };

                DrawTexture(monstro_texturas[monstros[i].direcao - 1], monstros[i].pos.atual.x, monstros[i].pos.atual.y, WHITE);
                if (CheckCollisionRecs(ret_link, ret_monstros[i]) && cooldownColisao <= 0)
                {
                    link.nro_vidas--;
                    cooldownColisao = 5;
                }
                else if (cooldownColisao > 0)
                    cooldownColisao -= delta;
            }
        }

        DrawTexture(link_texturas[link.direcao - 1], link.pos.atual.x, link.pos.atual.y, WHITE);

        for (i = 0; i < 20; i += 2)
        {
            DrawTexture(pedra, cord_pedras[i], cord_pedras[i + 1], WHITE);
        }

        char xat[10];
        char yat[10];
        char xdes[10];
        char ydes[10];
        sprintf(xat, "%0.5lf", link.pos.atual.x);
        sprintf(yat, "%0.5lf", link.pos.atual.y);
        sprintf(xdes, "%0.5lf", link.pos.destino.x);
        sprintf(ydes, "%0.5lf", link.pos.destino.y);
        DrawText(TextFormat("Vidas: %d", link.nro_vidas), 100, 50, 20, RED);

        EndDrawing();
    }

    CloseWindow();
}

int menu(void)
{

    const int screenWidth = 1200;
    const int screenHeight = 800;
    int escolha = 0, largura_opcao = 320, altura_opcao = 40;
    Rectangle retJOGAR = {80, 210, 165, altura_opcao};
    Rectangle retCARREGAR = {80, 310, 275, altura_opcao};
    Rectangle retRECORDES = {80, 410, 275, altura_opcao};
    Rectangle retSAIR = {80, 510, 120, altura_opcao};

    InitWindow(screenWidth, screenHeight, "ZIIL");
    Texture2D hub = LoadTexture("C:/Users/Pichau/OneDrive/Documentos/C C++/TRABALHOFINAL/image.png");

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
