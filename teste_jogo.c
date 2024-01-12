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

typedef struct tipo_score
{
    char nome[40];
    int score;
} TIPO_SCORE;


float velocidade_jogador = 0, aceleracao_jogador = 0.5; // velocidade e aceleracao com que o jogador cai
int vel_obstaculos = 10, inc_vel_obstaculos = 1; // velocidade com que os obstaculos vao em direcao ao jogador
int score = 0, score_var = 0; // score_var temporario para fazer o calculo do incremento de dificuldade
int gap = 300, dec_gap = 5;
int largura_obstaculos = 100;
int distanciaProximoObstaculo = 400; // distanciia entre os obstaculos
int dif_max_altura = 50, inc_dif_max_altura;
int score_threshold = 400;
int dificuldade = 1; // 0 - facil, 1 - medio, 2 - dificil, o jogo comeca no medio
GameScreen tela_atual = MENU; // Indica que o jogo comeca na tela do Menu

// Função para ler o arquivo txt contendo o ranking
void LerRanking(TIPO_SCORE ranking[5]){
    FILE *file;
    file = fopen("ranking.txt", "r");
    char line[40];
    
    for (int i = 0; i < 5; i++) {
        // Lendo as linhas com nome
        if (fgets(line, sizeof(line), file) != NULL) {
            // Achando o '\n' e trocando por '\0'
            for (int j = 0; j < sizeof(line); j++) {
                if (line[j] == '\n') {
                    line[j] = '\0';
                    break;
                }
            }

            // copiando o nome no array de rankings 
            strcpy(ranking[i].nome, line);
            
            // Lendo as linhas com score
            if (fgets(line, sizeof(line), file) != NULL) {
                // Armazenando o score no array
                sscanf(line, "%d", &ranking[i].score);
            } 
        }
    }
    fclose(file);
    // Ordenar o ranking
    OrdenarRanking(ranking);
}
// Funcao para ordenar o ranking em order decrescente com base nos scores
void OrdenarRanking(TIPO_SCORE ranking[5]){
    // Loop exterior iterando com base no tamanho do array
    for (int i = 0; i < 5 - 1; i++) {
        // Loop interior para cada elemento no array, menos os ultimos i elementos
        for (int j = 0; j < 5 - i - 1; j++) {
            // Comparando o score de j com o proximo elemento
            if (ranking[j].score < ranking[j + 1].score) {
                // Trocar os elementos se o score do elemento atual for menor que o do proximo elemento
                TIPO_SCORE temp = ranking[j];
                ranking[j] = ranking[j + 1];
                ranking[j + 1] = temp;
                // Com isso os maiores valores de score vão ficar em primeiro
            }
        }
    }
}

// Funcao para verificar se um score é maior que algum score do ranking, atualizar o ranking, ordenar e escrever no arquivo txt
void AtualizarRanking(TIPO_SCORE ranking[5], char nome[]){
    int menor_score = INT_MAX, idx_menor_score;
    
    
    // Achar o menor score em Ranking e salvando o seu indice
    for(int i=0; i < 5; i++){
        if (ranking[i].score < menor_score){
            menor_score = ranking[i].score;
            idx_menor_score = i;
        }
    }
    
    TIPO_SCORE jogador;
    // Armazenar o nome e score do jogador em uma estrutura
    strcpy(jogador.nome, nome);
    jogador.score = score;
    // Substituir o menor score
    ranking[idx_menor_score] = jogador;
    // Ordenar o ranking
    OrdenarRanking(ranking);
    // Escrever o novo ranking no arquivo txt
    EscreverRanking(ranking);
}

// Função para escrever o ranking passado como argumento no arquivo ranking.txt
void EscreverRanking(TIPO_SCORE ranking[5]){
    FILE *file;
    file = fopen("ranking.txt", "w");
    
    for (int i = 0; i<5; i++){
        // Escrevendo primeiro o nome e depois o score
        fprintf(file, "%s\n", ranking[i].nome);
        fprintf(file, "%d\n", ranking[i].score);
    }
    
    fclose(file);
}

// Funcao para verificar se o mouse esta em cima de um botao (retangulo), os argumentos passados são a posicao e tamanho do retangulo
bool MouseEstaSobreBotao(Vector2 posicao, Vector2 tamanho) {
    Vector2 mouse = GetMousePosition();
    return CheckCollisionPointRec(mouse, (Rectangle){posicao.x, posicao.y, tamanho.x, tamanho.y});
}

// Função para ler os arquivos binarios e ler os parametros de dificuldade
void LerDificuldade(int dificuldade){
    FILE *file;
    // dificuldade padrao é 1 (médio)
    switch (dificuldade){
        case 0: 
            file = fopen("easy.bin", "rb");
            break;
        case 1:
            file = fopen("medium.bin", "rb");
            break;
        case 2:
            file = fopen("hard.bin", "rb");
            break;
    }
    fscanf(file, "%d %d %d %d %d %d %d", &score_threshold, &gap, &dec_gap, &dif_max_altura, &inc_dif_max_altura, &vel_obstaculos, &inc_vel_obstaculos);
    
    fclose(file);
}

int CarregarObstaculos(Obstaculo obstaculos_baixo[], Obstaculo obstaculos_cima[]){
    int altura_obstaculos = GetRandomValue(500, 700);//GetRandomValue(10, ALTURA_TELA - gap - 10);

    for (int i = 0; i < NUM_OBSTACULOS; i++) {
        /////////////////////////////////////////////////////// FIX THIS
        altura_obstaculos = GetRandomValue(altura_obstaculos - dif_max_altura, altura_obstaculos + dif_max_altura);
        
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
    // Desenha a hitbox do jogador (circulo)
    DrawCircleV((*jogador).posicao, (*jogador).tamanho, BLANK);
    // Calcula o tamanho que a imagem tem que ter com base no tamanho do circulo determinado por jogador.tamanho
    float scaleFactor = 2.5f * (*jogador).tamanho / fmax(textura.width, textura.height);
    // Atualizar a posicao da imagem (textura) passada como argumento com base na posicao do jogador
    DrawTextureEx(textura, (Vector2){(*jogador).posicao.x - (*jogador).tamanho, (*jogador).posicao.y - (*jogador).tamanho}, 0, scaleFactor, WHITE);
    ////EndShaderMode();
    
    // Se o botao espaco for apertado, o jogador é impulsionado para cima
    if (IsKeyPressed(KEY_SPACE)) {
        *velocidade_jogador = -10;
    }
    // Atualizar a velocidade e poosicao do jogador
    *velocidade_jogador += aceleracao_jogador;
    (*jogador).posicao.y += *velocidade_jogador;

}
// Funcao para incrementar a dificuldade quando o score passar do score_threshold **** Adicionar incremento de dif_max_altura
void AumentarDificuldade(){
    // score_var é uma variavel temporaria que é incrementada igualmente em relacao a score
    if (score_var >= score_threshold){
        vel_obstaculos += inc_vel_obstaculos; // incremento da velocidade do cenario
        
        if (gap - dec_gap >= 100){
            gap -= dec_gap;        // gap entre obstaculos diminui até 100, distancia minima para continuar jogavel
        }
        if (dif_max_altura <= 500){
            dif_max_altura += inc_dif_max_altura; // diferena maxima de altura aumenta até 500, distancia maxima para continuar jogavel
        }
        
        score_var = 0;
        // score_var é zerado toda vez que o score passar do score_threshold
    }  
}

int AtualizarJogo(Obstaculo obstaculos_baixo[], Obstaculo obstaculos_cima[], Jogador *jogador, GameScreen *currentScreen, int altura_obstaculos, Texture2D textura_cano, Texture2D textura_cano_invertido){
    
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
        altura_obstaculos = GetRandomValue(altura_obstaculos - dif_max_altura, altura_obstaculos + dif_max_altura);
        
        if (altura_obstaculos >= ALTURA_TELA - gap - 100){
            altura_obstaculos = ALTURA_TELA - gap - 100;
        }
        else if (altura_obstaculos <= 50){
            altura_obstaculos = 50;
        }
        ///////////////////////////////////////////////////////
        
            // Check if obstacle went off the screen, reset its position and reset passou flag
        if (obstaculos_baixo[i].posicao.x + largura_obstaculos <= 0) {
            obstaculos_cima[i].posicao.x = LARGURA_TELA + distanciaProximoObstaculo - largura_obstaculos;
            obstaculos_cima[i].posicao.y = 0;
            obstaculos_cima[i].tamanho.y = ALTURA_TELA - altura_obstaculos - gap;//altura_obstaculos - gap / 2;

            obstaculos_baixo[i].posicao.x = LARGURA_TELA + distanciaProximoObstaculo - largura_obstaculos;
            obstaculos_baixo[i].posicao.y = ALTURA_TELA - altura_obstaculos;//altura_obstaculos + gap;
            obstaculos_baixo[i].tamanho.y = altura_obstaculos;//ALTURA_TELA - altura_obstaculos - gap;

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
        DrawTexture(textura_cano_invertido, obstaculos_cima[i].posicao.x, obstaculos_cima[i].tamanho.y - ALTURA_TELA+100, WHITE);
        DrawRectangleV(obstaculos_baixo[i].posicao, obstaculos_baixo[i].tamanho, BLANK);  
        DrawTexture(textura_cano, obstaculos_baixo[i].posicao.x, obstaculos_baixo[i].posicao.y, WHITE);
    }
    return 0;
}

int main() {

    InitWindow(LARGURA_TELA, ALTURA_TELA, "Flappy Bird");
    SetWindowPosition((GetMonitorWidth(0) - LARGURA_TELA)/2, 10); // posiciona a janela para melhor visualização do jogo
    SetTargetFPS(60); // 60 frames por segundo

    char dificuldade_selecionada[10] = "Medio"; // string padrão de display no menu de dificuldade
    Jogador jogador = {(Vector2){100, 300}, 20}; // inicia o jogador em x=100, y=300 e tamanho (raio) = 20

    Obstaculo obstaculos_baixo[NUM_OBSTACULOS]; // cria o array que armazenará os obstaculos de baixo
    Obstaculo obstaculos_cima[NUM_OBSTACULOS]; // cria o array que armazenará os obstaculos de cima
    
    //int altura_inicial = GetRandomValue(10, ALTURA_TELA - gap); // Gerando uma altura randomica inicial
    int altura_inicial; // Variavel inicial para armazenar o valor retornado por CarregarObstaculos

    char nome[MAX_INPUT_CHARS + 1] = "\0"; // inicializa o array que ira armazenar o nome do jogador
    int count_letras = 0; // contador de caracteres digitas no input do nome
    
    Rectangle BoxInputNome = { LARGURA_TELA-500, 10, 490, 25 }; // inicialiazao do retangulo do input do username
    bool mouseNoInput = false;

    int framesCounter = 0; // variavel para controlar a frequência que o '_' pisca
    
    TIPO_SCORE ranking[5]; // inicializando o array de scores
    LerRanking(ranking);   // lendo o ranking.txt e armazenando no array
    
    // Carregando o background do jogo
    Image backgroundImage = LoadImage("achoaqui800.png");
    Texture2D backgroundTexture = LoadTextureFromImage(backgroundImage);
    UnloadImage(backgroundImage);
    // Definindo a posição inicial do background e a velocidade dele
    float backgroundX = 0;
    float backgroundScrollSpeed = 3.0;

    Image passaro = LoadImage("yellowbird.png");                //
    Texture2D textura_passaro = LoadTextureFromImage(passaro);  // Carregando o png do passaro
    UnloadImage(passaro);                                       //                    
    Image cano = LoadImage("fornow.png");                       // 
    Texture2D textura_cano = LoadTextureFromImage(cano);        // Carregando o png do obstaculo de baixo
    ImageFlipVertical(&cano);                                   // Inverter a imagem para obter o obstaculo de cima                   
    Texture2D textura_cano_invertido = LoadTextureFromImage(cano);
    UnloadImage(cano);
    Image logo = LoadImage("logo.png");                         
    Texture2D textura_logo = LoadTextureFromImage(logo);        //  Carregando o logo do jogo para o menu
    UnloadImage(logo);
    

    while (!WindowShouldClose()) {
        BeginDrawing();
        // Atualizar a posicao do background
        backgroundX -= backgroundScrollSpeed;
        // Resetar a parte do background quando ela sair da tela
        if (backgroundX <= -backgroundTexture.width) {
            backgroundX = 0;
        }
        
        //DrawTextureEx(backgroundTexture, (Vector2){backgroundX, 0}, 0, 1, WHITE); // Desenhando o background
        DrawTexture(backgroundTexture, backgroundX, 0, WHITE);
        DrawTexture(backgroundTexture, backgroundX + backgroundTexture.width, 0, WHITE);
        //DrawTextureEx(backgroundTexture, (Vector2){backgroundX + backgroundTexture.width, 0}, 0, 1, WHITE); 
        

        switch (tela_atual) {
            
            case MENU: {
                DrawTexture(textura_logo, LARGURA_TELA/2-200, 200, WHITE); // Logo flappyinf
            
                // Botao Jogar
                if (MouseEstaSobreBotao((Vector2){LARGURA_TELA/2 - 90, ALTURA_TELA/2 - 90}, (Vector2){200, 50})) {
                    DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2-50, 200, 50, LIME);
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        jogador.posicao.y = 300; // reseta a posicao do jogador
                        velocidade_jogador = 0;  // reseta a velocidade
                        LerDificuldade(dificuldade); // Lê o arquivo bin conforme a dificuldade selecionada
                        // gera os obstaculos e retorna a altura que será passada como argumento para AtualizarJogo
                        altura_inicial = CarregarObstaculos(obstaculos_baixo, obstaculos_cima); 
                        tela_atual = GAMEPLAY; // muda a tela do menu para o jogo em si
                    }
                } else {
                    DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2-50, 200, 50, GREEN);
                }
                DrawText("Jogar", LARGURA_TELA/2 - 50, ALTURA_TELA/2-35, 20, WHITE);



               // Botao dificuldade
               if (MouseEstaSobreBotao((Vector2){LARGURA_TELA/2 - 100, ALTURA_TELA/2 - 30}, (Vector2){200, 50})) {
                   DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 +10, 200, 50, LIME);
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        tela_atual = DIFICULDADE_MENU; // muda para a tela do menu de dificuldade
                    }
               }
               else {
                DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 10, 200, 50, GREEN);
                }
                DrawText("Dificuldade", LARGURA_TELA/2 - 50, ALTURA_TELA/2 + 25, 20, WHITE);
                
                // Botao Ranking
               if (MouseEstaSobreBotao((Vector2){LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 30}, (Vector2){200, 50})) {
                   DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 70, 200, 50, LIME);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                       tela_atual = RANKING; // muda para a tela do scoreboard
                   }    
               }
               else {
                DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 70, 200, 50, GREEN);
                }
                DrawText("Ranking", LARGURA_TELA/2 - 50, ALTURA_TELA/2 + 85, 20, WHITE);
                
                // Button Sair
               if (MouseEstaSobreBotao((Vector2){LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 90}, (Vector2){200, 50})) {
                   DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 130, 200, 50, LIME);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                       CloseWindow(); // fecha o jogo
                   }
               }
                else {
                DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 130, 200, 50, GREEN);
                }
                DrawText("Sair do Jogo", LARGURA_TELA/2 - 50, ALTURA_TELA/2 + 145, 20, WHITE);
                
                
                // Checa se o ponteiro do mouse esta sobre a caixa de input do nome
                if (CheckCollisionPointRec(GetMousePosition(), BoxInputNome)){
                    mouseNoInput = true;
                }
                else {
                    mouseNoInput = false;
                }

                if (mouseNoInput) // Se o mouse estiver em cima da caixa de input:
                {
                    // Cursor vira 'I'
                    SetMouseCursor(MOUSE_CURSOR_IBEAM);

                    // Captura a tecla digitada, se for mais de uma em 1 frame, adiciona em uma fila
                    int tecla = GetCharPressed();
                    
                    // Checa se mais caracteres foram digitados no mesmo frame
                    while (tecla > 0) // quando nao tiver mais caracteres na fila, tecla = 0
                    {
                        // Limita teclas entre 32 e 125 em unicode e limita a quantidade de caracteres no username
                        if ((tecla >= 32) && (tecla <= 125) && (count_letras < MAX_INPUT_CHARS))
                        {
                            nome[count_letras] = (char)tecla; // Adiciona o caracter na string do nome do jogador
                            nome[count_letras+1] = '\0'; // Adiciona o finalizador de string 
                            count_letras++;              // conta a quantidade de caracteres no momento 
                        }

                        tecla = GetCharPressed();  // Checa o proximo caractere na fila
                    }

                    if (IsKeyPressed(KEY_BACKSPACE)) // Checa se o usuaria esta apagando caracteres
                    {
                        count_letras--; // numero de caracteres no username diminui
                        
                        if (count_letras < 0){
                            count_letras = 0;  // quando todos caracteres forem apagados, nao há mais o que apagar
                        }
                        nome[count_letras] = '\0';
                    }
                }
                else SetMouseCursor(MOUSE_CURSOR_DEFAULT); // Se o mouse nao estivera acima do input, cursor fica normal


                // Quando o mouse estiver em cima da caixa do input o contador será incrementado 60 vezes por segundo
                if (mouseNoInput){
                    framesCounter++; 
                }
                else {
                    framesCounter = 0; // reinicia quando o mouse sai do retangulo
                }
                
                // Se o mouse estiver em cima do input a caixa fica verde, se não fica branca
                if (mouseNoInput){ 
                    DrawRectangle(BoxInputNome.x, BoxInputNome.y+40, BoxInputNome.width, BoxInputNome.height, GREEN);
                }
                else {
                    DrawRectangle(BoxInputNome.x, BoxInputNome.y+40, BoxInputNome.width, BoxInputNome.height, WHITE);
                }
                
                // Contorno da caixa de input
                DrawRectangleLines(BoxInputNome.x, BoxInputNome.y+40, BoxInputNome.width, BoxInputNome.height, GRAY);

                // Display do nome que está sendo digitado e armazenado na string nome
                DrawText(nome, BoxInputNome.x + 5, BoxInputNome.y + 45, 20, BLACK);
               
                /// Se mouse estiver em cima do input e houver caracteres disponiveis, '_' irá piscar
                if (mouseNoInput){
                    if (count_letras < MAX_INPUT_CHARS){
                        //(framesCounter/20)%2) == 0 será verdade a cada 20 frames que passa, essa condição permite regular a freq. do pisca
                        if (((framesCounter/20)%2) == 0) DrawText("_", BoxInputNome.x + 8 + MeasureText(nome, 20), BoxInputNome.y+43, 25, BLACK);
                        // MeasureText retorna a quantidade de pixeis que uma string ocupa de acordo com o tamnho da fonte
                    }
                }   
            } break;
            
            case DIFICULDADE_MENU: {
                
                DrawText(TextFormat("Dificuldade: %s", dificuldade_selecionada), LARGURA_TELA/2 - 150, 200, 40, BLACK);
                //Botao facil
                if (MouseEstaSobreBotao((Vector2){LARGURA_TELA/2 - 100, ALTURA_TELA/2 -90}, (Vector2){200, 50})) {
                   DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 - 50, 200, 50, LIME);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        dificuldade = 0; // Se mouse for apertado, a dificuldade mudara para fácil
                        DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 - 50, 200, 50, LIME);
                        strcpy(dificuldade_selecionada, "Facil"); // Copia "Facil" para a string de display do menu
                   }
                }
                else {
                    DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 - 50, 200, 50, GREEN);
                    }
                    DrawText("Facil", LARGURA_TELA/2 - 50, ALTURA_TELA/2 - 35, 20, WHITE);
                
                //Botao medio
                if (MouseEstaSobreBotao((Vector2){LARGURA_TELA/2 - 100, ALTURA_TELA/2 - 30}, (Vector2){200, 50})) {
                   DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 10, 200, 50, LIME);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        dificuldade = 1; // Se mouse for apertado, a dificuldade mudara para médio (padrao)
                        DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 10, 200, 50, GREEN);
                        strcpy(dificuldade_selecionada, "Medio"); // Copia "Medio" para a string de display do menu
                   }
                }
                else {
                    DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 10, 200, 50, GREEN);
                    }
                    DrawText("Medio", LARGURA_TELA/2 - 50, ALTURA_TELA/2 + 25, 20, WHITE);
                
                //Botao dificil
                if (MouseEstaSobreBotao((Vector2){LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 30}, (Vector2){200, 50})) {
                   DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 70, 200, 50, LIME);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        dificuldade = 2; // Se mouse for apertado, a dificuldade mudara para médio 
                        DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 70, 200, 50, LIME);
                        strcpy(dificuldade_selecionada, "Dificil"); // Copia "Dificil" para a string de display do menu
                   }
                }
                else {
                    DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 70, 200, 50, GREEN);
                    }
                    DrawText("Dificil", LARGURA_TELA/2 - 50, ALTURA_TELA/2 + 85, 20, WHITE);
                
                //Botao voltar
                if (MouseEstaSobreBotao((Vector2){LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 110}, (Vector2){200, 50})) {
                   DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 150, 200, 50, LIME);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        tela_atual = MENU; // volta parar o menu
                   }
                }
                else {
                    DrawRectangle(LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 150, 200, 50, GREEN);
                    }
                    DrawText("Voltar", LARGURA_TELA/2 - 50, ALTURA_TELA/2 + 165, 20, WHITE);
                    
            } break;
            
            case RANKING: {
                // Cabeçalhos
                DrawText("Nome", LARGURA_TELA/4, 100, 50, BLACK);
                DrawText("Score", 3*LARGURA_TELA/4, 100, 50, BLACK);
                
                // Loop iterando pelo array ranking para mostrar os nomes e scores
                for (int i = 0; i < 5; i++) {
                    DrawText(TextFormat("%s", ranking[i].nome), LARGURA_TELA/4, 200 + i * 75, 30, BLACK);
                    DrawText(TextFormat("%d", ranking[i].score), 3*LARGURA_TELA/4, 200 + i * 75, 30, BLACK);
                }
                // Botao voltar
                if (MouseEstaSobreBotao((Vector2){LARGURA_TELA/2-100, ALTURA_TELA - 140}, (Vector2){200, 50})) {
                   DrawRectangle(LARGURA_TELA/2-100, ALTURA_TELA - 100, 200, 50, LIME);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        tela_atual = MENU; // volta para o menu
                   }
                }
                else {
                    DrawRectangle(LARGURA_TELA/2-100, ALTURA_TELA - 100, 200, 50, GREEN);
                    }
                    DrawText("Voltar", LARGURA_TELA/2-50, ALTURA_TELA - 85, 20, WHITE);
                
            } break;
            
            case GAMEPLAY: {   // Jogo em si
                
                // Atualiza a posição do jogador
                AtualizarJogador(&velocidade_jogador, aceleracao_jogador, &jogador, textura_passaro);
                // Aumenta a dificuldade quando o score passa o score_threshold. Os parametros sao dados pelos arquivos binarios lidos
                AumentarDificuldade();
                             
                // AtualizarJogo atualiza os obstaculos e checa se houve colisao do jogador com os obstaculos ou se o jogador esta fora da
                // tela. Retorna 1 se o jogo acabou.
                if (AtualizarJogo(obstaculos_baixo, obstaculos_cima, &jogador, &tela_atual, altura_inicial, textura_cano, textura_cano_invertido)){
                    // Se o jogo tiver acabado, atualiza o ranking, zera o score e o score temporario
                    AtualizarRanking(ranking, nome);
                    score = 0;
                    score_var = 0;
                }
                // display do score
                DrawText(TextFormat("Score: %d", score), 10, 50, 40, LIGHTGRAY);

            } break;
        }
        
        EndDrawing();
       // DrawText(" ", 0, 0, 0, WHITE);  
    }

    CloseWindow();
    return 0;
}
