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

constexpr int FILE_PAGESIZE = 8192;	                    // 内存页(==文件页)大小
constexpr int MEM_PAGEAMOUNT = 4096;                    // 内存页数量
constexpr int MAX_FILENAME_LEN = 256;                   // 文件名（包含路径）最大长度

/****************************************************************************************************************************************/





/********************************************************  B+tree Module  ***************************************************************/

constexpr int RecordColumnCount = 12 * 4;              // 记录字段数量限制,假设所有字段都是字符数组，一个字符数组字段需要4个字符->CXXX
constexpr int ColumnNameLength = 16;                   // 单个字段名称长度限制
constexpr int bptree_t = 40;                            // B+tree's degree, bptree_t >= 2
constexpr int MaxKeyCount = 2 * bptree_t;              // the max number of keys in a b+tree node
constexpr int MaxChildCount = 2 * bptree_t;            // the max number of child in a b+tree node

/****************************************************************************************************************************************/




/*********************************************  定义一个计数器类 统计数据库操作的耗时  ******************************************************/
class SQLTimer;

SQLTimer& GetTimer();                                 // 全局计时器
class SQLTimer
{
public:
	void Start();
	void Stop();
	double TimeSpan();                               // 返回经过的时间，单位：second
	void PrintTimeSpan();                            // 打印时间
private:
	steady_clock::time_point start_t;
	steady_clock::time_point stop_t;
	duration<double> time_span;
	const static unsigned int precision = 8;         // 输出时间的小数位精度
};

/****************************************************************************************************************************************/


int StrToInt(std::string str);                      // str to int
std::string IdxToDbf(std::string idx_name);         // file name convert .idx to .dbf 
std::string DbfToIdx(std::string idx_name);         // file name convert .dbf to .idx 
std::string StrToLower(std::string str);
std::string IntToStr3(unsigned int x);              // 小于1000的正数转为三个字符的字符串

