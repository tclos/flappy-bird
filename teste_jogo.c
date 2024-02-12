#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "raylib.h"

#define ALTURA_TELA 800
#define LARGURA_TELA 1200
#define NUM_OBSTACULOS 4
#define MAX_INPUT_CHARS 40 

typedef enum GameScreen { MENU, GAMEPLAY, DIFICULDADE_MENU, RANKING } GameScreen;

typedef struct {
    Vector2 posicao;
    float tamanho;
} Jogador;

typedef struct {
    Vector2 posicao;
    Vector2 tamanho;
    int passou;
} Obstaculo;

typedef struct tipo_score {
    char nome[40];
    int score;
} TIPO_SCORE;


float velocidade_jogador = 0, aceleracao_jogador = 0.5;           // velocidade e aceleracao com que o jogador cai
int vel_obstaculos = 10, inc_vel_obstaculos = 1;                  // velocidade com que os obstaculos vao em direcao ao jogador
int score = 0, score_var = 0;                                     // score_var temporario para fazer o calculo do incremento de dificuldade
int gap = 300, dec_gap = 5;
int largura_obstaculos = 100;
int distanciaProximoObstaculo = 400;                              // distanciia entre os obstaculos
int dif_max_altura = 50, inc_dif_max_altura;
int score_threshold = 400;
int dificuldade = 1;                                              // 0 - facil, 1 - medio, 2 - dificil, o jogo comeca no medio
GameScreen tela_atual = MENU;                                     // Indica que o jogo comeca na tela do Menu

// Funcao de inicializacao da janela, tamanho da janela, quadros por segundo e posição da janela
void InicializarJogo(){
    
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Flappy Bird");
    SetWindowPosition((GetMonitorWidth(0) - LARGURA_TELA)/2, 10);                   // posiciona a janela para melhor visualização do jogo
    SetTargetFPS(60);                                                               // 60 frames por segundo

}

// Funcao para desenhar botoes de acordo com as posicoes passadas e com o texto do botao
int DesenharBotao(int posicao_x, int posicao_y, char nome_botao[]){
    
    if (MouseEstaSobreBotao((Vector2){posicao_x, posicao_y - 40}, (Vector2){200, 50})) {
        
        DrawRectangle(posicao_x, posicao_y, 200, 50, LIME);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
             return 1;                                     // se o mouse estiver em cima do botao e ele for clicado a funcao retorna 1
             
    } else
        DrawRectangle(posicao_x, posicao_y, 200, 50, GREEN);
        
    DrawText(nome_botao, posicao_x + 50, posicao_y + 15, 20, WHITE);
    return 0;  
}

void DesenharInputBox(int *count_letras, char nome[]){
    
    Rectangle BoxInputNome = { LARGURA_TELA-500, 10, 490, 25 };                     // inicialiazao do retangulo do input do username
    
    if (CheckCollisionPointRec(GetMousePosition(), BoxInputNome)){    // Checa se o ponteiro do mouse esta sobre a caixa de input do nome
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
                    
        int tecla = GetCharPressed();                    // Captura a tecla digitada, se for mais de uma em 1 frame, adiciona em uma fila
                    
        while (tecla > 0){ // Checa se mais caracteres foram digitados no mesmo frame, quando nao tiver mais caracteres na fila, tecla = 0
            
            // Limita teclas entre 32 e 125 em unicode e limita a quantidade de caracteres no username
            if ((tecla >= 32) && (tecla <= 125) && (*count_letras < MAX_INPUT_CHARS)){ 
            
                nome[*count_letras] = (char)tecla;                           // Adiciona o caracter na string do nome do jogador
                (*count_letras)++;                                          // incrementa a quantidade de caracteres no momento 
                nome[(*count_letras)] = '\0';                               // Adiciona o finalizador de string 
            }

            tecla = GetCharPressed();                                       // Checa o proximo caractere na fila
        }

        if (IsKeyPressed(KEY_BACKSPACE) && (*count_letras > 0)){            // Checa se o usuaria esta apagando caracteres
                    
            (*count_letras)--;                                              // numero de caracteres no username diminui
            nome[(*count_letras)] = '\0';                       // poe o finalizador de string para desconsiderar os caracteres "apagados"
        }
        
        DrawRectangle(BoxInputNome.x, BoxInputNome.y+40, BoxInputNome.width, BoxInputNome.height, LIGHTGRAY);
        
    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);                               // Se o mouse nao estivera acima do input, cursor fica normal
        DrawRectangle(BoxInputNome.x, BoxInputNome.y+40, BoxInputNome.width, BoxInputNome.height, WHITE);
    }              
    
    DrawRectangleLines(BoxInputNome.x, BoxInputNome.y+40, BoxInputNome.width, BoxInputNome.height, GRAY);// Contorno da caixa de input
    DrawText(nome, BoxInputNome.x + 5, BoxInputNome.y + 45, 20, BLACK); // mostra o nome que está sendo digitado e armazenado na string nome
}

// Função para ler o arquivo txt contendo o ranking
void LerRanking(TIPO_SCORE ranking[5]){
    
    FILE *arq;
    arq = fopen("ranking.txt", "r");
    
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
void AtualizarRanking(TIPO_SCORE ranking[5], char nome[]){
    
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
    arq = fopen("ranking.txt", "w");
    
    if (arq != NULL) {
        for (int i = 0; i<5; i++){
            
            fprintf(arq, "%s\n", ranking[i].nome);                 // Escrevendo primeiro o nome e depois o score
            fprintf(arq, "%d\n", ranking[i].score);
        }
        
        fclose(arq);
    }
}

// Funcao para verificar se o mouse esta em cima de um botao (retangulo), os argumentos passados são a posicao e tamanho do botao
int MouseEstaSobreBotao(Vector2 posicao, Vector2 tamanho) {
    Vector2 mouse = GetMousePosition();                                             // pega as coordenadas atuais do mouse
    return CheckCollisionPointRec(mouse, (Rectangle){posicao.x, posicao.y, tamanho.x, tamanho.y});
}

// Função para ler os arquivos binarios e ler os parametros de dificuldade
void LerDificuldade(int dificuldade){
    FILE *arq;
    
    switch (dificuldade){                                          // dificuldade padrao é 1 (médio)
        case 0: 
            arq = fopen("facil.bin", "rb");
            break;
        case 1:
            arq = fopen("medio.bin", "rb");
            break;
        case 2:
            arq = fopen("dificil.bin", "rb");
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

int CarregarObstaculos(Obstaculo obstaculos_baixo[], Obstaculo obstaculos_cima[]){
    int altura_obstaculos = GetRandomValue(300, 700);//GetRandomValue(10, ALTURA_TELA - gap - 10);

    for (int i = 0; i < NUM_OBSTACULOS; i++) {
        /////////////////////////////////////////////////////// FIX THIS
        altura_obstaculos += GetRandomValue(-dif_max_altura, dif_max_altura);
        
        if (altura_obstaculos >= ALTURA_TELA - gap - 100){
            altura_obstaculos = ALTURA_TELA - gap - 100;//GetRandomValue(ALTURA_TELA - gap - dif_max_altura - 10, ALTURA_TELA - gap - 10);
        }
        else if (altura_obstaculos <= 50){
            altura_obstaculos = 50;//GetRandomValue(10, altura_obstaculos + dif_max_altura);
        }

        obstaculos_cima[i].posicao.x = LARGURA_TELA + i * distanciaProximoObstaculo;
        obstaculos_cima[i].posicao.y = 0;
        obstaculos_cima[i].tamanho.x = largura_obstaculos;
        obstaculos_cima[i].tamanho.y = ALTURA_TELA - altura_obstaculos - gap;

        obstaculos_baixo[i].posicao.x = LARGURA_TELA + i * distanciaProximoObstaculo;
        obstaculos_baixo[i].posicao.y = ALTURA_TELA - altura_obstaculos;
        obstaculos_baixo[i].tamanho.x = largura_obstaculos;
        obstaculos_baixo[i].tamanho.y = altura_obstaculos;
        obstaculos_baixo[i].passou = 0;
    }
    
    return altura_obstaculos;
}

// Funcao para atualizar a posicao e velocidade do jogador e para colocar a imagem do passaro no jogador
void AtualizarJogador(float *velocidade_jogador, float aceleracao_jogador, Jogador *jogador, Texture2D textura) {
    
    DrawCircleV((*jogador).posicao, (*jogador).tamanho, BLANK);                          // Desenha a hitbox do jogador (circulo)
    
    float escala = 2.5f * (*jogador).tamanho / fmax(textura.width, textura.height);      //calculo do tamanho que a imagem tem que ter
    
    // Atualizar a posicao da imagem (textura) passada como argumento com base na posicao do jogador
    DrawTextureEx(textura, (Vector2){(*jogador).posicao.x -(*jogador).tamanho, (*jogador).posicao.y- (*jogador).tamanho}, 0, escala, WHITE);
    
    if (IsKeyPressed(KEY_SPACE))                                    // Se o botao espaco for apertado, o jogador é impulsionado para cima
        *velocidade_jogador = -10;

    *velocidade_jogador += aceleracao_jogador;                      // Atualizar a velocidade e poosicao do jogador
    (*jogador).posicao.y += *velocidade_jogador;
}

// Funcao para incrementar a dificuldade quando o score passar do score_threshold **** Adicionar incremento de dif_max_altura
void AumentarDificuldade(){
    
    if (score_var >= score_threshold){        // score_var é uma variavel temporaria que é incrementada igualmente em relacao a score
    
        vel_obstaculos += inc_vel_obstaculos;                 // incremento da velocidade do cenario
        
        if (gap - dec_gap >= 100)
            gap -= dec_gap;                         // gap entre obstaculos diminui até 100, distancia minima para continuar jogavel

        if (dif_max_altura <= 500)
            dif_max_altura += inc_dif_max_altura;  // diferenca maxima de altura aumenta até 500, distancia maxima para continuar jogavel
        
        score_var = 0;                                       // score_var é zerado toda vez que o score passar do score_threshold
    }  
}

int AtualizarJogo(Obstaculo obstaculos_baixo[], Obstaculo obstaculos_cima[], Jogador *jogador, GameScreen *currentScreen, int *altura_obstaculos, Texture2D textura_obs, Texture2D textura_obsinv){

    for (int i = 0; i < NUM_OBSTACULOS; i++) {
        
        obstaculos_cima[i].posicao.x -= vel_obstaculos;
        obstaculos_baixo[i].posicao.x -= vel_obstaculos;

            // Check if obstacle passed the player and hasn't been scored yet
        if (obstaculos_baixo[i].posicao.x + largura_obstaculos < (*jogador).posicao.x && !obstaculos_baixo[i].passou) {
            score += 50;
            score_var += 50;
            obstaculos_baixo[i].passou = 1;
        }
        
        /////////////////////////////////////////////////////// TODO

        *altura_obstaculos += GetRandomValue(-dif_max_altura, dif_max_altura);
        
        if (*altura_obstaculos >= ALTURA_TELA - gap - 100){
            *altura_obstaculos = ALTURA_TELA - gap - 100;
        }
        else if (*altura_obstaculos <= 50){
            *altura_obstaculos = 50;
        }
        ///////////////////////////////////////////////////////

        
            // Check if obstacle went off the screen, reset its position and reset passou flag
        if (obstaculos_baixo[i].posicao.x + largura_obstaculos <= 0) {
            obstaculos_cima[i].posicao.x = LARGURA_TELA + distanciaProximoObstaculo - largura_obstaculos;
            obstaculos_cima[i].posicao.y = 0;
            obstaculos_cima[i].tamanho.y = ALTURA_TELA - *altura_obstaculos - gap;//altura_obstaculos - gap / 2;

            obstaculos_baixo[i].posicao.x = LARGURA_TELA + distanciaProximoObstaculo - largura_obstaculos;
            obstaculos_baixo[i].posicao.y = ALTURA_TELA - *altura_obstaculos;//altura_obstaculos + gap;
            obstaculos_baixo[i].tamanho.y = *altura_obstaculos;//ALTURA_TELA - altura_obstaculos - gap;

            obstaculos_baixo[i].passou = 0;
        }
            
        if (CheckCollisionCircleRec((*jogador).posicao, (*jogador).tamanho, (Rectangle){obstaculos_baixo[i].posicao.x, obstaculos_baixo[i].posicao.y, obstaculos_baixo[i].tamanho.x, obstaculos_baixo[i].tamanho.y}) ||
            CheckCollisionCircleRec((*jogador).posicao, (*jogador).tamanho, (Rectangle){obstaculos_cima[i].posicao.x, obstaculos_cima[i].posicao.y, obstaculos_cima[i].tamanho.x, obstaculos_cima[i].tamanho.y})) {
            DrawText(TextFormat("Collision!"), 500, 500, 40, BLACK); 
            tela_atual = MENU;
            return 1;
        }
        
        if((*jogador).posicao.y >= ALTURA_TELA){
            tela_atual = MENU;
            return 1;
        }

        DrawRectangleV(obstaculos_cima[i].posicao, obstaculos_cima[i].tamanho, BLANK);
        DrawTexture(textura_obsinv, obstaculos_cima[i].posicao.x, obstaculos_cima[i].tamanho.y - ALTURA_TELA+100, WHITE);
        DrawRectangleV(obstaculos_baixo[i].posicao, obstaculos_baixo[i].tamanho, BLANK);  
        DrawTexture(textura_obs, obstaculos_baixo[i].posicao.x, obstaculos_baixo[i].posicao.y, WHITE);
    }

    return 0;
}

int main() {

    InicializarJogo();

    // Inicializacao
    char dificuldade_selecionada[10] = "Medio";               // string padrão de display no menu de dificuldade
    int altura_inicial;                                       // Variavel inicial para armazenar o valor retornado por CarregarObstaculos
    char nome[MAX_INPUT_CHARS + 1] = "\0";                    // inicializa o array que ira armazenar o nome do jogador
    int count_letras = 0;                                     // contador de caracteres digitas no input do nome
    float backgroundX = 0;                                    // posicao inicial do background
    float backgroundScrollSpeed = 3.0;                        //velocidade do background
    
    Jogador jogador = {(Vector2){100, 300}, 20};              // inicia o jogador em x=100, y=300 e tamanho (raio) = 20
    TIPO_SCORE ranking[5];                                    // inicializando o array de scores
    Obstaculo obstaculos_baixo[NUM_OBSTACULOS];               // cria o array que armazenará os obstaculos de baixo
    Obstaculo obstaculos_cima[NUM_OBSTACULOS];                // cria o array que armazenará os obstaculos de cima
    
    
        
    Image backgroundImage = LoadImage("resources/background.png");
    Texture2D backgroundTexture = LoadTextureFromImage(backgroundImage);                   // Carregando o background do jogo
    UnloadImage(backgroundImage);
        
    Image passaro = LoadImage("resources/passaro.png");
    Texture2D textura_passaro = LoadTextureFromImage(passaro);                             // Carregando a imagem do passaro
    UnloadImage(passaro);
        
    Image obstaculo = LoadImage("resources/obstaculo.png");
    Texture2D textura_obs = LoadTextureFromImage(obstaculo);                               // Carregando a do obstac. inferior 
    ImageFlipVertical(&obstaculo);                                                         // invertendo para obter o obst sup              
    Texture2D textura_obsinv = LoadTextureFromImage(obstaculo);
    UnloadImage(obstaculo);
        
    Image logo = LoadImage("resources/logo.png");                         
    Texture2D textura_logo = LoadTextureFromImage(logo);                                   // Carregando o logo do flappyinf
    UnloadImage(logo);
    
    
    LerRanking(ranking);                                                // lendo o ranking.txt e armazenando no array ranking

    while (!WindowShouldClose()) {
        BeginDrawing();

        backgroundX -= backgroundScrollSpeed;                           // Atualizar a posicao do background
        
        if (backgroundX <= -backgroundTexture.width)                    // Resetar a parte do background quando ela sair da tela
            backgroundX = 0;
        DrawTexture(backgroundTexture, backgroundX, 0, WHITE);
        DrawTexture(backgroundTexture, backgroundX + backgroundTexture.width, 0, WHITE);     

        switch (tela_atual) {
            
            case MENU: {
                DrawTexture(textura_logo, LARGURA_TELA/2-200, 200, WHITE); // Logo flappyinf
            
                // Botao Jogar
                if(DesenharBotao(LARGURA_TELA/2 - 100, ALTURA_TELA/2-50, "Jogar")){
                    jogador.posicao.y = 300;                                        // reseta a posicao do jogador
                    velocidade_jogador = 0;                                         // reseta a velocidade
                    LerDificuldade(dificuldade);                                    // Lê o arquivo bin conforme a dificuldade selecionada
                    tela_atual = GAMEPLAY;                                          // muda a tela do menu para o jogo em si
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
                
                DesenharInputBox(&count_letras, nome);                               // Desenha a caixa de entrada do nome do jogador
                
            } break;
            
            case DIFICULDADE_MENU: {
                
                DrawText(TextFormat("Dificuldade: %s", dificuldade_selecionada), LARGURA_TELA/2 - 150, 200, 40, BLACK); //mostra a dif atual
                
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
                
                DrawText("Nome", LARGURA_TELA/4, 100, 50, BLACK);                   // Cabeçalhos 
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
            
            case GAMEPLAY: {                                 // Jogo em si
                
                AtualizarJogador(&velocidade_jogador, aceleracao_jogador, &jogador, textura_passaro); // Atualiza a posição do jogador
                // Aumenta a dificuldade quando o score passa o score_threshold. Os parametros sao dados pelos arquivos binarios lidos
                AumentarDificuldade();
                             
                // AtualizarJogo atualiza os obstaculos e checa se o jogador colidiu com os obstaculos ou se o jogador esta fora da tela.
                // Retorna 1 se o jogo acabou.
                if (AtualizarJogo(obstaculos_baixo, obstaculos_cima, &jogador, &tela_atual, &altura_inicial, textura_obs, textura_obsinv)){
                    
                    AtualizarRanking(ranking, nome); 
                    score = 0;                           // Se o jogo tiver acabado, atualiza o ranking, zera o score e o score temporario
                    score_var = 0;
                }
            
                DrawText(TextFormat("Score: %d", score), 10, 50, 40, LIGHTGRAY);                              // display do score

            } break;
        }
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
