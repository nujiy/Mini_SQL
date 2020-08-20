#pragma once
#include <vector>
#include <cassert>
#include "Global.h"
#include "Error.h"
extern "C" {
#include <io.h>
#include <fcntl.h>
}

#define FileAddrSize (sizeof(FileAddr))

class Clock;
class BUFFER;
Clock* GetGlobalClock();
BUFFER& GetGlobalFileBuffer();

const unsigned int FILECOND_RESERVE_SPACE = 512;  // 文件头预留空间

/*********************************************************
*             页头信息，用以标识文件页
**********************************************************/
class PAGEHEAD
{
public:
	void Initialize();          // 初始化为文件第一页
	unsigned long pageId;		// 页编号
	bool isFixed;				// 页是否常驻内存
};

/*********************************************************
*             文件地址,定位文件中的位置
**********************************************************/
class FileAddr
{
	friend class FILECOND;
public:
	void SetFileAddr(const unsigned long _filePageID, const unsigned int  _offSet);
	void ShiftOffset(const int OFFSET);

	unsigned long filePageID;     // 文件页编号
	unsigned int  offSet;         // 页内偏移量

	bool operator==(const FileAddr &rhs) const
	{
		return (this->filePageID == rhs.filePageID && this->offSet == rhs.offSet);
	}
	bool operator!=(const FileAddr &rhs) const
	{
		return !(this->filePageID == rhs.filePageID && this->offSet == rhs.offSet);
	}
	bool operator<(const FileAddr &rhs)const
	{
		return (this->filePageID < rhs.filePageID) || ((this->filePageID == rhs.filePageID) && (this->offSet < rhs.offSet));
	}
};

/*********************************************************
*               文件头信息
**********************************************************/
class FILECOND
{
public:
	void Initialize();
	FileAddr DelFirst;                         // 第一条被删除记录地址
	FileAddr DelLast;                          // 最后一条被删除记录地址  
	FileAddr NewInsert;                        // 文件末尾可插入新数据的地址
	unsigned long total_page;                  // 目前文件中共有页数
	char reserve[FILECOND_RESERVE_SPACE];      // 预留空间 
};

/*********************************************************
*
*   名称：  内存页类
*   功能：  提供保存文件页的空间，以及该页相关的信息
*   不变式：内存页的大小固定
*
**********************************************************/
class MemPage
{
	friend class MemFile;
	friend class Clock;
	friend class BUFFER;
public:
	MemPage();
	~MemPage();
private:
	void Back2File() const;            // 把内存中的页写回到文件中
	bool SetModified();                // 设置为脏页

public:
	unsigned long fileId;              // 文件指针，while fileId==0 时为被抛弃的页
	unsigned long filePageID;          // 文件页号

	mutable bool bIsLastUsed;          // 最近一次访问内存是否被使用，用于Clock算法
	mutable bool isModified;           // 是否脏页

	void *Ptr2PageBegin;               // 实际保存物理文件数据的地址
	PAGEHEAD *pageHead;                // 页头指针
	FILECOND* GetFileCond()const;      // 文件头指针（while filePageID == 0）
};

/*********************************************************
*
*   名称：  内存页管理类（Clock页面置换算法）
*   功能：  物理页面在内存中的缓存，加速对物理文件的读写
*   不变式：调用者保证需要被载入的物理文件都存在，且加载的页面不越界
*
**********************************************************/
struct TB_Insert_Info;
class Clock
{
	friend class MemFile;
	friend class BUFFER;
	friend class BTree;
	friend bool InsertRecord(TB_Insert_Info tb_insert_info, std::string path /*= std::string("./")*/);
	friend bool DropTable(std::string table_name, std::string path);
public:
	Clock();
	~Clock();
private:
	// 返回磁盘文件内存地址
	MemPage* GetMemAddr(unsigned long fileId, unsigned long filePageID);

	// 创建新页，适用于创建新文件或者添加新页的情况下
	MemPage* CreatNewPage(unsigned long fileId, unsigned long filePageID);

private:
	// 返回一个可替换的内存页索引
	// 原页面内容该写回先写回
	unsigned int GetReplaceablePage();

	// 如果目标文件页存在内存缓存则返回其地址，否则返回 nullptr
	MemPage* GetExistedPage(unsigned long fileId, unsigned long filePageID);
	MemPage* LoadFromFile(unsigned long fileId, unsigned long filePageID);

	// Clock置换算法
	unsigned long ClockSwap();


private:
	MemPage* MemPages[MEM_PAGEAMOUNT + 1];  // 内存页对象数组
};

/*********************************************************
*   名称：  内存文件类
*   功能：  通过物理文件在内存中的映射文件的操作，从而操作物理文件
*   不变式：假设所有被操作的文件都存在且已经打开
*   记录格式: 记录地址+记录数据
**********************************************************/
class MemFile
{
	friend class BUFFER;
	friend class BTree;
	friend bool DropTable(std::string table_name, std::string path);
public:
	const void* ReadRecord(FileAddr *address_delete)const;         // 读取某条记录,返回记录指针(包括记录地址数据)
	void* ReadWriteRecord(FileAddr *address_delete);         // 读取某条记录,返回记录指针(包括记录地址数据)
	FileAddr AddRecord(const void* const source_record, size_t sz_record);                        // 返回记录所添加的位置
	FileAddr DeleteRecord(FileAddr *address_delete, size_t);               // 返回删除的位置
	bool UpdateRecord(FileAddr *address_delete, void *record_data, size_t record_sz);

private:
	// 构造
	MemFile(const char *file_name, unsigned long file_id);
	// 写入数据
	void* MemRead(FileAddr *mem_to_read);                           // 读取内存文件,返回读取位置指针
	FileAddr MemWrite(const void* source, size_t length);           // 在可写入地址写入数据
	FileAddr MemWrite(const void* source, size_t length, FileAddr* dest);

	void MemWipe(void*source, size_t sz_wipe, FileAddr *fd_to_wipe);

	MemPage * AddExtraPage();                                       // 当前文件添加一页空间
	MemPage* GetFileFirstPage();                                    // 得到文件首页

private:
	char fileName[MAX_FILENAME_LEN];
	unsigned long fileId;                                          // 文件指针
	unsigned long total_page;                                      // 目前文件中共有页数
};


class BUFFER
{
	friend bool DropTable(std::string table_name, std::string path);
public:
	BUFFER() = default;
	~BUFFER();
	MemFile* operator[](const char *fileName);      // 打开文件，打开失败返回 nullptr

	void CreateFile(const char *fileName);          // 创建文件，并格式化
	void CloseFile(const char *FileName);
	void CloseAllFile();
private:
	// 返回文件所映射的内存文件
	MemFile* GetMemFile(const char *fileName);
private:
	std::vector<MemFile*> memFiles;  // 保存已经打开的文件列表
};

