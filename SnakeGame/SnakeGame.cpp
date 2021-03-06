// SnakeGame.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SnakeGame.h"

//TODO: Fix bug where energy can spawn under snake
//TODO: Score display at end of game
//TODO: Get timing information for the game (not sure if it slows down once the snake gets super long.

void PropogateMovement(Snake *thisSnake)
{
	Snake *traversal = thisSnake;
	while (traversal->next != nullptr)
	{
		traversal = traversal->next;
	}
	while (traversal->prior != nullptr)
	{
		traversal->moveNext = traversal->prior->moveNext;
		traversal = traversal->prior;
	}
}

void ClearPlayAreaBuffer(unsigned char *playField)
{
	for (int i = 0; i < FIELD_WIDTH * FIELD_HEIGHT; i++) {
		playField[i] = ' ';
	}
}

void ClearStatusAreaBuffer(unsigned char *statusArea)
{
	for (int i = 0; i < FIELD_WIDTH * 5; i++) {
		statusArea[i] = ' ';
	}
}

void ClearScreenBuffer(wchar_t *screenBuffer)
{
	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
	{
		screenBuffer[i] = L' ';
	}
}

void ResetPlayArea(unsigned char *playField, int pelletX, int pelletY)
{
	for (int x = 0; x < FIELD_WIDTH; x++)
	{
		for (int y = 0; y < FIELD_HEIGHT; y++)
		{
			if (y == 0 || x == 0 || y == (FIELD_HEIGHT - 1) || x == (FIELD_WIDTH - 1))
			{
				playField[(y * FIELD_WIDTH + x)] = L'B';
			}
			else if (x == pelletX && y == pelletY) {
				playField[(y * FIELD_WIDTH + x)] = L'P';
			}
			else
			{
				playField[(y * FIELD_WIDTH + x)] = L' ';
			}
		}
	}
}

void RenderSnakeInField(Snake * snake, unsigned char *playField)
{
	Snake *traversal = snake;
	while (traversal != nullptr)
	{
		playField[(traversal->posY * FIELD_WIDTH) + traversal->posX] = traversal->renderChar;
		traversal = traversal->next;
	}
}

void RenderGameOverScreen(int score, unsigned char *playField)
{
	for (int x = 0; x < FIELD_WIDTH * FIELD_HEIGHT; x++)
	{
		playField[x] = ' ';
	}

	for (int i = 0; i < 9; i++)
	{
		int startPos = (FIELD_WIDTH / 2) - 5;
		playField[startPos + i] = "GAME OVER"[i];
	}

	char strBuffer[30];
	for (int i = 0; i < 30; i++)
	{
		strBuffer[i] = ' ';
	}
	int len = sprintf_s(strBuffer, sizeof(strBuffer), "Your final score is %i.", score);

	for(int i = 0; i < len; i++)
	{ 
		int startPos = (FIELD_WIDTH + 1);
		playField[startPos + i] = strBuffer[i];
	}
}

void MoveSnake(Snake *thisSnake)
{
	Snake *traversal = thisSnake;
	while (traversal != nullptr) {
		switch (traversal->moveNext) 
		{
		case DIR_DOWN:
			traversal->posY++;
			break;
		case DIR_LEFT:
			traversal->posX--;
			break;
		case DIR_UP:
			traversal->posY--;
			break;
		case DIR_RIGHT:
			traversal->posX++;
			break;
		}
		traversal = traversal->next;
	}
}

snakeCollisionResult DetectSnakeCollision(Snake *thisSnake, unsigned char *field)
{
	Snake *traversal = thisSnake->next;
	while (traversal != nullptr)
	{
		if ((thisSnake->posX == traversal->posX) && (thisSnake->posY == traversal->posY)) {
			return COLLISION_OBSTACLE;
		}
		traversal = traversal->next;
	}

	switch (field[(thisSnake->posY * FIELD_WIDTH) + thisSnake->posX])
	{
	case 'S':
	case 'B':
		return COLLISION_OBSTACLE;
	case 'P':
		return COLLISION_ENERGY;
	default: 
		return COLLISION_NONE;
	}
}


void RenderBufferToScreenBuffer(unsigned char *playField, wchar_t *screenBuffer, int width = FIELD_WIDTH, int height = FIELD_HEIGHT, int startLine = 0)
{
	for (int x = 0; x < width; x++) 
	{
		for (int y = 0; y < height; y++) 
		{
			screenBuffer[(startLine * SCREEN_WIDTH) + ((y * (SCREEN_WIDTH + 2)) + x + 2)] = playField[(y * width) + x];
		}
	}
}

void AddToEndOfSnake(Snake *thisSnake)
{
	Snake *traversal = thisSnake;
	while (traversal->next != nullptr)
	{
		traversal = traversal->next;
	}
	Snake *newSnake = new Snake();
	newSnake->next = nullptr;
	newSnake->prior = nullptr;
	newSnake->moveNext = traversal->moveNext;
	newSnake->posX = traversal->posX;
	newSnake->posY = traversal->posY;	
	newSnake->renderChar = 'S';

	switch (traversal->moveNext) {
	case DIR_DOWN:
		newSnake->posY--;
		break;
	case DIR_RIGHT:
		newSnake->posX--;
		break;
	case DIR_UP:
		newSnake->posY++;
		break;
	case DIR_LEFT:
		newSnake->posX++;
		break;
	}
	newSnake->prior = traversal;
	traversal->next = newSnake;
}

int main()
{
	//initialize game
	bool gameOver = false;
	bool isPaused = true;
	
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> pelletXGen(2, FIELD_WIDTH - 2);
	std::uniform_int_distribution<int> pelletYGen(2, FIELD_HEIGHT - 2);
	
	Snake *snake = new Snake();
	snake->next = nullptr;
	snake->moveNext = DIR_RIGHT;
	snake->posX = FIELD_WIDTH / 2;
	snake->posY = FIELD_HEIGHT / 2;
	snake->renderChar = 'S';

	int pelletX = pelletXGen(gen);
	int pelletY = pelletYGen(gen);
	int moveCycleCounter = 0;

	int score = 0;
			
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);
	DWORD BytesWritten = 0;
	HWND ConsoleWindow = GetConsoleWindow();
	MoveWindow(ConsoleWindow, 10, 10, SCREEN_WIDTH * 8.48, SCREEN_HEIGHT * 14, true);

	//represents the game state
	unsigned char *Field = new unsigned char[FIELD_WIDTH * FIELD_HEIGHT];
	ClearPlayAreaBuffer(Field);
	//buffer that we will write to, which will write to the console
	wchar_t *screen = new wchar_t[SCREEN_WIDTH * SCREEN_HEIGHT];
	ClearScreenBuffer(screen);
	//buffer for holding game status information
	unsigned char *StatusDisplay = new unsigned char[FIELD_WIDTH * 5];

	//TODO: Resize console to fit game, if console isn't appropriate size for it.

	while (!gameOver)
	{
		ResetPlayArea(Field, pelletX, pelletY);
		//Game Timing
		Sleep(MS_PER_GAME_CYCLE);
		//Player Input
		

		bool moveUp = ((KEYSTATE_SIGNIFICANT_BIT & GetAsyncKeyState(VK_UP)) ||
			(KEYSTATE_SIGNIFICANT_BIT & GetAsyncKeyState('W')));

		bool moveDown = ((KEYSTATE_SIGNIFICANT_BIT & GetAsyncKeyState(VK_DOWN)) ||
			(KEYSTATE_SIGNIFICANT_BIT & GetAsyncKeyState('S')));

		bool moveLeft = ((KEYSTATE_SIGNIFICANT_BIT & GetAsyncKeyState(VK_LEFT)) ||
			(KEYSTATE_SIGNIFICANT_BIT & GetAsyncKeyState('A')));

		bool moveRight = ((KEYSTATE_SIGNIFICANT_BIT & GetAsyncKeyState(VK_RIGHT)) ||
			(KEYSTATE_SIGNIFICANT_BIT & GetAsyncKeyState('D')));


		if (moveUp && snake->moveNext != DIR_DOWN)
		{
			snake->moveNext = DIR_UP;
		}

		if (moveDown && snake->moveNext != DIR_UP)
	{
			snake->moveNext = DIR_DOWN;
		}

		if (moveLeft && snake->moveNext != DIR_RIGHT)
		{
			snake->moveNext = DIR_LEFT;
		}

		if (moveRight && snake->moveNext != DIR_LEFT)
		{
			snake->moveNext = DIR_RIGHT;
		}

		if (KEYSTATE_SIGNIFICANT_BIT & GetAsyncKeyState(VK_ESCAPE)) {
			gameOver = true;
		}

		if (KEYSTATE_SIGNIFICANT_BIT & GetAsyncKeyState(VK_BACK))
		{
			isPaused = !isPaused;
		}


		//We want to poll game input more frequently than we actually want to update the world state, so we
		//use this to achieve that goal.
		//TODO: Use explicit timing instead of sleep cycle counting to accomplish this goal
		if (!isPaused && moveCycleCounter++ >= MS_PER_SNAKE_MOVE / MS_PER_GAME_CYCLE)
		{
			//Update World State
			MoveSnake(snake);
			PropogateMovement(snake);
			snakeCollisionResult res = DetectSnakeCollision(snake, Field);
			switch (res) {
			case COLLISION_ENERGY:
				AddToEndOfSnake(snake);
				score++;
				pelletX = pelletXGen(gen);
				pelletY = pelletYGen(gen);
				break;
			case COLLISION_OBSTACLE:
				gameOver = true;
				break;

			}

			//Render world state
			RenderSnakeInField(snake, Field);
			RenderBufferToScreenBuffer(Field, screen);
			WriteConsoleOutputCharacter(console, screen, SCREEN_WIDTH * SCREEN_HEIGHT, { 0,0 }, &BytesWritten);
			moveCycleCounter = 0;
			if (gameOver) 
			{
				Sleep(500);
				RenderGameOverScreen(score, Field);
				RenderBufferToScreenBuffer(Field, screen); 
				WriteConsoleOutputCharacter(console, screen, SCREEN_WIDTH * SCREEN_HEIGHT, { 0,0 }, &BytesWritten);
				
				Sleep(5000);
			}
		}
		
	}
	//Free resources

	Snake *traverse = snake;
	while (traverse->next != nullptr)
	{
		traverse = traverse->next;
	}
	while (traverse->prior != nullptr)
	{
		traverse = traverse->prior;
		delete traverse->next;
	}
	delete snake;
	snake = nullptr;

	delete Field;
	Field = nullptr;

	delete screen;
	screen = nullptr;

    return 0;
}

