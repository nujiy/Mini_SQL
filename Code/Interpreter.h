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

	void Print(int len, std::string s); // 打印 |xxxx        | 其中竖线内长度为 len
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

// 主程序的交互接口 输入：命令参数和GUI类 调用API
void Interpreter(std::vector<std::string> sen_str, CmdType cmd_type, PrintWindow print_window);


/************************************************************************
*    类名：有意字串类
*    功能：将命令字符串解析为有意字串
*	 有意字串定义：有意字串即指一个以回车或空格或标志符来分割的有独立含义的字符
*                 串,标识符主要有逗号、括号、比较运算符、分号等。标识符也算作
*                 有意字串
************************************************************************/
class Meaning_String
{
public:
	Meaning_String(std::string srcstr = "");
	void SetSrcStr(std::string _srcstr);

	std::vector<std::string> GetSensefulStr()const;
	void Parse2();
private:

	void Parse();                                                       // 解析命令为有意字串


	std::string src_str;                                                // 原始命令字符串
	std::vector<std::string> sen_str;                                   // 解析后的又一字串
	std::string key_char = ";,()=<>\012\015\040";
	bool IsKeyChar(char c);
};

// 返回有意字串的操作类型,同时做类型检查
CmdType GetOpType(std::vector<std::string> sen_str);

// 显示数据库
std::string ShowDbInfo(std::vector<std::string> sen_str);

// 创建数据库,返回要创建的名称
std::string CreateDbInfo(std::vector<std::string> sen_str);

// 删除数据库,返回要删除的名称
std::string DeleteDbInfo(std::vector<std::string> sen_str);

// 使用数据库
std::string UseDbInfo(std::vector<std::string> sen_str);


// 表操作相关信息
bool CreateShowTableInfo(std::vector<std::string> sen_str);
TB_Create_Info CreateTableInfo(std::vector<std::string> sen_str);
TB_Insert_Info CreateInsertInfo(std::vector<std::string> sen_str);
std::string DropTableInfo(std::vector<std::string> sen_str);
TB_Select_Info TableSelectInfo(std::vector<std::string> sen_str);      //生成select操作所需的信息
TB_Update_Info TableUpdateInfo(std::vector<std::string> sen_str);
TB_Delete_Info TableDeleteInfo(std::vector<std::string> sen_str);

