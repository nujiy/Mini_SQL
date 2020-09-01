#include "Error.h"
#include "Color.h"
#include <vector>

namespace Error_SQL
{
	static std::fstream Log_file;

	void BaseError::PrintError() const
	{
		// 输出异常
		std::cout << RED << ErrorInfo << RESET;
	}

	void BaseError::WriteToLog() const
	{
		// 写入日志
		Log_file.open("log.txt", std::ios::out | std::ios::app);
		Log_file << ErrorInfo << std::endl;
		Log_file.close();
	}

	CMD_FORMAT_ERROR::CMD_FORMAT_ERROR(const std::string s)
	{
		ErrorInfo = "Command format error!The command is not exist!";
		error_info = s;
	}

	void CMD_FORMAT_ERROR::PrintError() const
	{
		// 输出异常
		std::cout << RED << error_info << ErrorInfo << RESET;
	}
	void Error_SQL::TABLE_ERROR::PrintError() const
	{
		std::cout << RED << error_info << ErrorInfo<< RESET;
	}

}

void Error_SQL::DispatchError(const BaseError &error)
{
	error.PrintError();
	error.WriteToLog();
}


Error_SQL::LSEEK_ERROR::LSEEK_ERROR()
{
	ErrorInfo = "(LSEEK_FAILED) The file handle is invalid or the value for origin is invalid "
		"or the position the position specified by offset is before the beginning of the file.";
}


Error_SQL::READ_ERROR::READ_ERROR()
{
	ErrorInfo = "(READ_FAILED) Illegal page number (less than zero).";
}


Error_SQL::WRITE_ERROR::WRITE_ERROR()
{
	ErrorInfo = "(WRITE_FAILED) The file handle is invalid or the file is not opened for writing "
		"or there is not enough space left on the device for the operation.";
}


Error_SQL::FILENAME_CONVERT_ERROR::FILENAME_CONVERT_ERROR()
{
	ErrorInfo = "(FILENAME_CONVERT_FAILED) File name convert failed";
}


Error_SQL::KEY_INSERT_ERROR::KEY_INSERT_ERROR()
{
	ErrorInfo = "(KEY_INSERT_FAILED) Key Word Insert Failed! The record that to inset has been excisted!";
}


Error_SQL::BPLUSTREE_DEGREE_TOOBIG_ERROR::BPLUSTREE_DEGREE_TOOBIG_ERROR()
{
	ErrorInfo = "(BPLUSTREE_DEGREE_TOOBIG) A page of file can not contain a tree node!";
}


Error_SQL::KeyAttr_NameLength_ERROR::KeyAttr_NameLength_ERROR()
{
	ErrorInfo = "(BPLUSTREE_DEGREE_TOOBIG) KeyAttr name length flowover,it may be happen in where you set the record's key!";
}

Error_SQL::TABLE_ERROR::TABLE_ERROR(const std::string s /*= std::string("")*/)
{
	ErrorInfo = "Table Operator Error!";
	error_info = s;
}



