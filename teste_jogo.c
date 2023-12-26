#include <stdio.h>
#include <string.h>
#include "raylib.h"


#define ALTURA_TELA 700
#define LARGURA_TELA 1200
#define NUM_OBSTACULOS 4

typedef enum GameScreen { MENU, GAMEPLAY, DIFICULDADE_MENU } GameScreen;

typedef struct {
    Vector2 posicao;
    float tamanho;
} Jogador;

typedef struct {
    Vector2 posicao;
    Vector2 tamanho;
    int passou;
} Obstaculo;

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
GameScreen tela_atual = MENU;

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
    FILE *file;
        
    file = fopen("data.txt", "w");
    for (int i = 0; i < NUM_OBSTACULOS; i++) {
        /////////////////////////////////////////////////////// FIX THIS
        altura_obstaculos = GetRandomValue(altura_obstaculos - dif_max_altura, altura_obstaculos + dif_max_altura);
        
        if (altura_obstaculos >= ALTURA_TELA - gap - 50){
            altura_obstaculos = ALTURA_TELA - gap - 50;//GetRandomValue(ALTURA_TELA - gap - dif_max_altura - 10, ALTURA_TELA - gap - 10);
        }
        else if (altura_obstaculos <= 50){
            altura_obstaculos = 50;//GetRandomValue(10, altura_obstaculos + dif_max_altura);
        }

        
        
        fprintf(file, "%d\n", altura_obstaculos);
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
    fclose(file);
    
    return altura_obstaculos;
}

void AtualizarJogador(float *velocidade_jogador, float aceleracao_jogador, Jogador *jogador) {
    DrawCircleV((*jogador).posicao, (*jogador).tamanho, RED);
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

void AtualizarJogo(Obstaculo obstaculos_baixo[], Obstaculo obstaculos_cima[], Jogador *jogador, GameScreen *currentScreen, int altura_obstaculos){
    
    FILE *file;
        
    file = fopen("data1.txt", "a");
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
        fprintf(file, "Posicao: %lf, Altura Obstaculo: %lf, Altura real: %d\n", obstaculos_baixo[i].posicao.y, obstaculos_baixo[i].tamanho.y, altura_obstaculos);
            
        if (CheckCollisionCircleRec((*jogador).posicao, (*jogador).tamanho, (Rectangle){obstaculos_baixo[i].posicao.x, obstaculos_baixo[i].posicao.y, obstaculos_baixo[i].tamanho.x, obstaculos_baixo[i].tamanho.y}) ||
            CheckCollisionCircleRec((*jogador).posicao, (*jogador).tamanho, (Rectangle){obstaculos_cima[i].posicao.x, obstaculos_cima[i].posicao.y, obstaculos_cima[i].tamanho.x, obstaculos_cima[i].tamanho.y})) {
            DrawText(TextFormat("Collision!"), 500, 500, 40, BLACK); 
            //tela_atual = MENU;
        }

        DrawRectangleV(obstaculos_cima[i].posicao, obstaculos_cima[i].tamanho, BLUE);
        DrawRectangleV(obstaculos_baixo[i].posicao, obstaculos_baixo[i].tamanho, BLUE);  
    }
    fclose(file);
}

int main() {

    InitWindow(LARGURA_TELA, ALTURA_TELA, "Flappy Bird");
    SetTargetFPS(60);

    char dificuldade_selecionada[10] = "Medio";
    Jogador jogador = {(Vector2){100, 300}, 20};

    Obstaculo obstaculos_baixo[NUM_OBSTACULOS];
    Obstaculo obstaculos_cima[NUM_OBSTACULOS];
    
    int altura_inicial = GetRandomValue(10, ALTURA_TELA - gap);
    //altura_inicial = CarregarObstaculos(obstaculos_baixo, obstaculos_cima, altura_inicial);

    //CarregarObstaculos(obstaculos_baixo, obstaculos_cima, &altura_inicial);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(WHITE);
        
        switch (tela_atual) {
            
            case MENU: {
                DrawText("Flappy Bird", 400, 200, 40, BLACK);
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
               //     if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
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
            
            case GAMEPLAY: {
                
                DrawText(TextFormat("Score: %d", score), 10, 10, 20, LIGHTGRAY);

                AtualizarJogador(&velocidade_jogador, aceleracao_jogador, &jogador);
                
                AumentarDificuldade();
                
                AtualizarJogo(obstaculos_baixo, obstaculos_cima, &jogador, &tela_atual, altura_inicial);
                
                if(jogador.posicao.y >= ALTURA_TELA){
                    tela_atual = MENU;
                }
                
            } break;
        }
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
