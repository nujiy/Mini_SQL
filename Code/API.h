#ifndef __MiniSql_H__
#define __MiniSql_H__

#include <algorithm>
#include <vector>
#include "Global.h"
#include "B+Tree.h"
#include "Record.h"
#include "Check.h"

//关系查找符
enum Operat_Type { B, BE, L, LE, E, NE };
Operat_Type GetOperatorType(std::string s);


class CompareCell                     //一个字段比较单元
{
public:
	CompareCell(Operat_Type t, Column_Cell cc) :OperType(t), cmp_value(cc) {}
	bool operator()(const Column_Cell &cc);
	Operat_Type	OperType;	        //比较关系关系运算符
	Column_Cell		cmp_value;
};
struct SelectPrintInfo
{
	std::string table_name;
	std::vector<std::string> name_selected_column;

	std::vector<std::pair<KeyAttr, FileAddr>> key_fd;  // keys 和 fds 保存着对应的关键字以及该关键字对应的记录地址

};

// 目录定位和切换 用于数据库和表的使用
class CatalogPosition
{
	friend bool UseDatabase(std::string db_name, CatalogPosition &cp);
public:
	CatalogPosition();
	bool ResetRootCatalog(std::string root_new);  // 重置根目录

	void SwitchToDatabase();// 转到数据库列表目录下


	bool SwitchToDatabase(std::string db_name);// 转到具体的数据库下

	std::string GetCurrentPath()const;
	std::string GetRootPath()const;
	std::string SetCurrentPath(std::string cur);
	bool GetIsInSpeDb() { return isInSpeDb; }
	bool SetInInSpeDb(bool new_b) { isInSpeDb = new_b; return new_b; }
private:
	static bool isInSpeDb;          //是否在某个具体的数据库目录下
	std::string root; // 根目录，数据库文件的保存位置
	std::string current_catalog;
};
CatalogPosition& GetCp();

/************************************************************************
*    表创建信息
************************************************************************/
struct TB_Create_Info
{
	using ColumnInfo = struct ColumnInfo               // 新建表的字段信息
	{
		std::string name;
		Column_Type type;
		bool isPrimary;                                // 是否主键
		int length;                                    // 字段数据长度
	};

	std::string table_name;                            // 新建的表名
	std::vector<ColumnInfo> columns_info;              // 表的各个字段

};

/************************************************************************
*    表插入信息
************************************************************************/
struct TB_Insert_Info
{
	using InsertInfo = struct {
		std::string column_name;                        // 插入的字段
		std::string column_value;                       // 插入的值
	};

	std::string table_name;                             // 插入的表名
	std::vector<InsertInfo> insert_info;                // 需要插入的字段集合
};

/************************************************************************
*    表选择信息
************************************************************************/
struct TB_Select_Info
{
	std::string table_name;                        // 选择的表名
	std::vector<std::string> name_selected_column; // 选择的字段名字
	std::vector<CompareCell> vec_cmp_cell;         // 选择条件
};

/************************************************************************
*    表更新信息
************************************************************************/
struct TB_Update_Info
{
	using NewValue = struct {
		std::string field;
		std::string value;
	};
	using Expr = struct {
		std::string field;
		std::string op;
		std::string value;
	};

	std::string table_name;
	std::vector<NewValue> field_value;  // 字段——值 向量
	std::vector<Expr> expr;             // 跟新的字段条件
};

/************************************************************************
*    表删除信息
************************************************************************/
struct TB_Delete_Info
{
	using Expr = struct 
	{
		std::string field;
		std::string op;
		std::string value;
	};
	std::string table_name;
	std::vector<Expr> expr;             // 删除的字段条件
};




// 创建数据库
bool CreateDatabase(std::string database_name, CatalogPosition &cp);

// Drop数据库
bool DropDatabase(std::string database_name, CatalogPosition &cp);
void DelFilesInFolder(std::string folderPath);  // 删除目录下的所有文件及文件夹

// 返回数据库名称向量
std::vector<std::string> ShowDatabase(CatalogPosition &cp);

// 选择数据库
bool UseDatabase(std::string db_name, CatalogPosition &cp);


// 创建表 eg. create table test1(id int,score double,Name char(20) primary);
bool CreateTable(TB_Create_Info tb_create_info, std::string path = std::string("./"));

// 删除表
bool DropTable(std::string table_name, std::string path = std::string("./"));

// 显示数据库下的表项
std::vector<std::string> ShowAllTable(bool b, std::string path = std::string("./"));

// 插入记录 eg. insert into test1(id, score, Name)values(10, 1.5, bcd);
bool InsertRecord(TB_Insert_Info tb_insert_info, std::string path = std::string("./"));

// 选择记录
SelectPrintInfo SelectTable(TB_Select_Info tb_select_info, std::string path = std::string("./"));

// 更新记录 
bool UpdateTable(TB_Update_Info tb_update_info, std::string path = std::string("./"));

// 删除记录
bool DeleteTable(TB_Delete_Info tb_delete_info, std::string path = std::string("./"));

// 打印整张表
std::vector<RecordHead> ShowTable(std::string table_name, std::string path = std::string("./"));

// 取出指定地址的数据
RecordHead GetDbfRecord(std::string table_name, FileAddr fd, std::string path = std::string("./"));
void PrintRecord(std::string table_name, KeyAttr key, FileAddr fd, std::string path = std::string("./"));

// 返回给定名表中各个字段名称以及对应类型
std::vector<std::pair<std::string, Column_Type>> GetColumnAndTypeFromTable(std::string table_name, std::string table_path);
Column_Type GetType(std::string name, std::vector<std::pair<std::string, Column_Type>> vec);

// 范围查找单个字段 返回查找的值在数据文件中的地址
std::vector<std::pair<KeyAttr, FileAddr>> Search(CompareCell compare_cell, std::string table_name, std::string path = std::string("./"));
std::vector<std::pair<KeyAttr, FileAddr>> KeySearch(CompareCell compare_cell, std::string table_name, std::string path = std::string("./"));  //主键查找
std::vector<std::pair<KeyAttr, FileAddr>> RangeSearch(CompareCell compare_cell, std::string table_name, std::string path = std::string("./"));  // 遍历查找


// 比较的字段名称，比较的字段类型，比较关系，比较的值
CompareCell CreateCmpCell(std::string column_name, Column_Type column_type, Operat_Type Optype, std::string value);

// 索引文件头信息管理类
class TableIndexHeadInfo
{
public:
	TableIndexHeadInfo(BTree &_tree) :tree(_tree) {}
	// 表的字段个数
	size_t GetColumnCount()const;
	// 各个字段名字
	std::vector<std::string> GetColumnNames()const;
	// 各个字段类型
	std::vector<Column_Type> GetColumnType()const;
	Column_Type GetColumnType(std::string column_name)const;
	// 各个字段的大小
	std::vector<int> GetColumnSize()const;
	// 第i个字段的数据大小
	int GetColumnSizeByIndex(int i)const;
	// 主键字段的索引
	int GetPrimaryIndex()const;

	// 判断该字段名是不是表的字段
	bool IsColumnName(std::string column_name)const;
	// 返回该字段在所有字段中的索引位置
	int GetIndex(std::string column_name)const;
	// 判断字段名是否为主键字段
	bool IsPrimary(std::string column_name)const;
	// 返回给定字段名字段距离数据头地址的偏移
	int GetColumnOffset(std::string column_name);
private:
	BTree &tree;
};


#endif
