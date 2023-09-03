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
  int quant_inicial_monstros;
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

/* Inicializa a janela do JOGO com as dimensões definidas, um título, e também
inicializa o dispositivo de audio */
void IniciaJanela(void) {
  InitWindow(LARGURA_TELA, ALTURA_TELA, "ZIIL");
  InitAudioDevice();
  SetTargetFPS(60);
}

/* Alterna entre janela e tela cheia toda vez que a tecla F11 é pressionada */
void checaTelaCheia(void) {
  if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
}

/* Lê o arquivo top5.bin, que contém as cinco maiores pontuações no jogo, e as
transfere para uma matriz de estruturas RECORDE. Devolve um booleano indicando
sucesso ou falha na operação. */
bool le_arquivo_recordes(RECORDE top5[5]) {
  bool sucesso = true;
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

/* Salva no arquivo top5.bin os recordes que estão atualmente na matriz de
estruturas RECORDE. Devolve um booleano indicando sucesso ou falha na operação.
*/
bool salva_arquivo(RECORDE top5[5]) {
  bool sucesso = true;
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

/* Troca no array de RECORDE as informações do RECORDE na posição posicao pelas
informações do novo RECORDE novor */
void troca_info_array(RECORDE top5[], RECORDE novor, int posicao) {
  top5[posicao] = novor;
}

/* Se há colisão atualmente entre o retângulo que representa o botão do menu e o
mouse, devolve a cor verde. Senão, devolve a cor marrom. */
Color defineCorBotaoMenu(Rectangle botao) {
  Color cor;
  if (CheckCollisionPointRec(GetMousePosition(), botao))
    cor = GREEN;
  else
    cor = MARROM;

  return cor;
}

/* Verifica se o retângulo que representa o botão do menu foi clicado com o
mouse, devolvendo o booleano correspondente */
bool botaoClicado(Rectangle botao) {
  bool botaoFoiClicado;

  if (CheckCollisionPointRec(GetMousePosition(), botao) &&
      IsMouseButtonPressed(0))
    botaoFoiClicado = true;
  else
    botaoFoiClicado = false;

  return botaoFoiClicado;
}

/* Exibe o menu principal do jogo enquanto toca uma música, retornando um
inteiro entre 1 e 4 dependendo do botão que foi clicado: 1 - JOGAR, 2 -
CARREGAR, 3 - RECORDES, 4 - SAIR */
int menu(void) {
  int escolha = 0;

  Rectangle botaoJOGAR = {80, 210, 165, ALTURA_BOTAO};
  Rectangle botaoCARREGAR = {80, 310, 275, ALTURA_BOTAO};
  Rectangle botaoRECORDES = {80, 410, 275, ALTURA_BOTAO};
  Rectangle botaoSAIR = {80, 510, 120, ALTURA_BOTAO};

  Texture2D hub = LoadTexture("assets/sprites/Menu_Zelda.png");
  Sound tema;

  tema = LoadSound("assets/sons/tema.mp3");
  while (!WindowShouldClose()) {
    checaTelaCheia();
    BeginDrawing();
    ClearBackground(PESSEGO);
    DrawRectangle(LARGURA_TELA, 0, 300, ALTURA_TELA, BLACK);
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

    if (escolha != 0) {
      StopSound(tema);
      return escolha;
    }
    if (!IsSoundPlaying(tema)) {
      PlaySound(tema);
    }
    EndDrawing();
  }
  return 4;
}

/* Lê do arquivo binário save.bin a estrutura MAPA, que contém todos os dados de
um jogo salvo previamente, e sobrescreve a estrutura MAPA do programa. Retorna
um booleano correspondendo ao sucesso da operação. */
bool carregaJogo(MAPA *dados) {
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
  return sucesso;
}

/* Grava no arquivo binário save.bin a estrutura MAPA atual do programa, para
carregar depois. Devolve um booleano informando sucesso ou não. */
bool salvaJogo(MAPA *dados) {
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
  return sucesso;
}

/* Mostra as 5 melhores pontuações contidas no arquivo top5.bin, com um plano de
fundo e uma música tocando. Se o botão VOLTAR for pressionado, retorna 0 para
voltar para o menu.*/
int mostraRecordes(RECORDE recordes[]) {
  Texture2D hub = LoadTexture("assets/sprites/Menu_Zelda.png");
  char pontos[30];
  Sound tema;
  Rectangle botaoVOLTAR = {80, 210, 165, ALTURA_BOTAO};
  tema = LoadSound("assets/sons/tema.mp3");
  bool botao_nao_clicado = true;

  while (!WindowShouldClose() && botao_nao_clicado) {
    checaTelaCheia();
    BeginDrawing();
    ClearBackground(PESSEGO);
    DrawRectangle(LARGURA_TELA, 0, 300, ALTURA_TELA, BLACK);
    DrawTexture(hub, 0, 0, RAYWHITE);

    DrawText(recordes[0].nome, 80, 150, 30, GOLD);
    sprintf(pontos, "%d", recordes[0].pontos);
    DrawText(pontos, 100, 200, 30, GOLD);

    DrawText(recordes[1].nome, 80, 250, 30, GRAY);
    sprintf(pontos, "%d", recordes[1].pontos);
    DrawText(pontos, 100, 300, 30, GRAY);

    DrawText(recordes[2].nome, 80, 350, 30, BROWN);
    sprintf(pontos, "%d", recordes[2].pontos);
    DrawText(pontos, 100, 400, 30, BROWN);

    DrawText(recordes[3].nome, 80, 450, 30, BLUE);
    sprintf(pontos, "%d", recordes[3].pontos);
    DrawText(pontos, 100, 500, 30, BLUE);

    DrawText(recordes[4].nome, 80, 550, 30, BLUE);
    sprintf(pontos, "%d", recordes[4].pontos);
    DrawText(pontos, 100, 600, 30, BLUE);

    DrawText("VOLTAR", botaoVOLTAR.x, botaoVOLTAR.y, 50,
             defineCorBotaoMenu(botaoVOLTAR));

    if (botaoClicado(botaoVOLTAR)) {
      botao_nao_clicado = false;
    }

    if (!IsSoundPlaying(tema)) {
      PlaySound(tema);
    }

    EndDrawing();
  }
  StopSound(tema);
  return 1;
}

/* Dado um array de MONSTRO, um índice, e uma posição x e y, cria uma estrutura
MONSTRO naquele índice do array, com os parâmetros determinados e posição no
mapa correspondente ao x e y informados */
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

/* Dada uma matriz de caracteres do mesmo tamanho de linhas e colunas da grade
do jogo, a preenche a partir de um arquivo texto de caracteres, escolhido
conforme o nível informado como parâmetro.  */
void leArquivoNivel(char mat[][COLUNAS], int nivel) {
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

/* Inicializa a estrutura de dados_jogo, conforme a matriz de nível carregada
 * previamente.*/
void montaMapa(char mat[][COLUNAS], MAPA *dados_jogo) {
  int i, j;
  dados_jogo->quant_monstros = 0;
  dados_jogo->quant_obstaculos = 0;
  dados_jogo->quant_inicial_monstros = 0;

  for (i = 0; i < LINHAS; i++)
    for (j = 0; j < COLUNAS; j++) {
      /* Se o caracter da posição atual da matriz é 'O', de obstáculo,
      inicializa uma estrutura Rectangle na matriz de obstáculos do MAPA,
      traduzindo as coordenadas da matriz para o jogo, e aumenta o contador de
      obstáculos*/
      if (mat[i][j] == 'O') {
        dados_jogo->obstaculos[dados_jogo->quant_obstaculos] = (Rectangle){
          x : j * 50,
          y : i * 50,
          width : QUADRADO,
          height : QUADRADO
        };
        dados_jogo->quant_obstaculos++;
      }

      /* Se o caracter da posição atual da matriz é 'M', de monstro, chama a
      função inicializaMonstro para inicializar um monstro na matriz de MONSTRO,
      usando como índice o contador de monstros e traduzindo as coordenadas da
      matriz, incrementando o contador de monstros ao final*/
      else if (mat[i][j] == 'M') {
        inicializaMonstro(dados_jogo->monstros, dados_jogo->quant_monstros,
                          j * 50, i * 50);
        dados_jogo->quant_monstros++;
        dados_jogo->quant_inicial_monstros++;
      }

      /* Se o caracter da posição atual da matriz é 'P', de player, inicializa a
      estrutura JOGADOR no campo apropriado do MAPA, traduzindo as coordenadas
      da matriz*/
      else if (mat[i][j] == 'P') {
        dados_jogo->jogador = (JOGADOR){
          pos : {
            atual : (Vector2){x : j * 50, y : i * 50},
            destino : (Vector2){x : (float)j * 50, y : (float)i * 50}
          },
          velocidade : VELOCIDADE_JOGADOR,
          direcao : S,
          nro_vidas :
              3,  // QUEREMOS RESETAR A QUANTIDADE DE VIDAS EM CADA NÍVEL?
          espada : false,
          cooldown_espada : 0.0
        };
      }
    }
}

/* Carrega as texturas usadas no jogo para dentro da estrutura TEXTURAS */
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
  /* Dada a estrutura POSIÇÃO (passada por referência) de uma entidade, e um
  Vector2 correspondente ao movimento a ser realizado, retorna Verdadeiro se
  o movimento pode ser realizado ou Falso caso contrário */
  int i;

  /* O movimento é permitido por padrão, */
  bool permitido = true;

  /* exceto se a possivel coordenada de destino do movimento acabar fora da área
  jogável */
  if (pos->atual.x + mov.x >= LARGURA_TELA || pos->atual.x + mov.x < 0 ||
      pos->atual.y + mov.y >= ALTURA_TELA || pos->atual.y + mov.y < 0) {
    permitido = false;
  }
  /*ou se o destino do movimento for a coordenada de um dos obstáculos do MAPA*/
  else {
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
  /* Dada uma POSICAO e a direção atual (passados por referência), bem como a
   direção desejada do movimento de uma entidade, analisa a direção do
   movimento a ser realizado e passa para a função de verificação o vetor
   desse movimento e os dados do jogo, alterando a coordenada de destino e a
   direção atual se o movimento for legal */

  /* Se o personagem não está com uma movimentação em andamento, ou seja, as
  posições atuais são iguais às de destino */
  if ((pos->atual.x == pos->destino.x) && (pos->atual.y == pos->destino.y)) {
    *direcao_atual = direcao_movimento;
    /* Altera a posição de destino na direção correspondente, primeiro
     * verificando através da função MovimentoEhLegal se é um movimento válido.
     */
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
  /* Dada a estrutura POSIÇÃO e um inteiro de direção atual (passados por
  referência) de um jogador, bem como o MAPA do jogo, traduz o aperto de uma
  determinada tecla para uma ação do jogo */

  /* Verifica se alguma tecla de movimentação está pressionada, chama a função
  de alteração de destino com os dados do jogador */
  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
    AlteraPosicaoDestino(pos, direcao_atual, L, dados);
  else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
    AlteraPosicaoDestino(pos, direcao_atual, O, dados);
  else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
    AlteraPosicaoDestino(pos, direcao_atual, S, dados);
  else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
    AlteraPosicaoDestino(pos, direcao_atual, N, dados);

  /* Se a tecla L for presionada, chama a função de salvamento do jogo */
  if (IsKeyPressed(KEY_L)) {
    salvaJogo(dados);
  }
}

void MovimentaEntidade(POSICAO *pos, int direcao, float velocidade,
                       float delta) {
  /* Dada a estrutura POSIÇÃO (passada por referência), um caractere de direção
  atual e um float de velocidade de uma entidade, bem como um float para o
  delta time (tempo passado desde o último frame), movimenta essa entidade na
  tela em direção a sua posição de destino */

  /* Se uma das coordenadas da posição atual não corresponde com a respectiva
  coordenada da posição destino */
  if ((pos->atual.x != pos->destino.x) || (pos->atual.y != pos->destino.y)) {
    /* Se a direção atual a "X", executa um movimento de 'velocidade' pixels por
    segundo nessa direção até alcançar a posição destino */
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
void movimentaMonstro(MONSTRO monstros[], JOGADOR jogador, TEXTURA texturas,
                      int quantidade_inicial_monstros, double cooldownColisao,
                      double delta, MAPA *dj) {
  int i;

  for (i = 0; i < quantidade_inicial_monstros; i++) {
    if (monstros[i].vivo) {
      if (abs(monstros[i].pos.atual.x - jogador.pos.atual.x) < 800 &&
          abs(monstros[i].pos.atual.y - jogador.pos.atual.y) < 800 &&
          cooldownColisao <= 0) {
        MonstroSegueJogador(&monstros[i].pos, &monstros[i].direcao, jogador.pos,
                            delta, dj);
      } else {
        AlteraPosicaoDestino(&monstros[i].pos, &monstros[i].direcao,
                             GetRandomValue(1, 4), dj);
      }

      MovimentaEntidade(&monstros[i].pos, monstros[i].direcao,
                        monstros[i].velocidade, delta);
      DrawTexture(texturas.monstro_texturas[monstros[i].direcao - 1],
                  monstros[i].pos.atual.x, monstros[i].pos.atual.y, WHITE);
    }
  }
}
void checaColisaoMonstroJogador(MONSTRO monstros[], int *nro_vidas,
                                Rectangle ret_link, double *cooldownColisao,
                                double delta, int quantidade_inicial_monstros) {
  int i;

  for (i = 0; i < quantidade_inicial_monstros; i++) {
    if (CheckCollisionRecs(ret_link, (Rectangle){monstros[i].pos.atual.x,
                                                 monstros[i].pos.atual.y,
                                                 QUADRADO, QUADRADO}) &&
        cooldownColisao <= 0) {
      *nro_vidas--;
      cooldownColisao = 15;

      DrawText("Colis�o", 400, 550, 30, PURPLE);
    } else if (cooldownColisao > 0)
      *cooldownColisao -= delta;
  }
}
int jogo(MAPA *dj) {
  double cooldownColisao = 0.0;
  TEXTURA texturas;

  carregaTexturas(&texturas);

  // Main game loop
  while (!WindowShouldClose()) {
    float delta = GetFrameTime();
    checaTelaCheia();
    BeginDrawing();
    ClearBackground((Color){253, 217, 169, 255});
    DrawRectangle(LARGURA_TELA, 0, 300, ALTURA_TELA, BLACK);

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

    if (dj->jogador.cooldown_espada > 0) dj->jogador.cooldown_espada -= delta;

    movimentaMonstro(&dj->monstros, dj->jogador, texturas,
                     dj->quant_inicial_monstros, cooldownColisao, delta, dj);
    checaColisaoMonstroJogador(dj->monstros, &dj->jogador.nro_vidas, ret_link,
                               &cooldownColisao, delta,
                               dj->quant_inicial_monstros);
    AtaqueEspada(&dj->jogador, &dj->quant_monstros, texturas, dj->monstros,
                 delta, dj);
    DrawTexture(texturas.link_texturas[dj->jogador.direcao - 1],
                dj->jogador.pos.atual.x, dj->jogador.pos.atual.y, WHITE);

    for (int i = 0; i < dj->quant_obstaculos; i++) {
      DrawTexture(texturas.pedra, dj->obstaculos[i].x, dj->obstaculos[i].y,
                  WHITE);
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

    EndDrawing();
  }
}

/* Dada a pontuação do jogador atual, devolve a posição (de 1 a 5) que ele
alcançou no top5, ou 6 se ele ficou fora do top5 */
int ehPontuaçãoRecorde(RECORDE recordes[], int pontuacao) {
  int i;
  int posicao = 6;

  for (i = 5; i >= 1; i--) {
    if (pontuacao > recordes[i - 1].pontos) posicao = i;
  }

  return posicao;
}

/* Reorganiza os recordes anteriores e insere o novo recorde na posição
 * informada */
void insereRecorde(int posicao, RECORDE atual, RECORDE top5[]) {
  int i;
  for (i = 3; i > posicao - 1; i--) {
    troca_info_array(top5, top5[i], i + 1);
  }
  troca_info_array(top5, atual, posicao - 1);
}

int main() {
  MAPA dados_jogo;
  char grid[LINHAS][COLUNAS];
  int resultado = 1;
  RECORDE top5[5];
  RECORDE atual;
  int posicao_pontuacao;

  if (!(le_arquivo_recordes(top5))) {
    puts("Erro na leitura do arquivo de recordes!");
  };

  IniciaJanela();
  int escolha = 0;

  while (escolha != 4) {
    escolha = menu();

    if (escolha == 1) {
      dados_jogo.fase = 1;
      leArquivoNivel(grid, dados_jogo.fase);
      montaMapa(grid, &dados_jogo);
      dados_jogo.pontuacao = 0;
      do {
        resultado = jogo(&dados_jogo);
        if (dados_jogo.fase > NUM_FASES) {
          if ((posicao_pontuacao =
                   ehPontuaçãoRecorde(top5, dados_jogo.pontuacao))) {
            printf("Insira seu nome");
          }
          printf("Vitória");
          CloseWindow();
        }

        else if (resultado == 1) {
          leArquivoNivel(grid, dados_jogo.fase);
          montaMapa(grid, &dados_jogo);
        }

        else if (resultado == 0) {
          if (posicao_pontuacao =
                  ehPontuaçãoRecorde(top5, dados_jogo.pontuacao)) {
            printf("Insira seu nome");
          }
          printf("Derrota");
          CloseWindow();
        }
      } while (resultado);
    }

    if (escolha == 2) {
      carregaJogo(&dados_jogo);
      do {
        resultado = jogo(&dados_jogo);
        if (dados_jogo.fase > NUM_FASES) {
          if (posicao_pontuacao =
                  ehPontuaçãoRecorde(top5, dados_jogo.pontuacao)) {
            printf("Insira seu nome");
          }
          printf("Vitória");
          CloseWindow();
        }

        else if (resultado == 1) {
          leArquivoNivel(grid, dados_jogo.fase);
          montaMapa(grid, &dados_jogo);
        }

        else if (resultado == 0) {
          if (posicao_pontuacao =
                  ehPontuaçãoRecorde(top5, dados_jogo.pontuacao)) {
            printf("Insira seu nome");
          }
          printf("Derrota");
          CloseWindow();
        }
      } while (resultado);
    }
    if (escolha == 3) escolha = mostraRecordes(top5);
  }
  CloseAudioDevice();
  CloseWindow();

  return 0;
}
