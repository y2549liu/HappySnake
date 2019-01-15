#include "stdafx.h"
#include "HappySnake.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	srand(time(nullptr));
	QApplication a(argc, argv);
	HappySnake w;
	w.show();
	return a.exec();
}
