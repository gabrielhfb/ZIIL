#include <math.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"

#define L 1
#define O 2
#define S 3
#define N 4
#define LARGURA_TELA 1200
#define ALTURA_TELA 800
#define ALTURA_BOTAO 40
#define QUADRADO 50
#define QUANT_MAX_OBSTACULOS 50
#define QUANT_MAX_MONSTROS 10
#define DELAY_ESPADA 0.25
#define COOLDOWN_ESPADA 2
#define VELOCIDADE_MONSTRO 100
#define VELOCIDADE_JOGADOR 145
#define LINHAS 16
#define COLUNAS 24
#define NUM_FASES 2

#define PESSEGO \
  (Color) { 255, 204, 197, 255 }
#define MARROM \
  (Color) { 153, 78, 0, 255 }

typedef struct posicao {
  Vector2 atual;
  Vector2 destino;
} POSICAO;

typedef struct highscore {
  char nome[45];
  int pontos;
} RECORDE;

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
  int quant_monstros;
  MONSTRO monstros[QUANT_MAX_MONSTROS];
  JOGADOR jogador;
  int pontuacao;
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

void checaTelaCheia(void) {
  if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
}

// Recebe o nome do arquivo a ser lido (ler no main) e armazena o
// conteúdo no array top5
// Abre e fecha o arquivo dentro desta função
// Retorna TRUE em caso de sucesso e FALSE no caso contrário
int le_arquivo(RECORDE top5[5]) {
  int sucesso = true;
  FILE *fp;

  if (!(fp = fopen("assets/top5.bin", "rb"))) {
    printf("Erro na abertura do arquivo!");
    sucesso = false;
  } else {
    if (fread(top5, sizeof(RECORDE), 5, fp) != 5) {
      printf("Erro na leitura");
      sucesso = false;
    }
  }
  fclose(fp);

  return sucesso;
}

// Recebe o nome do arquivo a ser escrito e armazena o
// conteúdo do array top5
// Abre e fecha o arquivo dentro desta função
// Retorna TRUE em caso de sucesso e FALSE no caso contrário
int salva_arquivo(RECORDE top5[5]) {
  int sucesso = true;
  FILE *fp;

  if (!(fp = fopen("assets/top5.bin", "wb"))) {
    printf("Erro na abertura do arquivo!");
    sucesso = false;
  } else {
    if (fwrite(&top5, sizeof(RECORDE), 5, fp) != 5) {
      printf("Erro na escrita");
      sucesso = false;
    }
  }
  fclose(fp);

  return sucesso;
}

// Troca no array as informações do jogador na posição posicao pelas informações
// do novo jogador novoj
void troca_info_array(RECORDE top5[], RECORDE novor, int posicao) {
  top5[posicao] = novor;
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
    checaTelaCheia();
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

bool carregaJogo(MAPA *dados){
  bool sucesso = true;
  FILE *fp;

  if (!(fp = fopen("assets/save.bin", "rb"))) {
    printf("Erro na abertura do arquivo!");
    sucesso = false;
  } else {
    if (fread(dados, sizeof(MAPA), 1, fp) != 1) {
      printf("Erro na escrita");
      sucesso = false;
    }
  }
  fclose(fp);
}

bool salvaJogo(MAPA *dados){
  bool sucesso = true;
  FILE *fp;

  if (!(fp = fopen("assets/save.bin", "wb"))) {
    printf("Erro na abertura do arquivo!");
    sucesso = false;
  } else {
    if (fwrite(dados, sizeof(MAPA), 1, fp) != 1) {
      printf("Erro na leitura");
      sucesso = false;
    }
  }
  fclose(fp);
}

void inicializaMonstro(MONSTRO monstros[], int indice, int x, int y) {
  monstros[indice] = (MONSTRO){
    pos : {
      atual : (Vector2){x : x, y : y},
      destino : (Vector2){x : (float)x, y : (float)y}
    },
    velocidade : VELOCIDADE_MONSTRO,
    direcao : S,
    vivo : true,
  };
}

void le_arquivo_nivel(char mat[][COLUNAS], int nivel) {
  FILE *fp;
  int i;
  int j;
  char nome_arq[32];

  switch (nivel) {
    case 1:
      strcpy(nome_arq, "assets/niveis/nivel1.txt");
      break;
    case 2:
      strcpy(nome_arq, "assets/niveis/nivel2.txt");
      break;
  }

  if (!(fp = fopen(nome_arq, "rb"))) {
    printf("Erro na abertura do arquivo!");
  } else {
    for (i = 0; i < LINHAS; i++) {
      for (j = 0; j < COLUNAS; j++) {
        mat[i][j] = getc(fp);
      }
      getc(fp);
    }
    fclose(fp);
  }
}

void monta_mapa(char mat[][COLUNAS], MAPA *dados_jogo) {
  int i, j;
  dados_jogo->pontuacao = 0;
  dados_jogo->quant_monstros = 0;
  dados_jogo->quant_obstaculos = 0;

  for (i = 0; i < LINHAS; i++)
    for (j = 0; j < COLUNAS; j++) {
      if (mat[i][j] == 'O') {
        dados_jogo->obstaculos[dados_jogo->quant_obstaculos] = (Rectangle){
          x : j * 50,
          y : i * 50,
          width : QUADRADO,
          height : QUADRADO
        };
        dados_jogo->quant_obstaculos++;
      }

      else if (mat[i][j] == 'M') {
        inicializaMonstro(dados_jogo->monstros, dados_jogo->quant_monstros,
                          j * 50, i * 50);
        dados_jogo->quant_monstros++;
      }

      else if (mat[i][j] == 'P') {
        dados_jogo->jogador = (JOGADOR){
          pos : {
            atual : (Vector2){x : j * 50, y : i * 50},
            destino : (Vector2){x : (float)j * 50, y : (float)i * 50}
          },
          velocidade : VELOCIDADE_JOGADOR,
          direcao : S,
          nro_vidas : 3,
          espada : false,
          cooldown_espada : 0.0
        };
      }
    }
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

bool MovimentoEhLegal(POSICAO *pos, Vector2 mov, MAPA *dados) {
  // Dada a estrutura POSI��O (passada por refer�ncia) de uma entidade, e um
  // Vector2 correspondente ao movimento a ser realizado, retorna Verdadeiro se
  // o movimento pode ser realizado ou Falso caso contr�rio
  int i;

  // O movimento � permitido por padr�o,
  bool permitido = true;

  // exceto se a possivel coordenada de destino do movimento acabar fora da �rea
  // jog�vel
  if (pos->atual.x + mov.x >= LARGURA_TELA || pos->atual.x + mov.x < 0 ||
      pos->atual.y + mov.y >= ALTURA_TELA || pos->atual.y + mov.y < 0) {
    permitido = false;
  } else {
    for (i = 0; i < dados->quant_obstaculos; i++) {
      if (CheckCollisionPointRec((Vector2){
            x : (pos->atual.x + mov.x) + 0.1,
            y : (pos->atual.y + mov.y) + 0.1
          },
                                 dados->obstaculos[i]))
        permitido = false;
    }
  }

  return permitido;
}

void AlteraPosicaoDestino(POSICAO *pos, int *direcao_atual,
                          int direcao_movimento, MAPA *dados) {
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
        MovimentoEhLegal(pos, (Vector2){x : QUADRADO, y : 0}, dados)) {
      pos->destino.x += QUADRADO;
    } else if (direcao_movimento == O &&
               MovimentoEhLegal(pos, (Vector2){x : -QUADRADO, y : 0}, dados)) {
      pos->destino.x -= QUADRADO;
    } else if (direcao_movimento == S &&
               MovimentoEhLegal(pos, (Vector2){x : 0, y : QUADRADO}, dados)) {
      pos->destino.y += QUADRADO;
    } else if (direcao_movimento == N &&
               MovimentoEhLegal(pos, (Vector2){x : 0, y : -QUADRADO}, dados)) {
      pos->destino.y -= QUADRADO;
    }
  }
}

void TraduzInputJogador(POSICAO *pos, int *direcao_atual, MAPA *dados) {
  // Dada a estrutura POSI��O e um caractere de dire��o atual (passados por
  // refer�ncia) de um jogador, traduz o aperto de uma determinada tecla para
  // uma a��o do jogo

  // Verifica se alguma tecla de movimenta��o est� pressionada, chama a fun��o
  // de altera��o de destino com os dados do jogador
  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
    AlteraPosicaoDestino(pos, direcao_atual, L, dados);
  else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
    AlteraPosicaoDestino(pos, direcao_atual, O, dados);
  else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
    AlteraPosicaoDestino(pos, direcao_atual, S, dados);
  else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
    AlteraPosicaoDestino(pos, direcao_atual, N, dados);
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
                  MONSTRO monstros[], float delta, MAPA *dados) {
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
      
      for (int i = 0; i < QUANT_MAX_MONSTROS; i++)
        if (monstros[i].vivo == true)
          if (CheckCollisionRecs(
                  espada_hitbox,
                  (Rectangle){monstros[i].pos.atual.x, monstros[i].pos.atual.y,
                              50, 50})) {
            monstros[i].vivo = false;
            *quant_monstros -= 1;
            dados->pontuacao += 100;
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
                         POSICAO link_pos, float delta, MAPA *dados) {
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

  if (MovimentoEhLegal(monstro_pos, newMov, dados)) {
    AlteraPosicaoDestino(monstro_pos, monstro_direcao, direcaoLink, dados);
  } else {
    AlteraPosicaoDestino(monstro_pos, monstro_direcao, GetRandomValue(1, 4),
                         dados);
  }
}

int jogo(MAPA *dj) {
  int i;
  double cooldownColisao = 0.0;
  int quantidade_inicial_monstros = dj->quant_monstros;
  TEXTURA texturas;

  carregaTexturas(&texturas);

  // Main game loop
  while (!WindowShouldClose()) {
    float delta = GetFrameTime();
    checaTelaCheia();
    BeginDrawing();
    ClearBackground((Color){253, 217, 169, 255});

    TraduzInputJogador(&dj->jogador.pos, &dj->jogador.direcao, dj);
    MovimentaEntidade(&dj->jogador.pos, dj->jogador.direcao,
                      dj->jogador.velocidade, delta);

    DrawTexture(texturas.link_texturas[dj->jogador.direcao - 1],
                dj->jogador.pos.atual.x, dj->jogador.pos.atual.y, WHITE);

    Rectangle ret_link = (Rectangle){
      x : dj->jogador.pos.atual.x,
      y : dj->jogador.pos.atual.y,
      width : QUADRADO,
      height : QUADRADO
    };

    if (dj->jogador.cooldown_espada > 0)
      dj->jogador.cooldown_espada -= delta;



    for (i = 0; i < quantidade_inicial_monstros; i++) {
      if (dj->monstros[i].vivo) {
        if (abs(dj->monstros[i].pos.atual.x - dj->jogador.pos.atual.x) <
                800 &&
            abs(dj->monstros[i].pos.atual.y - dj->jogador.pos.atual.y) <
                800 &&
            cooldownColisao <= 0) {
          MonstroSegueJogador(&dj->monstros[i].pos,
                              &dj->monstros[i].direcao, dj->jogador.pos,
                              delta, dj);
        } else {
          AlteraPosicaoDestino(&dj->monstros[i].pos,
                               &dj->monstros[i].direcao,
                               GetRandomValue(1, 4), dj);
        }

        MovimentaEntidade(&dj->monstros[i].pos, dj->monstros[i].direcao,
                          dj->monstros[i].velocidade, delta);
        DrawTexture(texturas.monstro_texturas[dj->monstros[i].direcao - 1],
                    dj->monstros[i].pos.atual.x,
                    dj->monstros[i].pos.atual.y, WHITE);

        if (CheckCollisionRecs(
                ret_link, (Rectangle){dj->monstros[i].pos.atual.x,
                                      dj->monstros[i].pos.atual.y, QUADRADO,
                                      QUADRADO}) &&
            cooldownColisao <= 0) {
          dj->jogador.nro_vidas--;
          cooldownColisao = 15;

          DrawText("Colis�o", 400, 550, 30, PURPLE);
        } else if (cooldownColisao > 0)
          cooldownColisao -= delta;
      }
    }
    AtaqueEspada(&dj->jogador, &dj->quant_monstros, texturas,
                 dj->monstros, delta, dj);
    DrawTexture(texturas.link_texturas[dj->jogador.direcao - 1],
                  dj->jogador.pos.atual.x, dj->jogador.pos.atual.y,
                  WHITE);

    for (int i = 0; i < dj->quant_obstaculos; i++) {
      DrawTexture(texturas.pedra, dj->obstaculos[i].x,
                  dj->obstaculos[i].y, WHITE);
    }

    DrawText(TextFormat("Vidas: %d", dj->jogador.nro_vidas), 100, 50, 20,
             VIOLET);
    DrawText(TextFormat("FASE: %d", dj->fase), 100, 70, 20, VIOLET);
    DrawText(TextFormat("Pontos: %d", dj->pontuacao), 100, 90, 20, VIOLET);

    if (IsKeyDown(KEY_ENTER)) {
      for (int i = 0; i < dj->quant_obstaculos; i++) {
        dj->obstaculos[i] = (Rectangle){
          x : GetRandomValue(0, (LARGURA_TELA - 50) / 50) * 50,
          y : GetRandomValue(0, (ALTURA_TELA - 50) / 50) * 50,
          width : 50,
          height : 50
        };
      }
    }

    if (dj->quant_monstros <= 0) {
      dj->fase++;
      return 1;
    }
    if (dj->jogador.nro_vidas <= 0) {
      return 0;
    }
    if(IsKeyPressed(KEY_L)){
      salvaJogo(dj);
    }

    EndDrawing();
  }
}

int main() {
  MAPA dados_jogo;
  char grid[LINHAS][COLUNAS];
  int continua = 1;
  RECORDE top5[5];

  le_arquivo(top5);

  IniciaJanela();
  int escolha = 0;

  while (escolha != 4) {
    escolha = menu();

    if (escolha == 1) {
      dados_jogo.fase = 1;
      le_arquivo_nivel(grid, dados_jogo.fase);
      monta_mapa(grid, &dados_jogo);
      do {
        continua = jogo(&dados_jogo);
        if (dados_jogo.fase > NUM_FASES) {
          CloseWindow();
        }

        else if (continua == 1) {
          le_arquivo_nivel(grid, dados_jogo.fase);
          monta_mapa(grid, &dados_jogo);
        }

        else if (continua == 0) {
          CloseWindow();
        }
      } while (continua);
    }
    if(escolha == 2){
      carregaJogo(&dados_jogo);
      do {
        continua = jogo(&dados_jogo);
        if (dados_jogo.fase > NUM_FASES) {
          CloseWindow();
        }

        else if (continua == 1) {
          le_arquivo_nivel(grid, dados_jogo.fase);
          monta_mapa(grid, &dados_jogo);
        }

        else if (continua == 0) {
          CloseWindow();
        }
      } while (continua);
    }
  }
  CloseWindow();
}
