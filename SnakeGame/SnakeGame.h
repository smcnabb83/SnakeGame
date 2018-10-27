#pragma once
#ifndef _SNAKE_GAME_H
#define _SNAKE_GAME_H

const int KEYSTATE_SIGNIFICANT_BIT = 0x8000;
constexpr int FIELD_WIDTH = 50;
constexpr int FIELD_HEIGHT = 30;
constexpr int SCREEN_WIDTH = 120;
constexpr int SCREEN_HEIGHT = 50;
constexpr int MS_PER_SNAKE_MOVE = 80;
constexpr int MS_PER_GAME_CYCLE = 10;

#include <iostream>
#include <Windows.h>
#include <random>

enum direction
{
	DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT
};

enum snakeCollisionResult
{
	COLLISION_NONE, COLLISION_ENERGY, COLLISION_OBSTACLE
};

struct Snake
{
	Snake *next;
	Snake *prior;
	direction moveNext;
	int posX;
	int posY;
	unsigned char renderChar;
};

void PropogateMovement(Snake *thisSnake);

void ClearPlayAreaBuffer(unsigned char *playField);

void ClearScreenBuffer(wchar_t *screenBuffer);

void ResetPlayArea(unsigned char *playField, int pelletX, int pelletY);

void RenderSnakeInField(Snake * snake, unsigned char *playField);

void MoveSnake(Snake *thisSnake);

snakeCollisionResult DetectSnakeCollision(Snake *thisSnake, unsigned char *field);

void RenderFieldToScreenBuffer(unsigned char *playField, wchar_t *screenBuffer);

void AddToEndOfSnake(Snake *thisSnake);

#endif // !_SNAKE_GAME_H

