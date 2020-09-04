#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>

#include "API.h"

#define PRINTLENGTH 63

class PrintWindow
{

	void Print(int len, std::string s); // ��ӡ |xxxx        | ���������ڳ���Ϊ len
	int GetColumnLength(std::string name, std::vector<std::string> col_name, std::vector<int> col_len);
public:
	void CreateTable(bool is_created);
	void ShowAllTable(std::vector<std::string> sen_str, std::string path);
	void DropTable(bool is_dropped);
	void SelectTable(SelectPrintInfo select_table_print_info);
	void InsertRecord(bool is_inserted);
	void CreateDB(bool is_created);
	void DropDB(bool is_dropped);
	void ShowDB(std::vector<std::string> db_names);
	void UseDB(bool isUsed);
	void UpdateTable(bool isUpdated);
	void DeleteTable(bool isDeleted);

};

// ������Ľ����ӿ� ���룺���������GUI�� ����API
void Interpreter(std::vector<std::string> sen_str, CmdType cmd_type, PrintWindow print_window);


/************************************************************************
*    �����������ִ���
*    ���ܣ��������ַ�������Ϊ�����ִ�
*	 �����ִ����壺�����ִ���ָһ���Իس���ո���־�����ָ���ж���������ַ�
*                 ��,��ʶ����Ҫ�ж��š����š��Ƚ���������ֺŵȡ���ʶ��Ҳ����
*                 �����ִ�
************************************************************************/
class Meaning_String
{
public:
	Meaning_String(std::string srcstr = "");
	void SetSrcStr(std::string _srcstr);

	std::vector<std::string> GetSensefulStr()const;
	void Parse2();
private:

	void Parse();                                                       // ��������Ϊ�����ִ�


	std::string src_str;                                                // ԭʼ�����ַ���
	std::vector<std::string> sen_str;                                   // ���������һ�ִ�
	std::string key_char = ";,()=<>\012\015\040";
	bool IsKeyChar(char c);
};

// ���������ִ��Ĳ�������,ͬʱ�����ͼ��
CmdType GetOpType(std::vector<std::string> sen_str);

// ��ʾ���ݿ�
std::string ShowDbInfo(std::vector<std::string> sen_str);

// �������ݿ�,����Ҫ����������
std::string CreateDbInfo(std::vector<std::string> sen_str);

// ɾ�����ݿ�,����Ҫɾ��������
std::string DeleteDbInfo(std::vector<std::string> sen_str);

// ʹ�����ݿ�
std::string UseDbInfo(std::vector<std::string> sen_str);


// ����������Ϣ
bool CreateShowTableInfo(std::vector<std::string> sen_str);
TB_Create_Info CreateTableInfo(std::vector<std::string> sen_str);
TB_Insert_Info CreateInsertInfo(std::vector<std::string> sen_str);
std::string DropTableInfo(std::vector<std::string> sen_str);
TB_Select_Info TableSelectInfo(std::vector<std::string> sen_str);      //����select�����������Ϣ
TB_Update_Info TableUpdateInfo(std::vector<std::string> sen_str);
TB_Delete_Info TableDeleteInfo(std::vector<std::string> sen_str);

