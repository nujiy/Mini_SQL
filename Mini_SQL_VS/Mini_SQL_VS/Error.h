#pragma once
#include <iostream>
#include <fstream>
#include <string>
namespace SQLError
{
	/************************************************************************
	*
	*   �ӿ���
	*
	*************************************************************************/
	extern std::fstream log_file;
	class BaseError
	{
	public:
		virtual void PrintError()const;
		virtual void WriteToLog()const;
	protected:
		std::string ErrorInfo;
		std::string ErrorPos;

	};
	// ��������
	void DispatchError(const SQLError::BaseError &error);


	/************************************************************************
	*
	*   ������
	*
	*************************************************************************/

	// ���������
	class LSEEK_ERROR :public BaseError
	{
	public:
		LSEEK_ERROR();
	};

	// �ļ�������
	class READ_ERROR :public BaseError
	{
	public:
		READ_ERROR();
	};

	// �ļ�д����
	class WRITE_ERROR :public BaseError
	{
	public:
		WRITE_ERROR();
	};

	// �ļ���ת������
	class FILENAME_CONVERT_ERROR :public BaseError
	{
	public:
		FILENAME_CONVERT_ERROR();
	};

	// �����ļ�����ؼ���ʧ��
	class KEY_INSERT_ERROR :public BaseError
	{
	public:
		KEY_INSERT_ERROR();
	};

	// B+���Ķ�ƫ��
	class BPLUSTREE_DEGREE_TOOBIG_ERROR :public BaseError
	{
	public:
		BPLUSTREE_DEGREE_TOOBIG_ERROR();
	};

	// �ؼ������ֳ��ȳ�������
	class KeyAttr_NameLength_ERROR :public BaseError
	{
	public:
		KeyAttr_NameLength_ERROR();
	};

	// --------------------------------------������ģ��------------------------------------
	class CMD_FORMAT_ERROR :public BaseError
	{
	public:
		CMD_FORMAT_ERROR(const std::string s = std::string(""));
		virtual void PrintError()const;
	protected:
		std::string error_info;
	};

	class TABLE_ERROR :public BaseError
	{
	public:
		TABLE_ERROR(const std::string s = std::string(""));
		virtual void PrintError()const;
	protected:
		std::string error_info;
	};
}

