// Gabriel Henrique Fiszczuk Brandeburski
// Rodrigo Grilli

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
#define COOLDOWN_ESPADA 1.75
#define COOLDOWN_COLISAO 7
#define VELOCIDADE_MONSTRO 115
#define VELOCIDADE_JOGADOR 160
#define LINHAS 16
#define COLUNAS 24
#define NUM_FASES 2

#define PESSEGO \
  (Color) { 255, 204, 197, 255 }
#define MARROM \
  (Color) { 153, 78, 0, 255 }
#define CIANO \
  (Color) { 35, 197, 231, 255 }

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
  Texture2D link_azul_texturas[4];
  Texture2D pedra;
  Sound som_espada;
  Sound som_hit;
  Sound som_levelup;
  Sound som_ambiente;
  Sound som_derrota;
  Sound som_vitoria;

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
bool salva_arquivo(RECORDE top5[]) {
  bool sucesso = true;
  FILE *fp;

  if (!(fp = fopen("assets/top5.bin", "wb+"))) {
    printf("Erro na abertura do arquivo!");
    sucesso = false;
  } else {
    if (fwrite(top5, sizeof(RECORDE), 5, fp) != 5) {
      printf("Erro na leitura");
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
  for (i = 3; i >= posicao - 1; i--) {
    troca_info_array(top5, top5[i], i + 1);
  }
  troca_info_array(top5, atual, posicao - 1);
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
  Rectangle botaoVOLTAR = {80, 50, 165, ALTURA_BOTAO};
  tema = LoadSound("assets/sons/tema.mp3");
  bool botao_nao_clicado = true;

  while (!WindowShouldClose() && botao_nao_clicado) {
    checaTelaCheia();
    BeginDrawing();
    ClearBackground(PESSEGO);
    DrawRectangle(LARGURA_TELA, 0, 300, ALTURA_TELA, BLACK);
    DrawTexture(hub, 0, 0, RAYWHITE);

    DrawText(recordes[0].nome, 100, 100, 30, GOLD);
    sprintf(pontos, "%d", recordes[0].pontos);
    DrawText(pontos, 150, 150, 30, GOLD);

    DrawText(recordes[1].nome, 100, 200, 30, GRAY);
    sprintf(pontos, "%d", recordes[1].pontos);
    DrawText(pontos, 150, 250, 30, GRAY);

    DrawText(recordes[2].nome, 100, 300, 30, BROWN);
    sprintf(pontos, "%d", recordes[2].pontos);
    DrawText(pontos, 150, 350, 30, BROWN);

    DrawText(recordes[3].nome, 100, 400, 30, BLUE);
    sprintf(pontos, "%d", recordes[3].pontos);
    DrawText(pontos, 150, 450, 30, BLUE);

    DrawText(recordes[4].nome, 100, 500, 30, BLUE);
    sprintf(pontos, "%d", recordes[4].pontos);
    DrawText(pontos, 150, 550, 30, BLUE);

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
          nro_vidas : 3,
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

  textura->link_azul_texturas[0] =
      LoadTexture("assets/sprites/Link_right_blue.png");
  textura->link_azul_texturas[1] =
      LoadTexture("assets/sprites/Link_left_blue.png");
  textura->link_azul_texturas[2] =
      LoadTexture("assets/sprites/Link_front_blue.png");
  textura->link_azul_texturas[3] =
      LoadTexture("assets/sprites/Link_back_blue.png");

  textura->pedra = LoadTexture("assets/sprites/Obstacle.png");

  textura->som_espada = LoadSound("assets/sons/espada.mp3");
  textura->som_hit = LoadSound("assets/sons/hit.mp3");
  textura->som_ambiente = LoadSound("assets/sons/jogo.mp3");
  textura->som_levelup = LoadSound("assets/sons/levelup.mp3");
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

/* Dado o array de texturas, o delta time e o MAPA do jogo, coordena os ataques
 * da espada */
void AtaqueEspada(TEXTURA texturas, float delta, MAPA *dados) {
  int posX = dados->jogador.pos.atual.x;
  int posY = dados->jogador.pos.atual.y;

  /* se a tecla J/espaço for pressionada ou a condição espada estiver
   * ativa */
  if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_J) ||
      dados->jogador.espada == true) {
    /* Se o período de cooldown da espada já passou */
    if (dados->jogador.cooldown_espada <= 0) {
      /* altera as variáveis posX e posY de acordo com a direção que o jogador
       * está virado */
      if (dados->jogador.direcao == 1)
        posX += QUADRADO;
      else if (dados->jogador.direcao == 2)
        posX -= QUADRADO;
      else if (dados->jogador.direcao == 3)
        posY += QUADRADO;
      else
        posY -= QUADRADO;

      /* Cria um retângulo com a posX ou posY deslocadas deslocadas de acordo
       * com o condicional acima, e desenha a textura da espada nesse retângulo
       */
      Rectangle espada_hitbox = (Rectangle){posX, posY, QUADRADO, QUADRADO};
      DrawTexture(texturas.espada[dados->jogador.direcao - 1], posX, posY,
                  WHITE);

      /* Toca o som da espada */
      if (!IsSoundPlaying(texturas.som_espada)) {
        PlaySound(texturas.som_espada);
      }

      /* Verifica monstro a monstro se eles estão na área de colisão da espada.
       * Se sim, troca o estado para morto, diminui o contador de monstros e
       * aumenta a pontuação */
      for (int i = 0; i < QUANT_MAX_MONSTROS; i++)
        if (dados->monstros[i].vivo == true)
          if (CheckCollisionRecs(espada_hitbox,
                                 (Rectangle){dados->monstros[i].pos.atual.x,
                                             dados->monstros[i].pos.atual.y,
                                             QUADRADO, QUADRADO})) {
            dados->monstros[i].vivo = false;
            dados->quant_monstros -= 1;
            dados->pontuacao += 100;
          }

      /* torna a condição espada verdadeira */
      dados->jogador.espada = true;
    }

    /* Diminui a variável cooldown da espada a cada frame que se passa */
    dados->jogador.cooldown_espada -= delta;
  }

  /* Se o período que a espada deve ficar ativada após apertar espaço já passou,
   * passa para a variável de cooldown o tempo que deve ser esperado até que ela
   * possa ser usada novamente, e desativa a espada*/
  if (dados->jogador.cooldown_espada <= -DELAY_ESPADA) {
    dados->jogador.cooldown_espada = COOLDOWN_ESPADA;
    dados->jogador.espada = false;
  }
}

/* Dado um monstro e a posição do jogador, o delta time e o MAPA do jogo, altera
a posição de destino do monstro para tentar se aproximar do jogador */
void MonstroSegueJogador(MONSTRO *monstro, POSICAO link_pos, float delta,
                         MAPA *dados) {
  int direcaoLink = 0;
  Vector2 newMov = (Vector2){x : 0, y : 0};

  /* Se a diferença de coordenadas x entre o monstro e o jogador é maior do que
   * a diferença de coordenadas y */
  if (abs(monstro->pos.atual.x - link_pos.atual.x) >
      abs(monstro->pos.atual.y - link_pos.atual.y)) {
    /* Se a coordenada x do monstro é maior do que a coordenada do jogador,
     * busca fazer um movimento para reduzí-la */
    if (monstro->pos.atual.x > link_pos.atual.x) {
      direcaoLink = O;
      newMov.x = -50;
    }
    /* Senão se a coordenada x do monstro é menor do que a coordenada do
     * jogador, busca fazer um movimento para aumentá-la */
    else if (monstro->pos.atual.x < link_pos.atual.x) {
      direcaoLink = L;
      newMov.x = 50;
    }
  }
  /* Senão, */
  else {
    /* Se a coordenada y do monstro é maior do que a coordenada do jogador,
     * busca fazer um movimento para reduzí-la */
    if (monstro->pos.atual.y > link_pos.atual.y) {
      direcaoLink = N;
      newMov.y = -50;
    }
    /* Senão se a coordenada y do monstro é menor do que a coordenada do
     * jogador, busca fazer um movimento para aumentá-la */
    else if (monstro->pos.atual.y < link_pos.atual.y) {
      direcaoLink = S;
      newMov.y = 50;
    }
  }

  /* Se o movimento desejado é permitido, chama a função para realizá-lo */
  if (MovimentoEhLegal(&monstro->pos, newMov, dados)) {
    AlteraPosicaoDestino(&monstro->pos, &monstro->direcao, direcaoLink, dados);
  }
  /* Senão, faz um movimento para uma direção aleatória */
  else {
    AlteraPosicaoDestino(&monstro->pos, &monstro->direcao, GetRandomValue(1, 4),
                         dados);
  }
}

/* Dadas as texturas do jogo, o cooldown da colisão entre monstro e jogador, o
 * delta time e os dados do jogo, executa a movimentação dos monstros */
void movimentaMonstro(TEXTURA texturas, double cooldownColisao, double delta,
                      MAPA *dj) {
  int i;

  /* Para cada um dos monstros */
  for (i = 0; i < dj->quant_inicial_monstros; i++) {
    /* Se ele ainda estiver vivo */
    if (dj->monstros[i].vivo) {
      /* E se ele estiver a uma distancia de menos de 800 pixels da posição do
       * jogador e o cooldown para colisão for menor do que zero, calcula o
       * movimento do monstro em direção ao jogador */
      if (abs(dj->monstros[i].pos.atual.x - dj->jogador.pos.atual.x) < 800 &&
          abs(dj->monstros[i].pos.atual.y - dj->jogador.pos.atual.y) < 800 &&
          cooldownColisao <= 0) {
        MonstroSegueJogador(&dj->monstros[i], dj->jogador.pos, delta, dj);
      }
      /* Senão, calcula o movimento do monstro numa direção aleatória */
      else {
        AlteraPosicaoDestino(&dj->monstros[i].pos, &dj->monstros[i].direcao,
                             GetRandomValue(1, 4), dj);
      }

      /* Executa de fato o movimento do monstro */
      MovimentaEntidade(&dj->monstros[i].pos, dj->monstros[i].direcao,
                        dj->monstros[i].velocidade, delta);
      /* Desenha a textura do monstro na tela */
      DrawTexture(texturas.monstro_texturas[dj->monstros[i].direcao - 1],
                  dj->monstros[i].pos.atual.x, dj->monstros[i].pos.atual.y,
                  WHITE);
    }
  }
}

/* Função que recebe uma estrutura MAPA de dados e executa a maior parte das
 * funções do jogo */
int jogo(MAPA *dj) {
  int i;
  float delta;
  double cooldownColisao = 0.0;
  TEXTURA texturas;
  Rectangle ret_link;

  /* Carrega as principais texturas do jogo */
  carregaTexturas(&texturas);

  /* Loop enquanto a janela do jogo estiver aberta */
  while (!WindowShouldClose()) {
    /* Atualiza variáveis dentro do loop */
    delta = GetFrameTime();

    ret_link = (Rectangle){
      x : dj->jogador.pos.atual.x,
      y : dj->jogador.pos.atual.y,
      width : QUADRADO,
      height : QUADRADO
    };

    /* Toca música ambiente */
    if (!IsSoundPlaying(texturas.som_ambiente)) {
      SetSoundVolume(texturas.som_ambiente, 0.4);
      PlaySound(texturas.som_ambiente);
    }

    /* Se o cooldown da espada é maior do que 0, decrementa ele com o tempo
     * passado entre frames */
    if (dj->jogador.cooldown_espada > 0) dj->jogador.cooldown_espada -= delta;

    /* Se o cooldown da colisão é maior do que zero, decrementa ele com o tempo
     * passado entre frames */
    if (cooldownColisao > 0) cooldownColisao -= delta;

    /* Busca input para alternar tela cheia */
    checaTelaCheia();

    /* Procura inputs de movimentação ou salvamento da fase */
    TraduzInputJogador(&dj->jogador.pos, &dj->jogador.direcao, dj);

    /* Movimenta a posição do jogador se necessário */
    MovimentaEntidade(&dj->jogador.pos, dj->jogador.direcao,
                      dj->jogador.velocidade, delta);

    /* Movimenta a posição dos monstros */
    movimentaMonstro(texturas, cooldownColisao, delta, dj);

    /* Varre o array de monstros para detectar se houve alguma colisão entre
     * eles e o jogador */
    for (i = 0; i < dj->quant_inicial_monstros; i++) {
      if (dj->monstros[i].vivo) {
        if (CheckCollisionRecs(
                ret_link,
                (Rectangle){dj->monstros[i].pos.atual.x,
                            dj->monstros[i].pos.atual.y, QUADRADO, QUADRADO}) &&
            cooldownColisao <= 0) {
          /* Se positivo e o cooldown de colisão não estiver ativo, diminui o
           * número de vidas do jogador e estabelece o cooldown de colisão */
          dj->jogador.nro_vidas--;
          cooldownColisao = COOLDOWN_COLISAO;
          if (!IsSoundPlaying(texturas.som_hit)) {
            PlaySound(texturas.som_hit);
          }
        }
      }
    }

    /* Início do Desenho */
    BeginDrawing();
    ClearBackground((Color){253, 217, 169, 255});
    DrawRectangle(LARGURA_TELA, 0, 300, ALTURA_TELA, BLACK);

    /* Desenha os obstáculos */
    for (int i = 0; i < dj->quant_obstaculos; i++) {
      DrawTexture(texturas.pedra, dj->obstaculos[i].x, dj->obstaculos[i].y,
                  WHITE);
    }

    /* Desenha o sprite do jogador */
    DrawTexture(texturas.link_texturas[dj->jogador.direcao - 1],
                dj->jogador.pos.atual.x, dj->jogador.pos.atual.y, WHITE);

    /* Após ser atingido, o sprite do Link fica azul enquanto ele estiver
     * invunerável */
    if (cooldownColisao > 0)
      DrawTexture(texturas.link_azul_texturas[dj->jogador.direcao - 1],
                  dj->jogador.pos.atual.x, dj->jogador.pos.atual.y, WHITE);

    /* Procura o input de ataque da espada, processa-o e desenha o seu sprite,
     * se necessário */
    AtaqueEspada(texturas, delta, dj);

    /* Elementos da bara de status */
    DrawText(TextFormat("Vidas: %d", dj->jogador.nro_vidas), 10, 0, 25, VIOLET);
    DrawText(TextFormat("Fase: %d", dj->fase), 140, 0, 25, VIOLET);
    DrawText(TextFormat("Pontos: %d", dj->pontuacao), 250, 0, 25, VIOLET);
    DrawText("F11: Fullscreen", 700, 0, 25, VIOLET);
    DrawText("L: Salvar Jogo", 1000, 0, 25, VIOLET);

    EndDrawing();
    /* Fim do desenho */

    /* Condições de saída */
    /* Se todos os monstros foram mortos, sobe de fase e retorna 1 */
    if (dj->quant_monstros <= 0) {
      PlaySound(texturas.som_levelup);
      dj->fase++;
      StopSound(texturas.som_ambiente);
      return 1;
    }

    /* Se todas as vidas foram perdidas, retorna 0 */
    if (dj->jogador.nro_vidas <= 0) {
      StopSound(texturas.som_ambiente);
      return 0;
    }
  }
  StopSound(texturas.som_ambiente);
  return -1;
}

/* Recebe uma estrutura RECORDE, abre uma tela com um campo para digitar um
 * nome, e armazena esse nome na estrutura*/
int perguntaNome(RECORDE *jogador) {
  int tecla, i = 0;
  sprintf(jogador->nome, "                             ");

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    DrawText("Insira seu nome ", 385, 250, 50, WHITE);
    tecla = GetCharPressed();
    if (tecla != 0) {
      jogador->nome[i] = tecla;
      i++;
    }
    DrawText(jogador->nome, 385, 500, 50, WHITE);

    EndDrawing();

    if (IsKeyPressed(KEY_ENTER)) return 1;
  }
  return 0;
}

/* Se há colisão atualmente entre o retângulo que representa o botão do menu e o
mouse, devolve a cor verde. Senão, devolve a cor vermelha. */
Color defineCorBotaoDerrota(Rectangle botao) {
  Color cor;
  if (CheckCollisionPointRec(GetMousePosition(), botao))
    cor = GREEN;
  else
    cor = RED;

  return cor;
}

/* Abre uma tela de fim de jogo, dando a opção do jogador continuar ou sair. No
 * primeiro caso, retorna 1, no segundo fecha a janela */
int gameOver(void) {
  int escolha = 0;
  Sound tema = LoadSound("assets/sons/derrota.mp3");
  Rectangle botaoTENTAR = {330, 610, 565, ALTURA_BOTAO};
  Rectangle botaoSAIR = {555, 710, 300, ALTURA_BOTAO};

  Texture2D tela = LoadTexture("assets/sprites/Tela_derrota.png");

  PlaySound(tema);

  while (!WindowShouldClose()) {
    checaTelaCheia();
    BeginDrawing();
    ClearBackground(PESSEGO);
    DrawRectangle(LARGURA_TELA, 0, 300, ALTURA_TELA, BLACK);

    DrawTextureEx(tela, (Vector2){x : 0, y : 0}, 0, 2, RAYWHITE);

    DrawText("VOCÊ PERDEU", 228, 50, 100, RED);

    DrawText("TENTAR NOVAMENTE", botaoTENTAR.x, botaoTENTAR.y, 50,
             defineCorBotaoDerrota(botaoTENTAR));
    if (botaoClicado(botaoTENTAR)) {
      escolha = 1;
      return escolha;
    }

    DrawText("SAIR", botaoSAIR.x, botaoSAIR.y, 50,
             defineCorBotaoDerrota(botaoSAIR));
    if (botaoClicado(botaoSAIR)) {
      CloseAudioDevice();
      CloseWindow();
    }
    EndDrawing();
  }
  CloseWindow();
  return 0;
}

/* Se há colisão atualmente entre o retângulo que representa o botão do menu e o
mouse, devolve a cor verde. Senão, devolve a cor ciano. */
Color defineCorBotaoVitoria(Rectangle botao) {
  Color cor;
  if (CheckCollisionPointRec(GetMousePosition(), botao))
    cor = GREEN;
  else
    cor = CIANO;

  return cor;
}

/* Abre uma tela de vitória, dando a opção do jogador continuar ou sair. No
 * primeiro caso, retorna 1, no segundo fecha a janela */
int vitoria(void) {
  int escolha = 0;
  Rectangle botaoCONTINUAR = {750, 510, 300, ALTURA_BOTAO};
  Rectangle botaoSAIR = {750, 610, 120, ALTURA_BOTAO};
  Sound tema = LoadSound("assets/sons/vitoria.mp3");
  Texture2D tela = LoadTexture("assets/sprites/Tela_vitoria.png");

  while (!WindowShouldClose()) {
    if (!IsSoundPlaying(tema)) {
      PlaySound(tema);
    }

    checaTelaCheia();
    BeginDrawing();
    ClearBackground(PESSEGO);

    DrawRectangle(LARGURA_TELA, 0, 300, ALTURA_TELA, BLACK);

    DrawTextureEx(tela, (Vector2){x : 0, y : 0}, 0, 1, RAYWHITE);

    DrawText("PARABÉNS", 340, 50, 100, CIANO);

    DrawText("CONTINUAR", botaoCONTINUAR.x, botaoCONTINUAR.y, 50,
             defineCorBotaoVitoria(botaoCONTINUAR));
    if (botaoClicado(botaoCONTINUAR)) {
      escolha = 1;
      StopSound(tema);
      return escolha;
    }

    DrawText("SAIR", botaoSAIR.x, botaoSAIR.y, 50,
             defineCorBotaoVitoria(botaoSAIR));
    if (botaoClicado(botaoSAIR)) {
      CloseAudioDevice();
      CloseWindow();
    }

    EndDrawing();
  }

  CloseWindow();
  return escolha;
}

/* Função principal */
int main(void) {
  MAPA dados_jogo;
  char grid[LINHAS][COLUNAS];
  int resultado = 1;
  int escolha = 0;
  RECORDE top5[5];
  RECORDE atual;
  int posicao_pontuacao;

  /* Lê o arquivo de recordes e armazena no vetor top5 */
  if (!(le_arquivo_recordes(top5))) {
    puts("Erro na leitura do arquivo de recordes!");
  };

  IniciaJanela();

  /* Enquanto a escolha não for sair (4) */
  while (escolha != 4) {
    /* Carrega o menu e atribui o valor devolvido a 'escolha' */
    escolha = menu();

    /* JOGAR */
    if (escolha == 1) {
      /* Coloca a fase atual como 1, lê o arquivo correspondente, monta o mapa e
       * zera a pontuação */
      dados_jogo.fase = 1;
      leArquivoNivel(grid, dados_jogo.fase);
      montaMapa(grid, &dados_jogo);
      dados_jogo.pontuacao = 0;
    }

    /* CARREGAR */
    if (escolha == 2) {
      /* Carrega a estrutura MAPA de um arquivo binário e sobrescreve os dados
       * do jogo atuais */
      carregaJogo(&dados_jogo);
    }

    /* JOGAR/CARREGAR */
    if (escolha == 1 || escolha == 2) {
      do {
        /* Inicializa o jogo e atribui o valor retornado a 'resultado' */
        resultado = jogo(&dados_jogo);

        /* Se a fase atual é superior à fase máxima, */
        if (dados_jogo.fase > NUM_FASES) {
          /* Verifica se a pontuação atual é superior para entrar no top 5 */
          posicao_pontuacao = ehPontuaçãoRecorde(top5, dados_jogo.pontuacao);
          /* Se sim, coloca a pontuação na posição correta */
          if (posicao_pontuacao < 6) {
            perguntaNome(&atual);
            atual.pontos = dados_jogo.pontuacao;
            insereRecorde(posicao_pontuacao, atual, top5);
            salva_arquivo(top5);
          }
          /* Chama a função da tela de vitória e coloca o resultado em 'escolha'
           */
          resultado = -1;
          escolha = vitoria();
        }

        else if (resultado == 1) {
          leArquivoNivel(grid, dados_jogo.fase);
          montaMapa(grid, &dados_jogo);
        }

        else if (resultado == 0) {
          /* Verifica se a pontuação atual é superior para entrar no top 5 */
          if ((posicao_pontuacao =
                   ehPontuaçãoRecorde(top5, dados_jogo.pontuacao)) < 6) {
            /* Se sim, coloca a pontuação na posição correta */
            perguntaNome(&atual);
            atual.pontos = dados_jogo.pontuacao;
            insereRecorde(posicao_pontuacao, atual, top5);
            salva_arquivo(top5);
          }
          /* Chama a função da tela de fim de jogo e coloca o resultado em
           * 'escolha'
           */
          escolha = gameOver();
        }
      } while (resultado == 1);
      /* Repete esse loop enquanto o resultado for 1, ou seja, o jogador avançou
       * de fase */
    }
    /* MOSTRA RECORDES */
    if (escolha == 3) escolha = mostraRecordes(top5);
  }
  CloseAudioDevice();
  CloseWindow();

  return 0;
}

// Rodrigo Grilli
// Gabriel Henrique Fiszczuk Brandeburski
