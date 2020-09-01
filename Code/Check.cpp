#include "Check.h"
#include "API.h"


void Check_TB_Create_Info(const TB_Create_Info &tb_create_info)
{
	// ����
	std::string table_name = tb_create_info.table_name;
	std::string idx_file = GetCp().GetCurrentPath() + table_name + ".idx";
	// �жϱ��Ƿ��Ѿ�����
	if (_access(idx_file.c_str(), 0) != -1) {  //�����
		throw Error_SQL::TABLE_ERROR("The table already exists!");
	}

	// ���ÿ���ֶε���Ϣ
	for (int i = 0; i < tb_create_info.columns_info.size(); i++)
	{
		auto &column = tb_create_info.columns_info[i];

		// ����ֶ����Ƴ���
		if (column.name.size() >= ColumnNameLength)
			throw Error_SQL::TABLE_ERROR("Error!Column name length overflow");

		// ����ֶ�����
		if (column.type != Column_Type::C && column.type != Column_Type::D&& column.type != Column_Type::I)
			throw Error_SQL::TABLE_ERROR("Column data type error!");
	}

	// ����Ƿ����ؼ���
	int primary_count = 0;
	for (auto &e : tb_create_info.columns_info)
		if (e.isPrimary) primary_count++;

	if (primary_count > 1)
		throw Error_SQL::TABLE_ERROR("Error!More than one primary key!");


	// ����ֶθ���
	if (tb_create_info.columns_info.size() > RecordColumnCount)
		throw Error_SQL::TABLE_ERROR("Error!Column count is overflow!");
}

void Check_TB_Insert_Info(const TB_Insert_Info &tb_insert_info)
{
	std::string idx_file = GetCp().GetCurrentPath() + tb_insert_info.table_name + ".idx";

	// ������ھ������ݿ�Ŀ¼�£����ܲ����¼
	if (!GetCp().GetIsInSpeDb())
		throw Error_SQL::TABLE_ERROR("Error!Not use database!");

	// �жϱ��Ƿ��Ѿ�����
	if (_access(idx_file.c_str(), 0) == -1) {  //������
		throw Error_SQL::TABLE_ERROR("The table is not exists!");
	}

	BTree tree(idx_file);
	TableIndexHeadInfo table_index_head_info(tree);

	// �����������ֶε���Ϣ
	for (int i = 0; i < tb_insert_info.insert_info.size(); i++)
	{
		auto &cur_columu = tb_insert_info.insert_info[i];
		// �ж��ֶ��Ƿ�Ϸ�
		if (!table_index_head_info.IsColumnName(cur_columu.column_name))
			throw Error_SQL::TABLE_ERROR("Fields do not exist!");

		// ����ֶδ�С
		int index = table_index_head_info.GetIndex(cur_columu.column_name);
		Column_Type column_type = table_index_head_info.GetColumnType(cur_columu.column_name);
		int column_size = table_index_head_info.GetColumnSizeByIndex(index);
		if (column_type == Column_Type::C && cur_columu.column_value.size() > column_size)
			throw Error_SQL::TABLE_ERROR("Field length overflow!");

		// ��������������ַ����ֶ�
		if (table_index_head_info.IsPrimary(cur_columu.column_name))
		{
			if (column_type == Column_Type::C && (cur_columu.column_value.size() > column_size) || cur_columu.column_value.size() > ColumnNameLength)
				throw Error_SQL::TABLE_ERROR("Primary key field length overflow!");
		}


	}
}

void Check_TB_Update_Info(const TB_Update_Info &tb_update_info)
{
	std::string idx_file = GetCp().GetCurrentPath() + tb_update_info.table_name + ".idx";

	// ������ھ������ݿ�Ŀ¼�£����ܲ����¼
	if (!GetCp().GetIsInSpeDb())
		throw Error_SQL::TABLE_ERROR("Error!Not use database!");

	// �жϱ��Ƿ��Ѿ�����
	if (_access(idx_file.c_str(), 0) == -1) {  //������
		throw Error_SQL::TABLE_ERROR("The table is not exists!");
	}

	BTree tree(idx_file);
	TableIndexHeadInfo table_index_head_info(tree);

	// ���Ҫ���µ���ֵ
	for (int j = 0; j < tb_update_info.field_value.size(); j++)
	{
		auto &new_value = tb_update_info.field_value[j];
		// ����ֶ�����
		if (!table_index_head_info.IsColumnName(new_value.field))
			throw Error_SQL::TABLE_ERROR("Fields do not exist!");

		// ����ֶδ�С
		int index = table_index_head_info.GetIndex(new_value.field);
		Column_Type column_type = table_index_head_info.GetColumnType(new_value.field);
		int column_size = table_index_head_info.GetColumnSizeByIndex(index);
		if (column_type == Column_Type::C && new_value.field.size() > column_size)
			throw Error_SQL::TABLE_ERROR("Field length overflow!");

		// ��������������ַ����ֶ�
		if (table_index_head_info.IsPrimary(new_value.field))
		{
			if (column_type == Column_Type::C && (new_value.field.size() > column_size) || new_value.field.size() > ColumnNameLength)
				throw Error_SQL::TABLE_ERROR("Primary key field length overflow!");
		}
	}

	// ����������
	for (int j = 0; j < tb_update_info.expr.size(); j++)
	{
		auto &expr_tmp = tb_update_info.expr[j];
		// ����ֶ�����
		if (!table_index_head_info.IsColumnName(expr_tmp.field))
			throw Error_SQL::TABLE_ERROR("Where Expr Fields do not exist!");
		if (expr_tmp.op != ">"&& expr_tmp.op != "=" && expr_tmp.op != "<" && expr_tmp.op != ">=" && expr_tmp.op != "<=" && expr_tmp.op != "!=")
			throw Error_SQL::TABLE_ERROR("Where Expr relational operator error!");

	}
}

void Check_TB_Delete_Info(const TB_Delete_Info &tb_delete_info)
{
	std::string idx_file = GetCp().GetCurrentPath() + tb_delete_info.table_name + ".idx";
	BTree tree(idx_file);
	TableIndexHeadInfo table_index_head_info(tree);

	// ������ھ������ݿ�Ŀ¼�£����ܲ����¼
	if (!GetCp().GetIsInSpeDb())
		throw Error_SQL::TABLE_ERROR("Error!Not use database!");

	// �жϱ��Ƿ��Ѿ�����
	if (_access(idx_file.c_str(), 0) == -1) {  //������
		throw Error_SQL::TABLE_ERROR("The table is not exists!");
	}

	// ���ɾ������
	for (int j = 0; j < tb_delete_info.expr.size(); j++)
	{
		auto &expr_tmp = tb_delete_info.expr[j];
		// ����ֶ�����
		if (!table_index_head_info.IsColumnName(expr_tmp.field))
			throw Error_SQL::TABLE_ERROR("Where Expr Fields do not exist!");
		if (expr_tmp.op != ">" && expr_tmp.op != "=" && expr_tmp.op != "<" && expr_tmp.op != ">=" && expr_tmp.op != "<=" && expr_tmp.op != "!=")
			throw Error_SQL::TABLE_ERROR("Where Expr relational operator error!");
	}
}

void Check_TB_Select_Info(const TB_Select_Info &tb_select_info)
{
	std::string idx_file = GetCp().GetCurrentPath() + tb_select_info.table_name + ".idx";
	BTree tree(idx_file);
	TableIndexHeadInfo table_index_head_info(tree);

	// ������ھ������ݿ�Ŀ¼�£����ܲ����¼
	if (!GetCp().GetIsInSpeDb())
		throw Error_SQL::TABLE_ERROR("Error!Not use database!");
	// �жϱ��Ƿ��Ѿ�����
	if (_access(idx_file.c_str(), 0) == -1) {  //������
		throw Error_SQL::TABLE_ERROR("The table is not exists!");
	}
	// ���Ҫ��ʾ���ֶ�����
	if (tb_select_info.name_selected_column.size() == 1 && tb_select_info.name_selected_column[0] == "*")
	{
		;
	}
	else
	{
		for (int j = 0; j < tb_select_info.name_selected_column.size(); j++)
		{
			std::string name = tb_select_info.name_selected_column[j];
			if (!table_index_head_info.IsColumnName(name))
				throw Error_SQL::TABLE_ERROR("Selected fields do not exist!");
		}
	}

	// ����������

	for (int j = 0; j < tb_select_info.vec_cmp_cell.size(); j++)
	{
		auto &cmp = tb_select_info.vec_cmp_cell[j];
		if (!table_index_head_info.IsColumnName(cmp.cmp_value.columu_name))
			throw Error_SQL::TABLE_ERROR("Where Expr Fields do not exist!");
		if (table_index_head_info.GetColumnType(cmp.cmp_value.columu_name) != cmp.cmp_value.column_type)
			throw Error_SQL::TABLE_ERROR("Where Expr Fields type error!");
		if (cmp.OperType != Operat_Type::B
			&& cmp.OperType != Operat_Type::BE
			&& cmp.OperType != Operat_Type::E
			&& cmp.OperType != Operat_Type::L
			&& cmp.OperType != Operat_Type::LE
			&& cmp.OperType != Operat_Type::NE)
			throw Error_SQL::TABLE_ERROR("Where Expr relational operator error!");

	}
}
