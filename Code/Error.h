#pragma once
#include <iostream>
#include <fstream>
#include <string>
namespace Error_SQL
{
	/************************************************************************
	*
	*   接口类
	*
	*************************************************************************/
	extern std::fstream Log_file;
	class BaseError
	{
	public:
		virtual void PrintError()const;
		virtual void WriteToLog()const;
	protected:
		std::string ErrorInfo;
		std::string ErrorPos;

	};
	// 错误处理函数
	void DispatchError(const Error_SQL::BaseError &error);


	/************************************************************************
	*
	*   具体类
	*
	*************************************************************************/

	// 派生类错误
	class LSEEK_ERROR :public BaseError
	{
	public:
		LSEEK_ERROR();
	};

	// 文件读错误
	class READ_ERROR :public BaseError
	{
	public:
		READ_ERROR();
	};

	// 文件写错误
	class WRITE_ERROR :public BaseError
	{
	public:
		WRITE_ERROR();
	};

	// 文件名转换错误
	class FILENAME_CONVERT_ERROR :public BaseError
	{
	public:
		FILENAME_CONVERT_ERROR();
	};

	// 索引文件插入关键字失败
	class KEY_INSERT_ERROR :public BaseError
	{
	public:
		KEY_INSERT_ERROR();
	};

	// B+树的度偏大
	class BPLUSTREE_DEGREE_TOOBIG_ERROR :public BaseError
	{
	public:
		BPLUSTREE_DEGREE_TOOBIG_ERROR();
	};

	// 关键字名字长度超过限制
	class KeyAttr_NameLength_ERROR :public BaseError
	{
	public:
		KeyAttr_NameLength_ERROR();
	};

	// --------------------------------------解释器模块------------------------------------
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

