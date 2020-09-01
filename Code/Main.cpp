#include <iostream>
#include <random>
#include <ctime>
//#include <windows.h>

#include "Color.h"
#include "Interpreter.h"
using namespace std;


void Help();
void InitMiniSQL();
void RunMiniSQL();
void sleep_(unsigned int n = 1); // Sleep
void IsPod(); // 判断?POD
string GetCommand(); // get 输入;前的命令
const string PROMPT = "Mini_SQL>>>";
//void TestFunc();


int main()
{
	// Initialize
	InitMiniSQL();

	// Run
	RunMiniSQL();

	// Exit
	cout << "Bye." << endl;

	sleep_();

	return 0;
}


void InitMiniSQL()
{
	cout << "(                      WELCOME TO MINI_SQL!                          )" << endl;
	cout << "(+------------------------------------------------------------------+)" << endl;
	cout << "(|You can typing the \"help\;\" cammand to get help.                   |)" << endl;
	cout << "(|                                                                  |)" << endl;
	cout << "(|You can enter \"exit\" OR \"quit\" to exit this program               |)" << endl;
	cout << "(+------------------------------------------------------------------+)" << endl;
}


void RunMiniSQL()
{
	Meaning_String senstr;
	PrintWindow print_window;
	while (1)
		try
		{
			std::string cmd = GetCommand();
			senstr.SetSrcStr(cmd);
			auto cmd_type = GetOpType(senstr.GetSensefulStr());

			if (cmd_type == CmdType::QUIT)break;
			if (cmd_type == CmdType::HELP)
			{
				Help();
				continue;
			}

			Interpreter(senstr.GetSensefulStr(), cmd_type, print_window);
		}
		catch (SQL_Error::BaseError &e)
		{
			SQL_Error::DispatchError(e);
			cout << endl;
			continue;
		}
		catch (...)
		{
			
		}
	
}

void IsPod()
{
	std::cout << std::is_pod<PAGEHEAD>::value << std::endl;
	std::cout << std::is_pod<FileAddr>::value << std::endl;
	std::cout << std::is_pod<FILECOND>::value << std::endl;
	std::cout << std::is_pod<BTNode>::value << std::endl;
	std::cout << std::is_pod<Column_Value>::value << std::endl;
	std::cout << std::is_pod<KeyAttr>::value << std::endl;
}


void Help()
{
	cout << BOLDCYAN <<R"(|                                           HELP                                                 |)" << endl;
	cout << "(+------------------------------------------------------------------------------------------------+)" << endl;
	cout << "(|A simple example to create a student databae named COLLEGE                                      |)" << endl;
	cout << "(+------------------------------------------------------------------------------------------------+)" << endl;
	cout << "(|Create database  : create database COLLEGE;                                                     |)" << endl;
	cout << "(|Use database     : use database COLLEGE;                                                        |)" << endl;
	cout << "(|Show database    : show databases;                                                              |)" << endl;
	cout << "(|Create Table     : create table instructor(ID int primary, Salary double, Name char(20));       |)" << endl;
	cout << "(|Insert Record(1) : insert into instructor(id,Salary,name)values(1,10000.0,ZhangSan);            |)" << endl;
	cout << "(|Insert Record(2) : insert into instructor(id,name)values(2,LiSi);     #LiSi has no Salary now   |)" << endl;
	cout << "(|UPDATE Table     : update instructor set Salary = 100.0 where name = LiSi;                      |)" << endl;
	cout << "(|Delete Table     : delete from instructor where ID = 1;               #ZhangSan is deleted      |)" << endl;
	cout << "(|Select Table(1)  : select * from instructor where ID = 2;                                       |)" << endl;
	cout << "(|Select Table(2)  : select * from instructor where ID > 1 and Salary < 2000;                     |)" << endl;
	cout << "(|Select Table(3)  : select ID,Salary from instructor where id > 1 and Salary < 2000.5;           |)" << endl;
	cout << "(|Drop database    : drop database COLLEGE;                                                       |)" << endl;
	cout << "(|Quit             : quit;  OR  exit;                                                             |)" << endl;
	cout << "(+------------------------------------------------------------------------------------------------+)" << endl;
	cout << "(|Anytime you want to end MiniSQL use \"quit;\" command please.                                     |)" << endl;
	cout << "(+------------------------------------------------------------------------------------------------+)" << endl << RESET;
}

std::string GetCommand()
{
	string res, tmp;
	int i = 0;
	do {
		if (i == 0) 
			std::cout << PROMPT;
		else 
			std::cout << YELLOW << "        >>>" << RESET;
		i += 1;
		getline(cin, tmp);
		res += tmp;
		if (tmp[tmp.size() - 1] != ';')
			res += " ";
	} while (tmp[tmp.size() - 1] != ';');
	return res;
}

void sleep_(unsigned int n)
{
	auto t_1 = time(0);
	auto t_2 = t_1;
	while ((t_2 - t_1) < n)
		t_2 = time(0);
	return;
}

/*
void TestFunc()
{
	Meaning_String senstr;
	PrintWindow print_window;
	std::default_random_engine e(time(NULL));
	std::uniform_real_distribution<double> d(0, 1000);
	std::uniform_int_distribution<int> u(1, 29);

	// 创建并使用数据库
	senstr.SetSrcStr("drop database STU;");
	auto cmd_type = GetOpType(senstr.GetSensefulStr());
	Interpreter(senstr.GetSensefulStr(), cmd_type, print_window);


	senstr.SetSrcStr("create database STU;");
	cmd_type = GetOpType(senstr.GetSensefulStr());
	Interpreter(senstr.GetSensefulStr(), cmd_type, print_window);

	senstr.SetSrcStr("use database STU;");
	cmd_type = GetOpType(senstr.GetSensefulStr());
	Interpreter(senstr.GetSensefulStr(), cmd_type, print_window);

	// 创建表
	senstr.SetSrcStr("create table stu(id int, score double, name char(30);");
	cmd_type = GetOpType(senstr.GetSensefulStr());
	Interpreter(senstr.GetSensefulStr(), cmd_type, print_window);

	//插入数据
	std::ofstream out;
	out.open("test_data.txt");
	static char alphabet[] = "abcdefghijklmnopqistuvwxyzABCDEFGHIJKLMNOPQISTUVWXYZ";
	int sz = 10000;
	for (int i = 1; i <= sz; i++)
	{
		std::string cmd_str = "insert into stu(id, score, name)values(";
		// id
		cmd_str += std::to_string(i);
		cmd_str += ",";

		// score
		cmd_str += std::to_string(d(e));
		cmd_str += ",";

		// name
		std::string name;
		int name_sz = u(e);
		if (!name_sz)name_sz++;
		for (int i = 0; i < name_sz; i++)
		{
			unsigned int index = u(e);
			name += alphabet[index];
		}
		cmd_str += name;
		cmd_str += ");";

		out << std::string(cmd_str.begin() + 38, cmd_str.end()) << std::endl;

		senstr.SetSrcStr(cmd_str);
		cmd_type = GetOpType(senstr.GetSensefulStr());
		Interpreter(senstr.GetSensefulStr(), cmd_type, print_window);
	}
	out.close();
}
*/