#include <stdio.h>
#include <assert.h>
#include "raylib.h"

#define TREE_SIZE     2
#define WIDHT         800/2
#define HEIGHT        600/2
#define RATE          0.1
#define RENDER_WIDTH  800
#define RENDER_HEIGHT 600

typedef struct Forest {
    unsigned char field[WIDHT * HEIGHT]; // никакого эффекта, может оптимизируется?
    unsigned char buffer[WIDHT * HEIGHT];

    /* int field[WIDHT*HEIGHT]; */
    /* int buffer[WIDHT*HEIGHT]; */
} Forest;
int str[WIDHT][HEIGHT];
Forest forest;

// статистика, не участвует в логике
int treeCount = 0;
int treeGrowthCount = 0;
int treeBurnedCount = 0;

int i = -1;

// формула модуля m = (a + b) % b
// нужна потому что стандартынй в сях не умеет с отрицательными числами работать
// впрочем, если число будет сильно отрицательным - формула сломается,
// но меньше -1 не подразумевается
int mod(int a, int b) { return (a + b) % b; }

void Update() {
    i++;

    static float elapsedSpawn = 0;
    static float elapsedFire = 0;
    static float elapsedBurning = 0;
    static float elapsedLightning = 20;
    static float elapsedGrowth = 0;

    float ft = GetFrameTime();
    elapsedBurning += ft;
    elapsedGrowth += ft;
    static int shouldBurn = 0;
    static int shouldGrowth = 0;

    if (shouldBurn) {
	shouldBurn = 0;
    }
    
    if (elapsedBurning > RATE) {
	shouldBurn = 1;
	elapsedBurning = 0;
    }

    if (shouldGrowth) {
	shouldGrowth = 0;
    }

    if (elapsedGrowth > RATE * 20) {
	shouldGrowth = 1;
	elapsedGrowth = 0;
    }
    

    for (int x = 0; x < WIDHT; ++x) {
	for (int y = 0; y < HEIGHT; ++y) {
	    int i = x + y * WIDHT;
	    unsigned int v = forest.field[i];
	    if (v == 0) {
		forest.buffer[i] = v;
	    } else if (v > 0 && v < 10) {
		if (shouldBurn) {
		    treeBurnedCount = 0;
		    forest.buffer[i] = v - 1;
		    if (forest.buffer[i] == 0) {
			treeCount--;
			treeBurnedCount++;
		    }
		} else {
		    forest.buffer[i] = v;
		}
	    } else if (v > 9 && v < 20) {
		if (shouldGrowth) {
		    // рост деревьев
		    forest.buffer[i] = v + 1;
		} else {
		    forest.buffer[i] = v;
		}
	    } else if (v == 20) {
		forest.buffer[i] = v;
	    } else {
		assert(0 && "can't happen that v > 20 or v < 0, but somehow...");
	    }
	}
    }
    
    elapsedSpawn += ft;
    if (elapsedSpawn > RATE) {
        // спавн деревьев    
        treeGrowthCount = 0;

        int count = GetRandomValue(400, 450);
        for (int i = 0; i < count; ++i) {
            int rnd = GetRandomValue(0, WIDHT * HEIGHT);
            if (forest.field[rnd] == 0) {
                forest.buffer[rnd] = 10;
                treeCount++;
                treeGrowthCount++;
            }
        }
        elapsedSpawn = 0;
    }

    elapsedFire += ft;
    if (elapsedFire > RATE * 0.1 ) {
        //распространение пожара
        //1. расчет
        for (int x = 0; x < WIDHT; ++x) {
            for (int y = 0; y < HEIGHT; ++y) {
                unsigned int v = forest.field[x + y * WIDHT];
                if (v > 0 && v < 10) {
                    int l;
                    // up    x   y+1
                    l = x + mod(y + 1, HEIGHT) * WIDHT;
                    if (forest.field[l] >= 10 && forest.field[l] <= 20) forest.buffer[l] = 9;
                    // right x+1 y
                    l = mod(x + 1, WIDHT) + y * WIDHT;
                    if (forest.field[l] >= 10 && forest.field[l] <= 20) forest.buffer[l] = 9;
                    // down  x   y-1
                    l = x + mod(y - 1, HEIGHT) * WIDHT;
                    if (forest.field[l] >= 10 && forest.field[l] <= 20) forest.buffer[l] = 9;
                    // left  x-1 y
                    l = mod(x - 1, WIDHT) + y * WIDHT;
                    if (forest.field[l] >= 10 && forest.field[l] <= 20) forest.buffer[l] = 9;
                }
            }
        }
        elapsedFire = 0;

    }
    
    elapsedLightning += ft;
    if (elapsedLightning > RATE	 * 5) {
    /* if (i % 20 == 0 /\* * 20 *\/) { */
        // удар молнии
        int rnd = GetRandomValue(0, WIDHT * HEIGHT);
        if (forest.buffer[rnd] >= 10 && forest.buffer[rnd] < 20) {
            forest.buffer[rnd] = 9;
        }
        elapsedLightning = 0;
    }

    for (int i = 0; i < WIDHT * HEIGHT; ++i) {
	forest.field[i] = forest.buffer[i];
	forest.buffer[i] = 0;
    }

}

void DebugRender(int str[WIDHT][HEIGHT]) {
    for (int y = 0; y < HEIGHT; ++y) {
	for (int x = 0; x < WIDHT; ++x) {
	    int i = x + y * WIDHT;
	    str[x][y] = forest.field[i];
	}
    }
}

void Render() {
    Image im = GenImageColor(WIDHT, HEIGHT, RED);

    Color c;
    for (int i = 0; i < WIDHT; ++i) {
        for (int j = 0; j < HEIGHT; ++j) {
            switch (forest.field[i + j * WIDHT]) {
            case 0:  c = BLACK; break;
            case 1:  
            case 2:  c = GRAY; break;
            case 3:  
            case 4:
            case 5:
            case 6:
            case 7:  c = RED;   break;
            case 8:
            case 9:  c = WHITE; break;
            case 10:
            case 11: c = DARKGREEN; break;
            case 12:
            case 13:
            case 14:
            case 15: c = LIME; break;
            case 16:
            case 17:
            case 18: c = GREEN; break;
            case 19:
            case 20: c = PINK; break;
            }
            ImageDrawPixel(&im, i, j, c);
        }
    }

    ImageResizeNN(&im, RENDER_WIDTH, RENDER_HEIGHT);

    Texture tex = LoadTextureFromImage(im);

    BeginDrawing();
    ClearBackground(WHITE);
    DrawTexture(tex, 0, 0, WHITE);
    DrawFPS(5, 5);
    EndDrawing();

    UnloadImage(im);
    UnloadTexture(tex);
}

int main() {
    //FILE* f = fopen("stats.csv", "w");
    //if (f == NULL) {
    //    return 69;
    //}
    SetTraceLogLevel(5);// log level - error

    InitWindow(RENDER_WIDTH, RENDER_HEIGHT, "forest");
    SetTargetFPS(120);
    for (int i = 0; i < WIDHT * HEIGHT; ++i) {
        forest.field[i] = 10;
    }

    while (!WindowShouldClose()) {

        //printf("Total: %6d Growth: %3d Burned: %4d\n",treeCount, treeGrowthCount, treeBurnedCount);
        //fprintf(f,"(%6d,%3d,%4d)\n" ,treeCount, treeGrowthCount, treeBurnedCount);
        Update();
        Render();
    }
    //fclose(f);
    return 0;
}
