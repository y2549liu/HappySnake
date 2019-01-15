#include <iostream>
#include <Windows.h>
#include <io.h>
#include <conio.h>
#include <fstream>
#include <experimental/filesystem>

#define NOTFOUND -1
#define OK 1
#define READONLY 0
#define READ_WRITE 1


enum ITEM_TYPE { TYPE_INT = 0, TYPE_DOUBLE, TYPE_STRING, TYPE_CHARACTER, TYPE_NULL , TYPE_HEX_INT};

int open_cfgfile(std::fstream& , const char* , const int); 

void close_cfgfile(std::fstream&);

int group_add(std::fstream& , const char*);

int group_del(std::fstream& ,const char* , const char*);

int item_add(std::fstream& , const char* , const char* , const void * , const enum ITEM_TYPE );

int item_del(std::fstream& , const char* , const char* , const char*) ;

int item_update(std::fstream& , const char* , const char* , const char* , const void * , const enum ITEM_TYPE ) ;

int item_get_value(std::fstream& , const char* , const char* , void * , const enum ITEM_TYPE ) ;


