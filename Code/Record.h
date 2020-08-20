#pragma once
#include <tuple>
#include "Error.h"
#include "Buffer.h"

/***********************************************************************************
*
*    �����¼�����ֶε�����
*    �ֶ����� I---int  C---�ַ���  D---Doouble
*
***********************************************************************************/
enum class Column_Type { I, C, D };

// �������͵��ַ�����ʽת��Ϊö������
Column_Type StrConvertToEnumType(std::string str_type);




/***********************************************************************************
*
*    ���ϵ����ݽṹ���� �ֶε�ֵ
*
***********************************************************************************/
union Column_Value
{
	int   		        IntValue;		 //����ֵ
	double 		        DoubleValue;     //������ֵ
	char*               StrValue;	     //�ַ���ָ�� 
};


/***********************************************************************************
*
*    ���������ļ��ؼ�������
*
***********************************************************************************/
class KeyAttr
{
public:
	using Key_Value = union {
		char                StrValue[ColumnNameLength];	     //�ַ���ָ�� 
		int   		        IntValue;		                 //����ֵ
		double 		        DoubleValue;                     //������ֵ	
	};
	Column_Type type;
	Key_Value value;

	bool operator<(const KeyAttr &rhs)const;
	bool operator>(const KeyAttr &rhs)const;
	bool operator==(const KeyAttr &rhs)const;
	bool operator<=(const KeyAttr &rhs)const;
	bool operator>=(const KeyAttr &rhs)const;
	bool operator!=(const KeyAttr &rhs)const;

};
std::ostream& operator<<(std::ostream &os, const KeyAttr &key);

/***********************************************************************************
*
*    ����ÿ���ֶεĵ�Ԫ����
*    �ֶε�Ԫ��¼�˸��ֶε��������͡����ݵ�ֵ���Լ����ֶε���һ���ֶ�ָ��
*    ע�⿽���͸�ֵ����
*    ���������ֶ����ַ������ͣ����ַ�����ǰ�����ַ���ʾ����ʱ������ַ�������
***********************************************************************************/

class Column_Cell
{
public:
	Column_Cell();
	Column_Cell(KeyAttr key);
	Column_Cell(const Column_Cell& rhs); // ��������
	Column_Cell& operator=(const Column_Cell&rhs); // ������ֵ

	Column_Cell(Column_Cell&& rhs) = delete; // �ƶ�����
	Column_Cell& operator=(Column_Cell&&rhs) = delete; // �ƶ���ֵ

	size_t size()const;
	void* data()const;
	~Column_Cell();

	Column_Type column_type;
	std::string columu_name;
	Column_Value column_value;
	Column_Cell *next;

	size_t sz = 0;   // �����ַ����ֶεĳ���
	// ����ת��
	operator KeyAttr()const;
};


/***********************************************************************************
*
*    ����һ����¼��ͷ�ṹ��Ψһ��־һ����¼
*    ��¼��ͷ��¼�˸ü�¼�ĵ�һ���ֶεĵ�ַ
*    ��¼�����ֶ����������ʽ�γ�������¼
*    �����ַ��������ֶΣ��ַ���ǰ�����ַ��Ǳ�ʾ���ֶ��ַ����ĳ���
*
***********************************************************************************/
class RecordHead
{
public:
	RecordHead();
	RecordHead(const RecordHead &rhs); // ��������
	RecordHead& operator=(const RecordHead&rhs);  // ������ֵ
	RecordHead(RecordHead &&rhs); // �ƶ�����
	RecordHead& operator=(RecordHead&&rhs);  // �ƶ���ֵ
	~RecordHead();

	void AddColumnCell(const Column_Cell &cc);
	size_t size()const;                         // ����������¼�Ĵ�С
	Column_Cell* GetFirstColumn()const;

private:
	Column_Cell *phead;  // ָ���¼�ĵ�һ���ֶ�
	Column_Cell *pLast;

};
std::ostream& operator<<(std::ostream &os, const RecordHead &rd);




/***********************************************************************************
*
*    ��¼��:  �������еļ�¼���ݲ�����
*    ע��:    ��¼��д������������(.idx)�����ݲ���(.dbf)��
*             �������ֵ��޸���B+����Ӧ��ģ�鸺��
*             ��ģ������в���ֻ�޸ļ�¼��ʵ�����ݲ��֡�
*
*    ˵����   �������еļ�¼�����ļ���ַΨһ��־��ɾ���Ͳ��Ҽ�¼�ĵ�ַ���������ṩ
*
***********************************************************************************/
class Record
{
public:
	// �����µļ�¼�������²����¼�����������ļ��ĵ�ַ
	FileAddr InsertRecord(const std::string dbf_name, const RecordHead &rd);

	// ɾ����¼������ɾ���ļ�¼���������ļ��ĵ�ַ
	FileAddr DeleteRecord(const std::string dbf_name, FileAddr fd, size_t);

	// ����������¼,�����ַ��������ֶΣ��ַ���ǰ�����ַ��Ǳ�ʾ���ֶ��ַ����ĳ���
	// ʵ��д��ʱ�������ַ����ĳ���д���ļ�
	bool UpdateRecord(const std::string dbf_name, const RecordHead &rd, FileAddr fd);

private:

	// �����������ʽ����ļ�¼���ݶ�ȡ������Ϊһ�������ݿ��Ա���д�������ļ�
	// tuple�ĵ�һ��Ԫ��Ϊ��¼���ݵĴ�С���ڶ���Ԫ��Ϊ���ݵ�ָ��
	std::tuple<unsigned long, char*> GetRecordData(const RecordHead &rd);
};
