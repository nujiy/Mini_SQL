#pragma once
#include <chrono>
#include <iomanip>
#include <direct.h>
#include <cstdlib>
#include "Error.h"

//#define NDEBUG
using namespace std::chrono;


/********************************************************  Buffer Module  ***************************************************************/

enum class CmdType
{
	TABLE_CREATE, TABLE_DROP, TABLE_SHOW, TABLE_SELECT, TABLE_INSERT, TABLE_UPDATE, TABLE_DELETE,
	DB_CREATE, DB_DROP, DB_SHOW, DB_USE,
	QUIT, HELP

};

/****************************************************************************************************************************************/



/********************************************************  Buffer Module  ***************************************************************/

constexpr int FILE_PAGESIZE = 8192;	                    // �ڴ�ҳ(==�ļ�ҳ)��С
constexpr int MEM_PAGEAMOUNT = 4096;                    // �ڴ�ҳ����
constexpr int MAX_FILENAME_LEN = 256;                   // �ļ���������·������󳤶�

/****************************************************************************************************************************************/





/********************************************************  B+tree Module  ***************************************************************/

constexpr int RecordColumnCount = 12 * 4;              // ��¼�ֶ���������,���������ֶζ����ַ����飬һ���ַ������ֶ���Ҫ4���ַ�->CXXX
constexpr int ColumnNameLength = 16;                   // �����ֶ����Ƴ�������
constexpr int bptree_t = 40;                            // B+tree's degree, bptree_t >= 2
constexpr int MaxKeyCount = 2 * bptree_t;              // the max number of keys in a b+tree node
constexpr int MaxChildCount = 2 * bptree_t;            // the max number of child in a b+tree node

/****************************************************************************************************************************************/




/*********************************************  ����һ���������� ͳ�����ݿ�����ĺ�ʱ  ******************************************************/
class SQLTimer;

SQLTimer& GetTimer();                                 // ȫ�ּ�ʱ��
class SQLTimer
{
public:
	void Start();
	void Stop();
	double TimeSpan();                               // ���ؾ�����ʱ�䣬��λ��second
	void PrintTimeSpan();                            // ��ӡʱ��
private:
	steady_clock::time_point start_t;
	steady_clock::time_point stop_t;
	duration<double> time_span;
	const static unsigned int precision = 8;         // ���ʱ���С��λ����
};

/****************************************************************************************************************************************/


int StrToInt(std::string str);                      // str to int
std::string IdxToDbf(std::string idx_name);         // file name convert .idx to .dbf 
std::string DbfToIdx(std::string idx_name);         // file name convert .dbf to .idx 
std::string StrToLower(std::string str);
std::string IntToStr3(unsigned int x);              // С��1000������תΪ�����ַ����ַ���

