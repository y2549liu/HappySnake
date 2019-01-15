#pragma execution_character_set("utf-8")
#include "stdafx.h"
#include "HappySnake.h"



int HappySnake::check(const coord &Loc) const
{
	if (Loc.x < 0 || Loc.x >= config.c || Loc.y < 0 || Loc.y >= config.r)
		return SNAKEOVERFLOW;
	Mylist<coord>::const_iterator iter = snakeLoc.begin();
	while (iter!=snakeLoc.end())
	{
		if ( *iter == Loc)
			return SNAKEOCCUPIED;
		iter++;
	}
	iter = foodLoc.begin();
	while (iter != foodLoc.end())
	{
		if (*iter == Loc)
			return FOODOCCUPIED;
		iter++;
	}
	return AVAILABLE;
}

void HappySnake::printSnake()
{
	int snakeLen = snakeLoc.size();
	if (snakeLen == 0)
		return;

	Mylist<coord>::iterator iter = snakeLoc.begin();
	snakePixelMap[iter->y][iter->x].setType(snakePixelType::SNAKE_HEAD,style);
	iter++;
	while (iter != snakeLoc.end())
	{
		snakePixelMap[iter->y][iter->x].setType(snakePixelType::SNAKE_BODY,style);
		iter++;
	}
}

void HappySnake::keyPressEvent(QKeyEvent * event)
{
	switch (event->key()) {
		case Qt::Key_Right:
			if (d != snakeDirection::RIGHT && d != snakeDirection::LEFT)
			{
				d = snakeDirection::RIGHT;
				snakeMoveSlot();
			}
			break;
		case Qt::Key_Left:
			if (d != snakeDirection::RIGHT && d != snakeDirection::LEFT)
			{
				d = snakeDirection::LEFT;
				snakeMoveSlot();
			}
			break;
		case Qt::Key_Up:
			if (d != snakeDirection::UP && d != snakeDirection::DOWN)
			{
				d = snakeDirection::UP;
				snakeMoveSlot();
			}
			break;
		case Qt::Key_Down:
			if (d != snakeDirection::UP && d != snakeDirection::DOWN)
			{
				d = snakeDirection::DOWN;
				snakeMoveSlot();
			}
			break;
		
		case Qt::Key_Space:
			if (timer.isActive())
				timer.stop();
			else
				timer.start();
			break;
		case Qt::Key_End:
			exit(true);
			break;
		default:
			break;
	}
}

/* 暂时不使用 */
bool HappySnake::checkGame()
{
	return 1;
}

void HappySnake::gameOver(const QString &msg)
{
	timer.stop();
	QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Game Over", msg + u8" Restart the Game?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	/* 退出 */
	if (rb == QMessageBox::No)
		exit(true);

	/* 继续 */
	startGame->setEnabled(true);
	startGame->setText(u8"Restart");
	startGame->show();

	reset();
}

/* constructor */
HappySnake::HappySnake(QWidget *parent): QMainWindow(parent)
{
	config.readConfig();
	/* 全局字体初始化 */
	font.setPointSize(config.blockSize / 3);
	font.setFamily("Microsoft YaHei");
	this->setFont(font);
	/* 背景style */
	int r, g, b;
	r = (config.bgColor >> 16) & (0xff);
	g = (config.bgColor >> 8) & (0xff);
	b = (config.bgColor) & (0xff);
	style = "background-color: rgb(" + QString::number(r) + ',' + QString::number(g) + ',' + QString::number(b) + ')';
	
	/* 标题 */
	this->setWindowTitle("Happy Snake");
	w = (config.blockSize) * (config.c);
	h = (config.blockSize) * (config.r + 1);
	this->resize(w,h);


	scoreLabel = new QLabel(u8"Score: ",this);
	scoreLabel->resize(w, config.blockSize);
	scoreLabel->setStyleSheet("QLabel{color : red}");

	init();
	
	/* 开始游戏按钮 */
	startGame = new QPushButton(this);
	startGame->setText(u8"Start Game!");
	startGame->setGeometry(w / 2 - 2 * config.blockSize, h / 2 - config.blockSize, 4 * config.blockSize, config.blockSize* 1.5);
	startGame->setStyleSheet("QPushButton{ background-color: rgb(127,127,255); border-radius: 5px; font: bold; color: white; }");

	QObject::connect(startGame, SIGNAL(clicked()), this, SLOT(startGameSlot()));
	QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(snakeMoveSlot()));
}

HappySnake::~HappySnake()
{
	delete startGame;
	delete scoreLabel;
	for (int i = 0; i < config.r; i++)
		delete[] snakePixelMap[i];
	delete[] snakePixelMap;
}



void HappySnake::init()
{
	snakePixelMap = new snakePixel* [config.r];
	for (int i = 0; i < config.r; i++)
	{
		snakePixelMap[i] = new snakePixel[ config.c ];
		for (int j = 0; j < config.c; j++)
		{
			emptyLoc.push_back(coord(j,i));
			snakePixelMap[i][j].x = j * config.blockSize;
			snakePixelMap[i][j].y = (i + 1) * config.blockSize;
			snakePixelMap[i][j].square = new QLabel(this);
			snakePixelMap[i][j].square->setGeometry(snakePixelMap[i][j].x, snakePixelMap[i][j].y, config.blockSize, config.blockSize);
			snakePixelMap[i][j].setType(snakePixelType::EMPTY,style);
		}
	}
}

void HappySnake::reset()
{
	Mylist<coord>::const_iterator iter = snakeLoc.begin();
	snakeLoc.clear();
	foodLoc.clear();
	emptyLoc.clear();
	for (int i = 0; i < config.r; i++)
	{
		for (int j = 0; j < config.c; j++)
		{
			emptyLoc.push_back(coord(j, i));
			snakePixelMap[i][j].setType(snakePixelType::EMPTY, style);
		}
	}
	config.score = 0;
	scoreLabel->setText(u8"Score: ");
	this->repaint();
}

void HappySnake::initSnake()
{
	for (int i = 2; i >= 0; i--)
	{
		snakeLoc.push_back(coord(i, 0));
		emptyLoc.remove(coord(i, 0));
	}
	printSnake();
	d = snakeDirection::RIGHT;
	getSnakeHeadTail();
}

void HappySnake::placeFood()
{
	int currentFoodNum = foodLoc.size();
	while (currentFoodNum < config.foodNum)
	{
		if (!emptyLoc.size()) 
			return;
		coord loc = emptyLoc.random_pop();
		snakePixelMap[loc.y][loc.x].setType(snakePixelType::FOOD,style);
		foodLoc.push_back(loc);
		currentFoodNum++;
	}
}

void HappySnake::getSnakeHeadTail()
{
	snakeLen = snakeLoc.size();
	if (snakeLen == 0)
		return;
	snakeHead = snakeLoc.front();
	snakeTail = snakeLoc.back();
}

void HappySnake::startGameSlot()
{
	startGame->hide();
	startGame->setEnabled(false);
	
	initSnake();
	placeFood();

	scoreLabel->setText(u8"Score: 0");
	timer.start( 1000/config.speed );
}
void HappySnake::snakeMoveSlot()
{
	getSnakeHeadTail();
	coord nextPos(snakeHead);
	switch (d) 
	{
		case snakeDirection::UP:	nextPos.y--;	break;
		case snakeDirection::DOWN:	nextPos.y++;	break;
		case snakeDirection::LEFT:	nextPos.x--;	break;
		case snakeDirection::RIGHT:	nextPos.x++;	break;
	}
	int nextPosObj = check(nextPos);
	if (nextPosObj == SNAKEOVERFLOW )
		gameOver(u8"Your snake hit the wall!");
	else if (nextPosObj == SNAKEOCCUPIED)
		gameOver(u8"Your snake hit itself!");
	else if (nextPosObj == FOODOCCUPIED)
	{
		/* 食物变成头 */
		snakePixelMap[nextPos.y][nextPos.x].setType(snakePixelType::SNAKE_HEAD,style);
		snakePixelMap[nextPos.y][nextPos.x].square->show();
		snakePixelMap[snakeHead.y][snakeHead.x].setType(snakePixelType::SNAKE_BODY, style);

		//snakeLoc.insert(snakeLoc.begin(), nextPos);
		snakeLoc.push_front(nextPos);
		emptyLoc.remove(nextPos);
		//remove(foodLoc, nextPos);
		foodLoc.remove(nextPos);

		config.score++;
		scoreLabel->setText(u8"Score：" + QString::number(config.score));
		placeFood();
	}
	else 
	{
		snakePixelMap[nextPos.y][nextPos.x].setType(snakePixelType::SNAKE_HEAD, style);
		snakePixelMap[snakeHead.y][snakeHead.x].setType(snakePixelType::SNAKE_BODY, style);
		snakePixelMap[snakeTail.y][snakeTail.x].setType(snakePixelType::EMPTY, style);

		snakeLoc.push_front(nextPos);
		emptyLoc.remove(nextPos);
		emptyLoc.push_front(snakeLoc.back());
		snakeLoc.pop_back();
	}
}

void snakeConfig::readConfig(const char* str)
{
	std::fstream fin;
	if (!open_cfgfile(fin,str, READONLY))
	{
		r = 15;
		c = 15;
		foodNum = 3;
		blockSize = 30;
		score = 0;
		bgColor = 0;
	}
	else
	{
		item_get_value(fin, nullptr, "row", &r, TYPE_INT);
		item_get_value(fin, nullptr, "column", &c, TYPE_INT);
		item_get_value(fin, nullptr, "speed", &speed, TYPE_INT);
		item_get_value(fin, nullptr, "block_size", &blockSize, TYPE_INT);
		item_get_value(fin, nullptr, "food_num", &foodNum, TYPE_INT);
		item_get_value(fin, nullptr, "back_ground_color", &bgColor, TYPE_HEX_INT);
		
		if (speed <= 0 || speed >= 20) speed = 2;
		if (r < 5 || r > 30)
			r = 10;
		if (c < 5 || c > 30)
			c = 10;
		if (blockSize < 20 || blockSize>50)
			blockSize = 30;
		if (foodNum < 1 || foodNum > 10) foodNum = 3;

		score = 0;
	}
	fin.close();
}


void snakePixel::setType(const snakePixelType &_type, const QString &style)
{
	type = _type;
	applyType(style);
}

void snakePixel::applyType(const QString &style)
{
	switch (type)
	{
		case snakePixelType::EMPTY: square->setStyleSheet(style); break;
		case snakePixelType::FOOD: square->setStyleSheet("background-color: red"); break;
		case snakePixelType::SNAKE_HEAD: square->setStyleSheet("background-color: rgb(127,127,255); border-radius: 10px");  break;
		case snakePixelType::SNAKE_BODY: square->setStyleSheet("background-color: green; border-radius: 10px"); break;
	}
}

snakePixel::snakePixel(QWidget *parent)
{
	square = new QLabel(parent);
}

snakePixel::~snakePixel()
{
	delete square;
}

bool coord::operator==(const coord &c) const
{
	return (memcmp((void*)this, (void*)&c, sizeof(coord)) == 0);
}
