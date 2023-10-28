#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <fstream>

#define SCALE 2

struct Tile
{
	int x;
	int y;
	int spriteX;
	int spriteY;
	bool foreground;
};

struct Collision
{
	int x;
	int y;
	int width;
	int height;
};

class Editor
{
public:
	bool Initialize();
	bool PollEvents();
	void Update();
	void RenderFrame();
	void Destroy();

private:
	void RenderTile(int index);
	bool AreColliding(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2);

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	SDL_Texture* backBuffer = nullptr;
	SDL_Texture* spritesTexture = nullptr;

	int mouseX = 0;
	int mouseY = 0;

	int placementX = 0;
	int placementY = 0;

	int previewHoveredX = 0;
	int previewHoveredY = 0;

	int selectedX = 0;
	int selectedY = 0;

	int collisionWidth = 1;
	int collisionHeight = 1;

	int playerX = 0;
	int playerY = 0;

	bool inPreview = false;
	int showGrid = 0;

	bool eraseMode = false;
	bool foregroundEditorMode = false;
	bool collisionsEditorMode = false;
	bool testMode = false;

	int tileTargetedIndex = -1;
	int collisionTargetedIndex = -1;

	std::vector<Tile> tiles;
	std::vector<Collision> collisions;
};