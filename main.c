#include <stdio.h>
#include "raylib.h"

#define TREE_SIZE     2
#define WIDHT         800/2
#define HEIGHT        600/2
#define RATE          0.25
#define RENDER_WIDTH  1800
#define RENDER_HEIGHT 1200

typedef struct Forest {
	int field[WIDHT*HEIGHT];
	int fireField[WIDHT*HEIGHT];
} Forest;

Forest forest;

// статистика, не участвует в логике
int treeCount = 0;
int treeGrowthCount = 0;
int treeBurnedCount = 0;

// формула модуля m = (a + b) % b
// нужна потому что стандартынй в сях не умеет с отрицательными числами работать
// впрочем, если число будет сильно отрицательным - формула сломается,
// но меньше -1 не подразумевается
int mod(int a, int b) { return (a + b) % b; }

void Update() {
	static float elapsedSpawn = 0;
	static float elapsedFire = 0;
	static float elapsedBurning = 0;
	static float elapsedLightning = 20;
	static float elapsedGrowth = 0;

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
	Image im = GenImageColor(WIDHT, HEIGHT, RED);
	
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
				case 19: c = PINK; break;
				case 2: c = WHITE; break;
				case 3: c = RED;   break;
			}
			ImageDrawPixel(&im, i, j, c);
		}
	}
	
	ImageResizeNN(&im, RENDER_WIDTH, RENDER_HEIGHT);
	
	Texture tex = LoadTextureFromImage(im);
	
	BeginDrawing();
	ClearBackground(WHITE);
	DrawTexture(tex, 0, 0, WHITE);
	DrawFPS(5,5);
	EndDrawing();
	
	UnloadImage(im);
	UnloadTexture(tex);
}

int main() {
	//FILE* f = fopen("stats.csv", "w");
	//if (f == NULL) {
	//	return 69;
	//}
	SetTraceLogLevel(5);// log level - error
	
	InitWindow(RENDER_WIDTH, RENDER_HEIGHT,"forest");
	SetTargetFPS(120);
	for(int i = 0; i < WIDHT * HEIGHT; ++i) {
		forest.field[i] = 10;
	}
	
	while(!WindowShouldClose()) {
		
		//printf("Total: %6d Growth: %3d Burned: %4d\n",treeCount, treeGrowthCount, treeBurnedCount);
		//fprintf(f,"(%6d,%3d,%4d)\n" ,treeCount, treeGrowthCount, treeBurnedCount);
		Update();
		Render();
	}
	//fclose(f);
	return 0;
}