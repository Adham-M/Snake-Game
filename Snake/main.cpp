﻿#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include "..\CMUgraphics\CMUgraphics.h"
#include <iostream>
#include <fstream>
#include <direct.h>
#include <cstring>
#include "Queue.h"
#include "string.h"
#include <time.h>
using namespace std;

#define KEY_UP 'w'
#define KEY_DOWN 's'
#define KEY_LEFT 'a'
#define KEY_RIGHT 'd'
#define KEY_PAUSE 'p'
#define KEY_QUIT 'q'

#define KEY_UP_c 'W'
#define KEY_DOWN_c 'S'
#define KEY_LEFT_c 'A'
#define KEY_RIGHT_c 'D'
#define KEY_PAUSE_c 'P'
#define KEY_QUIT_c 'Q'

#define ARROW_UP '\b'
#define ARROW_DOWN '\x2'
#define ARROW_LEFT '\x4'
#define ARROW_RIGHT '\x6'



bool quit = false;
bool pause = false;

//You can customize these parameters and the parameters in class Output
int Speed = 50; //The speed of the game, better be multiple of 5
bool bourderKills = false;


enum Directions
{
	UP, RIGHT, DOWN, LEFT
};

struct Point
{
	int x;
	int y;
};

struct UI_Info	//User Interface Info.
{
	int	dw, dh,				//Maximum snake width and height
		wx, wy,				//Window starting coordinates
		width, height,		//Window width and height
		StatusBarHeight,	//Status Bar Height
		SnakeSize,			//Snack body parts size
		TargetRadius;		//Target size

	color SnakeColor;		//Snake color
	color MsgColor;			//Messages color
	color BkGrndColor;		//Background color
	color StatusBarColor;	//Status bar color
	int PenWidth;			//width of the pen that draws shapes
	int StatusBarLineWidth;	//width of the above under the status bar


}UI;	//create a global object UI


class Output   // forward declaration
{
private:
	window* pWind;	//Pointer to the Graphics Window
public:
	Output()
	{
		UI.dw = 40; //Maximum snake parts fitting in the x-axis
		UI.dh = 30; //Maximum snake parts fitting in the y-axis 

		UI.SnakeSize = 18;
		UI.TargetRadius = UI.SnakeSize / 2;

		UI.StatusBarHeight = 50;
		UI.StatusBarLineWidth = 2;

		UI.wx = 350;
		UI.wy = 125;

		UI.width = (UI.dw /*+ UI.wx*/) * UI.SnakeSize + 15;
		UI.height = (UI.dh /*+ UI.wy*/) * UI.SnakeSize + UI.StatusBarHeight + UI.StatusBarLineWidth;


		UI.SnakeColor = WHITE;
		UI.MsgColor = WHITE;		//Messages color
		UI.BkGrndColor = BLACK;		//Background color
		UI.StatusBarColor = BLACK;
		UI.PenWidth = 3;			//width of the figures frames
		UI.StatusBarLineWidth = 2;	//width of the line under the tool bar

		//Create the output window
		pWind = CreateWind(UI.width, UI.height, UI.wx, UI.wy);
		//Change the title
		pWind->ChangeTitle("Snake game");

		CreateStatusBar();
	}


	window* CreateWind(int w, int h, int x, int y) const	//creates the application window
	{
		window* pW = new window(w, h, x, y);
		pW->SetBrush(UI.BkGrndColor);
		pW->SetPen(UI.BkGrndColor, 1);
		pW->DrawRectangle(0, 0, w, h);
		return pW;
	}

	window* GetWindow()const
	{
		return pWind;
	}

	void CreateStatusBar() const	//create the status bar
	{
		pWind->SetPen(WHITE, UI.StatusBarLineWidth);
		pWind->DrawLine(0, UI.height - UI.StatusBarHeight, UI.width, UI.height - UI.StatusBarHeight);
	}


	void ClearArea() const
	{
		pWind->SetBrush(UI.BkGrndColor);
		pWind->SetPen(UI.BkGrndColor, 1);
		pWind->DrawRectangle(0, 0, UI.width, UI.height - UI.StatusBarHeight + UI.StatusBarLineWidth);
	}

	void ClearStatusBar() const
	{
		//Clear Status bar by drawing a filled white rectangle
		pWind->SetPen(WHITE, UI.StatusBarLineWidth);
		pWind->DrawLine(0, UI.height - UI.StatusBarHeight, UI.width, UI.height - UI.StatusBarHeight);

		pWind->SetPen(UI.StatusBarColor, 1);
		pWind->SetBrush(UI.StatusBarColor);
		pWind->DrawRectangle(0, UI.height - UI.StatusBarHeight, UI.width, UI.height);
	}

	void PrintMessage(string msg) const	//Prints a message on status bar
	{
		ClearStatusBar();	//First clear the status bar

		pWind->SetPen(UI.MsgColor, 50);
		pWind->SetFont(20, BOLD, BY_NAME, "Arial");
		pWind->DrawString(10, UI.height - (int)(UI.StatusBarHeight / 1.2), msg);
	}

	void PrintMessage(string msg, int s) const	//Prints a message on status bar
	{
		pWind->SetPen(UI.MsgColor, 50);
		pWind->SetFont(20, BOLD, BY_NAME, "Arial");
		pWind->DrawString(s, UI.height - (int)(UI.StatusBarHeight / 1.2), msg);
	}

};


class BodyPart
{
	Point cord;
public:
	void SetCord(int ix, int iy)
	{
		cord.x = ix;
		cord.y = iy;
	}

	Point GetCord() const
	{
		return cord;
	}

	void DrawPart(Output* pout)
	{
		window* w = pout->GetWindow();
		w->SetPen(BLACK, 1);
		w->SetBrush(UI.SnakeColor);
		w->DrawRectangle(cord.x * UI.SnakeSize, cord.y * UI.SnakeSize, cord.x * UI.SnakeSize + UI.SnakeSize, cord.y * UI.SnakeSize + UI.SnakeSize, FILLED);
	}

	void ClearPart(Output* pout)
	{
		window* w = pout->GetWindow();
		w->SetPen(UI.BkGrndColor, 1);
		w->SetBrush(UI.BkGrndColor);
		w->DrawRectangle(cord.x * UI.SnakeSize, cord.y * UI.SnakeSize, cord.x * UI.SnakeSize + UI.SnakeSize, cord.y * UI.SnakeSize + UI.SnakeSize, FILLED);
	}
};

class Target
{
	Point position;
	int score;
public:

	Target()
	{
		srand(time(0));
		position.x = ((rand() % (UI.dw - 3)) + 1);
		position.y = ((rand() % (UI.dh - 3)) + 1);
		score = 0;
	}

	bool IsEaten(Point p)
	{
		if (p.x == position.x && p.y == position.y)
			return true;
		return false;
	}

	Point ChangeTarPos()
	{
		srand(time(0));
		position.x = ((rand() % (UI.dw - 3)) + 1);
		position.y = ((rand() % (UI.dh - 3)) + 1);

		return position;
	}

	Point ChangeTarPos(int x, int y)
	{
		position.x = position.x + x < UI.dw ? position.x + x : position.x + x - UI.dw;
		position.y = position.y + y < UI.dh ? position.y + y : position.y + y - UI.dh;
		return position;
	}

	int GetScore() const
	{
		return score;
	}

	void IncScore()
	{
		score++;
	}

	void DrawTarget(Output* pout) const
	{
		window* w = pout->GetWindow();
		w->SetPen(BLACK, 1);
		w->SetBrush(UI.SnakeColor);
		//w->DrawRectangle(position.x * UI.SnakeSize, position.y * UI.SnakeSize, position.x * UI.SnakeSize + UI.SnakeSize, position.y * UI.SnakeSize + UI.SnakeSize, FILLED);	//This function draws the target in a square form
		w->DrawCircle(position.x * UI.SnakeSize + UI.TargetRadius, position.y * UI.SnakeSize + UI.TargetRadius, UI.TargetRadius, FILLED);	//This function draws the target in a circle form
	}
};

class Snake
{
	BodyPart* head;
	BodyPart* tail;
	int CurrentDirection;
	BodyPart* body[500];
	Queue<BodyPart*> snake;
	int count = 4;
	bool alive;
	Output* pOut;

public:
	Snake(Output* pout)
	{
		alive = true;
		CurrentDirection = RIGHT;
		pOut = pout;
		head = new BodyPart;
		BodyPart * p1 = new BodyPart;
		BodyPart * p2 = new BodyPart;
		tail = new BodyPart;
		body[0] = head;
		body[1] = p1;
		body[2] = p2;
		body[3] = tail;
		int k = UI.dw / 3;
		head->SetCord(--k, UI.dh / 2);
		p1->SetCord(--k, UI.dh / 2);
		p2->SetCord(--k, UI.dh / 2);
		tail->SetCord(--k, UI.dh / 2);


		int i = 3;
		while (i > -1)
		{
			body[i]->DrawPart(pOut);
			snake.enqueue(body[i]);
			i--;
		}
	}

	void drawSnake(Output* pout, Target tar)
	{
		pOut->ClearArea();

		int i = 0;
		do{
			body[i]->DrawPart(pOut);
		} while (body[++i]);
		
		tar.DrawTarget(pOut);


		// turning the head gray
		if (pause)
			drawHead(pout);
	}

	bool move(Output* pout, Target tar)
	{
		window* w = pout->GetWindow();
		Point h = head->GetCord();
		int org = CurrentDirection;


		//If the user gave an action it is processed
		char current;

		if (w->GetKeyPress(current))
		{
			if (pause)
			{ pause = false; drawSnake(pout, tar); }
			
			if (current == KEY_UP || current == KEY_UP_c || current == ARROW_UP)
				CurrentDirection = UP;
			else if (current == KEY_RIGHT || current == KEY_RIGHT_c || current == ARROW_RIGHT)
				CurrentDirection = RIGHT;
			else if (current == KEY_DOWN || current == KEY_DOWN_c || current == ARROW_DOWN)
				CurrentDirection = DOWN;
			else if (current == KEY_LEFT || current == KEY_LEFT_c || current == ARROW_LEFT)
				CurrentDirection = LEFT;
			else if (current == KEY_QUIT || current == KEY_QUIT_c)
			{ quit = true; return false; }
			else if (current == KEY_PAUSE || current == KEY_PAUSE_c)
			{ pause = true; drawSnake(pout, tar); }
		}

		//If the user entered a reverse direction the snake will keep moving forward
		if (CurrentDirection == UP && org == DOWN)	CurrentDirection = DOWN;
		else if (CurrentDirection == RIGHT && org == LEFT)	CurrentDirection = LEFT;
		else if (CurrentDirection == DOWN && org == UP)	CurrentDirection = UP;
		else if (CurrentDirection == LEFT && org == RIGHT)	CurrentDirection = RIGHT;

		//Moving the snake
		if (pause) return false;
		if (CurrentDirection == UP)	h.y -= 1;
		else if (CurrentDirection == RIGHT)	h.x += 1;
		else if (CurrentDirection == DOWN)	h.y += 1;
		else if (CurrentDirection == LEFT)	h.x -= 1;

		//If the snake passed the wall it appears from the second one
		if (h.y < 0)
		{
			h.y = UI.dh - 1;
			alive = false;
		}
		else if (h.y > UI.dh - 1)
		{
			h.y = 0;
			alive = false;
		}
		else if (h.x < 0)
		{
			h.x = UI.dw - 1;
			alive = false;
		}
		else if (h.x > UI.dw - 1)
		{
			h.x = 0;
			alive = false;
		}

		if (!alive)
			if (!bourderKills)
				alive = true;



		//Cheaking if the snake killed itself
		snake.peekFront(tail);
		Point t = tail->GetCord();
		if (h.x != t.x || h.y != t.y)
			for (int i = 0; i < count; i++)
			{
				Point p = body[i]->GetCord();
				if (p.x == h.x && p.y == h.y)
					alive = false;
			}


		if (!tar.IsEaten(h)) //If the snake didnt eat
		{
			BodyPart* par;
			snake.dequeue(par);
			par->ClearPart(pOut);	//Clear tail
			par->SetCord(h.x, h.y);	//Set its new coords
			par->DrawPart(pOut);		//Draw it in the head position
			snake.enqueue(par);
			head = par;
			return false;
		}
		else  //If the snake ate
		{
			BodyPart* np = new BodyPart;	//Adding new part to the snake
			np->SetCord(h.x, h.y);			//Its coords is the same as the target
			np->DrawPart(pOut);
			snake.enqueue(np);
			head = np;
			body[count++] = np;
			return true;
		}
	}

	bool stillalive() const
	{
		return alive;
	}

	void drawHead(Output* pOut) const	//Draws the head of the snake
	{
		window* w = pOut->GetWindow();

		Point pos = head->GetCord();
		w->SetPen(BLACK, 1);
		w->SetBrush(GRAY);
		w->DrawRectangle(pos.x * UI.SnakeSize, pos.y * UI.SnakeSize, pos.x * UI.SnakeSize + UI.SnakeSize, pos.y * UI.SnakeSize + UI.SnakeSize, FILLED);
	}

	bool PosAvailable(Point t)
	{
		bool ok = true;
		for (int i = 0; i < count; i++)
		{
			Point p = body[i]->GetCord();
			if (p.x == t.x && p.y == t.y)
				ok = false;
		}
		return ok;
	}
};

void PrintScores(Output* pout, int cs, int hs)
{
	pout->PrintMessage("Score = " + to_string(cs));
	string s_hs = to_string(hs);
	pout->PrintMessage("High Score = " + s_hs, UI.width - 160 - (s_hs.length() * 6));
}

void printTrash(Output* pout, string tt) {
	pout->PrintMessage(tt);
}

int loadHighScore(Output* pout)
{
	char* pValue;
	size_t len;
	errno_t err = _dupenv_s(&pValue, &len, "APPDATA");
	string path;
	if (pValue != nullptr) {
		path = string(pValue) + "\\snakeGame\\data.txt";
		ifstream myfile(path);
		if (myfile.is_open()) {
			int temp;
			try 
			{
				myfile >> temp;
				return temp;
			}
			catch (exception e) 
			{
				return 0;
			}
		}
	}
	return 0;
}

void saveHighScore(int hs)
{
	char* pValue;
	size_t len;
	errno_t err = _dupenv_s(&pValue, &len, "APPDATA");

	string path, dirPath;
	if (pValue != nullptr) {
		path = string(pValue) + "\\snakeGame\\data.txt";
		dirPath = string(pValue) + "\\snakeGame";

		ofstream myfile;

		myfile.open(path);

		if (myfile.is_open()) 
		{
			myfile << hs << endl;
		}
		else {
			char* char_array;
			char_array = &dirPath[0];
			if (_mkdir(char_array)) {
				myfile.open(path);
				if (myfile.is_open())
					myfile << hs << endl;
			}
		}

		myfile.close();
	} 
}

int main()
{
	Output* pOut = new Output;

	Target* tar = new Target;
	Snake* snak = new Snake(pOut);

	int hs = loadHighScore(pOut);
	
	int cs = 0;

	tar->DrawTarget(pOut);

	PrintScores(pOut, cs, hs);

	bool scorePrinted = false;

	while (!quit)
	{
		if (snak->move(pOut, *tar))	//If the snake ate the target the "move" function returns true
		{
			tar->IncScore();		//Incrementing the score
			Point p = tar->ChangeTarPos();		//Changing target position
			while (!snak->PosAvailable(p))		//Cheak if it is a valid position
				p = tar->ChangeTarPos(rand() % 5, rand() % 5);
			tar->DrawTarget(pOut);		//Drawing the target
			cs = tar->GetScore();
			if (cs > hs) //Cheak if it is a high score or not
			{
				hs = cs;
				saveHighScore(hs);
			}
			PrintScores(pOut, cs, hs);		//printing scores
		}

		if (!snak->stillalive())	//If the snake ate itself it return true 
		{
			//Deleting last game objects and creating new
			delete snak;
			delete tar;
			tar = new Target;
			snak = new Snake(pOut);
			pOut->ClearArea();
			tar->DrawTarget(pOut);
			cs = tar->GetScore();
			PrintScores(pOut, cs, hs);
		}

		if (pause && !scorePrinted) {
			PrintScores(pOut, cs, hs);
			scorePrinted = true;
		}
		else if (!pause && scorePrinted){
			PrintScores(pOut, cs, hs);
			scorePrinted = false;
		}


		//Speed of the game is customizable
		if (Speed*1.3 - (cs / 10) > Speed)
			Sleep(Speed*1.3 - (cs / 10));
		else
			Sleep(Speed - (cs - Speed * 3) / Speed * 0.3);
	}
	
	return 0;
}
