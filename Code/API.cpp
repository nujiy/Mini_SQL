#include "API.h"
#include <algorithm>
#include <iterator>

CompareCell CreateCmpCell(std::string column_name, Column_Type column_type, Operator_Type Optype, std::string value)
{

	Column_Cell column_cell;
	column_cell.columu_name = column_name;
	Column_Type tmp = column_type;
	char*pChar = nullptr;
	switch (tmp)
	{
	case Column_Type::I:
		column_cell.column_type = Column_Type::I;
		column_cell.column_value.IntValue = stoi(value);
		break;

	case Column_Type::C:
		column_cell.column_type = Column_Type::C;
		pChar = (char*)malloc(value.size() + 1);
		strcpy(pChar, value.c_str());
		column_cell.column_value.StrValue = pChar;
		break;

	case Column_Type::D:
		column_cell.column_type = Column_Type::D;
		column_cell.column_value.DoubleValue = stod(value);
		break;
	default:
		break;
	}
	CompareCell cmp_cell(Optype, column_cell);

	return cmp_cell;
}

bool CreateDatabase(std::string db_name, CatalogPosition &cp)
{
	std::string tmp_path = cp.GetRootPath() + db_name;

	if (_access(tmp_path.c_str(), 0) == -1)  //判断数据库是否存在
	{
		tmp_path = cp.GetRootPath() + db_name;
		_mkdir(tmp_path.c_str());
		return true;
	}
	else
	{
		std::cout << "Database has been existed!Create Failed!" << std::endl;
		return false;
	}
}

bool DropDatabase(std::string db_name, CatalogPosition &cp)
{
	std::string tmp_path = cp.GetRootPath() + db_name;

	if (_access(tmp_path.c_str(), 0) == -1)  //判断数据库是否存在
	{
		std::cout << "数据库不存在" << std::endl;
		return false;
	}
	else
	{
		tmp_path = cp.GetRootPath() + db_name;

		// 删除目录下文件
		auto t = tmp_path + "/";
		DelFilesInFolder(t);
		// 删除目录

		_rmdir(tmp_path.c_str());

		return true;
	}

	return false;
}

void DelFilesInFolder(std::string folderPath)
{
	_finddata_t FileInfo;
	std::string strfind = folderPath + "*";
	decltype(_findfirst(folderPath.c_str(), &FileInfo)) Handle = _findfirst(strfind.c_str(), &FileInfo);

	if (Handle == -1L)
	{
		std::cerr << "can not match the folder path" << std::endl;
		return;
	}
	do {
		//判断是否有子目录  
		if (FileInfo.attrib & _A_SUBDIR)
		{
			//这个语句很重要  
			if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
			{
				std::string newPath = folderPath + FileInfo.name;
				newPath += "/";
				DelFilesInFolder(newPath);
				// 删除该文件夹
				_rmdir(newPath.c_str());
			}
		}
		else
		{
			// 删除文件
			auto old = GetCp().GetIsInSpeDb();
			GetCp().SetInInSpeDb(true);
			std::string name = folderPath + FileInfo.name;
			GetGlobalFileBuffer().CloseFile(name.c_str());
			remove(name.c_str());
			GetCp().SetInInSpeDb(old);

		}
	} while (_findnext(Handle, &FileInfo) == 0);

	_findclose(Handle);
}

std::vector<std::string> ShowDatabase(CatalogPosition &cp)
{
	_finddata_t FileInfo;
	std::string path = cp.GetRootPath() + "*.*";
	decltype(_findfirst(path.c_str(), &FileInfo)) k;
	decltype(_findfirst(path.c_str(), &FileInfo)) HANDLE;
	k = HANDLE = _findfirst(path.c_str(), &FileInfo);
	std::vector<std::string> dbs;

	while (k != -1)
	{
		// 如果是普通文件夹则输出
		if (FileInfo.attrib&_A_SUBDIR && strcmp(FileInfo.name, ".") != 0 && strcmp(FileInfo.name, "..") != 0)
		{
			dbs.push_back(FileInfo.name);
			//std::cout << FileInfo.name << std::endl;
		}

		k = _findnext(HANDLE, &FileInfo);
	}
	_findclose(HANDLE);

	return dbs;
}

bool UseDatabase(std::string db_name, CatalogPosition &cp)
{
	// 先判断数据库是否存在
	std::string tmp_path = cp.GetRootPath() + db_name;

	if (_access(tmp_path.c_str(), 0) == -1)  //判断数据库是否存在
	{
		return false;
	}
	else
	{
		cp.SetCurrentPath(cp.GetRootPath() + db_name + "/");
		cp.isInSpeDb = true;
		return true;
	}
}

bool CreateTable(TB_Create_Info tb_create_info, std::string path)
{
	// TODO 检查创建信息以及当前目录是否在数据库中
	Check_TB_Create_Info(tb_create_info);

	if (!GetCp().GetIsInSpeDb())
		return false;

	// 表名
	std::string table_name = tb_create_info.table_name;
	std::string idx_file = path + table_name + ".idx";
	std::string dbf_file = path + table_name + ".dbf";

	// 关键字位置,没有设置的情况下默认为第一个字段
	int KeyTypeIndex = 0;
	for (int j = 0; j < tb_create_info.columns_info.size(); j++)
	{
		if (tb_create_info.columns_info[j].isPrimary)
		{
			KeyTypeIndex = j;
			break;
		}
	}

	// 字段信息
	char RecordTypeInfo[RecordColumnCount];          // 记录字段类型信息
	char *ptype = RecordTypeInfo;
	char RecordColumnName[RecordColumnCount / 4 * ColumnNameLength];
	char *pname = RecordColumnName;

	const auto &column_info_ref = tb_create_info.columns_info;
	for (int i = 0; i < column_info_ref.size(); i++)
	{
		// 类型信息
		switch (column_info_ref[i].type)
		{
		case Column_Type::I:
			*ptype++ = 'I';
			break;

		case  Column_Type::D:
			*ptype++ = 'D';
			break;

		case Column_Type::C:
			*ptype++ = 'C';
			*ptype++ = column_info_ref[i].length / 100 + '0';
			*ptype++ = (column_info_ref[i].length % 100) / 10 + '0';
			*ptype++ = column_info_ref[i].length % 10 + '0';
		default:
			break;
		}
		// 名称信息
		strcpy(pname, column_info_ref[i].name.c_str());
		pname += ColumnNameLength;
	}
	*ptype = '\0';

	// 创建索引文件
	BTree tree(idx_file, KeyTypeIndex, RecordTypeInfo, RecordColumnName);

	// 创建数据文件
	GetGlobalFileBuffer().CreateFile(dbf_file.c_str());
	return true;
}

bool DropTable(std::string table_name, std::string path)
{
	std::string tmp_path = path + table_name;
	std::string idx = tmp_path + ".idx";
	std::string dbf = tmp_path + ".dbf";
	auto &buffer = GetGlobalFileBuffer();
	auto pClock = GetGlobalClock();
	if (!GetCp().GetIsInSpeDb())
		return false;

	if (_access(idx.c_str(), 0) == -1 || _access(dbf.c_str(), 0) == -1)  //判断表是否存在
	{
		return false;
	}
	else
	{
		// 如果文件已经被打开使用 需要先丢弃在内存中的文件页 避免程序结束再次写回
		MemFile *pIdxMF = buffer.GetMemFile(idx.c_str());
		if (pIdxMF)
		{
			for (int i = 1; i <= MEM_PAGEAMOUNT; i++)
			{
				if (pClock->MemPages[i] && pClock->MemPages[i]->fileId == pIdxMF->fileId)
				{
					pClock->MemPages[i]->fileId = 0;  // 丢弃该页
					pClock->MemPages[i]->isModified = false;
				}

			}
		}

		MemFile *pDbfMF = buffer.GetMemFile(dbf.c_str());
		if (pDbfMF)
		{
			for (int i = 1; i <= MEM_PAGEAMOUNT; i++)
			{
				if (pClock->MemPages[i] && pClock->MemPages[i]->fileId == pDbfMF->fileId)
				{
					pClock->MemPages[i]->fileId = 0;  // 丢弃该页
					pClock->MemPages[i]->isModified = false;
				}

			}
		}
		// 删除表文件
		close(pIdxMF->fileId);
		close(pDbfMF->fileId);
		remove(idx.c_str());
		remove(dbf.c_str());
		return true;
	}

	return false;
}

std::vector<std::string> ShowAllTable(bool b, std::string path /*= std::string("./")*/)
{
	std::vector<std::string> dbs;
	if (!b)
		return dbs;

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
			dbs.push_back(FileInfo.name);
		}

		k = _findnext(HANDLE, &FileInfo);
	}
	_findclose(HANDLE);

	return dbs;
}

bool InsertRecord(TB_Insert_Info tb_insert_info, std::string path /*= std::string("./")*/)
{

	Check_TB_Insert_Info(tb_insert_info);  // 错误检查

	std::string idx_file = path + tb_insert_info.table_name + ".idx";
	std::string dbf_file = path + tb_insert_info.table_name + ".dbf";
	BTree tree(idx_file);
	TableIndexHeadInfo table_index_head_info(tree);
	auto phead = tree.GetPtrIndexHeadNode();

	KeyAttr key;


	// 将记录信息封装成记录数据对象
	RecordHead record_head;
	int column_id = 0;
	for (int i = 0; phead->RecordTypeInfo[i] != '\0'; i++)
	{

		if (phead->RecordTypeInfo[i] == 'I')
		{
			Column_Cell cc;
			//找到对应的字段名称
			char *pColumnName = phead->RecordColumnName + column_id * ColumnNameLength;

			//在插入记录里寻找该字段的值
			int k = -1;
			for (int j = 0; j < tb_insert_info.insert_info.size(); j++)
			{
				if (pColumnName == tb_insert_info.insert_info[j].column_name)
				{
					k = j;
					break;
				}
			}

			if (k != -1)
			{
				cc.column_type = Column_Type::I;
				cc.column_value.IntValue = stoi(tb_insert_info.insert_info[k].column_value);
			}
			else
			{
				// 默认值填充
				cc.column_type = Column_Type::I;
				cc.column_value.IntValue = 0;
			}
			column_id++;
			record_head.AddColumnCell(cc);
		}

		if (phead->RecordTypeInfo[i] == 'D')
		{
			Column_Cell cc;
			//找到对应的字段名称
			char *pColumnName = phead->RecordColumnName + column_id * ColumnNameLength;
			//在插入记录里寻找该字段的值
			int k = -1;
			for (int j = 0; j < tb_insert_info.insert_info.size(); j++)
			{
				if (pColumnName == tb_insert_info.insert_info[j].column_name)
				{
					k = j;
					break;
				}
			}

			if (k != -1)
			{
				cc.column_type = Column_Type::D;
				cc.column_value.DoubleValue = stod(tb_insert_info.insert_info[k].column_value);
			}
			else
			{
				// 默认值填充
				cc.column_type = Column_Type::D;
				cc.column_value.DoubleValue = 0.0;
			}
			column_id++;
			record_head.AddColumnCell(cc);
		}

		if (phead->RecordTypeInfo[i] == 'C')
		{
			Column_Cell cc;
			//找到对应的字段名称
			char *pColumnName = phead->RecordColumnName + column_id * ColumnNameLength;
			//在插入记录里寻找该字段的值
			int k = -1;
			for (int j = 0; j < tb_insert_info.insert_info.size(); j++)
			{
				if (pColumnName == tb_insert_info.insert_info[j].column_name)
				{
					k = j;
					break;
				}
			}

			if (k != -1)
			{
				cc.column_type = Column_Type::C;
				cc.sz = table_index_head_info.GetColumnSizeByIndex(column_id);
				char*pChar = (char*)malloc(cc.sz);
				strcpy(pChar, tb_insert_info.insert_info[k].column_value.c_str());
				cc.column_value.StrValue = pChar;
			}
			else
			{
				// 默认值填充
				cc.column_type = Column_Type::C;
				cc.sz = table_index_head_info.GetColumnSizeByIndex(column_id);
				char*pChar = (char*)malloc(cc.sz);
				memset(pChar, 0, cc.sz);
				cc.column_value.StrValue = pChar;
			}
			column_id++;
			record_head.AddColumnCell(cc);
		}

	}


	// 检查插入的值是否已经存在
	try
	{
		KeyAttr key_tmp;
		int key_index_tmp = 0;
		auto p_tmp = record_head.GetFirstColumn();
		while (key_index_tmp != phead->KeyTypeIndex)
		{
			p_tmp = p_tmp->next;
			key_index_tmp++;
		}
		key_tmp = *p_tmp;
		auto is_fd = tree.Search(key_tmp);
		if (is_fd != FileAddr{ 0,0 })
		{
			SQLError::KEY_INSERT_ERROR e;
			throw e;
		}

	}
	catch (const SQLError::BaseError e)
	{
		SQLError::DispatchError(e);
		return false;
	}

	// 插入数据文件
	Record record;
	auto fd = record.InsertRecord(dbf_file, record_head);

	// 插入索引
	int key_index = 0;
	auto p = record_head.GetFirstColumn();
	while (key_index != phead->KeyTypeIndex)
	{
		p = p->next;
		key_index++;
	}
	key = *p;

	tree.Insert(key, fd);
	return true;
}

SelectPrintInfo SelectTable(TB_Select_Info tb_select_info, std::string path)
{
	Check_TB_Select_Info(tb_select_info);

	std::vector<std::pair<KeyAttr, FileAddr>> res;
	std::vector<std::pair<KeyAttr, FileAddr>> fds;
	GetTimer().Start();
	if (tb_select_info.vec_cmp_cell.empty())  // 查找所有记录
	{
		// 索引文件名
		std::string file_idx = path + tb_select_info.table_name + ".idx";

		// 读取索引 信息
		BTree tree(file_idx);
		auto phead = tree.GetPtrIndexHeadNode();

		// 第一个数据结点地址
		auto node_fd = phead->MostLeftNode;

		while (node_fd.offSet != 0)
		{
			//取出结点内记录
			auto pNode = tree.FileAddrToMemPtr(node_fd);
			for (int i = 0; i < pNode->count_valid_key; i++)
			{
				res.push_back({ pNode->key[i], pNode->children[i] });
			}
			// 下一个数据结点
			node_fd = tree.FileAddrToMemPtr(node_fd)->next;
		}
	}
	else
	{
		for (int i = 0; i < tb_select_info.vec_cmp_cell.size(); i++)
		{
			// 查找满足单个字段的记录
			fds = Search(tb_select_info.vec_cmp_cell[i], tb_select_info.table_name, GetCp().GetCurrentPath());
			// 新的结果和之前的结果求交集
			if (res.empty())
			{
				res = fds;
			}
			else
			{
				std::vector<std::pair<KeyAttr, FileAddr>> v;
				sort(fds.begin(), fds.end());
				sort(res.begin(), res.end());
				set_intersection(fds.begin(), fds.end(), res.begin(), res.end(), std::back_inserter(v));
				res = v;
			}

		}
	}
	GetTimer().Stop();
	SelectPrintInfo spi;
	spi.table_name = tb_select_info.table_name;
	spi.name_selected_column = tb_select_info.name_selected_column;
	spi.key_fd = res;
	return spi;
}

bool UpdateTable(TB_Update_Info tb_update_info, std::string path /*= std::string("./")*/)
{
	Check_TB_Update_Info(tb_update_info);

	std::string file_idx = path + tb_update_info.table_name + ".idx";
	std::string file_dbf = path + tb_update_info.table_name + ".dbf";
	BTree tree(file_idx);
	TableIndexHeadInfo table_index_head_info(tree);

	// 将更新操作的expr条件封装成查找条件
	std::vector<CompareCell> cmp_cells;
	auto fields_name = GetColumnAndTypeFromTable(tb_update_info.table_name, GetCp().GetCurrentPath());

	for (int i = 0; i < tb_update_info.expr.size(); i++)
	{
		CompareCell cmp_cell = CreateCmpCell(tb_update_info.expr[i].field, GetType(tb_update_info.expr[i].field, fields_name)
			, GetOperatorType(tb_update_info.expr[i].op), tb_update_info.expr[i].value);

		cmp_cells.push_back(cmp_cell);
	}

	// 查找满足更新条件的字段
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	std::vector<std::pair<KeyAttr, FileAddr>> fds;
	// 否则
	for (int i = 0; i < cmp_cells.size(); i++)
	{
		// 查找满足单个字段的记录
		fds = Search(cmp_cells[i], tb_update_info.table_name, GetCp().GetCurrentPath());
		// 新的结果和之前的结果求交集
		if (res.empty())
		{
			res = fds;
		}
		else
		{
			std::vector<std::pair<KeyAttr, FileAddr>> v;
			sort(fds.begin(), fds.end());
			sort(res.begin(), res.end());
			set_intersection(fds.begin(), fds.end(), res.begin(), res.end(), std::back_inserter(v));
			res = v;
		}
		//for (auto e : res)
			//PrintRecord(tb_update_info.table_name, e.first, e.second, path);
	}

	//更新记录
	for (int i = 0; i < res.size(); i++)
	{
		// 字段记录
		auto pdata = (char*)(GetGlobalFileBuffer()[file_dbf.c_str()]->ReadWriteRecord(&res[i].second));  // 记录数据裸指针
		pdata += sizeof(FileAddr);  // 跳过地址数据

		//更该每个要更新的字段值
		for (int j = 0; j < tb_update_info.field_value.size(); j++)
		{
			char *p = pdata + table_index_head_info.GetColumnOffset(tb_update_info.field_value[j].field);
			Column_Type column_type = table_index_head_info.GetColumnType(tb_update_info.field_value[j].field);

			switch (column_type)
			{
			case Column_Type::I:
				*(int*)p = stoi(tb_update_info.field_value[j].value);
				break;

			case Column_Type::D:
				*(double*)p = stod(tb_update_info.field_value[j].value);
				break;

			case Column_Type::C:
				strcpy(p, tb_update_info.field_value[j].value.c_str());
				break;

			default:
				break;
			}
		}


		// 判断主键的值有没有被更改
		bool isPrimary = false;
		std::string new_primary_key;
		int index = 0; // 如果主键被修改，主键新值的位置
		for (int j = 0; j < tb_update_info.field_value.size(); j++)
		{
			if (table_index_head_info.IsPrimary(tb_update_info.field_value[j].field))
			{
				isPrimary = true;
				new_primary_key = tb_update_info.field_value[j].value;
				index = j;
				break;
			}
		}

		// 修改主键索引
		if (isPrimary)
		{
			// TODO
			Column_Type old_key_type = res[i].first.type;
			tree.Delete(res[i].first);
			KeyAttr new_key;
			new_key.type = old_key_type;
			switch (new_key.type)
			{
			case Column_Type::I:
				new_key.value.IntValue = stoi(new_primary_key);
				break;
			case Column_Type::D:
				new_key.value.IntValue = stod(new_primary_key);
				break;
			case Column_Type::C:
				strcpy(new_key.value.StrValue, new_primary_key.c_str());
				break;
			default:
				break;
			}
			tree.Insert(new_key, res[i].second);
		}
	}
	return true;
}

bool DeleteTable(TB_Delete_Info tb_delete_info, std::string path /*= std::string("./")*/)
{
	std::string file_idx = path + tb_delete_info.table_name + ".idx";
	std::string file_dbf = path + tb_delete_info.table_name + ".dbf";

	// 将更新操作的expr条件封装成查找条件
	std::vector<CompareCell> cmp_cells;
	auto fields_name = GetColumnAndTypeFromTable(tb_delete_info.table_name, GetCp().GetCurrentPath());

	for (int i = 0; i < tb_delete_info.expr.size(); i++)
	{
		CompareCell cmp_cell = CreateCmpCell(tb_delete_info.expr[i].field, GetType(tb_delete_info.expr[i].field, fields_name)
			, GetOperatorType(tb_delete_info.expr[i].op), tb_delete_info.expr[i].value);
		cmp_cells.push_back(cmp_cell);
	}

	// 查找满足更新条件的字段
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	std::vector<std::pair<KeyAttr, FileAddr>> fds;

	for (int i = 0; i < cmp_cells.size(); i++)
	{
		// 查找满足单个字段的记录
		fds = Search(cmp_cells[i], tb_delete_info.table_name, GetCp().GetCurrentPath());
		// 新的结果和之前的结果求交集
		if (res.empty())
		{
			res = fds;
		}
		else
		{
			std::vector<std::pair<KeyAttr, FileAddr>> v;
			sort(fds.begin(), fds.end());
			sort(res.begin(), res.end());
			set_intersection(fds.begin(), fds.end(), res.begin(), res.end(), std::back_inserter(v));
			res = v;
		}
	}

	// 删除所有删除的结果
	BTree tree(file_idx);
	Record record;
	for (int i = 0; i < res.size(); i++)
	{
		tree.Delete(res[i].first);
		record.DeleteRecord(file_dbf, res[i].second, 0);
	}

	return true;
}

std::vector<RecordHead> ShowTable(std::string table_name, std::string path /*= std::string("./")*/)
{
	std::string idx_file = path + table_name + ".idx";
	std::string dbf_file = path + table_name + ".dbf";
	BTree tree(idx_file);
	std::vector<RecordHead> vec_record_head;

	auto data_fd = tree.GetPtrIndexHeadNode()->MostLeftNode;
	while (data_fd.offSet != 0)
	{
		auto pNode = tree.FileAddrToMemPtr(data_fd);

		for (int i = 0; i < pNode->count_valid_key; i++)
		{
			auto tmp = GetDbfRecord(table_name, pNode->children[i], path);
			vec_record_head.push_back(tmp);
		}


		data_fd = pNode->next;
	}

	return vec_record_head;
}

RecordHead GetDbfRecord(std::string table_name, FileAddr fd, std::string path /*= std::string("./")*/)
{
	std::string idx_file = path + table_name + ".idx";
	std::string dbf_file = path + table_name + ".dbf";
	BTree tree(idx_file);

	RecordHead record_head;
	// 获取结点内存地址
	char* pRecTypeInfo = tree.GetPtrIndexHeadNode()->RecordTypeInfo;
	//std::cout << pRecTypeInfo << std::endl;
	auto pdata = (char*)GetGlobalFileBuffer()[dbf_file.c_str()]->ReadRecord(&fd);
	pdata += sizeof(FileAddr);  // 每条记录头部默认添加该记录的地址值

	auto vec_name_type = GetColumnAndTypeFromTable(table_name, path);
	int index = 0;
	while (*pRecTypeInfo != '\0')
	{
		Column_Cell cc;
		switch (*pRecTypeInfo)
		{
		case 'I':
			cc.column_type = Column_Type::I;
			cc.columu_name = vec_name_type[index].first;
			cc.column_value.IntValue = *(int*)pdata;
			pdata += sizeof(int);
			record_head.AddColumnCell(cc);
			index++;
			break;

		case 'D':
			cc.column_type = Column_Type::D;
			cc.columu_name = vec_name_type[index].first;
			cc.column_value.DoubleValue = *(double*)pdata;
			pdata += sizeof(double);
			record_head.AddColumnCell(cc);
			index++;
			break;

		case 'C':
			cc.column_type = Column_Type::C;
			cc.columu_name = vec_name_type[index].first;
			// 读取字符串长度
			int sz = 0;
			sz = (*(pRecTypeInfo + 1) - '0') * 100 + (*(pRecTypeInfo + 2) - '0') * 10 + (*(pRecTypeInfo + 3) - '0');
			auto pchar = (char*)malloc(sz);
			memcpy(pchar, pdata, sz);
			cc.column_value.StrValue = pchar;
			pdata += sz;
			record_head.AddColumnCell(cc);
			index++;
			break;
		}
		pRecTypeInfo++;
	}

	return record_head;
}


void PrintRecord(std::string table_name, KeyAttr key, FileAddr fd, std::string path /*= std::string("./")*/)
{
	std::string idx_file = path + table_name + ".idx";
	std::string dbf_file = path + table_name + ".dbf";
	BTree tree(idx_file);
	auto tree_head = tree.GetPtrIndexHeadNode();
	// 输出关键字
	std::cout << key;
	// 输出记录值
	RecordHead rd = GetDbfRecord(table_name, fd, path);
	std::cout << rd << std::endl;
}

Operator_Type GetOperatorType(std::string s)
{
	s = StrToLower(s);
	if (s == ">")
	{
		return Operator_Type::B;
	}
	else if (s == ">=")
	{
		return Operator_Type::BE;
	}
	else if (s == "<")
	{
		return Operator_Type::L;
	}
	else if (s == "<=")
	{
		return Operator_Type::LE;
	}
	else if (s == "=")
	{
		return Operator_Type::E;
	}
	else if (s == "!=")
	{
		return Operator_Type::NE;
	}
	else
	{
		return Operator_Type::B;
	}
}

std::vector<std::pair<std::string, Column_Type>> GetColumnAndTypeFromTable(std::string table_name, std::string table_path)
{
	std::string idx_file = table_path + table_name + ".idx";
	std::string dbf_file = table_path + table_name + ".dbf";
	BTree tree(idx_file);

	auto phead = tree.GetPtrIndexHeadNode();

	// 记录各个字段类型
	std::vector<Column_Type> tb_types;
	int sz_col = 0;// 字段个数
	for (int i = 0; phead->RecordTypeInfo[i] != '\0'; i++)
	{
		if (phead->RecordTypeInfo[i] == 'I')
		{
			tb_types.push_back(Column_Type::I);
			sz_col++;
		}
		else if (phead->RecordTypeInfo[i] == 'D')
		{
			tb_types.push_back(Column_Type::D);
			sz_col++;
		}
		else if (phead->RecordTypeInfo[i] == 'C')
		{
			tb_types.push_back(Column_Type::C);
			sz_col++;
		}

	}

	// 记录各个字段名称
	std::vector<std::string> tb_names;
	char *pColumnName = phead->RecordColumnName;
	for (int j = 0; j < sz_col; j++)
	{
		tb_names.push_back(pColumnName);
		pColumnName += ColumnNameLength;
	}

	std::vector<std::pair<std::string, Column_Type>> res;
	for (int i = 0; i < tb_names.size(); i++)
	{
		res.push_back({ tb_names[i], tb_types[i] });
	}
	return res;
}


Column_Type GetType(std::string name, std::vector<std::pair<std::string, Column_Type>> vec)
{
	for (int i = 0; i < vec.size(); i++)
	{
		if (vec[i].first == name)
		{
			return vec[i].second;
		}
	}

	return Column_Type::I;
}

std::vector<std::pair<KeyAttr, FileAddr>> Search(CompareCell compare_cell, std::string table_name, std::string path /*= std::string("./")*/)
{
	// 保存查找结果
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	// 索引文件名
	std::string file_idx = path + table_name + ".idx";

	// 索引信息
	BTree tree(file_idx);
	TableIndexHeadInfo table_index_head_info(tree);

	// 判断待查找的字段是否是主键字段
	bool bKeyComumn = false;
	bKeyComumn = table_index_head_info.IsPrimary(compare_cell.cmp_value.columu_name);

	// 查找

	if (bKeyComumn)
	{
		res = KeySearch(compare_cell, table_name, path);
	}
	else
	{
		res = RangeSearch(compare_cell, table_name, path);
	}

	return res;
}

std::vector<std::pair<KeyAttr, FileAddr>> KeySearch(CompareCell compare_cell, std::string table_name, std::string path /*= std::string("./")*/)
{
	// 保存查找结果
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	// 索引文件名
	std::string file_idx = path + table_name + ".idx";

	// 读取索引 信息
	BTree tree(file_idx);
	auto phead = tree.GetPtrIndexHeadNode();

	// 如果是查找相等的值
	if (compare_cell.OperType == Operator_Type::E)
	{
		FileAddr fd = tree.Search(compare_cell.cmp_value);
		if (fd.offSet != 0)
		{
			res.push_back({ compare_cell.cmp_value , fd });
		}
	}
	else  // 可优化::关键字二分查找
	{
		// 第一个数据结点地址
		auto node_fd = phead->MostLeftNode;

		while (node_fd.offSet != 0)
		{
			//取出结点内记录
			const BTNode *pNode = tree.FileAddrToMemPtr(node_fd);
			for (int i = 0; i < pNode->count_valid_key; i++)
			{
				// 查找比较的字段
				Column_Cell cc(pNode->key[i]);
				bool isSearched = compare_cell(cc);
				if (isSearched)  // 满足条件
				{
					res.push_back({ pNode->key[i] ,pNode->children[i] });
				}
			}
			// 下一个数据结点
			node_fd = tree.FileAddrToMemPtr(node_fd)->next;
		}
	}

	return res;
}

std::vector<std::pair<KeyAttr, FileAddr>> RangeSearch(CompareCell compare_cell, std::string table_name, std::string path)
{
	// 保存查找结果
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	// 索引文件名
	std::string file_idx = path + table_name + ".idx";

	// 读取索引 信息
	BTree tree(file_idx);
	const auto phead = tree.GetPtrIndexHeadNode();

	// 第一个数据结点地址
	auto node_fd = phead->MostLeftNode;

	while (node_fd.offSet != 0)
	{
		//取出结点内记录
		const  BTNode *pNode = tree.FileAddrToMemPtr(node_fd);

		for (int i = 0; i < pNode->count_valid_key; i++)
		{
			RecordHead record = GetDbfRecord(table_name, pNode->children[i], path);

			// 查找比较的字段
			const Column_Cell *pColumn = record.GetFirstColumn();
			while (pColumn && pColumn->columu_name != compare_cell.cmp_value.columu_name)pColumn = pColumn->next;
			bool isSearched = compare_cell(*pColumn);
			if (isSearched)  // 满足条件
			{
				res.push_back({ pNode->key[i] ,pNode->children[i] });
			}

		}

		// 下一个B+tree结点
		node_fd = tree.FileAddrToMemPtr(node_fd)->next;
	}

	return res;
}


CatalogPosition& GetCp()
{
	static CatalogPosition cp;
	return cp;
}

bool CatalogPosition::isInSpeDb = false;

CatalogPosition::CatalogPosition()
	:root("./DB/"), current_catalog("./DB/")
{
	// 如果当前目录下没有 DB 文件见则创建
	std::string tmp_path = "./DB";

	if (_access(tmp_path.c_str(), 0) == -1)
	{
		_mkdir(tmp_path.c_str());
	}
}

bool CatalogPosition::ResetRootCatalog(std::string root_new)
{
	if (root_new[root_new.size() - 1] == '/')
	{
		root = root_new;
		current_catalog = root;
		isInSpeDb = false;
		return true;
	}
	else
	{
		return false;
	}
}

void CatalogPosition::SwitchToDatabase()
{
	current_catalog = root;
	isInSpeDb = false;
}

bool CatalogPosition::SwitchToDatabase(std::string db_name)
{
	std::string tmp_path = root + db_name;

	if (_access(tmp_path.c_str(), 0) == -1)  //判断数据库是否存在
	{
		return false;
	}
	else
	{
		current_catalog = root + db_name + "/";
		isInSpeDb = true;
		return true;
	}

}

std::string CatalogPosition::GetCurrentPath() const
{
	return current_catalog;
}

std::string CatalogPosition::GetRootPath() const
{
	return root;
}

std::string CatalogPosition::SetCurrentPath(std::string cur)
{
	current_catalog = cur;
	return current_catalog;
}

size_t TableIndexHeadInfo::GetColumnCount()const
{
	size_t n = 0;

	const IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	const char *pColumnTypeInfo = pHeadNode->RecordTypeInfo;

	while ((*pColumnTypeInfo) != '\0')
	{
		char c = *pColumnTypeInfo;
		if (c == 'I' || c == 'D' || c == 'C')  n++;
		pColumnTypeInfo++;
	}

	return n;
}

std::vector<std::string> TableIndexHeadInfo::GetColumnNames() const
{
	std::vector<std::string> column_names;
	size_t column_count = GetColumnCount();

	const IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	const char *pColumnName = pHeadNode->RecordColumnName;
	for (size_t i = 0; i < column_count; i++)
	{
		column_names.push_back(pColumnName);
		pColumnName += ColumnNameLength;
	}

	return column_names;
}

std::vector<Column_Type> TableIndexHeadInfo::GetColumnType()const
{
	std::vector<Column_Type> column_types;

	const IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	const char *pColumnTypeInfo = pHeadNode->RecordTypeInfo;

	while ((*pColumnTypeInfo) != '\0')
	{
		char c = *pColumnTypeInfo;

		switch (c)
		{
		case 'I':
			column_types.push_back(Column_Type::I);
			break;
		case 'D':
			column_types.push_back(Column_Type::D);
			break;
		case 'C':
			column_types.push_back(Column_Type::C);
			break;

		default:
			break;
		}

		pColumnTypeInfo++;
	}

	return column_types;
}

Column_Type TableIndexHeadInfo::GetColumnType(std::string column_name) const
{
	auto column_names = GetColumnNames();
	int k = 0;
	for (int i = 0; i < column_names.size(); i++)
	{
		if (column_names[i] == column_name)
		{
			k = i;
			break;
		}
	}

	auto column_types = GetColumnType();
	return column_types[k];
}

std::vector<int> TableIndexHeadInfo::GetColumnSize() const
{
	std::vector<int> column_size;

	const IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	const char *pColumnTypeInfo = pHeadNode->RecordTypeInfo;

	while ((*pColumnTypeInfo) != '\0')
	{
		char c = *pColumnTypeInfo;
		size_t sz = 0;
		switch (c)
		{
		case 'I':
			column_size.push_back(sizeof(int));
			break;
		case 'D':
			column_size.push_back(sizeof(double));
			break;
		case 'C':
			sz = (pColumnTypeInfo[1] - '0') * 100 + (pColumnTypeInfo[2] - '0') * 10 + (pColumnTypeInfo[3] - '0') * 1;
			column_size.push_back(sz);
			break;

		default:
			break;
		}

		pColumnTypeInfo++;
	}

	return column_size;
}

int TableIndexHeadInfo::GetColumnSizeByIndex(int i) const
{
	auto SZ = GetColumnSize();
	return SZ[i];
}

int TableIndexHeadInfo::GetPrimaryIndex()const
{
	const IndexHeadNode* pHeadNode = tree.GetPtrIndexHeadNode();
	return pHeadNode->KeyTypeIndex;
}

bool TableIndexHeadInfo::IsColumnName(std::string column_name) const
{
	auto table_names = GetColumnNames();
	auto pos = std::find(table_names.begin(), table_names.end(), column_name);

	return pos != table_names.end();
}

int TableIndexHeadInfo::GetIndex(std::string column_name) const
{
	auto names = GetColumnNames();
	for (int i = 0; i < names.size(); i++)
	{
		if (names[i] == column_name)
			return i;
	}
	return 0;
}

bool TableIndexHeadInfo::IsPrimary(std::string column_name) const
{
	auto column_names = GetColumnNames();
	auto index = GetPrimaryIndex();
	if (!column_names.empty())
	{
		return column_names[index] == column_name;
	}
	return false;
}

int TableIndexHeadInfo::GetColumnOffset(std::string column_name)
{
	std::vector<int> column_size = GetColumnSize();
	int index = GetIndex(column_name);

	size_t offset = 0;
	for (int i = 0; i < index; i++)
		offset += column_size[i];
	return offset;
}

//
//RecordHead TableIndexHeadInfo::CreateRecordObject(std::vector<std::pair<std::string, std::string>> ColumnName_ColumnValue)
//{
//	size_t column_count = GetColumnCount();
//	std::vector<std::string> column_names = GetColumnNames();
//
//	RecordHead rd;
//	// 依次封装每个字段
//	for (size_t i = 0; i < column_count; i++)
//	{
//		//该字段有没有提供初始值
//		int n = -1;
//		for (int j = 0; j < ColumnName_ColumnValue.size(); j++)
//		{
//			if (ColumnName_ColumnValue[j].first == column_names[i])
//			{
//				n = j;
//				break;
//			}
//		}
//
//		if (n != -1)
//		{
//
//		}
//		else
//		{
//			CompareCell cc;
//			cc.OperType
//		}
//	}
//}
