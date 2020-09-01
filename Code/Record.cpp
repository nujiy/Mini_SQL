#include "Record.h"




RecordHead::RecordHead()
	:phead(nullptr), pLast(nullptr)
{

}

RecordHead::RecordHead(const RecordHead &rhs)
{
	//std::cout << "RecordHead 拷贝构造" << std::endl;
	auto &tmp = const_cast<RecordHead&>(rhs);

	phead = tmp.phead;
	tmp.phead = nullptr;
	pLast = tmp.pLast;
	tmp.pLast = nullptr;


}

RecordHead::RecordHead(RecordHead &&rhs)
{
	while (phead)
	{
		auto next = phead->next;
		delete phead;
		phead = next;
	}

	phead = rhs.phead;
	pLast = rhs.pLast;
	rhs.phead = nullptr;
	rhs.pLast = nullptr;
}

RecordHead& RecordHead::operator=(RecordHead&&rhs)
{
	phead = rhs.phead;
	pLast = rhs.pLast;
	rhs.phead = nullptr;
	rhs.pLast = nullptr;

	return *this;
}

RecordHead& RecordHead::operator=(const RecordHead&rhs)
{
	//std::cout << "RecordHead 拷贝赋值" << std::endl;
	auto &tmp = const_cast<RecordHead&>(rhs);

	phead = tmp.phead;
	tmp.phead = nullptr;
	pLast = tmp.pLast;
	tmp.pLast = nullptr;


	return *this;
}

size_t RecordHead::size() const
{
	unsigned long sz = 0;
	auto p = phead;
	while (p)
	{
		sz += p->size();
		p = p->next;
	}
	return sz;
}

Column_Cell* RecordHead::GetFirstColumn()const
{
	return phead;
}

RecordHead::~RecordHead()
{
	while (phead)
	{
		auto next = phead->next;
		delete phead;
		phead = next;
	}

	//if (data) delete data;
}

void RecordHead::AddColumnCell(const Column_Cell &cc)
{
	if (!phead)
	{
		phead = new Column_Cell;
		*phead = cc;
		phead->next = nullptr;
		pLast = phead;
	}
	else
	{
		pLast->next = new Column_Cell;
		*(pLast->next) = cc;
		pLast = pLast->next;
		pLast->next = nullptr;
	}
}

Column_Cell::Column_Cell(const Column_Cell& rhs)
{
	//std::cout << "Column_Cell 拷贝构造" << std::endl;
	column_type = rhs.column_type;
	columu_name = rhs.columu_name;
	sz = rhs.sz;
	next = nullptr;

	// 如果是字符串
	if (rhs.column_type == Column_Type::C)
	{
		column_value.StrValue = (char*)malloc(strlen(rhs.column_value.StrValue) + 1);
		strcpy(column_value.StrValue, rhs.column_value.StrValue);
	}
	else if (rhs.column_type == Column_Type::I)
	{
		column_value.IntValue = rhs.column_value.IntValue;
	}
	else if (rhs.column_type == Column_Type::D)
	{
		column_value.DoubleValue = rhs.column_value.DoubleValue;
	}
}




Column_Cell::Column_Cell(KeyAttr key)
{
	int len = 0;
	column_type = key.type;
	switch (key.type)
	{
	case Column_Type::I:
		column_value.IntValue = key.value.IntValue;
		break;

	case Column_Type::C:
		len = strlen(key.value.StrValue) + 1;
		column_value.StrValue = (char*)malloc(len);
		strcpy(column_value.StrValue, key.value.StrValue);
		break;

	case Column_Type::D:
		column_value.DoubleValue = key.value.DoubleValue;
		break;
	default:
		break;
	}
}

Column_Cell::Column_Cell()
{
	memset(&column_value, 0, sizeof(column_value));
	column_type = Column_Type::I;
	next = nullptr;
}

size_t Column_Cell::size() const
{
	switch (column_type)
	{
	case Column_Type::I:
		return sizeof(int);
		break;

	case Column_Type::C:
		return sz;
		break;

	case Column_Type::D:
		return sizeof(double);
		break;
	default:
		break;
	}
	return 0;
}

void* Column_Cell::data() const
{
	switch (column_type)
	{
	case Column_Type::I:
		return (void*)&column_value.IntValue;
		break;
	case Column_Type::C:
		return column_value.StrValue;
		break;
	case Column_Type::D:
		return (void*)&column_value.DoubleValue;
		break;
	default:
		break;
	}
	return 0;
}

Column_Cell::~Column_Cell()
{
	if (column_type == Column_Type::C)
	{
		if (column_value.StrValue)
		{
			free(column_value.StrValue);
			column_value.StrValue = nullptr;
		}
	}
}

Column_Cell::operator KeyAttr() const
{
	KeyAttr key_attr;
	memset(&key_attr, 0, sizeof(KeyAttr));
	switch (column_type)
	{
	case Column_Type::I:
		key_attr.type = column_type;
		key_attr.value.IntValue = column_value.IntValue;
		break;

	case Column_Type::C:
		key_attr.type = column_type;
		if (strlen(column_value.StrValue) > ColumnNameLength)
		{
			throw SQL_Error::KeyAttr_NameLength_ERROR();
		}
		else
		{
			strcpy(key_attr.value.StrValue, column_value.StrValue);
		}
		break;

	case Column_Type::D:
		key_attr.type = column_type;
		key_attr.value.DoubleValue = column_value.DoubleValue;
		break;
	default:
		break;
	}

	return key_attr;
}

Column_Cell& Column_Cell::operator=(const Column_Cell&rhs)
{
	//std::cout << "Column_Cell 拷贝构造" << std::endl;
	column_type = rhs.column_type;
	columu_name = rhs.columu_name;
	sz = rhs.sz;
	next = nullptr;

	// 如果是字符串
	if (rhs.column_type == Column_Type::C)
	{
		if (column_value.StrValue)
			free(column_value.StrValue);

		column_value.StrValue = (char*)malloc(strlen(rhs.column_value.StrValue) + 1);
		strcpy(column_value.StrValue, rhs.column_value.StrValue);
	}
	else if (rhs.column_type == Column_Type::I)
	{
		column_value.IntValue = rhs.column_value.IntValue;
	}
	else if (rhs.column_type == Column_Type::D)
	{
		column_value.DoubleValue = rhs.column_value.DoubleValue;
	}

	return *this;
}

FileAddr Record::InsertRecord(const std::string dbf_name, const RecordHead &rd)
{
	// 记录数据的副本
	auto tp = GetRecordData(rd);

	// 插入记录
	auto fd = GetGlobalFileBuffer()[dbf_name.c_str()]->AddRecord(std::get<1>(tp), std::get<0>(tp));
	delete std::get<1>(tp);  // 释放副本内存
	return fd;
}

FileAddr Record::DeleteRecord(const std::string dbf_name, FileAddr fd, size_t record_size)
{
	return GetGlobalFileBuffer()[dbf_name.c_str()]->DeleteRecord(&fd, record_size);
}

bool Record::UpdateRecord(const std::string dbf_name, const RecordHead &rd, FileAddr fd)
{
	// 记录数据的副本
	auto tp = GetRecordData(rd);
	auto bUpdate = GetGlobalFileBuffer()[dbf_name.c_str()]->UpdateRecord(&fd, std::get<1>(tp), std::get<0>(tp));
	free(std::get<1>(tp));  // 释放副本内存
	return bUpdate;
}

std::tuple<unsigned long, char*> Record::GetRecordData(const RecordHead &rd)
{
	// 记录数据的副本
	unsigned long data_size = rd.size();
	char *rd_data = (char*)malloc(data_size);
	memset(rd_data, 0, data_size);
	auto pcolumn = rd.GetFirstColumn();

	unsigned long offset = 0;
	while (pcolumn)
	{
		memcpy(rd_data + offset, pcolumn->data(), pcolumn->size());
		offset += pcolumn->size();
		pcolumn = pcolumn->next;
	}
	assert(data_size == offset);

	auto tp = std::make_tuple(data_size, rd_data);
	return tp;
}

Column_Type StrConvertToEnumType(std::string str_type)
{
	for (auto &c : str_type)
		tolower(c);

	if (str_type == "int")
	{
		return Column_Type::I;
	}
	if (str_type == "char")
	{
		return Column_Type::C;
	}
	if (str_type == "double")
	{
		return Column_Type::D;
	}

	return Column_Type::I;
}



bool KeyAttr::operator<(const KeyAttr &rhs)const
{
	if (this->type != rhs.type)
		return false;

	bool res = true;
	std::string s1;
	std::string s2;

	switch (this->type)
	{
	case Column_Type::I:
		res = this->value.IntValue < rhs.value.IntValue;
		break;

	case Column_Type::C:
		s1 = std::string(this->value.StrValue);
		s2 = std::string(rhs.value.StrValue);
		res = s1 < s2;
		break;

	case Column_Type::D:
		res = this->value.DoubleValue < rhs.value.DoubleValue;
		break;
	default:
		break;
	}
	return res;
}

bool KeyAttr::operator>(const KeyAttr &rhs)const
{
	if (this->type != rhs.type)
		return false;

	bool res = true;
	std::string s1;
	std::string s2;

	switch (this->type)
	{
	case Column_Type::I:
		res = this->value.IntValue > rhs.value.IntValue;
		break;

	case Column_Type::C:
		s1 = std::string(this->value.StrValue);
		s2 = std::string(rhs.value.StrValue);
		res = s1 > s2;
		break;

	case Column_Type::D:
		res = this->value.DoubleValue > rhs.value.DoubleValue;
		break;
	default:
		break;
	}
	return res;
}

bool KeyAttr::operator>=(const KeyAttr &rhs)const
{
	if (this->type != rhs.type)
		return false;

	return !(*this < rhs);
}

bool KeyAttr::operator!=(const KeyAttr &rhs)const
{
	if (this->type != rhs.type)
		return false;

	return !(*this == rhs);
}

bool KeyAttr::operator<=(const KeyAttr &rhs)const
{
	if (this->type != rhs.type)
		return false;

	return !(*this > rhs);
}

bool KeyAttr::operator==(const KeyAttr &rhs)const
{
	if (this->type != rhs.type)
		return false;

	bool res = true;
	std::string s1;
	std::string s2;

	switch (this->type)
	{
	case Column_Type::I:
		res = (this->value.IntValue == rhs.value.IntValue);
		break;

	case Column_Type::C:
		s1 = std::string(this->value.StrValue);
		s2 = std::string(rhs.value.StrValue);
		res = (s1 == s2);
		break;

	case Column_Type::D:
		res = (this->value.DoubleValue == rhs.value.DoubleValue);
		break;
	default:
		break;
	}
	return res;
}

std::ostream& operator<<(std::ostream &os, const KeyAttr &key)
{
	switch (key.type)
	{
	case Column_Type::I:
		os << key.value.IntValue << " ";
		break;

	case Column_Type::C:
		os << key.value.StrValue << " ";
		break;

	case Column_Type::D:
		os << key.value.DoubleValue << " ";
		break;
	default:
		break;
	}

	return os;
}

std::ostream& operator<<(std::ostream &os, const RecordHead &rd)
{
	auto pColumn = rd.GetFirstColumn();
	while (pColumn)
	{
		switch (pColumn->column_type)
		{
		case Column_Type::I:
			os << pColumn->column_value.IntValue << " ";
			break;

		case Column_Type::D:
			os << pColumn->column_value.DoubleValue << " ";
			break;

		case Column_Type::C:
			os << pColumn->column_value.StrValue << " ";
			break;

		default:
			break;
		}
		pColumn = pColumn->next;
	}

	return os;
}

