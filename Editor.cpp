#include "Editor.h"

bool Editor::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		return false;
	}

	int flags = IMG_INIT_JPG | IMG_INIT_PNG;
	if ((IMG_Init(flags) & flags) != flags)
	{
		return false;
	}

	window = SDL_CreateWindow("Strange Awakening [MAP EDITOR]",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640 * SCALE,
		480 * SCALE,
		0);
	if (window == nullptr)
	{
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	if (renderer == nullptr)
	{
		return false;
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	backBuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 640, 480);
	if (backBuffer == nullptr)
	{
		return false;
	}

	SDL_Surface* spritesSurface = IMG_Load("data/textures/sprites.png");
	if (spritesSurface == nullptr)
	{
		return false;
	}
	spritesTexture = SDL_CreateTextureFromSurface(renderer, spritesSurface);
	if (spritesTexture == nullptr)
	{
		return false;
	}
	SDL_FreeSurface(spritesSurface);

	// line writed by Loane
	return true;
}

bool Editor::PollEvents()
{
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		switch (sdlEvent.type)
		{
		case SDL_MOUSEBUTTONDOWN:
			if (sdlEvent.button.button == SDL_BUTTON_LEFT)
			{
				if (inPreview)
				{
					selectedX = previewHoveredX;
					selectedY = previewHoveredY;
				}
				else
				{
					if (foregroundEditorMode)
					{
						for (int i = 0; i < tiles.size(); i++)
						{
							if (i == tileTargetedIndex)
							{
								tiles[i].foreground = !tiles[i].foreground;
								break;
							}
						}
					}
					else if (eraseMode)
					{
						if (collisionsEditorMode)
						{
							for (int i = static_cast<int>(collisions.size()) - 1; i >= 0; i--)
							{
								if (i == collisionTargetedIndex)
								{
									collisions.erase(collisions.begin() + i);
									eraseMode = false;
									if (collisions.empty())
									{
										collisionsEditorMode = false;
									}
									break;
								}
							}
						}
						else
						{
							for (int i = static_cast<int>(tiles.size()) - 1; i >= 0; i--)
							{
								if (i == tileTargetedIndex)
								{
									tiles.erase(tiles.begin() + i);
									eraseMode = false;
									break;
								}
							}
						}
					}
					else
					{
						if (collisionsEditorMode)
						{
							collisions.push_back({ placementX, placementY, collisionWidth, collisionHeight });
						}
						else
						{
							tiles.push_back({ placementX, placementY, selectedX, selectedY, false });
						}
					}
				}
			}
			break;
		case SDL_MOUSEWHEEL:
			if (sdlEvent.wheel.y < 0)
			{
				if (selectedX < 7 || selectedY < 7)
				{
					selectedX++;
					if (selectedX > 7)
					{
						selectedX = 0;
						selectedY += 1;
					}
				}
			}
			else if (sdlEvent.wheel.y > 0)
			{
				if (selectedX > 0 || selectedY > 0)
				{
					selectedX--;
					if (selectedX < 0)
					{
						selectedX = 7;
						selectedY -= 1;
					}
				}
			}
			break;
		case SDL_KEYDOWN:
			switch (sdlEvent.key.keysym.sym)
			{
			case SDLK_g:
				showGrid = (showGrid + 1) % 3;
				break;
			case SDLK_e:
				eraseMode = !eraseMode;
				foregroundEditorMode = false;
				break;
			case SDLK_f:
				foregroundEditorMode = !foregroundEditorMode;
				eraseMode = false;
				collisionsEditorMode = false;
				break;
			case SDLK_c:
				collisionsEditorMode = !collisionsEditorMode;
				collisionWidth = 1;
				collisionHeight = 1;
				eraseMode = false;
				foregroundEditorMode = false;
				break;
			case SDLK_t:
				testMode = !testMode;
				eraseMode = false;
				collisionsEditorMode = false;
				foregroundEditorMode = false;
				showGrid = 0;
				break;
			case SDLK_s:
			{
				std::ofstream outFile("data/levels/map.dat", std::ios::binary);

				if (outFile)
				{
					size_t numTiles = tiles.size();
					outFile.write(reinterpret_cast<char*>(&numTiles), sizeof(numTiles));
					outFile.write(reinterpret_cast<char*>(&tiles[0]), numTiles * sizeof(Tile));

					size_t numCollisions = collisions.size();
					outFile.write(reinterpret_cast<char*>(&numCollisions), sizeof(numCollisions));
					outFile.write(reinterpret_cast<char*>(&collisions[0]), numCollisions * sizeof(Collision));

					outFile.close();
				}
				break;
			}
			case SDLK_l:
			{
				std::ifstream inFile("data/levels/map.dat", std::ios::binary);

				if (inFile)
				{
					size_t numTiles = 0;
					inFile.read(reinterpret_cast<char*>(&numTiles), sizeof(numTiles));
					tiles.resize(numTiles);
					inFile.read(reinterpret_cast<char*>(&tiles[0]), numTiles * sizeof(Tile));

					size_t numCollisions = 0;
					inFile.read(reinterpret_cast<char*>(&numCollisions), sizeof(numCollisions));
					collisions.resize(numCollisions);
					inFile.read(reinterpret_cast<char*>(&collisions[0]), numCollisions * sizeof(Collision));

					inFile.close();
				}
				break;
			}
			case SDLK_LEFT:
				if (!testMode)
				{
					if (collisionsEditorMode)
					{
						if (collisionWidth > 1) collisionWidth--;
					}
					else
					{
						for (int i = 0; i < tiles.size(); i++) tiles[i].x -= 1;
						for (int i = 0; i < collisions.size(); i++) collisions[i].x -= 2;
					}
				}
				break;
			case SDLK_UP:
				if (!testMode)
				{
					if (collisionsEditorMode)
					{
						if (collisionHeight > 1) collisionHeight--;
					}
					else
					{
						for (int i = 0; i < tiles.size(); i++) tiles[i].y -= 1;
						for (int i = 0; i < collisions.size(); i++) collisions[i].y -= 2;
					}
				}
				break;
			case SDLK_RIGHT:
				if (!testMode)
				{
					if (collisionsEditorMode)
					{
						collisionWidth++;
					}
					else
					{
						for (int i = 0; i < tiles.size(); i++) tiles[i].x += 1;
						for (int i = 0; i < collisions.size(); i++) collisions[i].x += 2;
					}
				}
				break;
			case SDLK_DOWN:
				if (!testMode)
				{
					if (collisionsEditorMode)
					{
						collisionHeight++;
					}
					else
					{
						for (int i = 0; i < tiles.size(); i++) tiles[i].y += 1;
						for (int i = 0; i < collisions.size(); i++) collisions[i].y += 2;
					}
				}
			}
			break;
		case SDL_QUIT:
			return false;
		}
	}

	return true;
}

void Editor::Update()
{
	tileTargetedIndex = -1;
	collisionTargetedIndex = -1;

	if (testMode)
	{
		const Uint8* keyboard = SDL_GetKeyboardState(nullptr);
		if (keyboard[SDL_SCANCODE_LEFT] != keyboard[SDL_SCANCODE_RIGHT])
		{
			if (keyboard[SDL_SCANCODE_LEFT])
			{
				playerX -= 1;
				for (int i = 0; i < collisions.size(); i++)
				{
					if (AreColliding(playerX, playerY + 3 * 4, 16, 4, collisions[i].x * 4, collisions[i].y * 4, collisions[i].width * 4, collisions[i].height * 4))
					{
						playerX = (collisions[i].x + collisions[i].width) * 4;
						break;
					}
				}
			}
			else if (keyboard[SDL_SCANCODE_RIGHT])
			{
				playerX += 1;
				for (int i = 0; i < collisions.size(); i++)
				{
					if (AreColliding(playerX, playerY + 3 * 4, 16, 4, collisions[i].x * 4, collisions[i].y * 4, collisions[i].width * 4, collisions[i].height * 4))
					{
						playerX = collisions[i].x * 4 - 16;
						break;
					}
				}
			}
		}
		if (keyboard[SDL_SCANCODE_UP] != keyboard[SDL_SCANCODE_DOWN])
		{
			if (keyboard[SDL_SCANCODE_UP])
			{
				playerY -= 1;
				for (int i = 0; i < collisions.size(); i++)
				{
					if (AreColliding(playerX, playerY + 3 * 4, 16, 4, collisions[i].x * 4, collisions[i].y * 4, collisions[i].width * 4, collisions[i].height * 4))
					{
						playerY = (collisions[i].y + collisions[i].height - 3) * 4;
						break;
					}
				}
			}
			else if (keyboard[SDL_SCANCODE_DOWN])
			{
				playerY += 1;
				for (int i = 0; i < collisions.size(); i++)
				{
					if (AreColliding(playerX, playerY + 3 * 4, 16, 4, collisions[i].x * 4, collisions[i].y * 4, collisions[i].width * 4, collisions[i].height * 4))
					{
						playerY = collisions[i].y * 4 - 16;
						break;
					}
				}
			}
		}
	}

	SDL_GetMouseState(&mouseX, &mouseY);
	mouseX /= SCALE;
	mouseY /= SCALE;

	inPreview = AreColliding(mouseX, mouseY, 1, 1, 640 - 128, 0, 128, 128);
	if (inPreview)
	{
		previewHoveredX = (mouseX - 640 + 128) / 16;
		previewHoveredY = mouseY / 16;
	}
	else
	{
		if (collisionsEditorMode)
		{
			placementX = mouseX / 4;
			placementY = mouseY / 4;
		}
		else
		{
			placementX = mouseX / 8;
			placementY = mouseY / 8;
		}

		for (int i = static_cast<int>(tiles.size()) - 1; i >= 0; i--)
		{
			if (AreColliding(mouseX, mouseY, 1, 1, tiles[i].x * 8, tiles[i].y * 8, 16, 16))
			{
				tileTargetedIndex = i;
				break;
			}
		}

		for (int i = static_cast<int>(collisions.size()) - 1; i >= 0; i--)
		{
			if (AreColliding(mouseX, mouseY, 1, 1, collisions[i].x * 4, collisions[i].y * 4, collisions[i].width * 4, collisions[i].height * 4))
			{
				collisionTargetedIndex = i;
				break;
			}
		}
	}
}

void Editor::RenderFrame()
{
	SDL_SetRenderTarget(renderer, backBuffer);

	if (testMode)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
	}
	SDL_RenderClear(renderer);

	// background tiles
	for (int i = 0; i < tiles.size(); i++)
	{
		if (!tiles[i].foreground || playerY > tiles[i].y * 8) RenderTile(i);
	}

	// player
	if (testMode)
	{
		SDL_Rect playerRect = { playerX, playerY, 16, 16 };
		SDL_SetRenderDrawColor(renderer, 0, 255, 255, 128);
		SDL_RenderFillRect(renderer, &playerRect);
		SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &playerRect);
	}

	// foreground tiles
	for (int i = 0; i < tiles.size(); i++)
	{
		if (tiles[i].foreground && playerY <= tiles[i].y * 8) RenderTile(i);
	}

	// collisions
	if (collisionsEditorMode)
	{
		for (int i = 0; i < collisions.size(); i++)
		{
			SDL_Rect collisionDstRect = { collisions[i].x * 4, collisions[i].y * 4, collisions[i].width * 4, collisions[i].height * 4 };
			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128);
			SDL_RenderFillRect(renderer, &collisionDstRect);
			if (eraseMode && !inPreview && i == collisionTargetedIndex)
			{
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
			}
			SDL_RenderDrawRect(renderer, &collisionDstRect);
		}
	}

	if (!inPreview && !eraseMode && !foregroundEditorMode)
	{
		if (collisionsEditorMode)
		{
			// collision placement
			SDL_Rect collisionPlacementDstRect = { placementX * 4, placementY * 4, collisionWidth * 4, collisionHeight * 4 };
			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128);
			SDL_RenderFillRect(renderer, &collisionPlacementDstRect);
			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
			SDL_RenderDrawRect(renderer, &collisionPlacementDstRect);
		}
		else
		{
			// tile placement
			SDL_Rect tilePlacementSrcRect = { selectedX * 16, selectedY * 16, 16, 16 };
			SDL_Rect tilePlacementDstRect = { placementX * 8, placementY * 8, 16, 16 };
			SDL_SetTextureAlphaMod(spritesTexture, 128);
			SDL_RenderCopy(renderer, spritesTexture, &tilePlacementSrcRect, &tilePlacementDstRect);
			SDL_SetTextureAlphaMod(spritesTexture, 255);
		}
	}

	// grid
	if (showGrid > 0)
	{
		if (showGrid > 1)
		{
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 8);
			for (int i = 0; i < 640 / 16; i++)
			{
				SDL_Rect verticalLine = { i * 16 - 1 + 8, 0, 2, 480 };
				SDL_RenderFillRect(renderer, &verticalLine);
			}
			for (int i = 0; i < 480 / 16; i++)
			{
				SDL_Rect horizontalLine = { 0, i * 16 - 1 + 8, 640, 2 };
				SDL_RenderFillRect(renderer, &horizontalLine);
			}
		}

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 16);
		for (int i = 0; i < 640 / 16 + 1; i++)
		{
			SDL_Rect verticalLine = { i * 16 - 1, 0, 2, 480 };
			SDL_RenderFillRect(renderer, &verticalLine);
		}
		for (int i = 0; i < 480 / 16 + 1; i++)
		{
			SDL_Rect horizontalLine = { 0, i * 16 - 1, 640, 2 };
			SDL_RenderFillRect(renderer, &horizontalLine);
		}
	}

	if (!testMode)
	{
		// global preview background
		SDL_Rect globalPreviewBackgroundRect = { 640 - 128, 0, 128, 128 };
		SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
		SDL_RenderFillRect(renderer, &globalPreviewBackgroundRect);

		// global preview
		SDL_Rect globalPreviewDstRect = { 640 - 128, 0, 128, 128 };
		SDL_RenderCopy(renderer, spritesTexture, nullptr, &globalPreviewDstRect);

		// preview selected
		SDL_Rect previewSelectedRect = { selectedX * 16 + 640 - 128, selectedY * 16, 16, 16 };
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderDrawRect(renderer, &previewSelectedRect);

		// preview hovered
		if (inPreview && !(previewHoveredX == selectedX && previewHoveredY == selectedY))
		{
			SDL_Rect previewHoveredRect = { previewHoveredX * 16 + 640 - 128, previewHoveredY * 16, 16, 16 };
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderDrawRect(renderer, &previewHoveredRect);
		}
	}

	// erase mode borders
	if (eraseMode)
	{
		SDL_Rect eraseModeBordersRect = { 0, 0, 640, 480 };
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderDrawRect(renderer, &eraseModeBordersRect);
	}

	// foreground editor mode borders
	if (foregroundEditorMode)
	{
		SDL_Rect foregroundEditorModeBordersRect = { 0, 0, 640, 480 };
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
		SDL_RenderDrawRect(renderer, &foregroundEditorModeBordersRect);
	}

	SDL_SetRenderTarget(renderer, nullptr);
	SDL_RenderCopy(renderer, backBuffer, nullptr, nullptr);

	SDL_RenderPresent(renderer);
}

void Editor::Destroy()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	IMG_Quit();
	SDL_Quit();
}

void Editor::RenderTile(int index)
{
	SDL_Rect tileSrcRect = { tiles[index].spriteX * 16, tiles[index].spriteY * 16, 16, 16};
	SDL_Rect tileDstRect = { tiles[index].x * 8, tiles[index].y * 8, 16, 16 };
	SDL_RenderCopy(renderer, spritesTexture, &tileSrcRect, &tileDstRect);

	if (foregroundEditorMode)
	{
		if (index == tileTargetedIndex)
		{
			SDL_Rect tileTargetedRect = { tiles[index].x * 8, tiles[index].y * 8, 16, 16 };
			if (tiles[index].foreground)
			{
				SDL_SetRenderDrawColor(renderer, 192, 192, 0, 255);
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			}
			SDL_RenderDrawRect(renderer, &tileTargetedRect);
		}
		else if (tiles[index].foreground)
		{
			SDL_Rect tileTargetedRect = { tiles[index].x * 8, tiles[index].y * 8, 16, 16 };
			SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
			SDL_RenderDrawRect(renderer, &tileTargetedRect);
		}
	}

	if (eraseMode && !collisionsEditorMode && index == tileTargetedIndex)
	{
		SDL_Rect tileTargetedRect = { tiles[index].x * 8, tiles[index].y * 8, 16, 16 };
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderDrawRect(renderer, &tileTargetedRect);
	}
}

bool Editor::AreColliding(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2)
{
	return x1 + width1 > x2 && y1 + height1 > y2 && x1 < x2 + width2 && y1 < y2 + height2;
}