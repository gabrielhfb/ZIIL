#include <math.h>
#include <stdio.h>

#include "raylib.h"

#define L 1
#define O 2
#define S 3
#define N 4
#define LARGURA_TELA 1200
#define ALTURA_TELA 800
#define ALTURA_BOTAO 40
#define QUADRADO 50
#define QUANT_MAX_OBSTACULOS 10
#define QUANT_MONSTROS 5
#define DELAY_ESPADA 0.25
#define COOLDOWN_ESPADA 2
#define VELOCIDADE_MONSTRO 0
#define LINHAS 16
#define COLUNAS 24

#define PESSEGO \
  (Color) { 255, 204, 197, 255 }
#define MARROM \
  (Color) { 153, 78, 0, 255 }

typedef struct posicao {
  Vector2 atual;
  Vector2 destino;
} POSICAO;

typedef struct jogador {
  POSICAO pos;
  int direcao;
  float velocidade;
  int nro_vidas;
  int pontos;
  bool espada;
  double cooldown_espada;
} JOGADOR;

typedef struct monstro {
  POSICAO pos;
  int direcao;
  float velocidade;
  bool vivo;
} MONSTRO;

typedef struct mapa {
  int quant_obstaculos;
  Rectangle obstaculos[QUANT_MAX_OBSTACULOS];
  int fase;
  int quant_inicial_monstros;
  Vector2 pos_inicial_monstros[QUANT_MONSTROS];
} MAPA;

typedef struct texturas {
  Texture2D espada[4];
  Texture2D monstro_texturas[4];
  Texture2D link_texturas[4];
  Texture2D pedra;
} TEXTURA;

void IniciaJanela(void) {
  InitWindow(LARGURA_TELA, ALTURA_TELA, "ZIIL");
  SetTargetFPS(60);
}

void checaTelaCheia() {
  if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
}

Color defineCorBotaoMenu(Rectangle botao) {
  Color cor;
  if (CheckCollisionPointRec(GetMousePosition(), botao))
    cor = GREEN;
  else
    cor = MARROM;

  return cor;
}

bool botaoClicado(Rectangle botao) {
  bool botaoFoiClicado;

  if (CheckCollisionPointRec(GetMousePosition(), botao) &&
      IsMouseButtonPressed(0))
    botaoFoiClicado = true;
  else
    botaoFoiClicado = false;

  return botaoFoiClicado;
}

int menu(void) {
  int escolha = 0;

  Rectangle botaoJOGAR = {80, 210, 165, ALTURA_BOTAO};
  Rectangle botaoCARREGAR = {80, 310, 275, ALTURA_BOTAO};
  Rectangle botaoRECORDES = {80, 410, 275, ALTURA_BOTAO};
  Rectangle botaoSAIR = {80, 510, 120, ALTURA_BOTAO};

  Texture2D hub = LoadTexture("assets/sprites/Menu_Zelda.png");

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(PESSEGO);
    DrawTexture(hub, 0, 0, RAYWHITE);

    DrawText("JOGAR", botaoJOGAR.x, botaoJOGAR.y, 50,
             defineCorBotaoMenu(botaoJOGAR));
    if (botaoClicado(botaoJOGAR)) {
      escolha = 1;
    }

    DrawText("CARREGAR", botaoCARREGAR.x, botaoCARREGAR.y, 50,
             defineCorBotaoMenu(botaoCARREGAR));
    if (botaoClicado(botaoCARREGAR)) {
      escolha = 2;
    }

    DrawText("RECORDES", botaoRECORDES.x, botaoRECORDES.y, 50,
             defineCorBotaoMenu(botaoRECORDES));
    if (botaoClicado(botaoRECORDES)) {
      escolha = 3;
    }

    DrawText("SAIR", botaoSAIR.x, botaoSAIR.y, 50,
             defineCorBotaoMenu(botaoSAIR));
    if (botaoClicado(botaoSAIR)) {
      escolha = 4;
    }

    if (escolha != 0) return escolha;

    EndDrawing();
  }
  return 4;
}

// Função que recebe o nome do arquivo nível e preenche uma matriz de
// caracteres com o conteúdo. A rotina deve retornar o numero de monstros
// presente no arquivo
// Abertura e fechamento do arquivo nesta função
int le_arquivo_nivel(char mat[][COLUNAS], char nome_arq[]) {
  FILE *fp;
  int i;
  int j;
  int contador_monstros = 0;

  if (!(fp = fopen(nome_arq, "rb"))) {
    printf("Erro na abertura do arquivo!");
  } else {
    for (i = 0; i < LINHAS; i++) {
      for (j = 0; j < COLUNAS; j++) {
        mat[i][j] = getc(fp);
        if (mat[i][j] == 'M') contador_monstros++;
      }
      getc(fp);
    }
    fclose(fp);
  }

  return contador_monstros;
}

void carregaTexturas(TEXTURA *textura) {
  textura->espada[0] = LoadTexture("assets/sprites/Attack_right.png");
  textura->espada[1] = LoadTexture("assets/sprites/Attack_left.png");
  textura->espada[2] = LoadTexture("assets/sprites/Attack_down.png");
  textura->espada[3] = LoadTexture("assets/sprites/Attack_up.png");

  textura->monstro_texturas[0] = LoadTexture("assets/sprites/Enemy_right.png");
  textura->monstro_texturas[1] = LoadTexture("assets/sprites/Enemy_left.png");
  textura->monstro_texturas[2] = LoadTexture("assets/sprites/Enemy_front.png");
  textura->monstro_texturas[3] = LoadTexture("assets/sprites/Enemy_back.png");

  textura->link_texturas[0] = LoadTexture("assets/sprites/Link_right.png");
  textura->link_texturas[1] = LoadTexture("assets/sprites/Link_left.png");
  textura->link_texturas[2] = LoadTexture("assets/sprites/Link_front.png");
  textura->link_texturas[3] = LoadTexture("assets/sprites/Link_back.png");

  textura->pedra = LoadTexture("assets/sprites/Obstacle.png");
}

void inicializaMonstro(MONSTRO monstros[], int quantidade,
                       Vector2 pos_inicial[]) {
  int i;

  for (i = 0; i < quantidade; i++) {
    monstros[i] = (MONSTRO){
      pos : {
        atual : (Vector2){x : pos_inicial[i].x, y : pos_inicial[i].y},
        destino : (Vector2){x : pos_inicial[i].x, y : pos_inicial[i].y}
      },
      velocidade : VELOCIDADE_MONSTRO,
      direcao : S,
      vivo : true,
    };
  }
}

bool MovimentoEhLegal(POSICAO *pos, Vector2 mov) {
  // Dada a estrutura POSI��O (passada por refer�ncia) de uma entidade, e um
  // Vector2 correspondente ao movimento a ser realizado, retorna Verdadeiro se
  // o movimento pode ser realizado ou Falso caso contr�rio
  // int i;

  // O movimento � permitido por padr�o,
  bool permitido = true;

  // exceto se a possivel coordenada de destino do movimento acabar fora da �rea
  // jog�vel
  if (pos->atual.x + mov.x >= LARGURA_TELA || pos->atual.x + mov.x < 0 ||
      pos->atual.y + mov.y >= ALTURA_TELA || pos->atual.y + mov.y < 0) {
    permitido = false;
  } /*else {
    for (i = 0; i < mapa.quant_obstaculos; i++) {
      if (CheckCollisionPointRec((Vector2){
            x : (pos->atual.x + mov.x) + 0.1,
            y : (pos->atual.y + mov.y) + 0.1
          },
                                 mapa.obstaculos[i]))
        permitido = false;
    }
  }*/

  return permitido;
}

void AlteraPosicaoDestino(POSICAO *pos, int *direcao_atual,
                          int direcao_movimento) {
  // Dada uma POSICAO e a dire��o atual (passados por refer�ncia), bem como a
  // dire��o desejada do movimento de uma entidade, analisa a dire��o do
  // movimento a ser realizado e passa para a fun��o de verifica��o o vetor
  // desse movimento, alterando a coordenada de destino e a dire��o atual se o
  // movimento for legal

  // Se o personagem n�o est� com uma movimenta��o em andamento, ou seja, as
  // posi��es atuais s�o iguais �s de destino
  if ((pos->atual.x == pos->destino.x) && (pos->atual.y == pos->destino.y)) {
    *direcao_atual = direcao_movimento;
    if (direcao_movimento == L &&
        MovimentoEhLegal(pos, (Vector2){x : QUADRADO, y : 0})) {
      pos->destino.x += QUADRADO;
    } else if (direcao_movimento == O &&
               MovimentoEhLegal(pos, (Vector2){x : -QUADRADO, y : 0})) {
      pos->destino.x -= QUADRADO;
    } else if (direcao_movimento == S &&
               MovimentoEhLegal(pos, (Vector2){x : 0, y : QUADRADO})) {
      pos->destino.y += QUADRADO;
    } else if (direcao_movimento == N &&
               MovimentoEhLegal(pos, (Vector2){x : 0, y : -QUADRADO})) {
      pos->destino.y -= QUADRADO;
    }
  }
}

void TraduzInputJogador(POSICAO *pos, int *direcao_atual) {
  // Dada a estrutura POSI��O e um caractere de dire��o atual (passados por
  // refer�ncia) de um jogador, traduz o aperto de uma determinada tecla para
  // uma a��o do jogo

  // Verifica se alguma tecla de movimenta��o est� pressionada, chama a fun��o
  // de altera��o de destino com os dados do jogador
  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
    AlteraPosicaoDestino(pos, direcao_atual, L);
  else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
    AlteraPosicaoDestino(pos, direcao_atual, O);
  else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
    AlteraPosicaoDestino(pos, direcao_atual, S);
  else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
    AlteraPosicaoDestino(pos, direcao_atual, N);
}

void MovimentaEntidade(POSICAO *pos, int direcao, float velocidade,
                       float delta) {
  // Dada a estrutura POSI��O (passada por refer�ncia), um caractere de dire��o
  // atual e um float de velocidade de uma entidade, bem como um float para o
  // delta time (tempo passado desde o �ltimo frame), movimenta essa entidade na
  // tela em dire��o � sua posi��o de destino

  // Se uma das coordenadas da posi��o atual n�o corresponde com a respectiva
  // coordenada da posi��o destino
  if ((pos->atual.x != pos->destino.x) || (pos->atual.y != pos->destino.y)) {
    // Se a dire��o atual � "X", executa um movimento de 'velocidade' pixels por
    // segundo nessa dire��o at� alcan�ar a posi��o destino
    if (direcao == L) {
      pos->atual.x += velocidade * delta;
      if (pos->atual.x >= pos->destino.x) pos->atual.x = pos->destino.x;
    }

    else if (direcao == O) {
      pos->atual.x -= velocidade * delta;
      if (pos->atual.x <= pos->destino.x) pos->atual.x = pos->destino.x;
    }

    else if (direcao == S) {
      pos->atual.y += velocidade * delta;
      if (pos->atual.y >= pos->destino.y) pos->atual.y = pos->destino.y;
    }

    else if (direcao == N) {
      pos->atual.y -= velocidade * delta;
      if (pos->atual.y <= pos->destino.y) pos->atual.y = pos->destino.y;
    }
  }
}

void AtaqueEspada(JOGADOR *link, int *quant_monstros, TEXTURA texturas,
                  MONSTRO monstros[]) {
  int delta = GetFrameTime();

  int posX = link->pos.atual.x;
  int posY = link->pos.atual.y;

  if (IsKeyPressed(KEY_SPACE) || link->espada == true) {
    if (link->cooldown_espada <= 0) {
      if (link->direcao == 1)
        posX += 50;
      else if (link->direcao == 2)
        posX -= 50;
      else if (link->direcao == 3)
        posY += 50;
      else
        posY -= 50;

      Rectangle espada_hitbox = (Rectangle){posX, posY, 50, 50};
      DrawTexture(texturas.espada[link->direcao - 1], posX, posY, WHITE);

      for (int i = 0; i < QUANT_MONSTROS; i++)
        if (monstros[i].vivo == true)
          if (CheckCollisionRecs(
                  espada_hitbox,
                  (Rectangle){monstros[i].pos.atual.x, monstros[i].pos.atual.y,
                              50, 50})) {
            monstros[i].vivo = false;
            *quant_monstros -= 1;
          }

      link->espada = true;
    }

    link->cooldown_espada -= delta;
  }

  if (link->cooldown_espada <= -DELAY_ESPADA) {
    link->cooldown_espada = COOLDOWN_ESPADA;
    link->espada = false;
  }
}

void MonstroSegueJogador(POSICAO *monstro_pos, int *monstro_direcao,
                         POSICAO link_pos, float delta) {
  int direcaoLink = 0;
  Vector2 newMov = (Vector2){x : 0, y : 0};

  if (abs(monstro_pos->atual.x - link_pos.atual.x) >
      abs(monstro_pos->atual.y - link_pos.atual.y)) {
    if (monstro_pos->atual.x > link_pos.atual.x) {
      direcaoLink = O;
      newMov.x = -50;
    } else if (monstro_pos->atual.x < link_pos.atual.x) {
      direcaoLink = L;
      newMov.x = 50;
    }
  } else {
    if (monstro_pos->atual.y > link_pos.atual.y) {
      direcaoLink = N;
      newMov.y = -50;
    } else if (monstro_pos->atual.y < link_pos.atual.y) {
      direcaoLink = S;
      newMov.y = 50;
    }
  }

  if (MovimentoEhLegal(monstro_pos, newMov)) {
    AlteraPosicaoDestino(monstro_pos, monstro_direcao, direcaoLink);
  } else {
    AlteraPosicaoDestino(monstro_pos, monstro_direcao, GetRandomValue(1, 4));
  }
}

void jogo(MAPA *dados_jogo) {
  int i;
  double cooldownColisao = 0.0;
  int quant_monstros = dados_jogo->quant_inicial_monstros;

  MONSTRO monstros[QUANT_MONSTROS];
  TEXTURA texturas;

  carregaTexturas(&texturas);

  JOGADOR link = (JOGADOR){
    pos : {atual : (Vector2){x : 0, y : 0}, destino : (Vector2){x : 0, y : 0}},
    velocidade : 350.0,
    direcao : S,
    nro_vidas : 3,
    espada : false,
    cooldown_espada : 0.0
  };

  /*for (int i = 0; i < QUANT_PEDRAS; i++) {
    pedras[i] = (Rectangle){
      x : posicao_pedras[i][0],
      y : posicao_pedras[i][1],
      width : 50,
      height : 50
    };
  }*/

  inicializaMonstro(monstros, dados_jogo->quant_inicial_monstros,
                    dados_jogo->pos_inicial_monstros);

  // Main game loop
  while (!WindowShouldClose()) {
    float delta = GetFrameTime();

    BeginDrawing();
    ClearBackground((Color){253, 217, 169, 255});

    TraduzInputJogador(&link.pos, &link.direcao);
    MovimentaEntidade(&link.pos, link.direcao, link.velocidade, delta);
    AtaqueEspada(&link, &quant_monstros, texturas, monstros);
    DrawTexture(texturas.link_texturas[link.direcao - 1], link.pos.atual.x,
                link.pos.atual.y, WHITE);

    Rectangle ret_link = (Rectangle){
      x : link.pos.atual.x,
      y : link.pos.atual.y,
      width : QUADRADO,
      height : QUADRADO
    };

    if (link.cooldown_espada > 0) link.cooldown_espada -= delta;

    for (i = 0; i < quant_monstros; i++) {
      if (monstros[i].vivo) {
        if (abs(monstros[i].pos.atual.x - link.pos.atual.x) < 800 &&
            abs(monstros[i].pos.atual.y - link.pos.atual.y) < 800 &&
            cooldownColisao <= 0) {
          MonstroSegueJogador(&monstros[i].pos, &monstros[i].direcao, link.pos,
                              delta);
        } else {
          AlteraPosicaoDestino(&monstros[i].pos, &monstros[i].direcao,
                               GetRandomValue(1, 4));
        }

        MovimentaEntidade(&monstros[i].pos, monstros[i].direcao,
                          monstros[i].velocidade, delta);
        DrawTexture(texturas.monstro_texturas[monstros[i].direcao - 1],
                    monstros[i].pos.atual.x, monstros[i].pos.atual.y, WHITE);

        if (CheckCollisionRecs(ret_link, (Rectangle){monstros[i].pos.atual.x,
                                                     monstros[i].pos.atual.y,
                                                     QUADRADO, QUADRADO}) &&
            cooldownColisao <= 0) {
          link.nro_vidas--;
          cooldownColisao = 15;

          DrawText("Colis�o", 400, 550, 30, PURPLE);
        } else if (cooldownColisao > 0)
          cooldownColisao -= delta;
      }
    }

    DrawTexture(texturas.link_texturas[link.direcao - 1], link.pos.atual.x,
                link.pos.atual.y, WHITE);

    for (int i = 0; i < dados_jogo->quant_obstaculos; i++) {
      DrawTexture(texturas.pedra, dados_jogo->obstaculos[i].x,
                  dados_jogo->obstaculos[i].y, WHITE);
    }

    /*if (quant_monstros == 0) {
      dados_jogo.fase += 1;
      jogo();
    }*/

    DrawText(TextFormat("Vidas: %d", link.nro_vidas), 100, 50, 20, VIOLET);
    DrawText(TextFormat("FASE: %d", dados_jogo->fase), 100, 70, 20, VIOLET);

    if (IsKeyDown(KEY_ENTER)) {
      for (int i = 0; i < QUANT_MAX_OBSTACULOS; i++) {
        dados_jogo->obstaculos[i] = (Rectangle){
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

int main() {
  IniciaJanela();
  int escolha = 0;
  char configuracao_inicial_grid[LINHAS][COLUNAS];

  escolha = menu();

  MAPA jogo_dados = (MAPA){
    quant_obstaculos : 1,
    obstaculos :
        {(Rectangle){x : 300, y : 400, width : QUADRADO, height : QUADRADO}},
    fase : 1,
    quant_inicial_monstros : 1,
    pos_inicial_monstros : {(Vector2){x : 600, y : 700}},
  };

  if (escolha == 1) {
    jogo(&jogo_dados);
  }
  if (escolha == 4) {
    CloseWindow();
  }
}
