#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "raylib.h"


#define ALTURA_TELA 700
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
    char name[40];
    int score;
} TIPO_SCORE;


float velocidade_jogador = 0, aceleracao_jogador = 0.5;
int vel_obstaculos = 10, inc_vel_obstaculos = 1;
int score = 0, score_var = 0;
int gap = 300, dec_gap = 5;
int largura_obstaculos = 100;
//int altura_obstaculos = 50;
//int altura_obstaculos;
int distanciaProximoObstaculo = 400;
int dif_max_altura = 50, inc_dif_max_altura;
int score_threshold = 400;
//int altura_inicial;
int dificuldade = 1;
//int game_over = 0;
GameScreen tela_atual = MENU;


void LerRanking(TIPO_SCORE ranking[5]){
    FILE *file;
    file = fopen("ranking.txt", "r");
    char line[40];
    
    for (int i = 0; i < 5; i++) {
        if (fgets(line, sizeof(line), file) != NULL) {
            line[strcspn(line, "\n")] = '\0';
            // Remove the newline character
            strncpy(ranking[i].name, line, sizeof(ranking[i].name) - 1);
            ranking[i].name[sizeof(ranking[i].name) - 1] = '\0';  // Ensure null-termination

            // Read the score and handle newline characters
            if (fgets(line, sizeof(line), file) != NULL) {
                sscanf(line, "%d", &ranking[i].score);
            } 
        }
    }
    fclose(file);
    
    // ordenar
    OrdenarRanking(ranking);
}
void OrdenarRanking(TIPO_SCORE ranking[5]){
    for (int i = 0; i < 5 - 1; i++) {
        for (int j = 0; j < 5 - i - 1; j++) {
            if (ranking[j].score < ranking[j + 1].score) {
                // Swap the entries
                TIPO_SCORE temp = ranking[j];
                ranking[j] = ranking[j + 1];
                ranking[j + 1] = temp;
            }
        }
    }
    
}

void AtualizarRanking(TIPO_SCORE ranking[5], char name[]){
    int menor_score = INT_MAX, idx_menor_score;
    for(int i=0; i < 5; i++){
        if (ranking[i].score < menor_score){
            menor_score = ranking[i].score;
            idx_menor_score = i;
        }
        
    }
    
    TIPO_SCORE jogador;
    strcpy(jogador.name, name);
    jogador.score = score;
    ranking[idx_menor_score] = jogador;

    OrdenarRanking(ranking);

    EscreverRanking(ranking);
}

void EscreverRanking(TIPO_SCORE ranking[5]){
    FILE *file;
    file = fopen("ranking.txt", "w");
    for (int i = 0; i<5; i++){
        
        fprintf(file, "%s\n", ranking[i].name);
        fprintf(file, "%d\n", ranking[i].score);
    }
    fclose(file);
    
}

bool MouseEstaSobreBotao(Vector2 posicao, Vector2 tamanho) {
    Vector2 mouse = GetMousePosition();
    return CheckCollisionPointRec(mouse, (Rectangle){posicao.x, posicao.y, tamanho.x, tamanho.y});
}

void LerDificuldade(int dificuldade){
    FILE *file;
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

int CarregarObstaculos(Obstaculo obstaculos_baixo[], Obstaculo obstaculos_cima[], int altura_obstaculos){
    altura_obstaculos = GetRandomValue(100, 600);//GetRandomValue(10, ALTURA_TELA - gap - 10);
    //FILE *file;
        
    //file = fopen("data.txt", "w");
    for (int i = 0; i < NUM_OBSTACULOS; i++) {
        /////////////////////////////////////////////////////// FIX THIS
        altura_obstaculos = GetRandomValue(altura_obstaculos - dif_max_altura, altura_obstaculos + dif_max_altura);
        
        if (altura_obstaculos >= ALTURA_TELA - gap - 50){
            altura_obstaculos = ALTURA_TELA - gap - 50;//GetRandomValue(ALTURA_TELA - gap - dif_max_altura - 10, ALTURA_TELA - gap - 10);
        }
        else if (altura_obstaculos <= 50){
            altura_obstaculos = 50;//GetRandomValue(10, altura_obstaculos + dif_max_altura);
        }

        
        
        //fprintf(file, "%d\n", altura_obstaculos);
        ///////////////////////////////////////////////////////

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
    //fclose(file);
    
    return altura_obstaculos;
}

void AtualizarJogador(float *velocidade_jogador, float aceleracao_jogador, Jogador *jogador, Texture2D textura) {
    DrawCircleV((*jogador).posicao, (*jogador).tamanho, BLANK);
    BeginShaderMode(LoadShader(0, TextFormat("resources/shaders/circle_mask.fs", LARGURA_TELA, ALTURA_TELA)));
    //DrawTexture(textura, (*jogador).posicao.x - textura.width / 2, (*jogador).posicao.y - textura.height / 2, WHITE);
    float scaleFactor = 2.5f * (*jogador).tamanho / fmax(textura.width, textura.height);
    DrawTextureEx(textura, (Vector2){(*jogador).posicao.x - (*jogador).tamanho, (*jogador).posicao.y - (*jogador).tamanho}, 0, scaleFactor, WHITE);
    EndShaderMode();
    if (IsKeyPressed(KEY_SPACE)) {
        *velocidade_jogador = -10;
    }

    *velocidade_jogador += aceleracao_jogador;
    (*jogador).posicao.y += *velocidade_jogador;

}

void AumentarDificuldade(){
    if (score_var / score_threshold >= 1){
        vel_obstaculos += inc_vel_obstaculos;
        
        if (gap - dec_gap >= 100){
            gap -= dec_gap;
        }
            score_var = 0;
    }  
}

int AtualizarJogo(Obstaculo obstaculos_baixo[], Obstaculo obstaculos_cima[], Jogador *jogador, GameScreen *currentScreen, int altura_obstaculos, TIPO_SCORE ranking[5]){
    
    
    Image cano = LoadImage("fornow.png");
    Texture2D textura_cano = LoadTextureFromImage(cano);
    ImageFlipVertical(&cano);
    Texture2D textura_cano_invertido = LoadTextureFromImage(cano);
    UnloadImage(cano);

    //FILE *file;
        
    //file = fopen("data1.txt", "a");
    for (int i = 0; i < NUM_OBSTACULOS; i++) {
        
        obstaculos_cima[i].posicao.x -= vel_obstaculos;
        obstaculos_baixo[i].posicao.x -= vel_obstaculos;

            // Check if obstacle passed the player and hasn't been scored yet
        if (obstaculos_baixo[i].posicao.x + largura_obstaculos < (*jogador).posicao.x && !obstaculos_baixo[i].passou) {
            score += 50;
            score_var += 50;
            obstaculos_baixo[i].passou = 1;
        }
        
        /////////////////////////////////////////////////////// FIX THIS
        altura_obstaculos = GetRandomValue(altura_obstaculos - dif_max_altura, altura_obstaculos + dif_max_altura);
        //altura_obstaculos = GetRandomValue(*altura_inicial - dif_max_altura, *altura_inicial + dif_max_altura);
        
        if (altura_obstaculos >= ALTURA_TELA - gap - 50){
            altura_obstaculos = ALTURA_TELA - gap - 50;//GetRandomValue(ALTURA_TELA - gap - dif_max_altura - 10, ALTURA_TELA - gap - 10);
        }
        else if (altura_obstaculos <= 50){
            altura_obstaculos = 50;//GetRandomValue(10, altura_obstaculos + dif_max_altura);
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
        //fprintf(file, "Posicao: %d, Altura Obstaculo: %d, Altura real: %d\n", (int)obstaculos_baixo[i].posicao.y, (int)obstaculos_baixo[i].tamanho.y, altura_obstaculos);
            
        if (CheckCollisionCircleRec((*jogador).posicao, (*jogador).tamanho, (Rectangle){obstaculos_baixo[i].posicao.x, obstaculos_baixo[i].posicao.y, obstaculos_baixo[i].tamanho.x, obstaculos_baixo[i].tamanho.y}) ||
            CheckCollisionCircleRec((*jogador).posicao, (*jogador).tamanho, (Rectangle){obstaculos_cima[i].posicao.x, obstaculos_cima[i].posicao.y, obstaculos_cima[i].tamanho.x, obstaculos_cima[i].tamanho.y})) {
            DrawText(TextFormat("Collision!"), 500, 500, 40, BLACK); 
            //score = 0;
           // game_over = 1;
            tela_atual = MENU;
            return 1;
        }
        
        if((*jogador).posicao.y >= ALTURA_TELA){
            //score = 0;
            //game_over = 1;
            tela_atual = MENU;
            return 1;
        }

        DrawRectangleV(obstaculos_cima[i].posicao, obstaculos_cima[i].tamanho, BLANK);
        DrawTexture(textura_cano_invertido, obstaculos_cima[i].posicao.x, obstaculos_cima[i].tamanho.y - ALTURA_TELA, WHITE);
        DrawRectangleV(obstaculos_baixo[i].posicao, obstaculos_baixo[i].tamanho, BLANK);  
        DrawTexture(textura_cano, obstaculos_baixo[i].posicao.x, obstaculos_baixo[i].posicao.y, WHITE);
    }
    //fclose(file);
    return 0;
}

int main() {

    InitWindow(LARGURA_TELA, ALTURA_TELA, "Flappy Bird");
    SetTargetFPS(60);

    char dificuldade_selecionada[10] = "Medio";
    Jogador jogador = {(Vector2){100, 300}, 30};

    Obstaculo obstaculos_baixo[NUM_OBSTACULOS];
    Obstaculo obstaculos_cima[NUM_OBSTACULOS];
    
    int altura_inicial = GetRandomValue(10, ALTURA_TELA - gap);

    char name[MAX_INPUT_CHARS + 1] = "\0";
    int letterCount = 0;
    
    Rectangle textBox = { LARGURA_TELA-500, ALTURA_TELA - 50, 490, 25 };
    bool mouseOnText = false;

    int framesCounter = 0;
    
    TIPO_SCORE ranking[5];
    LerRanking(ranking);
    
    Image backgroundImage = LoadImage("achoaqui.png");
    Texture2D backgroundTexture = LoadTextureFromImage(backgroundImage);
    UnloadImage(backgroundImage);
    float backgroundX = 0.0f;
    float backgroundScrollSpeed = 3.0f;
    Image passaro = LoadImage("yellowbird.png");
    Texture2D textura_passaro = LoadTextureFromImage(passaro);
    

    while (!WindowShouldClose()) {
        BeginDrawing();
        
        backgroundX -= backgroundScrollSpeed;
        if (backgroundX <= -backgroundTexture.width) {
            backgroundX = 0;
        }
        DrawTextureEx(backgroundTexture, (Vector2){backgroundX, 0}, 0.0f, 1.0f, WHITE);
        DrawTextureEx(backgroundTexture, (Vector2){backgroundX + backgroundTexture.width, 0}, 0.0f, 1.0f, WHITE);
        

        switch (tela_atual) {
            
            case MENU: {
                DrawText("Flappy Bird", 400, 200, 40, BLACK);
                
                //score = 0;
                //score_var = 0;
                // Botao Jogar
                
                if (MouseEstaSobreBotao((Vector2){400, 300}, (Vector2){200, 50})) {
                    DrawRectangle(400, 300, 200, 50, GRAY);
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        jogador.posicao.y = 300;
                        velocidade_jogador = 0;
                        LerDificuldade(dificuldade);
                        altura_inicial = CarregarObstaculos(obstaculos_baixo, obstaculos_cima, altura_inicial);
                        tela_atual = GAMEPLAY;
                    }
                } else {
                    DrawRectangle(400, 300, 200, 50, LIGHTGRAY);
                }
                DrawText("Jogar", 450, 315, 20, BLACK);


               // Botao dificuldade
               if (MouseEstaSobreBotao((Vector2){400, 360}, (Vector2){200, 50})) {
                   DrawRectangle(400, 360, 200, 50, GRAY);
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        tela_atual = DIFICULDADE_MENU;
                    }
               }
               else {
                DrawRectangle(400, 360, 200, 50, LIGHTGRAY);
                }
                DrawText("Dificuldade", 450, 375, 20, BLACK);
                
                // Botao Ranking
               if (MouseEstaSobreBotao((Vector2){400, 420}, (Vector2){200, 50})) {
                   DrawRectangle(400, 420, 200, 50, GRAY);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                       
                       tela_atual = RANKING;
                   }    
       
               }
               else {
                DrawRectangle(400, 420, 200, 50, LIGHTGRAY);
                }
                DrawText("Ranking", 450, 435, 20, BLACK);
                
                // Button Sair
               if (MouseEstaSobreBotao((Vector2){400, 480}, (Vector2){200, 50})) {
                   DrawRectangle(400, 480, 200, 50, GRAY);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                       CloseWindow();
                   }
               }
                else {
                DrawRectangle(400, 480, 200, 50, LIGHTGRAY);
                }
                DrawText("Sair do Jogo", 450, 495, 20, BLACK);
                
                
                if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = true;
                else mouseOnText = false;

                if (mouseOnText)
                {
                    // Set the window's cursor to the I-Beam
                    SetMouseCursor(MOUSE_CURSOR_IBEAM);

                    // Get char pressed (unicode character) on the queue
                    int key = GetCharPressed();

                    // Check if more characters have been pressed on the same frame
                    while (key > 0)
                    {
                        // NOTE: Only allow keys in range [32..125]
                        if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS))
                        {
                            name[letterCount] = (char)key;
                            name[letterCount+1] = '\0'; // Add null terminator at the end of the string.
                            letterCount++;
                        }

                        key = GetCharPressed();  // Check next character in the queue
                    }

                    if (IsKeyPressed(KEY_BACKSPACE))
                    {
                        letterCount--;
                        if (letterCount < 0) letterCount = 0;
                        name[letterCount] = '\0';
                    }
                }
                else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

                if (mouseOnText) framesCounter++;
                else framesCounter = 0;
                
                DrawRectangleRec(textBox, LIGHTGRAY);
                if (mouseOnText) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
                else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);

                DrawText(name, (int)textBox.x + 5, (int)textBox.y + 5, 20, MAROON);

                //DrawText(TextFormat("INPUT CHARS: %i/%i", letterCount, MAX_INPUT_CHARS), 315, 250, 20, DARKGRAY);
               
                if (mouseOnText)
                {
                    if (letterCount < MAX_INPUT_CHARS)
                    {
                        // Draw blinking underscore char
                        if (((framesCounter/20)%2) == 0) DrawText("_", (int)textBox.x + 8 + MeasureText(name, 20), (int)textBox.y+3, 25, MAROON);
                    }
                    //else DrawText("Press BACKSPACE to delete chars...", 230, 300, 20, GRAY);
                }
                
                
                
                
            } break;
            
            case DIFICULDADE_MENU: {
                
                DrawText(TextFormat("Dificuldade: %s", dificuldade_selecionada), 400, 200, 40, BLACK);
                //Botao facil
                if (MouseEstaSobreBotao((Vector2){400, 300}, (Vector2){200, 50})) {
                   DrawRectangle(400, 300, 200, 50, GRAY);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        dificuldade = 0;
                        DrawRectangle(400, 300, 200, 50, GRAY);
                        strcpy(dificuldade_selecionada, "Facil");
                      //  DrawText(TextFormat("Dificuldade: %s", dificuldade_selecionada), 400, 200, 40, BLACK);
                   }
                }
                else {
                    DrawRectangle(400, 300, 200, 50, LIGHTGRAY);
                    }
                    DrawText("Facil", 450, 315, 20, BLACK);
                
                //Botao medio
                if (MouseEstaSobreBotao((Vector2){400, 360}, (Vector2){200, 50})) {
                   DrawRectangle(400, 360, 200, 50, GRAY);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        dificuldade = 1;
                        DrawRectangle(400, 360, 200, 50, GRAY);
                        strcpy(dificuldade_selecionada, "Medio");
                       // DrawText(TextFormat("Dificuldade: %s", dificuldade_selecionada), 400, 200, 40, BLACK);
                   }
                }
                else {
                    DrawRectangle(400, 360, 200, 50, LIGHTGRAY);
                    }
                    DrawText("Medio", 450, 375, 20, BLACK);
                
                //Botao dificil
                if (MouseEstaSobreBotao((Vector2){400, 420}, (Vector2){200, 50})) {
                   DrawRectangle(400, 420, 200, 50, GRAY);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        dificuldade = 2;
                        DrawRectangle(400, 420, 200, 50, GRAY);
                        strcpy(dificuldade_selecionada, "Dificil");
                        //DrawText(TextFormat("Dificuldade: %s", dificuldade_selecionada), 400, 200, 40, BLACK);
                   }
                }
                else {
                    DrawRectangle(400, 420, 200, 50, LIGHTGRAY);
                    }
                    DrawText("Dificil", 450, 435, 20, BLACK);
                
                //Botao voltar
                if (MouseEstaSobreBotao((Vector2){400, 480}, (Vector2){200, 50})) {
                   DrawRectangle(400, 480, 200, 50, GRAY);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        tela_atual = MENU;
                   }
                }
                else {
                    DrawRectangle(400, 480, 200, 50, LIGHTGRAY);
                    }
                    DrawText("Voltar", 450, 495, 20, BLACK);
            } break;
            
            case RANKING: {
                DrawText("Nome", LARGURA_TELA/4, 50, 50, BLACK);
                DrawText("Score", 3*LARGURA_TELA/4, 50, 50, BLACK);
                for (int i = 0; i < 5; i++) {
                    DrawText(TextFormat("%s", ranking[i].name), LARGURA_TELA/4, 200 + i * 50, 30, BLACK);
                    DrawText(TextFormat("%d", ranking[i].score), 3*LARGURA_TELA/4, 200 + i * 50, 30, BLACK);
                }
                // Botao voltar
                if (MouseEstaSobreBotao((Vector2){400, 480}, (Vector2){200, 50})) {
                   DrawRectangle(400, 480, 200, 50, GRAY);
                   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        tela_atual = MENU;
                   }
                }
                else {
                    DrawRectangle(400, 480, 200, 50, LIGHTGRAY);
                    }
                    DrawText("Voltar", 450, 495, 20, BLACK);
                
            } break;
            
            case GAMEPLAY: {
                
                DrawText(TextFormat("Score: %d", score), 10, 10, 20, LIGHTGRAY);

                AtualizarJogador(&velocidade_jogador, aceleracao_jogador, &jogador, textura_passaro);
                
                AumentarDificuldade();
                
                //if(AtualizarJogo(obstaculos_baixo, obstaculos_cima, &jogador, &tela_atual, altura_inicial, ranking)){
                    //AtualizarRanking(ranking, score, name);
                             

                if (AtualizarJogo(obstaculos_baixo, obstaculos_cima, &jogador, &tela_atual, altura_inicial, ranking)){
                    AtualizarRanking(ranking, name);
                    score = 0;
                    score_var = 0;
                }

            } break;
        }
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
