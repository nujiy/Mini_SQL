#include "Interpreter.h"



std::string CreateDbInfo(std::vector<std::string> sen_str)
{
	if ((sen_str.size() != 3)
		|| (StrToLower(sen_str[0]) != "create")
		|| (StrToLower(sen_str[1]) != "database")
		)
		throw Error_SQL::CMD_FORMAT_ERROR();
	return sen_str[2];
}

std::string DeleteDbInfo(std::vector<std::string> sen_str)
{
	if ((sen_str.size() != 3)
		|| (StrToLower(sen_str[0]) != "drop")
		|| (StrToLower(sen_str[1]) != "database")
		)
		throw Error_SQL::CMD_FORMAT_ERROR();
	return sen_str[2];
}

std::string UseDbInfo(std::vector<std::string> sen_str)
{
	if ((sen_str.size() != 3)
		|| (StrToLower(sen_str[0]) != "use")
		|| (StrToLower(sen_str[1]) != "database")
		)
		throw Error_SQL::CMD_FORMAT_ERROR();
	return sen_str[2];
}

std::string ShowDbInfo(std::vector<std::string> sen_str)
{

	// show databases
	if ((sen_str.size() != 2)
		|| (StrToLower(sen_str[0]) != "show")
		|| (StrToLower(sen_str[1]) != "databases")
		)
		throw Error_SQL::CMD_FORMAT_ERROR();
	return std::string();
}

std::string DropTableInfo(std::vector<std::string> sen_str)
{
	if ((sen_str.size() < 3)
		|| (StrToLower(sen_str[0]) != "drop")
		|| (StrToLower(sen_str[1]) != "table")
		)
		throw Error_SQL::CMD_FORMAT_ERROR();
	return sen_str[2];
}

TB_Select_Info TableSelectInfo(std::vector<std::string> sen_str)
{
	TB_Select_Info tb_select_info;
	// 选择的字段名称
	if (StrToLower(sen_str[0]) != "select")
		throw Error_SQL::CMD_FORMAT_ERROR();
	int name_L_index = 1;
	int name_R_index = 0;
	for (int i = 0; i < sen_str.size(); i++)
	{
		if (StrToLower(sen_str[i]) == "from")
		{
			name_R_index = i - 1;
			break;
		}
	}
	if (!name_R_index)
		throw Error_SQL::CMD_FORMAT_ERROR();
	for (int i = name_L_index; i <= name_R_index; i++)
	{
		tb_select_info.name_selected_column.push_back(sen_str[i]);
	}

	if (sen_str.size() - 1 < (name_R_index + 2))
		throw Error_SQL::CMD_FORMAT_ERROR();
	tb_select_info.table_name = sen_str[name_R_index + 2];

	int name_where_index = name_R_index + 3;
	if (sen_str.size() - 1 < name_where_index)
		return tb_select_info;

	auto mpair = GetColumnAndTypeFromTable(tb_select_info.table_name, GetCp().GetCurrentPath());
	// 打包查找条件
	for (int i = name_where_index + 1; i < sen_str.size();)
	{
		if (StrToLower(sen_str[i]) == ";")
			break;
		CompareCell cmp_cell = CreateCmpCell(sen_str[i], GetType(sen_str[i], mpair), GetOperatorType(sen_str[i + 1]), sen_str[i + 2]);
		tb_select_info.vec_cmp_cell.push_back(cmp_cell);

		// 下一个查找条件
		if ((i + 3) < sen_str.size() && StrToLower(sen_str[i + 3]) == "and")
		{
			i += 4;
		}
		else
		{
			break;
		}

	}
	return tb_select_info;
}

TB_Update_Info TableUpdateInfo(std::vector<std::string> sen_str)
{
	// TODO 语法检擦

	TB_Update_Info tb_update_info;
	int UPDATE = 0;
	int SET = 2;
	int WHERE = 0;
	for (int i = 0; i < sen_str.size(); i++)
	{
		if (StrToLower(sen_str[i]) == "where")
		{
			WHERE = i;
			break;
		}
	}

	// 新的字段值
	for (int j = SET + 1; j < WHERE;)
	{
		TB_Update_Info::NewValue new_value;

		new_value.field = sen_str[j];
		new_value.value = sen_str[j + 2];
		tb_update_info.field_value.push_back(new_value);

		if (sen_str[j + 3] == ",")
			j += 4;
		else
			j += 3;
	}

	// 需要更新的字段条件
	for (int j = WHERE + 1; j < sen_str.size();)
	{
		TB_Update_Info::Expr expr;
		expr.field = sen_str[j];
		expr.op = sen_str[j + 1];
		expr.value = sen_str[j + 2];
		tb_update_info.expr.push_back(expr);
		j += 4;
	}
	tb_update_info.table_name = sen_str[UPDATE + 1];
	return tb_update_info;
}

TB_Delete_Info TableDeleteInfo(std::vector<std::string> sen_str)
{
	TB_Delete_Info tb_delete_info;
	tb_delete_info.table_name = sen_str[2];

	for (int i = 4; i < sen_str.size(); )
	{
		if (sen_str[i] == ";")
			break;
		TB_Delete_Info::Expr expr;
		expr.field = sen_str[i];
		expr.op = sen_str[i + 1];
		expr.value = sen_str[i + 2];
		tb_delete_info.expr.push_back(expr);
		i += 4;
	}
	return tb_delete_info;
}

bool CreateShowTableInfo(std::vector<std::string> sen_str)
{

	if (!GetCp().GetIsInSpeDb() || sen_str.size() < 2 || StrToLower(sen_str[0]) != "show" || StrToLower(sen_str[1]) != "tables")
	{
		return false;
	}
	else
	{
		return true;
	}
}

TB_Create_Info CreateTableInfo(std::vector<std::string> sen_str)
{
	TB_Create_Info tb_create_info;

	/*assert(sen_str.size()>=3);
	tb_create_info.table_name = sen_str[2];

	if (sen_str[3] != "(")
		throw Error_SQL::CMD_FORMAT_ERROR();

	int i = 4;
	while (i < sen_str.size())
	{
		if (sen_str[i] == ";")
			break;
		if(i+1>=sen_str.size())
			throw Error_SQL::CMD_FORMAT_ERROR();

		TB_Create_Info::ColumnInfo colmu_info;
		colmu_info.name = sen_str[i];
		colmu_info.type = StrConvertToEnumType(sen_str[i + 1]);
		colmu_info.isPrimary = false;

		if (sen_str[i + 2] == ",")
		{
			tb_create_info.columns_info.push_back(colmu_info);
			i += 3;
			continue;
		}
		else if (sen_str[i + 2] == "primary")
		{
			colmu_info.isPrimary = true;
			tb_create_info.columns_info.push_back(colmu_info);
			i += 4;
			continue;
		}
		else if (sen_str[i + 2] == "(")
		{
			colmu_info.length = StrToInt(sen_str[i + 3]);
			if (sen_str[i + 4] != ")")
				throw Error_SQL::CMD_FORMAT_ERROR();
			if (sen_str[i + 5] == "primary")
			{
				colmu_info.isPrimary = true;
				tb_create_info.columns_info.push_back(colmu_info);
				i += 7;
				continue;
			}
			else
			{
				tb_create_info.columns_info.push_back(colmu_info);
				i += 6;
				continue;
			}
		}
		else if (sen_str[i + 2] == ")")
		{
			i += 3;
			continue;
		}
	}*/

	if (sen_str.size() < 3 || StrToLower(sen_str[0]) != "create" || StrToLower(sen_str[1]) != "table")
		throw Error_SQL::CMD_FORMAT_ERROR();
	// 表名
	tb_create_info.table_name = sen_str[2];

	bool HasPrimary = false;
	// 添加各个字段
	for (int j = 3; j < sen_str.size();)
	{
		TB_Create_Info::ColumnInfo column_info;
		column_info.isPrimary = false;
		// 列名
		column_info.name = sen_str[j];

		// 列类型
		if (j + 1 >= sen_str.size()) throw Error_SQL::CMD_FORMAT_ERROR();

		if (StrToLower(sen_str[j + 1]) == "int")
		{
			column_info.type = Column_Type::I;
			column_info.length = sizeof(int);
			j += 2;
		}
		else if (StrToLower(sen_str[j + 1]) == "double")
		{
			column_info.type = Column_Type::D;
			column_info.length = sizeof(double);
			j += 2;
		}
		else if (StrToLower(sen_str[j + 1]) == "char")
		{
			column_info.type = Column_Type::C;
			if (j + 2 >= sen_str.size())throw Error_SQL::CMD_FORMAT_ERROR();
			column_info.length = stoi(sen_str[j + 2]);

			j += 3;
		}
		else
		{
			throw Error_SQL::CMD_FORMAT_ERROR("Unsupported data types!");
		}

		// 是否主键
		if (j < sen_str.size() && (sen_str[j] == "primary"))
		{
			if (HasPrimary)
				throw Error_SQL::CMD_FORMAT_ERROR("Error!More than one primary key!");
			HasPrimary = true;
			column_info.isPrimary = true;
			j++;
		}


		tb_create_info.columns_info.push_back(column_info);
	}
	if (!HasPrimary)
		tb_create_info.columns_info[0].isPrimary = true;

	return tb_create_info;
}

TB_Insert_Info CreateInsertInfo(std::vector<std::string> sen_str)
{
	TB_Insert_Info tb_insert_info;


	if (sen_str.size() < 3 || StrToLower(sen_str[0]) != "insert" || StrToLower(sen_str[1]) != "into")
		throw Error_SQL::CMD_FORMAT_ERROR();

	int values_index = -1;
	for (int i = 0; i < sen_str.size(); i++)
	{
		if (StrToLower(sen_str[i]) == "values")
		{
			values_index = i;
			break;
		}
	}
	if (values_index <= 0)
		throw Error_SQL::CMD_FORMAT_ERROR();

	// 读取表名
	tb_insert_info.table_name = sen_str[2];

	// 读取字段
	int p, q;
	for (p = 3, q = values_index + 1; p < values_index && q < sen_str.size(); p++, q++)
	{
		tb_insert_info.insert_info.push_back({ sen_str[p],sen_str[q] });
	}
	if ((p - 3) != (sen_str.size() - 1 - values_index))
		throw Error_SQL::CMD_FORMAT_ERROR("The size of fields is not match the size of values!");
	return tb_insert_info;
}


CmdType GetOpType(std::vector<std::string> sen_str)
{
	for (auto&e : sen_str)
		StrToLower(e);

	if (sen_str[0] == "create"&&sen_str[1] == "table")
	{
		return CmdType::TABLE_CREATE;
	}


	if (sen_str[0] == "create"&&sen_str[1] == "database")
	{
		return CmdType::DB_CREATE;
	}


	if (sen_str[0] == "drop"&&sen_str[1] == "table")
	{
		return CmdType::TABLE_DROP;
	}


	if (sen_str[0] == "drop"&&sen_str[1] == "database")
	{
		return CmdType::DB_DROP;
	}

	if (sen_str[0] == "show"&&sen_str[1] == "tables")
	{
		return CmdType::TABLE_SHOW;
	}

	if (sen_str[0] == "show"&&sen_str[1] == "databases")
	{
		return CmdType::DB_SHOW;
	}

	if (sen_str[0] == "use")
	{
		return CmdType::DB_USE;
	}

	if (sen_str[0] == "select")
	{
		return CmdType::TABLE_SELECT;
	}

	if (sen_str[0] == "insert")
	{
		return CmdType::TABLE_INSERT;
	}

	if (sen_str[0] == "update")
	{
		return CmdType::TABLE_UPDATE;
	}

	if (sen_str[0] == "delete")
	{
		return CmdType::TABLE_DELETE;
	}

	if (sen_str[0] == "delete")
	{
		return CmdType::TABLE_SELECT;
	}

	if (sen_str[0] == "quit" || sen_str[0] == "exit")
	{
		return CmdType::QUIT;
	}
	if (sen_str[0] == "help")
	{
		return CmdType::HELP;
	}

	throw Error_SQL::CMD_FORMAT_ERROR();

}


bool CompareCell::operator()(const Column_Cell &cc)
{
	switch (cmp_value.column_type)
	{
	case Column_Type::I:
		switch (OperType)
		{
		case B:
			return cc.column_value.IntValue > cmp_value.column_value.IntValue;
			break;
		case BE:
			return cc.column_value.IntValue >= cmp_value.column_value.IntValue;
			break;
		case L:
			return cc.column_value.IntValue < cmp_value.column_value.IntValue;
			break;
		case LE:
			return cc.column_value.IntValue <= cmp_value.column_value.IntValue;
			break;
		case E:
			return cc.column_value.IntValue == cmp_value.column_value.IntValue;
			break;
		case NE:
			return cc.column_value.IntValue != cmp_value.column_value.IntValue;
			break;
		default:
			return false;
			break;
		}
		break;
	case Column_Type::D:
		switch (OperType)
		{
		case B:
			return cc.column_value.DoubleValue > cmp_value.column_value.DoubleValue;
			break;
		case BE:
			return cc.column_value.DoubleValue >= cmp_value.column_value.DoubleValue;
			break;
		case L:
			return cc.column_value.DoubleValue < cmp_value.column_value.DoubleValue;
			break;
		case LE:
			return cc.column_value.DoubleValue <= cmp_value.column_value.DoubleValue;
			break;
		case E:
			return cc.column_value.DoubleValue == cmp_value.column_value.DoubleValue;
			break;
		case NE:
			return cc.column_value.DoubleValue != cmp_value.column_value.DoubleValue;
			break;
		default:
			return false;
			break;
		}
		break;
	case Column_Type::C:
		switch (OperType)
		{
		case B:
			return std::string(cc.column_value.StrValue) > std::string(cmp_value.column_value.StrValue);
			break;
		case BE:
			return std::string(cc.column_value.StrValue) >= std::string(cmp_value.column_value.StrValue);
			break;
		case L:
			return std::string(cc.column_value.StrValue) < std::string(cmp_value.column_value.StrValue);
			break;
		case LE:
			return std::string(cc.column_value.StrValue) <= std::string(cmp_value.column_value.StrValue);
			break;
		case E:
			return std::string(cc.column_value.StrValue) == std::string(cmp_value.column_value.StrValue);
			break;
		case NE:
			return std::string(cc.column_value.StrValue) != std::string(cmp_value.column_value.StrValue);
			break;
		default:
			return false;
			break;
		}
		break;
	default:
		return false;
		break;
	}
	return false;
}


void PrintWindow::CreateTable(bool is_created)
{
	if (is_created)
	{
		std::cout << "table create succeed!" << std::endl;
	}
	else
	{
		std::cout << "table create failed!" << std::endl;
	}
}



void PrintWindow::ShowAllTable(std::vector<std::string> sen_str, std::string path)
{
	std::cout << "There are all the tables !" << std::endl;
	std::cout << "+---------------------------------------------------------------+" << std::endl;
	Print(PRINTLENGTH, "table");
	std::cout << "+---------------------------------------------------------------+" << std::endl;

	if (!GetCp().GetIsInSpeDb() || sen_str.size() < 2)
	{
		throw Error_SQL::CMD_FORMAT_ERROR("Not use database or ");
	}

	std::vector<std::string> tables;

	_finddata_t FileInfo;
	path += "*.*";
	decltype(_findfirst(path.c_str(), &FileInfo)) k;
	decltype(_findfirst(path.c_str(), &FileInfo)) HANDLE;
	k = HANDLE = _findfirst(path.c_str(), &FileInfo);


	while (k != -1)
	{
		// 如果是普通文件夹则输出
		if (!(FileInfo.attrib&_A_SUBDIR) && strcmp(FileInfo.name, ".") != 0 && strcmp(FileInfo.name, "..") != 0)
		{
			// 只检查后缀.idx的文件
			std::string tmp_file(FileInfo.name);
			int index = tmp_file.size() - 1;


			if (tmp_file.size() < 4 || tmp_file[index] != 'x' || tmp_file[index - 1] != 'd' || tmp_file[index - 2] != 'i' || tmp_file[index - 3] != '.')
			{
				;
			}
			else
			{
				tables.push_back(std::string(tmp_file.begin(), tmp_file.begin() + tmp_file.size() - 4));
			}
		}

		k = _findnext(HANDLE, &FileInfo);
	}
	_findclose(HANDLE);

	// 排序
	std::sort(tables.begin(), tables.end());
	for (auto e : tables)
		Print(PRINTLENGTH, e);
	std::cout << "+---------------------------------------------------------------+" << std::endl;
	std::cout << tables.size() << " row in set." << std::endl;
}

void PrintWindow::DropTable(bool is_dropped)
{
	if (is_dropped)
	{
		std::cout << "Drop table succeed!" << std::endl;
	}
	else
	{
		std::cout << "Drop table succeed!" << std::endl;
	}
}


void PrintWindow::SelectTable(SelectPrintInfo select_table_print_info)
{
	for (auto it = select_table_print_info.name_selected_column.begin(); it != select_table_print_info.name_selected_column.end();)
	{
		if (*it == ",")
		{
			select_table_print_info.name_selected_column.erase(it);
		}
		else
		{
			it++;
		}
	}
	if (select_table_print_info.key_fd.size() < 1)
		return;
	std::string idx_file = GetCp().GetCurrentPath() + select_table_print_info.table_name + ".idx";
	std::string dbf_file = GetCp().GetCurrentPath() + select_table_print_info.table_name + ".dbf";
	BTree tree(idx_file);
	TableIndexHeadInfo table_index_head_info(tree);
	int total_length = 0;
	auto key_fd = select_table_print_info.key_fd;



	auto all_column_name = table_index_head_info.GetColumnNames();
	auto all_column_len = table_index_head_info.GetColumnSize();
	std::vector<std::string> out_name = select_table_print_info.name_selected_column;
	if (out_name.size() == 1 && out_name[0] == "*")
		out_name = all_column_name;
	int n_output_col = out_name.size();

	int total_record_len = 2;
	std::vector<int> out_len;
	for (int i = 0; i < out_name.size(); i++)
	{
		auto it = find(all_column_name.begin(), all_column_name.end(), out_name[i]);
		Column_Type type = table_index_head_info.GetColumnType()[it - all_column_name.begin()];
		switch (type)
		{
		case Column_Type::I:
			out_len.push_back(8);
			total_record_len += 8;
			break;
		case Column_Type::C:
			out_len.push_back(all_column_len[it - all_column_name.begin()]);
			total_record_len += all_column_len[it - all_column_name.begin()];
			break;
		case Column_Type::D:
			out_len.push_back(15);
			total_record_len += 15;
			break;
		default:
			break;
		}
	}

	total_record_len += (out_name.size() - 1);
	//打印头部
	std::cout << "+";
	for (int i = 0; i < out_name.size(); i++)
	{
		int len = out_len[i] + 1;
		if (i == out_name.size() - 1)len--;
		for (int j = 0; j < len; j++)
		{
			std::cout << "-";
		}

	}
	std::cout << "+";


	std::cout << std::endl;
	// 打印列名
	for (int i = 0; i < out_name.size(); i++)
	{
		std::cout << "|" << std::left << std::setw(out_len[i]) << out_name[i];
	}
	std::cout << "|" << std::endl;
	// 分割线
	std::cout << "+";
	for (int i = 0; i < total_record_len - 2; i++)std::cout << "-";
	std::cout << "+" << std::endl;

	// 打印每一条记录
	for (int i = 0; i < key_fd.size(); i++)
	{
		RecordHead record_head = GetDbfRecord(select_table_print_info.table_name, key_fd[i].second, GetCp().GetCurrentPath());
		auto pColumn = record_head.GetFirstColumn();
		while (pColumn)
		{
			auto it = find(out_name.begin(), out_name.end(), pColumn->columu_name);
			if (it != out_name.end())
			{
				switch (pColumn->column_type)
				{
				case Column_Type::I:
					std::cout << "|" << std::left << std::setw(out_len[it - out_name.begin()]) << pColumn->column_value.IntValue;
					break;
				case Column_Type::D:

					std::cout << "|" << std::left << std::setw(out_len[it - out_name.begin()]) << pColumn->column_value.DoubleValue;
					break;
				case Column_Type::C:

					std::cout << "|" << std::left << std::setw(out_len[it - out_name.begin()]) << pColumn->column_value.StrValue;
					break;
				default:
					break;
				}
			}
			pColumn = pColumn->next;
		}
		std::cout << "|" << std::endl;
	}



	// 输出最后一行
	std::cout << "+";
	for (int i = 0; i < total_record_len - 2; i++)std::cout << "-";
	std::cout << "+";
	std::cout << std::endl;

	std::cout << select_table_print_info.key_fd.size() << " row in set.[";
	GetTimer().PrintTimeSpan();
	std::cout << "used.]" << std::endl;
}

void PrintWindow::InsertRecord(bool is_inserted)
{
	if (is_inserted)
	{
		std::cout << "Insert succeed!" << std::endl;
	}
	else
	{
		std::cout << "Insert succeed!" << std::endl;
	}
}

void PrintWindow::CreateDB(bool is_created)
{
	if (is_created)
	{
		std::cout << "Create succeed!" << std::endl;
	}
	else
	{
		std::cout << "Create failed!" << std::endl;
	}
}

void PrintWindow::DropDB(bool is_dropped)
{
	if (is_dropped)
	{
		std::cout << "Drop database succeed!" << std::endl;
	}
	else
	{
		std::cout << "Drop database failed!" << std::endl;
	}
}

void PrintWindow::ShowDB(std::vector<std::string> db_names)
{
	std::cout << "There are all the databases !" << std::endl;
	std::cout << "+---------------------------------------------------------------+" << std::endl;
	Print(PRINTLENGTH, "Database");
	std::cout << "+---------------------------------------------------------------+" << std::endl;
	for (auto e : db_names)
	{
		Print(PRINTLENGTH, e);
	}
	std::cout << "+---------------------------------------------------------------+" << std::endl;
	std::cout << db_names.size() << " row in set." << std::endl;
}

void PrintWindow::UseDB(bool isUsed)
{
	if (isUsed)
	{
		std::cout << "databae changed!" << std::endl;
	}
	else
	{
		std::cout << "database absent" << std::endl;
	}
}


void PrintWindow::UpdateTable(bool isUpdated)
{
	if (isUpdated)
	{
		std::cout << "Update succeed!" << std::endl;
	}
	else
	{
		std::cout << "Update failed" << std::endl;
	}
}

void PrintWindow::DeleteTable(bool isDeleted)
{
	if (isDeleted)
	{
		std::cout << "Delete succeed!" << std::endl;
	}
	else
	{
		std::cout << "Delete failed" << std::endl;
	}
}

void PrintWindow::Print(int len, std::string s)
{
	std::cout << "|";
	std::cout << s;
	for (int i = 0; i < (PRINTLENGTH - s.size()); i++)std::cout << " ";
	std::cout << "|";
	std::cout << std::endl;

}

int PrintWindow::GetColumnLength(std::string name, std::vector<std::string> col_name, std::vector<int> col_len)
{
	for (int j = 0; j < col_name.size(); j++)
	{
		if (name == col_name[j])
		{
			return col_len[j] > col_name[j].size() ? col_len[j] : col_name[j].size();
		}
	}
	return 0;
}

void Interpreter(std::vector<std::string> sen_str, CmdType cmd_type, PrintWindow print_window)
{
	auto &cp = GetCp();
	TB_Select_Info tb_select_info;
	std::vector<FileAddr> fds;


	switch (cmd_type)
	{
	case CmdType::TABLE_CREATE:      // 创建表
		print_window.CreateTable(CreateTable(CreateTableInfo(sen_str), cp.GetCurrentPath()));
		break;

	case CmdType::TABLE_DROP:        // 删除表
		print_window.DropTable(DropTable(DropTableInfo(sen_str), cp.GetCurrentPath()));
		break;

	case CmdType::TABLE_SHOW:        // 列出当前数据库下所有表
		print_window.ShowAllTable(sen_str, cp.GetCurrentPath());
		break;

	case CmdType::TABLE_SELECT:      // 选择表的特定记录
		print_window.SelectTable(SelectTable(TableSelectInfo(sen_str), cp.GetCurrentPath()));
		break;

	case CmdType::TABLE_INSERT:      // 插入新的记录
		print_window.InsertRecord(InsertRecord(CreateInsertInfo(sen_str), cp.GetCurrentPath()));
		break;

	case CmdType::TABLE_UPDATE:      // 更新表的记录
		print_window.UpdateTable(UpdateTable(TableUpdateInfo(sen_str), cp.GetCurrentPath()));
		break;


	case CmdType::TABLE_DELETE:      // 删除表的记录
		print_window.DeleteTable(DeleteTable(TableDeleteInfo(sen_str), cp.GetCurrentPath()));
		break;

	case CmdType::DB_CREATE:         // 创建数据库
		print_window.CreateDB(CreateDatabase(CreateDbInfo(sen_str), cp));
		break;

	case CmdType::DB_DROP:           // 删除数据库
		print_window.DropDB(DropDatabase(DeleteDbInfo(sen_str), cp));
		break;

	case CmdType::DB_SHOW:           // 列出所有数据库
		print_window.ShowDB(ShowDatabase(cp));
		break;

	case CmdType::DB_USE:            // 使用数据库
		print_window.UseDB(UseDatabase(UseDbInfo(sen_str), cp));
		break;

	default:
		throw Error_SQL::CMD_FORMAT_ERROR();
		break;
	}
}

Meaning_String::Meaning_String(std::string srcstr /*= ""*/)
	:src_str(srcstr)
{
	Parse();
}

void Meaning_String::SetSrcStr(std::string _srcstr)
{
	src_str = _srcstr;
	sen_str.clear();
	Parse();
}

std::vector<std::string> Meaning_String::GetSensefulStr() const
{
	return sen_str;
}

void Meaning_String::Parse()
{
	int i = 0;
	sen_str.clear();
	std::string token;
	while (i < src_str.size())
	{
		if (src_str[i] == 34 || src_str[i] == 39)
		{
			token.clear();
			i++;
			while ((src_str[i] != 34) && (src_str[i] != 39))
			{
				token += src_str[i];
				i++;
			}
			i++;
			sen_str.push_back(token);
			token.clear();
			continue;
		}
		if (IsKeyChar(src_str[i]))
		{
			if (!token.empty())
				sen_str.push_back(token);
			token.clear();
			// 跳过关键字符，除了>=<比较符
			while (IsKeyChar(src_str[i]))
			{
				std::string tmp_token;
				if (src_str[i] == '>' || src_str[i] == '=' || src_str[i] == '<')  // 比较符号
				{
					tmp_token += src_str[i];
					if (src_str[i + 1] == '=')
					{
						tmp_token += src_str[i + 1];
						i += 2;
					}
					else
					{
						i++;
					}
					sen_str.push_back(tmp_token);
				}
				else
				{
					i++;
				}
			}

		}
		else
		{
			token += src_str[i];
			i++;
		}
	}
}

void Meaning_String::Parse2()
{
	int i = 0;
	sen_str.clear();
	std::string token;
	while (i < src_str.size())
	{
		if (src_str[i] == 34 || src_str[i] == 39)
		{
			token.clear();
			i++;
			while ((src_str[i] != 34) && (src_str[i] != 39))
			{
				token += src_str[i];
				i++;
			}
			i++;
			sen_str.push_back(token);
			token.clear();
			continue;
		}
		if (IsKeyChar(src_str[i]))
		{
			if (!token.empty())
				sen_str.push_back(token);
			token.clear();
			// 跳过关键字符，除了>=<比较符
			while (IsKeyChar(src_str[i]))
			{
				std::string tmp_token;
				if (src_str[i] == '>' || src_str[i] == '=' || src_str[i] == '<')  // 比较符号
				{
					tmp_token += src_str[i];
					if (src_str[i + 1] == '=')
					{
						tmp_token += src_str[i + 1];
						i += 2;
					}
					else
					{
						i++;
					}
					sen_str.push_back(tmp_token);
				}
				else
				{
					i++;
				}
			}

		}
		else
		{
			token += src_str[i];
			i++;
		}
	}

	for (auto e : sen_str)
		std::cout << e << " " << std::endl;
	std::cout << std::endl;
}

bool Meaning_String::IsKeyChar(char c)
{
	auto it = std::find(key_char.begin(), key_char.end(), c);
	return (it != key_char.end());
}
