#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "common_cfgfile_tools.h"
#include <cstdint>
using namespace std;

/***************************************************************************
文件名称：
功    能：该工具函数集用于读取配置
修改日期：2018/4/27
说    明：
***************************************************************************/

void clear(fstream& fp)
{
	if (fp.fail())
	{
		fp.clear();
		fp.seekg(0, ios::beg);
	}
}

int findc(char* str, char ch)
{
	for (size_t i = 0; i < strlen(str); i++)
	{
		if (str[i] == ch)
			return i;
	}
	return NOTFOUND;
}

/* 修剪字符串update */
void trim(char* str)
{
	size_t i;
	for (i = 0; i < strlen(str); i++) //注释后的内容全部忽略
	{
		if (str[i] == '#' || str[i] == ';')
		{
			str[i] = '\0';
			break;
		}
	}
	for (i = strlen(str) - 1; i >= 0; i--)
	{
		if (findc("\r\n\t ",str[i]) == NOTFOUND ) 
		{
			str[i + 1] = '\0';
			break;
		}
	}
	for (i = 0; i < strlen(str); i++)
	{
		if (findc("\r\n\t ", str[i]) == NOTFOUND)
			break;
	}
	memcpy(str, &str[i], 1 + strlen(&str[i]));
}

/* 修剪等式字符串得到等式右值 */
int str_cut_value(char* str)
{
	for (size_t i = 0; i < strlen(str); i++)
	{
		if (str[i] == '=')
		{
			memcpy(str, &str[i + 1], 1 + strlen(&str[i + 1]));
			trim(str);
			return 1;
		}
	}
	return NOTFOUND;
}

/* 检查行类型 */
int check_line(char* str)
{
	int line_type = 0;
	trim(str);
	if (str[0]=='['&&str[strlen(str)-1]==']')
	{
		return 1;   //[****]
	}
	else if (findc(str, ';') != -1|| findc(str, ';') != -1)
	{
		//合法性判断
		if (str[0]==';'||str[0]=='#')
			return 3;
		else return 0;
	}
	else if (findc(str, '=') != -1)
	{
		//合法性判断
		return 2;
	}
	return 0;
}

/* 坚坚写的文件长度函数 */
static int file_length(fstream &fp)
{
	int file_len, now_pos;
	/* 当前为错误状态则返回-1 */
	clear(fp);

	/* 取fp当前指针位置 */
	now_pos = int(fp.tellg());

	/* 将文件指针移动到最后，此时tellg的结果就是文件大小 */
	fp.seekg(0, ios::end);
	file_len = int(fp.tellg());
	/* 指针移动回函数调用前的原位置 */
	fp.seekg(now_pos, ios::beg);
	return file_len;
}
/* 坚坚写的调整文件长度函数 */
static int file_resize(const char *filename, fstream &fp, int newsize)
{
	int now_pos;
	/* 当前为错误状态则返回-1 */
	clear(fp);

	/* 取fp当前指针位置 */
	now_pos = int(fp.tellg());

	/* 如果大小是负数则直接返回-1 */
	if (newsize < 0)
		return -1;

	experimental::filesystem::resize_file(filename, newsize);

	/* 如果当前文件指针超过了文件大小，则回到文件头 */
	if (now_pos > newsize)
		fp.seekg(0, ios::beg);

	return 0;
}

/* 在文件尾部插入回车 */ 
void fix_fp_end(fstream &fp)
{
	size_t pos = fp.tellg();
	if (file_length(fp) == 0)
	{
		fp << "\r\n";
		return;
	}
	fp.seekg(-1, ios::end);
	char ch = fp.get();
	if (ch != '\n')
	{
		fp << "\r\n";
	}
	fp.seekg(pos);
}
/***************************************************************************
函数名称：
功    能：
输入参数：
返 回 值：position
说    明：此函数从fstream 当前文件指针位置寻找str
***************************************************************************/


streamoff find_str(fstream &fp, const char *str, int spos = -1, int epos = -1)
{
	clear(fp);
	if (spos == -1)
		spos = fp.tellg();

	if (epos == -1)
		epos = file_length(fp);
	
	char tmp[256];
	streamoff cur=0;

	fp.seekg(spos, ios::beg);
	/* 逗号表达式 */
	while (cur = fp.tellg(), cur < epos - strlen(str) && cur >= 0)
	{
		fp.getline(tmp, 256);
		if (strstr(tmp, str))  //找到了str
			return cur + (strstr(tmp, str) - tmp);
	}
	return NOTFOUND;
}

streamoff find_item(fstream &fp, const char *str, int spos = -1, int epos = -1)
{
	clear(fp);
	if (spos == -1)
		spos = fp.tellg();

	if (epos == -1)
		epos = file_length(fp);

	fp.seekg(spos, ios::beg);
	streamoff cur = 0;
	while (cur = fp.tellg(), cur <(epos- strlen(str))  && cur >=0)
	{
		char tmp[256];
		fp.getline(tmp, 256);
		if (strchr(tmp, '='))
		{
			char* p = strchr(tmp, '=');
			*p = '\0';
			trim(tmp);
			if (strcmp(tmp, str) == 0)
				return cur;
		}
	}
	return NOTFOUND;
}

streamoff find_group(fstream &fp,const char *group_name)
{
	clear(fp);
	fp.seekg(0, ios::beg);
	char tmp[256];
	size_t cur = 0;
	while (cur=fp.tellg(),cur>=0)
	{
		fp.getline(tmp, 256);
		trim(tmp);
		if (strcmp(tmp, group_name) == 0)
			return cur;
	}
	return NOTFOUND;
}

/***************************************************************************
函数名称：
功    能：
输入参数：
返 回 值：position
说    明：此函数从fstream偏移offset位置寻找下个groupname
***************************************************************************/
streamoff find_next_group(fstream &fp, size_t offset)
{
	clear(fp);
	fp.seekg(offset , ios::beg);
	char tmp[256];
	fp.getline(tmp,256);//从当前位置的下一行开始寻找下一组
	streamoff cur;
	while (cur= fp.tellg(),cur>=0)
	{
		fp.getline(tmp, 256);
		if (check_line(tmp) == 1)
			return cur;
	}
	return file_length(fp) ; //未找到则返回文件尾；
}
/***************************************************************************
函数名称：
功    能：
输入参数：
返 回 值：
说    明：行插入
***************************************************************************/
streamoff insert_line(fstream &fp, char* str,int insertpos)
{
	streamoff pos = fp.tellg();
	fp.seekg(insertpos, ios::beg);
	char* tmp;
	streamoff tmplen = file_length(fp) - insertpos;
	if (tmplen == 0) //已经在末尾
	{
		fp <<str << "\r\n";
		fp.seekg(pos, ios::beg);
		return 1;
	}
	tmp = new char[tmplen+1];//多一位给\0
	memset(tmp, 0, tmplen + 1);
	fp.read(tmp, tmplen);
	fp.seekg(pos, ios::beg);
	fp.write(str, strlen(str));
	fp <<"\r\n";
	fp.write(tmp,tmplen);
	delete(tmp);
	fp.seekg(0, ios::beg);
	return 1;
}
streamoff delete_block(fstream &fp, const char* filename, int spos, int epos)
{
	char *tmp;
	streamoff tmplen = file_length(fp) - epos;
	tmp = new char[tmplen+1];
	fp.seekg(epos, ios::beg);
	//endpos后内容写入内存
	fp.read(tmp, tmplen);
	file_resize(filename, fp, spos + tmplen);
	fp.seekg(spos, ios::beg);
	fp.write(tmp, tmplen);
	return 1;
}
streamoff delete_line(fstream &fp, const char* filename)
{
	streamoff startpos = fp.tellg();
	char tmp[256];
	fp.getline(tmp, 256); //现在移动到了\n后
	streamoff endpos = fp.tellg();
	return delete_block(fp, filename, startpos, endpos);	
}
int open_cfgfile(fstream &fp, const char *cfgname, const int opt)
{
	if (opt < 0 || opt >1)
		return -1;
	else
	{
		if (opt == 0)//只读
			fp.open(cfgname, ios::in|ios::binary);
		else if (opt == 1)//读写
		{
			fp.open(cfgname, ios::in|ios::out|ios::binary);
			if (fp.is_open()) //文件存在时直接打开
				return 1;
			else 
			{
				//文件不存在时创建
				fp.open(cfgname, ios::out);
				fp.close();
				//重新打开
				fp.open(cfgname, ios::in | ios::out | ios::binary);
			}
		}
		if (fp.is_open())
		{
			fix_fp_end(fp);//如果尾行没有回车，则插入回车以防getline在最后一行出错
			return 1;
		}
		else
			return 0;
	}
}

void close_cfgfile(fstream &fp)
{
	fp.close();
}

int group_add(fstream &fp, const char *group_name)
{
	//从头检查groupname是否存在
	fp.seekg(0, ios::beg);
	char groupname[256];
	memset(groupname, 0, 256);
	groupname[0] = '[';
	strcat(groupname+1, group_name);
	groupname[strlen(groupname)] = ']';
	//如果未找到groupname
	if (find_str(fp, groupname) == -1)
	{
		fp.seekg(0, ios::end);
		//如果不是首行则增加一个回车
		if ((int)(fp.tellg()) != 0)
			fp << "\r\n";
		fp << groupname<<"\r\n";
		return 1;
	}
	return 0;
}
char *getgroupname(const char* group_name)
{
	//构造groupname;
	static char groupname[256];
	memset(groupname, 0, 256);
	groupname[0] = '[';
	strcat(groupname + 1, group_name);
	groupname[strlen(groupname)] = ']';
	return groupname;
}

int group_del(fstream &fp, const char *filename, const char *group_name)
{
	//从当前位置检查groupname是否存在
	//构造groupname;
	int pos= find_str(fp, getgroupname(group_name));
	//如果未找到组名
	if (pos == -1)
	{
		return 0;
	}
	else
	{
		//转到该位置
		fp.seekg(pos, ios::beg);
		//找到下一个组名
		int nextgrouppos = find_next_group(fp, 1);
		if (nextgrouppos == -1)
		{
			//删除pos到文件尾的全部内容
			file_resize(filename, fp, pos);
			return 1;
		}
		else
		{
			//删除pos到nextgrouppos的全部内容
			delete_block(fp, filename, pos, nextgrouppos);
			fp.seekp(pos, ios::beg);
			return group_del(fp, filename, group_name) + 1;
		}
	}
	return 0;
}
/***************************************************************************
函数名称：
功    能：
输入参数：
返 回 值：
说    明：此函数从fstream 当前文件指针位置寻找groupname 并添加item
***************************************************************************/
int item_add(fstream &fp, const char *group_name, const char *item_name, const void *item_value, const enum ITEM_TYPE item_type)
{
	int pos = fp.tellg();
	char tmp[256];
	streamoff item_pos;
	if (group_name == NULL)
	{
		fp.seekg(0, ios::end);
	}
	else
	{
		fp.seekg(0, ios::beg);
		//cout << fp.tellg() << endl;
		int gpos = find_str(fp, getgroupname(group_name));
		if (gpos == -1)
			return 0;
		if (find_str(fp, item_name, gpos, find_next_group(fp, 1)) != -1)
			return 0;
	
		fp.getline(tmp, 256); //使得fp在groupname回车后
		item_pos = fp.tellg();
		memset(tmp, 0, 256);
	}	
	//处理写入内容
	switch (item_type)
	{
		case TYPE_INT:
			sprintf(tmp, "%s=%d", item_name, *(int*)item_value);
			break;
		case TYPE_DOUBLE:
			sprintf(tmp, "%s=%lf", item_name, *(double*)item_value);
			break;
		case TYPE_STRING:
			sprintf(tmp, "%s=%s", item_name, (char*)item_value);
			break;
		case TYPE_CHARACTER:
			sprintf(tmp, "%s=%c", item_name, *(char*)item_value);
			break;
		case TYPE_NULL:
			sprintf(tmp, "%s=", item_name);
			break;
		default:
			return 0;
	}
	return insert_line(fp, tmp ,item_pos);
}

int item_del(fstream &fp, const char *filename, const char *group_name, const char *item_name)
{
	streamoff pos = fp.tellg();
	fp.seekg(0, ios::beg);
	if (group_name == NULL)
	{
		int item_pos = find_str(fp, item_name);
		if (item_pos == -1)
			return 0;
		delete_line(fp, filename);
		return item_del(fp, filename, group_name, item_name) + 1;
	}
	else
	{
		char* groupname = getgroupname(group_name);
		streamoff group_pos = find_str(fp, groupname);
		if (group_pos == -1)
			return 0;
		streamoff item_pos = find_str(fp, item_name, group_pos, find_next_group(fp, group_pos));
		if (item_pos == -1)
			return 0;
		fp.seekg(item_pos, ios::beg);
		delete_line(fp, filename);
		return item_del(fp, filename,group_name,item_name) + 1;
	}
	return 0;
}

int item_update(fstream &fp, const char *filename, const char *group_name, const char *item_name, const void *item_value, const enum ITEM_TYPE item_type)
{
	streamoff pos = fp.tellg();
	char* groupname = getgroupname(group_name);
	streamoff gpos = find_str(fp, groupname);
	if (gpos == -1) 
		return 0;
	//尝试删除原有项,没有也无所谓
	item_del(fp, filename, group_name, item_name);
	fp.seekg(pos, ios::beg);
	if (item_add(fp,group_name,item_name,item_value,item_type)==0)
		return 0;
	return 1;
}
/***************************************************************************
函数名称：
功    能：
输入参数：
返 回 值：value
说    明：此函数从fstream中寻找第一个group->item 
***************************************************************************/
int item_get_value(fstream &fp, const char *group_name, const char *item_name, void *item_value, const enum ITEM_TYPE item_type)
{
	clear(fp);
	fp.seekg(0, ios::beg);
	streamoff item_pos, group_pos;
	/* 简单配置 */
	if (group_name == nullptr)
		item_pos = find_item(fp, item_name, 0 , file_length(fp));
	/* 分组配置 */
	else
	{
		char* groupname = getgroupname(group_name);
		group_pos = find_group(fp, groupname);
		if (group_pos == NOTFOUND)
			return 0;
		item_pos = find_item(fp, item_name, group_pos, find_next_group(fp, group_pos));
	}
	
	if (item_pos == NOTFOUND)
		return 0;
	clear(fp);
	fp.seekg(item_pos, ios::beg);
	char tmp[256];
	fp.getline(tmp, 256);
	trim(tmp);
	str_cut_value(tmp);
	switch (item_type)
	{
		case TYPE_INT:
			sscanf(tmp,"%d",(int*)item_value);
			break;
		case TYPE_HEX_INT:
			sscanf(tmp, "%x", (int*)item_value);
			break;
		case TYPE_DOUBLE:
			sscanf(tmp, "%lf", (double*)item_value);
			break;
		case TYPE_STRING:
			sscanf(tmp, "%s", (char*)item_value);
			break;
		case TYPE_CHARACTER:
			sscanf(tmp, "%c", (char*)item_value);
			break;
		case TYPE_NULL:
			return 1;
		default:
			return 0;
	}
}

template<class T>
int item_get_value(std::fstream& fp, const char* group_name, const char* item_name, T& item)
{
	clear(fp);
	fp.seekg(0, ios::beg);
	streamoff item_pos, group_pos;
	/* 简单配置 */
	if (group_name == NULL)
		item_pos = find_item(fp, item_name, 0, file_length(fp));
	/* 分组配置 */
	else
	{
		char* groupname = getgroupname(group_name);
		group_pos = find_group(fp, groupname);
		if (group_pos == NOTFOUND)
			return 0;
		item_pos = find_item(fp, item_name, group_pos, find_next_group(fp, group_pos));
	}

	if (item_pos == NOTFOUND)
		return 0;
	clear(fp);
	fp.seekg(item_pos, ios::beg);
	while (fp.peek() != '\n' && !fp.eof())
	{
		if (fin.get() == '=')
		{
			fin >> item;
			return 1;
		}
	}
	return 0;
}