#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "raylib.h"

#define ALTURA_TELA 800
#define LARGURA_TELA 1200
#define NUM_OBSTACULOS 4
#define MAX_INPUT_CHARS 40 

typedef enum GameScreen { MENU, GAMEPLAY, DIFICULDADE_MENU, RANKING, SCORE } GameScreen;         // todas diferentes telas do jogo

typedef struct {
    Vector2 posicao;
    float tamanho;
} Jogador;

typedef struct {
    Vector2 posicao;
    Vector2 tamanho;
    int passou;   // indica se o jogador passou do obstaculo
} Obstaculo;

typedef struct tipo_score {
    char nome[40];
    int score;
} TIPO_SCORE;


float aceleracao_jogador = 0.5;                                   // velocidade e aceleracao com que o jogador cai
int largura_obstaculos = 100;
int distanciaProximoObstaculo = 400;                              // distanciia entre os obstaculos
int vel_obstaculos; inc_vel_obstaculos;                           // velocidade com que os obstaculos vao em direcao ao jogador
int gap, dec_gap;                                                 // distancia entre o obstaculo inferior e superior
int dif_max_altura, inc_dif_max_altura;                           // diferenca maxima de altura entre dois obstaculos consecutivos
int score_threshold;                                              // indica a cada quantos pontos a dificuldade é incrementada

GameScreen tela_atual = MENU;                                     // Indica que o jogo comeca na tela do Menu


// Funcao de inicializacao da janela, tamanho da janela, quadros por segundo e posição da janela
void InicializarJanela(){
    
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Flappy Bird");
    InitAudioDevice();
    SetWindowPosition((GetMonitorWidth(0) - LARGURA_TELA)/2, 10);                   // posiciona a janela para melhor visualização do jogo
    SetTargetFPS(60);                                                               // 60 frames por segundo
}


// Funcao para verificar se o mouse esta em cima de um botao (retangulo), os argumentos passados são a posicao e tamanho do botao
int MouseEstaSobreBotao(Vector2 posicao, Vector2 tamanho) {
    
    Vector2 mouse = GetMousePosition();                                             // pega as coordenadas atuais do mouse
    return CheckCollisionPointRec(mouse, (Rectangle){posicao.x, posicao.y, tamanho.x, tamanho.y});
}


// Funcao para desenhar botoes de acordo com as posicoes passadas e com o texto do botao
int DesenharBotao(int posicao_x, int posicao_y, char nome_botao[]){
    
    if (MouseEstaSobreBotao((Vector2){posicao_x, posicao_y - 40}, (Vector2){200, 50})) {
        
        DrawRectangle(posicao_x, posicao_y, 200, 50, LIME);
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
             return 1;                                     // se o mouse estiver em cima do botao e ele for clicado a funcao retorna 1
             
    } else
        DrawRectangle(posicao_x, posicao_y, 200, 50, GREEN);
        
    DrawText(nome_botao, posicao_x + 50, posicao_y + 15, 20, WHITE);          // Desenha o texto no botao
    return 0;  
}


// Funcao armazena caracteres digitados em uma string e a imprime na tela
void DesenharNome(int *count_letras, char nome[]){
    
    int tecla = GetCharPressed();                    // Captura a tecla digitada, se for mais de uma em 1 frame, adiciona em uma fila
                    
    while (tecla > 0){ // Checa se mais caracteres foram digitados no mesmo frame, quando nao tiver mais caracteres na fila, tecla = 0
            
        // Limita teclas entre 32 e 125 em unicode e limita a quantidade de caracteres no username
        if ((tecla >= 32) && (tecla <= 125) && (*count_letras < MAX_INPUT_CHARS)){ 
            
            nome[*count_letras] = (char)tecla;                          // Adiciona o caractere na string do nome do jogador
            (*count_letras)++;                                          // incrementa a quantidade de caracteres no momento 
            nome[(*count_letras)] = '\0';                               // Adiciona o finalizador de string 
        }

        tecla = GetCharPressed();                                       // Checa o proximo caractere na fila
        }

    if (IsKeyPressed(KEY_BACKSPACE) && (*count_letras > 0)){            // Checa se o usuario esta apagando caracteres
                    
        (*count_letras)--;                                              // numero de caracteres no username diminui
        nome[(*count_letras)] = '\0';                       // poe o finalizador de string para desconsiderar os caracteres "apagados"
    }
    
    int x = LARGURA_TELA/2 - MeasureText(nome, 80)/2;                   // Calcula a posicao centralizada para imprimir o nome

    DrawText("HIGHSCORE! NOME:", LARGURA_TELA/2 - MeasureText("HIGHSCORE! NOME:", 80)/2, ALTURA_TELA/2-100, 80, WHITE);
    DrawText(nome, x, ALTURA_TELA/2, 80, BLACK);                        // mostra o nome que está sendo digitado e armazenado na string nome
}


// Funcao move o background
void AtualizarBackground(int *backgroundX, int backgroundScrollSpeed, Texture2D backgroundTexture){
    
    *backgroundX -= backgroundScrollSpeed;                           // Atualizar a posicao do background
        
    if (*backgroundX <= -backgroundTexture.width)                    // Resetar a parte do background quando ela sair da tela
            *backgroundX = 0;
            
    DrawTexture(backgroundTexture, *backgroundX, 0, WHITE);                               // Desenha a textura  do background
    DrawTexture(backgroundTexture, *backgroundX + backgroundTexture.width, 0, WHITE);
}


// Função para ler o arquivo txt contendo o ranking
void LerRanking(TIPO_SCORE ranking[5]){
    
    FILE *arq;
    arq = fopen("txt/ranking.txt", "r");
    
    char linha[40];                                         // variavel para armazenar temporariamente o conteudo lido de cada linha do txt
    
    if (arq != NULL) {
    
        for (int i = 0; i < 5; i++) {
            
            if (fgets(linha, sizeof(linha), arq) != NULL) {                              // Lendo as linhas com nome
                
                for (int j = 0; j < sizeof(linha); j++) {                                // Achando o '\n' e trocando por '\0'
                    
                    if (linha[j] == '\n') {
                        linha[j] = '\0';
                        break;
                    }
                }
                strcpy(ranking[i].nome, linha);                                          // copiando o nome no array de rankings 
                
                fscanf(arq, "%d", &ranking[i].score);                                    // Lendo as linhas com score
                
                fgetc(arq);                                                              // pega o \n depois do score
            }
        }
        fclose(arq);
    }
    OrdenarRanking(ranking);                                                            // Ordena o ranking recem lido
}


// Funcao para ordenar o ranking em order decrescente com base nos scores
void OrdenarRanking(TIPO_SCORE ranking[5]){
    
    // Loop exterior iterando com base no tamanho do array
    for (int i = 0; i < 5 - 1; i++) {
        // Loop interior para cada elemento no array, menos os ultimos i elementos
        for (int j = 0; j < 5 - i - 1; j++) {
            
            if (ranking[j].score < ranking[j + 1].score) {                          // Comparando o score de j com o proximo elemento
                
                TIPO_SCORE temp = ranking[j]; 
                ranking[j] = ranking[j + 1];      // Trocar os elementos se o score do elemento atual for menor que o do proximo elemento
                ranking[j + 1] = temp; 
            }                                       // Com isso os maiores valores de score vão ficar em primeiro
        }
    }
}


// Funcao para verificar se um score é maior que algum score do ranking, atualizar o ranking, ordenar e escrever no arquivo txt
void AtualizarRanking(TIPO_SCORE ranking[5], char nome[], int score){
    
    int menor_score = INT_MAX, idx_menor_score;
    
    for(int i=0; i < 5; i++){
     
        if (ranking[i].score < menor_score){                                // Achar o menor score em Ranking e salvando o seu indice
        
            menor_score = ranking[i].score;
            idx_menor_score = i;
        }
    }
    
    TIPO_SCORE jogador;                                                     // Armazenar o nome e score do jogador em uma estrutura
    
    strcpy(jogador.nome, nome);
    jogador.score = score;
    
    ranking[idx_menor_score] = jogador;                                     // Substituir o menor score

    OrdenarRanking(ranking);                                                // Ordenar o ranking
    
    EscreverRanking(ranking);                                               // Escrever o novo ranking no arquivo txt
}


// Função para escrever o ranking passado como argumento no arquivo ranking.txt
void EscreverRanking(TIPO_SCORE ranking[5]){
    
    FILE *arq;
    arq = fopen("txt/ranking.txt", "w");
    
    if (arq != NULL) {
        for (int i = 0; i<5; i++){
            
            fprintf(arq, "%s\n", ranking[i].nome);                 // Escrevendo primeiro o nome e depois o score
            fprintf(arq, "%d\n", ranking[i].score);
        }
        
        fclose(arq);
    }
}


// Função para ler os arquivos binarios e ler os parametros de dificuldade
void LerDificuldade(int dificuldade){
    FILE *arq;
    
    switch (dificuldade){                                          // dificuldade padrao é 1 (médio)
        case 0: 
            arq = fopen("bin/facil.bin", "rb");
            break;
        case 1:
            arq = fopen("bin/medio.bin", "rb");
            break;
        case 2:
            arq = fopen("bin/dificil.bin", "rb");
            break;
    }
    
    if (arq != NULL) {                                              // Lendo cada valor separadamente e armazenando na var correspondente
        fread(&score_threshold, sizeof(int), 1, arq);
        fread(&gap, sizeof(int), 1, arq);
        fread(&dec_gap, sizeof(int), 1, arq);
        fread(&dif_max_altura, sizeof(int), 1, arq);
        fread(&inc_dif_max_altura, sizeof(int), 1, arq);
        fread(&vel_obstaculos, sizeof(int), 1, arq);
        fread(&inc_vel_obstaculos, sizeof(int), 1, arq);

        fclose(arq);
    } 
}


// Funcao para incrementar a dificuldade quando o score passar do score_threshold **** Adicionar incremento de dif_max_altura
void AumentarDificuldade(int *score_var){
    
    if (*score_var >= score_threshold){        // score_var é uma variavel temporaria que é incrementada igualmente em relacao a score
    
        vel_obstaculos += inc_vel_obstaculos;                 // incremento da velocidade do cenario
        
        if (gap - dec_gap >= 100)
            gap -= dec_gap;                         // gap entre obstaculos diminui até 100, distancia minima para continuar jogavel

        if (dif_max_altura <= 500)
            dif_max_altura += inc_dif_max_altura;  // diferenca maxima de altura aumenta até 500, distancia maxima para continuar jogavel
        
        *score_var = 0;                                       // score_var é zerado toda vez que o score passar do score_threshold
    }  
}


// Funcao para inicializar os obstaculos e armazena-los em um array vazio. Retorna a ultima altura dos obstaculos gerada
int CarregarObstaculos(Obstaculo obstaculos_baixo[], Obstaculo obstaculos_cima[]){
    
    int altura_obstaculos = GetRandomValue(100, ALTURA_TELA - 100);                     // inicializa a altura dos objetos com valor random

    for (int i = 0; i < NUM_OBSTACULOS; i++) {
        
        altura_obstaculos += GetRandomValue(-dif_max_altura, dif_max_altura);  // incrementa/decrementa com valor no range de difmaxaltura
        
        // limitando a altura dos obstaculos
        if (altura_obstaculos >= ALTURA_TELA - gap - 100)                      
            altura_obstaculos = ALTURA_TELA - gap - 100;                       
        else if (altura_obstaculos <= 50)
            altura_obstaculos = 50;
      
        
        obstaculos_cima[i].posicao.x = LARGURA_TELA + i * distanciaProximoObstaculo;   // Configurando a posição dos obstaculos de cima
        obstaculos_cima[i].posicao.y = 0;                                                 
        obstaculos_cima[i].tamanho.x = largura_obstaculos;                             // Configurando o tamanho dos obstaculos de cima
        obstaculos_cima[i].tamanho.y = ALTURA_TELA - altura_obstaculos - gap;

        obstaculos_baixo[i].posicao.x = LARGURA_TELA + i * distanciaProximoObstaculo;  // Configurando a posição dos obstaculos de baixo
        obstaculos_baixo[i].posicao.y = ALTURA_TELA - altura_obstaculos;
        obstaculos_baixo[i].tamanho.x = largura_obstaculos;                            // Configurando o tamanho dos obstaculos de baixo
        obstaculos_baixo[i].tamanho.y = altura_obstaculos;
        
        obstaculos_baixo[i].passou = 0;                                                // 'passou' registra se o jogador passou pelo obst.
    }
    
    return altura_obstaculos;                                                          // retorna altura gerada na ultima iteracao
}


// Funcao para atualizar as posições e alturas dos obstaculos. Retorna 1 se deu gameover
void AtualizarJogo(Obstaculo obstaculos_baixo[], Obstaculo obstaculos_cima[], int *altura_obstaculos, Texture2D textura_obs, Texture2D textura_obsinv){

    for (int i = 0; i < NUM_OBSTACULOS; i++) {
        
        obstaculos_cima[i].posicao.x -= vel_obstaculos;                          // Atualiza a posição dos obstaculos
        obstaculos_baixo[i].posicao.x -= vel_obstaculos;
        
        if (obstaculos_baixo[i].posicao.x + largura_obstaculos <= 0) {   // Checa se o obstaculo saiu da tela, reseta a posicao e 'passou'
            
            *altura_obstaculos += GetRandomValue(-dif_max_altura, dif_max_altura);     // calculando a nova altura dos obstaculos
        
            if (*altura_obstaculos >= ALTURA_TELA - gap - 100){
                *altura_obstaculos = ALTURA_TELA - gap - 100;
            }                                                                               // limitando a altura
            else if (*altura_obstaculos <= 50){
                *altura_obstaculos = 50;
            }
            
            obstaculos_cima[i].posicao.x = LARGURA_TELA + distanciaProximoObstaculo - largura_obstaculos;    // Reseta a posicao dos obst
            obstaculos_cima[i].posicao.y = 0;
            obstaculos_cima[i].tamanho.y = ALTURA_TELA - *altura_obstaculos - gap;

            obstaculos_baixo[i].posicao.x = LARGURA_TELA + distanciaProximoObstaculo - largura_obstaculos;
            obstaculos_baixo[i].posicao.y = ALTURA_TELA - *altura_obstaculos;
            obstaculos_baixo[i].tamanho.y = *altura_obstaculos;
            
            obstaculos_baixo[i].passou = 0;                                             // reseta a flag
        }
        
        // Desenhando os obstaculos inferiores e superiores e colocando a textura neles
        DrawRectangleV(obstaculos_cima[i].posicao, obstaculos_cima[i].tamanho, BLANK);
        DrawTexture(textura_obsinv, obstaculos_cima[i].posicao.x, obstaculos_cima[i].tamanho.y - ALTURA_TELA+100, WHITE);
        DrawRectangleV(obstaculos_baixo[i].posicao, obstaculos_baixo[i].tamanho, BLANK);  
        DrawTexture(textura_obs, obstaculos_baixo[i].posicao.x, obstaculos_baixo[i].posicao.y, WHITE);
    }
}


// Funcao checa se houve colisao entre o jogador e os obstaculos ou se o jogador "caiu"
int ChecarGameover(Obstaculo obstaculos_baixo[], Obstaculo obstaculos_cima[], Jogador *jogador){

    for (int i = 0; i < NUM_OBSTACULOS; i++){
        
        // Checa se o jogador bateu no obstaculo inferior
        if (CheckCollisionCircleRec((*jogador).posicao, (*jogador).tamanho, (Rectangle){obstaculos_baixo[i].posicao.x, obstaculos_baixo[i].posicao.y, obstaculos_baixo[i].tamanho.x, obstaculos_baixo[i].tamanho.y}))
            return 1;
            
        // Checa se o jogador bateu no obstaculo superior
        if (CheckCollisionCircleRec((*jogador).posicao, (*jogador).tamanho, (Rectangle){obstaculos_cima[i].posicao.x, obstaculos_cima[i].posicao.y, obstaculos_cima[i].tamanho.x, obstaculos_cima[i].tamanho.y}))
            return 1;

        // Checa se o jogador está fora do alcance inferior da tela 
        if((*jogador).posicao.y >= ALTURA_TELA)                         
            return 1;
    }
    return 0;
}


// Funcao incrementa o score quando o jogador passa completamente pelo obstaculo
void AtualizarScore(Obstaculo obstaculos_baixo[], Jogador jogador, int *score, int *score_var, Sound point){
    for (int i = 0; i < NUM_OBSTACULOS; i++) {

        // Checa se o jogador passou de um obstaculo e incrementa o score, e atualiza a flag passou
        if (obstaculos_baixo[i].posicao.x + largura_obstaculos < jogador.posicao.x && !obstaculos_baixo[i].passou) {
            
            *score += 50;
            *score_var += 50;
            obstaculos_baixo[i].passou = 1;
            
            PlaySound(point);    // Som de ponto
        }
    }
}


// Funcao para atualizar a posicao e velocidade do jogador e para colocar a imagem do passaro no jogador
void AtualizarJogador(float *velocidade_jogador, Jogador *jogador, Texture2D textura, Sound wing) {
    
    DrawCircleV((*jogador).posicao, (*jogador).tamanho, BLANK);                          // Desenha a hitbox do jogador (circulo)
    
    float escala = 2.5f * (*jogador).tamanho / fmax(textura.width, textura.height);      //calculo do tamanho que a imagem tem que ter
    
    // Atualizar a posicao da imagem (textura) passada como argumento com base na posicao do jogador
    DrawTextureEx(textura, (Vector2){(*jogador).posicao.x -(*jogador).tamanho, (*jogador).posicao.y- (*jogador).tamanho}, 0, escala, WHITE);
    
    if (IsKeyPressed(KEY_SPACE)){                                    // Se o botao espaco for apertado, o jogador é impulsionado para cima
        *velocidade_jogador = -10;
        PlaySound(wing);                                             // Som da asa batendo quando o botao for apertado
    }
    
    *velocidade_jogador += aceleracao_jogador;                      // Atualizar a velocidade e poosicao do jogador
    (*jogador).posicao.y += *velocidade_jogador;
}

void IniciarJogo(Jogador *jogador, float *velocidade_jogador, int dificuldade){
    
    (*jogador).posicao.y = 300;                                     // reseta a posicao do jogador
    *velocidade_jogador = 0;                                        // reseta a velocidade
    LerDificuldade(dificuldade);                                    // Lê o arquivo bin conforme a dificuldade selecionada
    tela_atual = GAMEPLAY;                                          // muda a tela do menu para o jogo em si           
}


int main() {

    InicializarJanela();

    // Inicializacao
    char dificuldade_selecionada[10] = "Medio";               // string padrão de display no menu de dificuldade
    int altura_inicial;                                       // Variavel inicial para armazenar o valor retornado por CarregarObstaculos
    int score = 0, score_var = 0;                             // score_var temporario para fazer o calculo do incremento de dificuldade
    int dificuldade = 1;                                      // 0 - facil, 1 - medio, 2 - dificil, o jogo comeca no medio
    float velocidade_jogador = 0;                             // Velocidade vertical
    char nome[MAX_INPUT_CHARS + 1] = "\0";                    // inicializa o array que ira armazenar o nome do jogador
    int count_letras = 0;                                     // contador de caracteres digitas no input do nome
    int backgroundX = 0;                                      // posicao inicial do background
    int backgroundScrollSpeed = 3;                            // velocidade do background
    
    Jogador jogador = {(Vector2){100, 300}, 20};              // inicia o jogador em x=100, y=300 e tamanho (raio) = 20
    TIPO_SCORE ranking[5];                                    // inicializando o array de scores
    Obstaculo obstaculos_baixo[NUM_OBSTACULOS];               // cria o array que armazenará os obstaculos de baixo
    Obstaculo obstaculos_cima[NUM_OBSTACULOS];                // cria o array que armazenará os obstaculos de cima
        
    Image backgroundImage = LoadImage("resources/images/background.png");
    Texture2D backgroundTexture = LoadTextureFromImage(backgroundImage);                   // Carregando o background do jogo
    UnloadImage(backgroundImage);
        
    Image passaro = LoadImage("resources/images/passaro.png");
    Texture2D textura_passaro = LoadTextureFromImage(passaro);                             // Carregando a imagem do passaro
    UnloadImage(passaro);
        
    Image obstaculo = LoadImage("resources/images/obstaculo.png");
    Texture2D textura_obs = LoadTextureFromImage(obstaculo);                               // Carregando a img do obstac. inferior 
    ImageFlipVertical(&obstaculo);                                                         // invertendo para obter o obst sup              
    Texture2D textura_obsinv = LoadTextureFromImage(obstaculo);
    UnloadImage(obstaculo);
        
    Image logo = LoadImage("resources/images/logo.png");                         
    Texture2D textura_logo = LoadTextureFromImage(logo);                                   // Carregando o logo do flappyinf
    UnloadImage(logo);
    
    Sound hit = LoadSound("resources/audio/hit.wav");                                      // Carregando os audios
    Sound wing = LoadSound("resources/audio/wing.wav");
    Sound point = LoadSound("resources/audio/point.wav");    
    
    
    LerRanking(ranking);                                                // lendo o ranking.txt e armazenando no array ranking


    while (!WindowShouldClose()) {
        
        BeginDrawing();
    
        AtualizarBackground(&backgroundX, backgroundScrollSpeed, backgroundTexture);

        switch (tela_atual) {
            
            case MENU: {
                DrawTexture(textura_logo, LARGURA_TELA/2-200, 200, WHITE);          // Logo flappyinf
            
                // Botao Jogar
                if(DesenharBotao(LARGURA_TELA/2 - 100, ALTURA_TELA/2-50, "Jogar")){
                    
                    IniciarJogo(&jogador, &velocidade_jogador, dificuldade);
                    
                    // gera os obstaculos e retorna a altura que será passada como argumento para AtualizarJogo 
                    altura_inicial = CarregarObstaculos(obstaculos_baixo, obstaculos_cima);
                }

               // Botao dificuldade
                if(DesenharBotao(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 10, "Dificuldade"))
                    tela_atual = DIFICULDADE_MENU;
                
                // Botao Ranking
                if(DesenharBotao(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 70, "Ranking"))
                    tela_atual = RANKING;
                
                // Button Sair
                if(DesenharBotao(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 130, "Sair"))              
                    CloseWindow();  
                
            } break;
            
            case DIFICULDADE_MENU: {
                
                DrawText(TextFormat("Dificuldade: %s", dificuldade_selecionada), LARGURA_TELA/2 - 150, 200, 40, WHITE); //mostra a dif atual
                
                //Botao facil
                if(DesenharBotao(LARGURA_TELA/2 - 100, ALTURA_TELA/2 - 50, "Facil")){
                    
                    dificuldade = 0;                                    // Se mouse for apertado, a dificuldade mudara para fácil
                    strcpy(dificuldade_selecionada, "Facil");            // Copia "Facil" para a string de display do menu
                    DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 - 50, 200, 50, LIME);
                }
                
                //Botao medio                  
                if(DesenharBotao(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 10, "Medio")){
                    
                    dificuldade = 1;                                    // Se mouse for apertado, a dificuldade mudara para médio (padrao)
                    strcpy(dificuldade_selecionada, "Medio");           // Copia "Medio" para a string de display do menu
                    DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 10, 200, 50, GREEN);
                }
                
                //Botao dificil
                if(DesenharBotao(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 70, "Dificil")){
                    
                    dificuldade = 2;                                        // Se mouse for apertado, a dificuldade mudara para médio 
                    strcpy(dificuldade_selecionada, "Dificil");             // Copia "Dificil" para a string de display do menu
                    DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 70, 200, 50, LIME);
                }
                
                //Botao voltar
                if(DesenharBotao(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 150, "Voltar"))
                    tela_atual = MENU;                                                          // volta parar o menu
                                   
            } break;
            
            case RANKING: {
                
                DrawText("Nome", LARGURA_TELA/4, 100, 50, BLACK);                           // Cabeçalhos 
                DrawText("Score", 3*LARGURA_TELA/4, 100, 50, BLACK);
                
                // Loop iterando pelo array ranking para mostrar os nomes e scores
                for (int i = 0; i < 5; i++) {
                    DrawText(TextFormat("%s", ranking[i].nome), LARGURA_TELA/4, 200 + i * 75, 30, BLACK);
                    DrawText(TextFormat("%d", ranking[i].score), 3*LARGURA_TELA/4, 200 + i * 75, 30, BLACK);
                }
                
                // Botao voltar
                if(DesenharBotao(LARGURA_TELA/2 - 100, ALTURA_TELA - 100, "Voltar"))
                    tela_atual = MENU;                                                      // volta parar o menu
                
            } break;
            
            case SCORE: {
                
                DrawText(TextFormat("Score: %d", score), 10, 50, 40, LIGHTGRAY);            // display do score
                
                DesenharNome(&count_letras, nome);     // Imprime na tela o texto digitado
                
                if (IsKeyPressed(KEY_ENTER)){          // Se o jogador apertar 'enter', o programa registra o novo scsore
                    
                    AtualizarRanking(ranking, nome, score);
                    score = 0;                         // Se o jogo tiver acabado, atualiza o ranking, zera o score e o score temporario
                    score_var = 0;

                    nome[0] = '\0';                    // reseta a string nome e zera o contador de letras
                    count_letras = 0;
                    
                    tela_atual = MENU;                 // vai pro menu
                }
            } break;
            
            case GAMEPLAY: {   // Jogo
                
                AtualizarJogador(&velocidade_jogador, &jogador, textura_passaro, wing); // Atualiza a posição do jogador
                
                // Aumenta a dificuldade quando o score passa o score_threshold.
                AumentarDificuldade(&score_var);
                             
                // Incrementa o score quando o jogador passa pelo obstaculo
                AtualizarScore(obstaculos_baixo, jogador, &score, &score_var, point);
                
                // Reseta a posicao dos obstaculos e a altura
                AtualizarJogo(obstaculos_baixo, obstaculos_cima, &altura_inicial, textura_obs, textura_obsinv);
                
                // Checa se o jogador colidiu com os obstaculos ou se o jogador esta fora da tela
                if (ChecarGameover(obstaculos_baixo, obstaculos_cima, &jogador)){
                
                    PlaySound(hit);                  // Som quando o jogador perde
                
                    if (score > ranking[4].score)    
                        tela_atual = SCORE;          // Se o score obtido for maior que o menor score do leaderboard, o programa pede o nome
                        
                    else{
                        score = 0;                  //Se nao, zera os scores e vai direto pro menu
                        score_var = 0;
                        tela_atual = MENU;
                    }
                }
                DrawText(TextFormat("Score: %d", score), 10, 50, 40, LIGHTGRAY);          // display do score

            } break;
        }
        
        EndDrawing();
    }
    
    UnloadSound(hit);     // Descarregando os audios
    UnloadSound(point);
    UnloadSound(wing);
    
    CloseAudioDevice(); 
    CloseWindow();
    
    return 0;
}
