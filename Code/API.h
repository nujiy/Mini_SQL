#ifndef __MiniSql_H__
#define __MiniSql_H__

#include <algorithm>
#include <vector>
#include "Global.h"
#include "B+Tree.h"
#include "Record.h"
#include "Check.h"

//��ϵ���ҷ�
enum Operat_Type { B, BE, L, LE, E, NE };
Operat_Type GetOperatorType(std::string s);


class CompareCell                     //һ���ֶαȽϵ�Ԫ
{
public:
	CompareCell(Operat_Type t, Column_Cell cc) :OperType(t), cmp_value(cc) {}
	bool operator()(const Column_Cell &cc);
	Operat_Type	OperType;	        //�ȽϹ�ϵ��ϵ�����
	Column_Cell		cmp_value;
};
struct SelectPrintInfo
{
	std::string table_name;
	std::vector<std::string> name_selected_column;

	std::vector<std::pair<KeyAttr, FileAddr>> key_fd;  // keys �� fds �����Ŷ�Ӧ�Ĺؼ����Լ��ùؼ��ֶ�Ӧ�ļ�¼��ַ

};

// Ŀ¼��λ���л� �������ݿ�ͱ��ʹ��
class CatalogPosition
{
	friend bool UseDatabase(std::string db_name, CatalogPosition &cp);
public:
	CatalogPosition();
	bool ResetRootCatalog(std::string root_new);  // ���ø�Ŀ¼

	void SwitchToDatabase();// ת�����ݿ��б�Ŀ¼��


	bool SwitchToDatabase(std::string db_name);// ת����������ݿ���

	std::string GetCurrentPath()const;
	std::string GetRootPath()const;
	std::string SetCurrentPath(std::string cur);
	bool GetIsInSpeDb() { return isInSpeDb; }
	bool SetInInSpeDb(bool new_b) { isInSpeDb = new_b; return new_b; }
private:
	static bool isInSpeDb;          //�Ƿ���ĳ����������ݿ�Ŀ¼��
	std::string root; // ��Ŀ¼�����ݿ��ļ��ı���λ��
	std::string current_catalog;
};
CatalogPosition& GetCp();

/************************************************************************
*    ������Ϣ
************************************************************************/
struct TB_Create_Info
{
	using ColumnInfo = struct ColumnInfo               // �½�����ֶ���Ϣ
	{
		std::string name;
		Column_Type type;
		bool isPrimary;                                // �Ƿ�����
		int length;                                    // �ֶ����ݳ���
	};

	std::string table_name;                            // �½��ı���
	std::vector<ColumnInfo> columns_info;              // ��ĸ����ֶ�

};

/************************************************************************
*    �������Ϣ
************************************************************************/
struct TB_Insert_Info
{
	using InsertInfo = struct {
		std::string column_name;                        // ������ֶ�
		std::string column_value;                       // �����ֵ
	};

	std::string table_name;                             // ����ı���
	std::vector<InsertInfo> insert_info;                // ��Ҫ������ֶμ���
};

/************************************************************************
*    ��ѡ����Ϣ
************************************************************************/
struct TB_Select_Info
{
	std::string table_name;                        // ѡ��ı���
	std::vector<std::string> name_selected_column; // ѡ����ֶ�����
	std::vector<CompareCell> vec_cmp_cell;         // ѡ������
};

/************************************************************************
*    �������Ϣ
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
	std::vector<NewValue> field_value;  // �ֶΡ���ֵ ����
	std::vector<Expr> expr;             // ���µ��ֶ�����
};

/************************************************************************
*    ��ɾ����Ϣ
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
	std::vector<Expr> expr;             // ɾ�����ֶ�����
};




// �������ݿ�
bool CreateDatabase(std::string database_name, CatalogPosition &cp);

// Drop���ݿ�
bool DropDatabase(std::string database_name, CatalogPosition &cp);
void DelFilesInFolder(std::string folderPath);  // ɾ��Ŀ¼�µ������ļ����ļ���

// �������ݿ���������
std::vector<std::string> ShowDatabase(CatalogPosition &cp);

// ѡ�����ݿ�
bool UseDatabase(std::string db_name, CatalogPosition &cp);


// ������ eg. create table test1(id int,score double,Name char(20) primary);
bool CreateTable(TB_Create_Info tb_create_info, std::string path = std::string("./"));

// ɾ����
bool DropTable(std::string table_name, std::string path = std::string("./"));

// ��ʾ���ݿ��µı���
std::vector<std::string> ShowAllTable(bool b, std::string path = std::string("./"));

// �����¼ eg. insert into test1(id, score, Name)values(10, 1.5, bcd);
bool InsertRecord(TB_Insert_Info tb_insert_info, std::string path = std::string("./"));

// ѡ���¼
SelectPrintInfo SelectTable(TB_Select_Info tb_select_info, std::string path = std::string("./"));

// ���¼�¼ 
bool UpdateTable(TB_Update_Info tb_update_info, std::string path = std::string("./"));

// ɾ����¼
bool DeleteTable(TB_Delete_Info tb_delete_info, std::string path = std::string("./"));

// ��ӡ���ű�
std::vector<RecordHead> ShowTable(std::string table_name, std::string path = std::string("./"));

// ȡ��ָ����ַ������
RecordHead GetDbfRecord(std::string table_name, FileAddr fd, std::string path = std::string("./"));
void PrintRecord(std::string table_name, KeyAttr key, FileAddr fd, std::string path = std::string("./"));

// ���ظ��������и����ֶ������Լ���Ӧ����
std::vector<std::pair<std::string, Column_Type>> GetColumnAndTypeFromTable(std::string table_name, std::string table_path);
Column_Type GetType(std::string name, std::vector<std::pair<std::string, Column_Type>> vec);

// ��Χ���ҵ����ֶ� ���ز��ҵ�ֵ�������ļ��еĵ�ַ
std::vector<std::pair<KeyAttr, FileAddr>> Search(CompareCell compare_cell, std::string table_name, std::string path = std::string("./"));
std::vector<std::pair<KeyAttr, FileAddr>> KeySearch(CompareCell compare_cell, std::string table_name, std::string path = std::string("./"));  //��������
std::vector<std::pair<KeyAttr, FileAddr>> RangeSearch(CompareCell compare_cell, std::string table_name, std::string path = std::string("./"));  // ��������


// �Ƚϵ��ֶ����ƣ��Ƚϵ��ֶ����ͣ��ȽϹ�ϵ���Ƚϵ�ֵ
CompareCell CreateCmpCell(std::string column_name, Column_Type column_type, Operat_Type Optype, std::string value);

// �����ļ�ͷ��Ϣ������
class TableIndexHeadInfo
{
public:
	TableIndexHeadInfo(BTree &_tree) :tree(_tree) {}
	// ����ֶθ���
	size_t GetColumnCount()const;
	// �����ֶ�����
	std::vector<std::string> GetColumnNames()const;
	// �����ֶ�����
	std::vector<Column_Type> GetColumnType()const;
	Column_Type GetColumnType(std::string column_name)const;
	// �����ֶεĴ�С
	std::vector<int> GetColumnSize()const;
	// ��i���ֶε����ݴ�С
	int GetColumnSizeByIndex(int i)const;
	// �����ֶε�����
	int GetPrimaryIndex()const;

	// �жϸ��ֶ����ǲ��Ǳ���ֶ�
	bool IsColumnName(std::string column_name)const;
	// ���ظ��ֶ��������ֶ��е�����λ��
	int GetIndex(std::string column_name)const;
	// �ж��ֶ����Ƿ�Ϊ�����ֶ�
	bool IsPrimary(std::string column_name)const;
	// ���ظ����ֶ����ֶξ�������ͷ��ַ��ƫ��
	int GetColumnOffset(std::string column_name);
private:
	BTree &tree;
};


#endif
