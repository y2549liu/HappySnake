#pragma once

/* c++ 标准头 */
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <list>
#include <algorithm> 
#include <time.h>

/* qt头 */
#include <QtWidgets/QMainWindow>
#include <QEvent>
#include <QKeyEvent>
#include "common_cfgfile_tools.h"

using std::list;
using std::ifstream;
using std::move;


#define SNAKEOCCUPIED 3
#define FOODOCCUPIED 2
#define AVAILABLE 1
#define SNAKEOVERFLOW 0


template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
class Mylist :public list<_Tp, _Alloc>
{
public:
	typedef typename list<_Tp, _Alloc>::const_iterator _Mylist_const_iterator;

	inline _Tp operator[](size_t n)
	{
		size_t size = this->size();
		if (n >= size)
			return move(this->back());

		else if (n < size / 2)
		{
			_Mylist_const_iterator iter = this->begin();
			while (n--)
				iter++;
			return move(*iter);
		}
		else
		{
			_Mylist_const_iterator iter = this->end();
			n = size - n;
			while (n--)
				iter--;
			return move(*iter);
		}
	}
	inline _Tp random_pop()
	{
		size_t size = this->size();
		if (size == 0)
			throw("error");

		size_t n = rand() % size;
		
		if (n < size / 2)
		{
			_Mylist_const_iterator iter = this->begin();
			while (n--)
				iter++;
			_Tp _tmp= *iter;
			this->erase(iter);
			return move(_tmp);
		}
		else
		{
			_Mylist_const_iterator iter = this->end();
			n = size - n;
			while (n--)
				iter--;
			_Tp _tmp = *iter;
			this->erase(iter);
			return _tmp;
		}
	}
};




enum class snakePixelType {SOVERFLOW, EMPTY, FOOD, SNAKE_HEAD, SNAKE_BODY, BRICK};
enum class snakeDirection { LEFT, RIGHT, UP, DOWN };

class HappySnake;
class snakePixel;

class snakeConfig
{
private:
	int r, c;
	int foodNum;
	int score;
	int bgColor;
	int blockSize;
	int speed;
public:
	void readConfig(const char* str="HappySnake.conf");

	friend HappySnake;
};

class snakePixel {
private:
	QLabel *square;
	int x, y;
	snakePixelType type;
public:
	void setType(const snakePixelType &_type, const QString &style);
	void applyType(const QString &style);

	snakePixel(QWidget *parent=nullptr);
	~snakePixel();
	friend HappySnake;
};
 
struct coord {
	int x, y;
	coord(int _x=0, int _y=0) :x(_x), y(_y) {};
	bool operator==(const coord&) const;
};

class HappySnake : public QMainWindow
{
	Q_OBJECT
private:
	int w, h;
	Mylist<coord> snakeLoc;
	Mylist<coord> foodLoc;
	Mylist<coord> emptyLoc;
	QTimer timer;
	QString style;
	snakeConfig config;
	QPushButton *startGame;
	QLabel *scoreLabel;
	QFont font;
	snakePixel **snakePixelMap;
	snakeDirection d;
	coord snakeHead, snakeTail;
	int snakeLen;

	int check(const coord &Loc) const;
	void printSnake();
	bool checkGame();
	void gameOver(const QString &msg="");
	void init();
	void reset();
	void initSnake();
	void placeFood();
	void getSnakeHeadTail();


protected:
	void keyPressEvent(QKeyEvent *event);


public:
	HappySnake(QWidget *parent = Q_NULLPTR);
	~HappySnake();

private slots:
	void snakeMoveSlot();
	void startGameSlot();
};
