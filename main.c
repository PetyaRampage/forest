
#include <stdio.h>
#include <assert.h>
#include "raylib.h"

#define TREE_SIZE     2     // size in pixels
#define WIDHT         800/2 // field width
#define HEIGHT        600/2 // field height
#define RATE          0.1   // in seconds - the smaller the faster simulation
#define RENDER_WIDTH  800   // window width
#define RENDER_HEIGHT 600   // window height

//for using from debugger
int str[WIDHT][HEIGHT];

typedef struct Forest {
    int field[WIDHT*HEIGHT];
    int buffer[WIDHT*HEIGHT];
} Forest;

Forest forest;

// statistics, not participate in any logic
int treeCount = 0;
int treeGrowthCount = 0;
int treeBurnedCount = 0;

int i = -1;

// module formula is m = (a + b) % b
// needed because just % fails on negative numbers
// however this will fail too if number is too negative
// but values greater than -1 is expected
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
    

    for (int i = 0; i < WIDHT * HEIGHT; ++i) {
	unsigned int v = forest.field[i];
	assert(v >= 0 || v <= 20); // [0,20]

	if (shouldBurn && v > 0 && v < 10) {
	    treeBurnedCount = 0;
	    forest.buffer[i] = v - 1;
	    if (forest.buffer[i] == 0) {
		treeCount--;
		treeBurnedCount++;
	    }
	} else if (shouldGrowth && v > 9 && v < 20) {
	    forest.buffer[i] = v + 1;
	} else {
	    forest.buffer[i] = v;
	}
    }
    
    elapsedSpawn += ft;
    if (elapsedSpawn > RATE) {
        // tree spawn    
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
        //fire spread
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
        // lightning strike
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

// for using from debugger
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
            case 2:  
            case 3:  
            case 4:
            case 5:  c = GRAY; break;
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
    SetTraceLogLevel(5);// log level - error

    InitWindow(RENDER_WIDTH, RENDER_HEIGHT, "forest");
    SetTargetFPS(120);
    for (int i = 0; i < WIDHT * HEIGHT; ++i) {
        forest.field[i] = 10;
    }

    while (!WindowShouldClose()) {
        Update();
        Render();
    }
    return 0;
}
