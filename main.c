#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"

// global settings block
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;
int TREE_SIZE = 2;

int WIDHT = 800 / 2;
int HEIGHT = 600 / 2;

#define RATE 0.25 //speed of simulation, the lower is faster
// end of global settings block



typedef struct Forest {
    int *field;
    int *fireField;
} Forest;

typedef struct Tree {
    int x,y;
    int age;
} Tree;

typedef struct Trees {
    Tree* tree;
    int capacity;
    int count;
} Trees;

Forest forest;
float elapsedSpawn = 0;
float elapsedFire = 0;
float elapsedBurning = 0;
float elapsedLightning = 20;
float elapsedGrowth = 0;

int treeCount = 0;
int treeGrowthCount = 0;
int treeBurnedCount = 0;

// формула модуля m = (a + b) % b
// нужна потому что стандартынй в сях не умеет с отрицательными числами работать
// впрочем, если число будет сильно отрицательным - формула сломается,
// но меньше -1 не подразумевается
int mod(int a, int b) { return (a + b) % b; }

void Update() {
    float ft = GetFrameTime();
    
    elapsedFire += ft;
    if(elapsedFire > RATE * 0.1){
        //распространение пожара
        //1. расчет
        for(int x = 0; x < WIDHT; ++x) {
            for(int y = 0; y < HEIGHT; ++y) {
                if(forest.field[x + y * WIDHT] == 2) {
                    forest.fireField[x + y * WIDHT] = 3;
                    int l;
                    // up    x   y+1
                    l =     x            + mod(y + 1, HEIGHT) * WIDHT;
                    if (forest.field[l] >= 10 && forest.field[l] < 20) forest.fireField[l] = 2;
                    // right x+1 y
                    l = mod(x + 1,WIDHT) +     y              * WIDHT;
                    if (forest.field[l] >= 10 && forest.field[l] < 20) forest.fireField[l] = 2;
                    // down  x   y-1
                    l =     x            + mod(y - 1, HEIGHT) * WIDHT;
                    if (forest.field[l] >= 10 && forest.field[l] < 20) forest.fireField[l] = 2;
                    // left  x-1 y
                    l = mod(x - 1,WIDHT) +     y              * WIDHT;
                    if (forest.field[l] >= 10 && forest.field[l] < 20) forest.fireField[l] = 2;
                }
            }
        }
        //2.применение результата(надо просто второй буффер делать и менять, пофиг)
        for(int i = 0; i < WIDHT * HEIGHT; ++i) {
            if(forest.fireField[i] == 2 || forest.fireField[i] == 3)
                forest.field[i] = forest.fireField[i];
            forest.fireField[i] = 0;
        }
        
        elapsedFire = 0;
    }
    
    elapsedBurning += ft;
    if(elapsedBurning > RATE) {
        treeBurnedCount = 0;
        
        for(int i = 0; i < WIDHT * HEIGHT; ++i) {
            if(forest.field[i] == 3){
                forest.field[i] = 0;
                treeCount--;
                treeBurnedCount++;
            }
        }
        elapsedBurning = 0;
    }        
    
    elapsedSpawn += ft;
    if(elapsedSpawn > RATE) {
     // спавн деревьев    
        treeGrowthCount = 0;
        
        int count = GetRandomValue(400,450);
        for(int i = 0; i < count; ++i) {
            int rnd = GetRandomValue(0, WIDHT*HEIGHT);
            if (forest.field[rnd] == 0) {
                forest.field[rnd] = 10;
                treeCount++;
                treeGrowthCount++;
            }
        }
        elapsedSpawn = 0;
    }    
    
    elapsedGrowth += ft;
    if(elapsedGrowth > RATE * 20) {
        // рост деревьев
        for(int i = 0; i < WIDHT * HEIGHT; ++i) {
            if (forest.field[i] >= 10 && forest.field[i] < 20 - 1) {
                forest.field[i]++;
            }
        }
        elapsedGrowth = 0;
    }
    
    elapsedLightning += ft;
    if(elapsedLightning > RATE * 20) {
         // удар молнии
        int rnd = GetRandomValue(0, WIDHT*HEIGHT);
        if (forest.field[rnd] >= 10 && forest.field[rnd] < 20) {
            forest.field[rnd] = 2;
        }
        elapsedLightning = 0;
    }
}


void Render() {

    Color c;
    for(int i = 0; i < WIDHT; ++i) {
        for(int j = 0; j < HEIGHT; ++j) {
            switch(forest.field[i + j * WIDHT]) {
                case 0: c = BLACK; break;
                case 10: 
                case 11: c = DARKGREEN; break;
                case 12:
                case 13:
                case 14:
                case 15: c = LIME; break;
                case 16:
                case 17:
                case 18: c = GREEN; break;
                case 19: c = ORANGE; break;
                case 2: c = WHITE; break;
                case 3: c = RED;   break;
            }
            DrawRectangle(i * TREE_SIZE, j * TREE_SIZE, TREE_SIZE, TREE_SIZE, c);
        }
    }
}

int main(int argc, char* argv[]) {
    for(int i = 1; i < argc; i++) {
        char* key = argv[i];
        if(!strcmp(key, "-w")) {
            char* value = argv[i + 1];
            printf("width: %d\n", (int)strtol(value, NULL, 10));
            SCREEN_WIDTH = (int)strtol(value, NULL, 10);
            i++;
        } else if (!strcmp(key, "-h")) {
            char* value = argv[i + 1];
            printf("height: %d\n", (int)strtol(value, NULL, 10));
            SCREEN_HEIGHT = (int)strtol(value, NULL, 10);
            i++;
        } else if (!strcmp(key, "-t")) {
            char* value = argv[i + 1];
            printf("tree size: %d\n", (int)strtol(value, NULL, 10));
            TREE_SIZE = (int)strtol(value, NULL, 10);
            i++;
        }
    }

    WIDHT = SCREEN_WIDTH / TREE_SIZE;
    HEIGHT = SCREEN_HEIGHT / TREE_SIZE;

    forest = (Forest){malloc(WIDHT * HEIGHT * sizeof(int)), malloc(WIDHT * HEIGHT * sizeof(int))};
    
    //FILE* f = fopen("stats.csv", "w");
    //if (f == NULL) {
    //    return 69;
    //}
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT,"forest");
    SetTargetFPS(15);
    for(int i = 0; i < WIDHT * HEIGHT; ++i) {
        forest.field[i] = 10;
        treeCount++;
    }
    
    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        //printf("Total: %6d Growth: %3d Burned: %4d\n",treeCount, treeGrowthCount, treeBurnedCount);
        //fprintf(f,"(%6d,%3d,%4d)\n" ,treeCount, treeGrowthCount, treeBurnedCount);
        Update();
        Render();
        EndDrawing();
    }
    //fclose(f);

    return 0;
}
