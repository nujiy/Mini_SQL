#include"Buffer.h"
#include <iostream>
#include <string>

// 定义全局内存缓冲页
Clock* GetGlobalClock()
{
	static Clock MemClock;
	return &MemClock;
}

BUFFER& GetGlobalFileBuffer()
{
	static BUFFER buffer;
	return buffer;
}


void PAGEHEAD::Initialize()
{
	pageId = 0;
	isFixed = 1;
}

void FILECOND::Initialize()
{
	total_page = 1;

	FileAddr fd_tmp;
	fd_tmp.SetFileAddr(0, 0);
	DelFirst = fd_tmp;
	DelLast = fd_tmp;
	fd_tmp.offSet = sizeof(PAGEHEAD) + sizeof(FILECOND);
	NewInsert = fd_tmp;
	memset(reserve, 0, FILECOND_RESERVE_SPACE);
}

const void* MemFile::ReadRecord(FileAddr *address_delete)const
{
	auto pMemPage = GetGlobalClock()->GetMemAddr(this->fileId, address_delete->filePageID);
	return (char*)(pMemPage->Ptr2PageBegin) + address_delete->offSet;
}

void* MemFile::ReadWriteRecord(FileAddr *address_delete)
{
	auto pMemPage = GetGlobalClock()->GetMemAddr(this->fileId, address_delete->filePageID);
	pMemPage->isModified = true;
	return (char*)(pMemPage->Ptr2PageBegin) + address_delete->offSet;
}

// 返回新添加记录的地址
FileAddr MemFile::AddRecord(const void* const source, size_t sz_record)
{
	auto pMemPage = GetGlobalClock()->GetMemAddr(this->fileId, 0);
	auto pFileCond = pMemPage->GetFileCond();
	FileAddr fd; // 写入的位置
	void *tmp_source;
	if (pFileCond->DelFirst.offSet == 0 && pFileCond->DelLast.offSet == 0)
	{
		// 没有被删除过的空余空间，直接在文件尾插入数据
		// 将添加的新地址作为记录数据的一部分写入
		tmp_source = malloc(sz_record + sizeof(FileAddr));
		memcpy(tmp_source, &pFileCond->NewInsert, sizeof(FileAddr));
		memcpy((char*)tmp_source + sizeof(FileAddr), source, sz_record);
		auto real_pos = MemWrite(tmp_source, sz_record + sizeof(FileAddr));
		MemWrite(&real_pos, sizeof(FileAddr), &real_pos);
		fd = real_pos;
	}
	else if (pFileCond->DelFirst == pFileCond->DelLast)
	{
		// 在第一个被删除的数据处，填加新数据
		tmp_source = malloc(sz_record + sizeof(FileAddr));
		memcpy(tmp_source, &pFileCond->DelFirst, sizeof(FileAddr));
		memcpy((char*)tmp_source + sizeof(FileAddr), source, sz_record);
		MemWrite(tmp_source, sz_record + sizeof(FileAddr), &pFileCond->DelFirst);
		fd = pFileCond->DelFirst;
		pFileCond->DelFirst.offSet = 0;
		pFileCond->DelLast.offSet = 0;
	}
	else
	{
		auto first_del_pos = pFileCond->DelFirst;
		fd = pFileCond->DelFirst;
		pFileCond->DelFirst = *(FileAddr*)MemRead(&pFileCond->DelFirst);

		tmp_source = malloc(sz_record + sizeof(FileAddr));
		memcpy(tmp_source, &first_del_pos, sizeof(FileAddr));
		memcpy((char*)tmp_source + sizeof(FileAddr), source, sz_record);
		MemWrite(tmp_source, sz_record + sizeof(FileAddr), &first_del_pos);
	}
	delete tmp_source;
	pMemPage->SetModified();
	return fd;
}

FileAddr MemFile::DeleteRecord(FileAddr *address_delete, size_t)// record_sz 保留参数位置
{
	auto pMemPage = GetGlobalClock()->GetMemAddr(this->fileId, 0);
	auto pFileCond = pMemPage->GetFileCond();

	// 如果待删除数据地址的地址标识和本身地址不等，则是已经删除过的数据
	FileAddr fd = *(FileAddr*)MemRead(address_delete);
	if (fd != *address_delete)
	{
		return FileAddr{ 0,0 };  // 删除失败,数据已经被删除过
	}
	else if (pFileCond->DelFirst.offSet == 0 && pFileCond->DelLast.offSet == 0)  // 之前没有删除过记录
	{
		pFileCond->DelFirst = pFileCond->DelLast = *address_delete;
		FileAddr tmp{ 0,0 };
		MemWrite(&tmp, sizeof(FileAddr), &pFileCond->DelLast);
	}
	else
	{
		// 删除记录
		MemWrite(address_delete, sizeof(FileAddr), &pFileCond->DelLast);
		pFileCond->DelLast = *address_delete;
		FileAddr tmp{ 0,0 };
		MemWrite(&tmp, sizeof(FileAddr), &pFileCond->DelLast);
	}

	pMemPage->SetModified();
	return *address_delete;
}

bool MemFile::UpdateRecord(FileAddr *address, void *record_data, size_t record_sz)
{
	auto pMemPage = GetGlobalClock()->GetMemAddr(this->fileId, address->filePageID);
	auto pdest = (char*)pMemPage->Ptr2PageBegin + address->offSet + sizeof(FileAddr);
	memcpy(pdest, record_data, record_sz);
	pMemPage->isModified = true;
	return true;
}

void* MemFile::MemRead(FileAddr *dest_to_read)
{
	auto pMemPage = GetGlobalClock()->GetMemAddr(this->fileId, dest_to_read->filePageID);
	pMemPage->bIsLastUsed = true;
	return (char*)pMemPage->Ptr2PageBegin + dest_to_read->offSet;

}

// 任意位置写入任意数据,返回写入后的下一个地址位置，写入失败返回原地址
FileAddr MemFile::MemWrite(const void* source, size_t length, FileAddr* dest)
{
	auto pMemPage = GetGlobalClock()->GetMemAddr(this->fileId, dest->filePageID);
	// 如果该页剩余空间不足
	if ((FILE_PAGESIZE - dest->offSet) < length)
	{
		return *dest;
	}
	memcpy((void*)((char*)pMemPage->Ptr2PageBegin + dest->offSet), source, length);
	pMemPage->isModified = true;
	pMemPage->bIsLastUsed = true;

	//dest->offSet += length;
	FileAddr fd;
	fd.SetFileAddr(dest->filePageID, dest->offSet + length);
	return fd;
}

// 在可写入地址写入数据，若空间不足则申请新的页, 返回数据写入的地址
FileAddr MemFile::MemWrite(const void* source, size_t length)
{
	// 获取可写入地址
	FileAddr InsertPos = GetGlobalClock()->GetMemAddr(this->fileId, 0)->GetFileCond()->NewInsert;

	// 写入
	FileAddr write_res = MemWrite(source, length, &InsertPos);
	if (write_res.filePageID == InsertPos.filePageID && write_res.offSet == InsertPos.offSet)  //空间不足，需要开辟新的页
	{
		AddExtraPage();
		InsertPos.SetFileAddr(InsertPos.filePageID + 1, sizeof(PAGEHEAD));
		write_res = MemWrite(source, length, &InsertPos);  // 重新写入
	}

	// 更新可写入位置
	GetGlobalClock()->GetMemAddr(this->fileId, 0)->GetFileCond()->NewInsert = write_res;
	GetGlobalClock()->GetMemAddr(this->fileId, 0)->SetModified();
	GetGlobalClock()->GetMemAddr(this->fileId, 0)->bIsLastUsed = true;
	return InsertPos;
}


void MemFile::MemWipe(void*source, size_t sz_wipe, FileAddr *fd_to_wipe)
{
	auto pMemPage = GetGlobalClock()->GetMemAddr(this->fileId, fd_to_wipe->filePageID);
	// wipe
	memcpy((char*)pMemPage->Ptr2PageBegin + fd_to_wipe->offSet, source, sz_wipe);
	pMemPage->isModified = true;
	pMemPage->bIsLastUsed = true;
}

MemFile::MemFile(const char *file_name, unsigned long file_id)
{
	strcpy(this->fileName, file_name);
	this->fileId = file_id;
	this->total_page = GetGlobalClock()->GetMemAddr(this->fileId, 0)->GetFileCond()->total_page;
}

MemPage * MemFile::AddExtraPage()
{
	Clock *pMemClock = GetGlobalClock();
	//获取文件首页
	MemPage *FileFirstPage = pMemClock->GetMemAddr(this->fileId, 0);
	this->total_page = FileFirstPage->GetFileCond()->total_page + 1;
	FileFirstPage->GetFileCond()->total_page += 1;
	FileFirstPage->SetModified();
	FileFirstPage->bIsLastUsed = true;
	//创建新内存页
	MemPage * newMemPage = pMemClock->CreatNewPage(this->fileId, FileFirstPage->GetFileCond()->total_page - 1);
	newMemPage->isModified = true;
	newMemPage->bIsLastUsed = true;

	return newMemPage;
}

MemPage* MemFile::GetFileFirstPage()
{
	return GetGlobalClock()->GetMemAddr(this->fileId, 0);  // 文件首页
}

MemPage::MemPage()
{
	Ptr2PageBegin = malloc(FILE_PAGESIZE);
	pageHead = (PAGEHEAD*)Ptr2PageBegin;
	fileId = 0;
	isModified = false;
	bIsLastUsed = false;
}

MemPage::~MemPage()
{
	// 脏页且不是抛弃的页需要写回
	if (this->isModified && this->fileId > 0)
		this->Back2File();
	delete Ptr2PageBegin;
}

void MemPage::Back2File() const
{
	// 脏页需要写回
	if (this->isModified && this->fileId > 0)
	{
		int temp = 0;
		temp = lseek(this->fileId, this->filePageID*FILE_PAGESIZE, SEEK_SET);
		if (temp == -1)throw SQLError::LSEEK_ERROR();

		temp = write(this->fileId, this->Ptr2PageBegin, FILE_PAGESIZE); // 写回文件
		if (temp != FILE_PAGESIZE) throw SQLError::WRITE_ERROR();  // 写失败
		isModified = false;
		bIsLastUsed = true;
	}
}

bool MemPage::SetModified()
{
	isModified = true;
	return true;
}

FILECOND* MemPage::GetFileCond()const
{
	return (FILECOND*)((char*)Ptr2PageBegin + sizeof(PAGEHEAD));
}

Clock::Clock()
{
	for (int i = 0; i <= MEM_PAGEAMOUNT; i++)
	{
		MemPages[i] = nullptr;
	}
}

Clock::~Clock()
{
	for (int i = 0; i <= MEM_PAGEAMOUNT; i++)
	{
		if (MemPages[i] != nullptr)
			delete MemPages[i];
	}
}

MemPage* Clock::GetMemAddr(unsigned long fileId, unsigned long filePageID)
{
	// 先查找是否存在内存中
	MemPage* pMemPage = GetExistedPage(fileId, filePageID);
	if (pMemPage != nullptr)
		return pMemPage;

	// 否则，从磁盘换入
	return LoadFromFile(fileId, filePageID);
}

MemPage* Clock::CreatNewPage(unsigned long file_id, unsigned long file_page_id)
{
	// 初始化内存页对象
	auto i = GetReplaceablePage();
	memset(MemPages[i]->Ptr2PageBegin, 0, FILE_PAGESIZE);
	MemPages[i]->fileId = file_id;
	MemPages[i]->filePageID = file_page_id;
	MemPages[i]->isModified = true;  // 新页设置为脏页，需要写回

	// 初始化新页的页头信息
	MemPages[i]->pageHead->pageId = file_page_id;
	if (file_page_id != 0)
	{
		MemPages[i]->pageHead->isFixed = 0;
	}
	else
	{
		MemPages[i]->pageHead->isFixed = 1;
		MemPages[i]->GetFileCond()->Initialize();
	}
	return MemPages[i];

}

MemPage* Clock::GetExistedPage(unsigned long fileId, unsigned long filePageID)
{
	// look up for the page in memPage list
	for (int i = 1; i <= MEM_PAGEAMOUNT; i++)
	{
		if (MemPages[i] && MemPages[i]->fileId == fileId && MemPages[i]->filePageID == filePageID)
			return MemPages[i];
	}
	return nullptr;
}



// 从磁盘加载文件页
MemPage* Clock::LoadFromFile(unsigned long fileId, unsigned long filePageID)
{
	unsigned int freePage = GetReplaceablePage();
	MemPages[freePage]->fileId = fileId;
	MemPages[freePage]->filePageID = filePageID;
	MemPages[freePage]->isModified = false;
	MemPages[freePage]->bIsLastUsed = true;

	try {
		assert(fileId > 0);
		assert(filePageID >= 0);
		long offset_t = lseek(fileId, filePageID*FILE_PAGESIZE, SEEK_SET);       // 定位到将要取出的文件页的首地址
		if (offset_t == -1)throw SQLError::LSEEK_ERROR();
		long byte_count = read(fileId, MemPages[freePage]->Ptr2PageBegin, FILE_PAGESIZE);          // 读到内存中
		if (byte_count == 0)throw SQLError::READ_ERROR();
	}
	catch (const SQLError::BaseError &e)
	{
		DispatchError(e);
	}

	return MemPages[freePage];
}

unsigned long Clock::ClockSwap()
{
	static unsigned long index = 1;
	assert(MemPages[index] != nullptr);

	while (MemPages[index]->bIsLastUsed)     // 最近被使用过
	{
		MemPages[index]->bIsLastUsed = 0;
		index = (index + 1) % MEM_PAGEAMOUNT;
		if (index == 0)index++;
	}

	auto res = index;
	MemPages[index]->bIsLastUsed = 1;
	index = (index + 1) % MEM_PAGEAMOUNT;
	if (index == 0)index++;
	return res;

}

unsigned int Clock::GetReplaceablePage()
{
	// 查找没有分配的内存页
	for (int i = 1; i <= MEM_PAGEAMOUNT; i++)
	{
		if (MemPages[i] == nullptr)
		{
			MemPages[i] = new MemPage();
			return i;
		}
	}

	// 查找被抛弃的页
	for (int i = 1; i <= MEM_PAGEAMOUNT; i++)
	{
		if (MemPages[i]->fileId == 0)
			return i;
	}

	// clock算法
	unsigned int i = ClockSwap();
	//unsigned int i = rand() % MEM_PAGEAMOUNT;
	if (i == 0)i++;
	MemPages[i]->Back2File();
	return i;
}

BUFFER::~BUFFER()
{
	//CloseAllFile();
	for (auto e : memFiles)
		delete e;
}

MemFile* BUFFER::GetMemFile(const char *fileName)
{
	// 如果文件已经打开
	for (size_t i = 0; i < memFiles.size(); i++)
	{
		if ((strcmp(memFiles[i]->fileName, fileName) == 0))
			return memFiles[i];
	}

	// 文件存在但是没打开
	int Ptr2File = open(fileName, _O_BINARY | O_RDWR, 0664);
	if (Ptr2File != -1)
	{

		MemFile* newFile = new MemFile(fileName, Ptr2File);
		memFiles.push_back(newFile);
		return newFile;
	}


	// 文件不存在
	return nullptr;

}


void BUFFER::CreateFile(const char *fileName)
{
	// 文件存在 创建失败
	int Ptr2File = open(fileName, _O_BINARY | O_RDWR, 0664);
	if (Ptr2File != -1)
	{
		close(Ptr2File);
		return;
	}

	//创建文件
	int newFile = open(fileName, _O_BINARY | O_RDWR | O_CREAT, 0664); // 新建文件(打开文件)

	void *ptr = malloc(FILE_PAGESIZE);
	memset(ptr, 0, FILE_PAGESIZE);
	PAGEHEAD *pPageHead = (PAGEHEAD *)ptr;
	FILECOND *pFileCond = (FILECOND *)((char*)ptr + sizeof(PAGEHEAD));
	pPageHead->pageId = 0;
	pPageHead->isFixed = 1;
	pFileCond->Initialize();
	// 写回
	write(newFile, ptr, FILE_PAGESIZE);
	close(newFile);
	delete ptr;
	return;
}

void BUFFER::CloseAllFile()
{
	while (!memFiles.empty())
	{
		CloseFile((*memFiles.begin())->fileName);
		//memFiles.erase(memFiles.begin());
	}
}

void BUFFER::CloseFile(const char *FileName)
{
	auto pMemPage = GetMemFile(FileName);
	// 内存中所有保存该文件的页全部写回
	auto pClock = GetGlobalClock();
	for (int i = 1; i <= MEM_PAGEAMOUNT; i++)
	{
		if (pClock->MemPages[i] && pClock->MemPages[i]->fileId == pMemPage->fileId)
		{
			assert(pClock->MemPages[i]);
			pClock->MemPages[i]->Back2File();
			pClock->MemPages[i]->bIsLastUsed = 0;
			pClock->MemPages[i]->isModified = false;
			pClock->MemPages[i]->fileId = 0;
		}
	}

	for (auto it = memFiles.begin(); it != memFiles.end();)
	{
		if (strcmp((*it)->fileName, FileName) == 0)
		{
			close((*it)->fileId);
			delete (*it);
			memFiles.erase(it);
			break;
		}
		it++;
	}
}

MemFile* BUFFER::operator[](const char *fileName)
{
	return GetMemFile(fileName);
}

void FileAddr::SetFileAddr(const unsigned long _filePageID /*= 0*/, const unsigned int _offSet /*= 0*/)
{
	filePageID = _filePageID;
	offSet = _offSet;
}
void FileAddr::ShiftOffset(const int OFFSET)
{
	this->offSet += OFFSET;
}