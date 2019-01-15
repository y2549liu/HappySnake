#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "common_cfgfile_tools.h"
#include <cstdint>
using namespace std;

/***************************************************************************
�ļ����ƣ�
��    �ܣ��ù��ߺ��������ڶ�ȡ����
�޸����ڣ�2018/4/27
˵    ����
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

/* �޼��ַ���update */
void trim(char* str)
{
	size_t i;
	for (i = 0; i < strlen(str); i++) //ע�ͺ������ȫ������
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

/* �޼���ʽ�ַ����õ���ʽ��ֵ */
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

/* ��������� */
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
		//�Ϸ����ж�
		if (str[0]==';'||str[0]=='#')
			return 3;
		else return 0;
	}
	else if (findc(str, '=') != -1)
	{
		//�Ϸ����ж�
		return 2;
	}
	return 0;
}

/* ���д���ļ����Ⱥ��� */
static int file_length(fstream &fp)
{
	int file_len, now_pos;
	/* ��ǰΪ����״̬�򷵻�-1 */
	clear(fp);

	/* ȡfp��ǰָ��λ�� */
	now_pos = int(fp.tellg());

	/* ���ļ�ָ���ƶ�����󣬴�ʱtellg�Ľ�������ļ���С */
	fp.seekg(0, ios::end);
	file_len = int(fp.tellg());
	/* ָ���ƶ��غ�������ǰ��ԭλ�� */
	fp.seekg(now_pos, ios::beg);
	return file_len;
}
/* ���д�ĵ����ļ����Ⱥ��� */
static int file_resize(const char *filename, fstream &fp, int newsize)
{
	int now_pos;
	/* ��ǰΪ����״̬�򷵻�-1 */
	clear(fp);

	/* ȡfp��ǰָ��λ�� */
	now_pos = int(fp.tellg());

	/* �����С�Ǹ�����ֱ�ӷ���-1 */
	if (newsize < 0)
		return -1;

	experimental::filesystem::resize_file(filename, newsize);

	/* �����ǰ�ļ�ָ�볬�����ļ���С����ص��ļ�ͷ */
	if (now_pos > newsize)
		fp.seekg(0, ios::beg);

	return 0;
}

/* ���ļ�β������س� */ 
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
�������ƣ�
��    �ܣ�
���������
�� �� ֵ��position
˵    �����˺�����fstream ��ǰ�ļ�ָ��λ��Ѱ��str
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
	/* ���ű��ʽ */
	while (cur = fp.tellg(), cur < epos - strlen(str) && cur >= 0)
	{
		fp.getline(tmp, 256);
		if (strstr(tmp, str))  //�ҵ���str
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
�������ƣ�
��    �ܣ�
���������
�� �� ֵ��position
˵    �����˺�����fstreamƫ��offsetλ��Ѱ���¸�groupname
***************************************************************************/
streamoff find_next_group(fstream &fp, size_t offset)
{
	clear(fp);
	fp.seekg(offset , ios::beg);
	char tmp[256];
	fp.getline(tmp,256);//�ӵ�ǰλ�õ���һ�п�ʼѰ����һ��
	streamoff cur;
	while (cur= fp.tellg(),cur>=0)
	{
		fp.getline(tmp, 256);
		if (check_line(tmp) == 1)
			return cur;
	}
	return file_length(fp) ; //δ�ҵ��򷵻��ļ�β��
}
/***************************************************************************
�������ƣ�
��    �ܣ�
���������
�� �� ֵ��
˵    �����в���
***************************************************************************/
streamoff insert_line(fstream &fp, char* str,int insertpos)
{
	streamoff pos = fp.tellg();
	fp.seekg(insertpos, ios::beg);
	char* tmp;
	streamoff tmplen = file_length(fp) - insertpos;
	if (tmplen == 0) //�Ѿ���ĩβ
	{
		fp <<str << "\r\n";
		fp.seekg(pos, ios::beg);
		return 1;
	}
	tmp = new char[tmplen+1];//��һλ��\0
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
	//endpos������д���ڴ�
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
	fp.getline(tmp, 256); //�����ƶ�����\n��
	streamoff endpos = fp.tellg();
	return delete_block(fp, filename, startpos, endpos);	
}
int open_cfgfile(fstream &fp, const char *cfgname, const int opt)
{
	if (opt < 0 || opt >1)
		return -1;
	else
	{
		if (opt == 0)//ֻ��
			fp.open(cfgname, ios::in|ios::binary);
		else if (opt == 1)//��д
		{
			fp.open(cfgname, ios::in|ios::out|ios::binary);
			if (fp.is_open()) //�ļ�����ʱֱ�Ӵ�
				return 1;
			else 
			{
				//�ļ�������ʱ����
				fp.open(cfgname, ios::out);
				fp.close();
				//���´�
				fp.open(cfgname, ios::in | ios::out | ios::binary);
			}
		}
		if (fp.is_open())
		{
			fix_fp_end(fp);//���β��û�лس��������س��Է�getline�����һ�г���
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
	//��ͷ���groupname�Ƿ����
	fp.seekg(0, ios::beg);
	char groupname[256];
	memset(groupname, 0, 256);
	groupname[0] = '[';
	strcat(groupname+1, group_name);
	groupname[strlen(groupname)] = ']';
	//���δ�ҵ�groupname
	if (find_str(fp, groupname) == -1)
	{
		fp.seekg(0, ios::end);
		//�����������������һ���س�
		if ((int)(fp.tellg()) != 0)
			fp << "\r\n";
		fp << groupname<<"\r\n";
		return 1;
	}
	return 0;
}
char *getgroupname(const char* group_name)
{
	//����groupname;
	static char groupname[256];
	memset(groupname, 0, 256);
	groupname[0] = '[';
	strcat(groupname + 1, group_name);
	groupname[strlen(groupname)] = ']';
	return groupname;
}

int group_del(fstream &fp, const char *filename, const char *group_name)
{
	//�ӵ�ǰλ�ü��groupname�Ƿ����
	//����groupname;
	int pos= find_str(fp, getgroupname(group_name));
	//���δ�ҵ�����
	if (pos == -1)
	{
		return 0;
	}
	else
	{
		//ת����λ��
		fp.seekg(pos, ios::beg);
		//�ҵ���һ������
		int nextgrouppos = find_next_group(fp, 1);
		if (nextgrouppos == -1)
		{
			//ɾ��pos���ļ�β��ȫ������
			file_resize(filename, fp, pos);
			return 1;
		}
		else
		{
			//ɾ��pos��nextgrouppos��ȫ������
			delete_block(fp, filename, pos, nextgrouppos);
			fp.seekp(pos, ios::beg);
			return group_del(fp, filename, group_name) + 1;
		}
	}
	return 0;
}
/***************************************************************************
�������ƣ�
��    �ܣ�
���������
�� �� ֵ��
˵    �����˺�����fstream ��ǰ�ļ�ָ��λ��Ѱ��groupname �����item
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
	
		fp.getline(tmp, 256); //ʹ��fp��groupname�س���
		item_pos = fp.tellg();
		memset(tmp, 0, 256);
	}	
	//����д������
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
	//����ɾ��ԭ����,û��Ҳ����ν
	item_del(fp, filename, group_name, item_name);
	fp.seekg(pos, ios::beg);
	if (item_add(fp,group_name,item_name,item_value,item_type)==0)
		return 0;
	return 1;
}
/***************************************************************************
�������ƣ�
��    �ܣ�
���������
�� �� ֵ��value
˵    �����˺�����fstream��Ѱ�ҵ�һ��group->item 
***************************************************************************/
int item_get_value(fstream &fp, const char *group_name, const char *item_name, void *item_value, const enum ITEM_TYPE item_type)
{
	clear(fp);
	fp.seekg(0, ios::beg);
	streamoff item_pos, group_pos;
	/* ������ */
	if (group_name == nullptr)
		item_pos = find_item(fp, item_name, 0 , file_length(fp));
	/* �������� */
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
	/* ������ */
	if (group_name == NULL)
		item_pos = find_item(fp, item_name, 0, file_length(fp));
	/* �������� */
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