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

const unsigned int FILECOND_RESERVE_SPACE = 512;  // �ļ�ͷԤ���ռ�

/*********************************************************
*             ҳͷ��Ϣ�����Ա�ʶ�ļ�ҳ
**********************************************************/
class PAGEHEAD
{
public:
	void Initialize();          // ��ʼ��Ϊ�ļ���һҳ
	unsigned long pageId;		// ҳ���
	bool isFixed;				// ҳ�Ƿ�פ�ڴ�
};

/*********************************************************
*             �ļ���ַ,��λ�ļ��е�λ��
**********************************************************/
class FileAddr
{
	friend class FILECOND;
public:
	void SetFileAddr(const unsigned long _filePageID, const unsigned int  _offSet);
	void ShiftOffset(const int OFFSET);

	unsigned long filePageID;     // �ļ�ҳ���
	unsigned int  offSet;         // ҳ��ƫ����

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
*               �ļ�ͷ��Ϣ
**********************************************************/
class FILECOND
{
public:
	void Initialize();
	FileAddr DelFirst;                         // ��һ����ɾ����¼��ַ
	FileAddr DelLast;                          // ���һ����ɾ����¼��ַ  
	FileAddr NewInsert;                        // �ļ�ĩβ�ɲ��������ݵĵ�ַ
	unsigned long total_page;                  // Ŀǰ�ļ��й���ҳ��
	char reserve[FILECOND_RESERVE_SPACE];      // Ԥ���ռ� 
};

/*********************************************************
*
*   ���ƣ�  �ڴ�ҳ��
*   ���ܣ�  �ṩ�����ļ�ҳ�Ŀռ䣬�Լ���ҳ��ص���Ϣ
*   ����ʽ���ڴ�ҳ�Ĵ�С�̶�
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
	void Back2File() const;            // ���ڴ��е�ҳд�ص��ļ���
	bool SetModified();                // ����Ϊ��ҳ

public:
	unsigned long fileId;              // �ļ�ָ�룬while fileId==0 ʱΪ��������ҳ
	unsigned long filePageID;          // �ļ�ҳ��

	mutable bool bIsLastUsed;          // ���һ�η����ڴ��Ƿ�ʹ�ã�����Clock�㷨
	mutable bool isModified;           // �Ƿ���ҳ

	void *Ptr2PageBegin;               // ʵ�ʱ��������ļ����ݵĵ�ַ
	PAGEHEAD *pageHead;                // ҳͷָ��
	FILECOND* GetFileCond()const;      // �ļ�ͷָ�루while filePageID == 0��
};

/*********************************************************
*
*   ���ƣ�  �ڴ�ҳ�����ࣨClockҳ���û��㷨��
*   ���ܣ�  ����ҳ�����ڴ��еĻ��棬���ٶ������ļ��Ķ�д
*   ����ʽ�������߱�֤��Ҫ������������ļ������ڣ��Ҽ��ص�ҳ�治Խ��
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
	// ���ش����ļ��ڴ��ַ
	MemPage* GetMemAddr(unsigned long fileId, unsigned long filePageID);

	// ������ҳ�������ڴ������ļ����������ҳ�������
	MemPage* CreatNewPage(unsigned long fileId, unsigned long filePageID);

private:
	// ����һ�����滻���ڴ�ҳ����
	// ԭҳ�����ݸ�д����д��
	unsigned int GetReplaceablePage();

	// ���Ŀ���ļ�ҳ�����ڴ滺���򷵻����ַ�����򷵻� nullptr
	MemPage* GetExistedPage(unsigned long fileId, unsigned long filePageID);
	MemPage* LoadFromFile(unsigned long fileId, unsigned long filePageID);

	// Clock�û��㷨
	unsigned long ClockSwap();


private:
	MemPage* MemPages[MEM_PAGEAMOUNT + 1];  // �ڴ�ҳ��������
};

/*********************************************************
*   ���ƣ�  �ڴ��ļ���
*   ���ܣ�  ͨ�������ļ����ڴ��е�ӳ���ļ��Ĳ������Ӷ����������ļ�
*   ����ʽ���������б��������ļ����������Ѿ���
*   ��¼��ʽ: ��¼��ַ+��¼����
**********************************************************/
class MemFile
{
	friend class BUFFER;
	friend class BTree;
	friend bool DropTable(std::string table_name, std::string path);
public:
	const void* ReadRecord(FileAddr *address_delete)const;         // ��ȡĳ����¼,���ؼ�¼ָ��(������¼��ַ����)
	void* ReadWriteRecord(FileAddr *address_delete);         // ��ȡĳ����¼,���ؼ�¼ָ��(������¼��ַ����)
	FileAddr AddRecord(const void* const source_record, size_t sz_record);                        // ���ؼ�¼����ӵ�λ��
	FileAddr DeleteRecord(FileAddr *address_delete, size_t);               // ����ɾ����λ��
	bool UpdateRecord(FileAddr *address_delete, void *record_data, size_t record_sz);

private:
	// ����
	MemFile(const char *file_name, unsigned long file_id);
	// д������
	void* MemRead(FileAddr *mem_to_read);                           // ��ȡ�ڴ��ļ�,���ض�ȡλ��ָ��
	FileAddr MemWrite(const void* source, size_t length);           // �ڿ�д���ַд������
	FileAddr MemWrite(const void* source, size_t length, FileAddr* dest);

	void MemWipe(void*source, size_t sz_wipe, FileAddr *fd_to_wipe);

	MemPage * AddExtraPage();                                       // ��ǰ�ļ����һҳ�ռ�
	MemPage* GetFileFirstPage();                                    // �õ��ļ���ҳ

private:
	char fileName[MAX_FILENAME_LEN];
	unsigned long fileId;                                          // �ļ�ָ��
	unsigned long total_page;                                      // Ŀǰ�ļ��й���ҳ��
};


class BUFFER
{
	friend bool DropTable(std::string table_name, std::string path);
public:
	BUFFER() = default;
	~BUFFER();
	MemFile* operator[](const char *fileName);      // ���ļ�����ʧ�ܷ��� nullptr

	void CreateFile(const char *fileName);          // �����ļ�������ʽ��
	void CloseFile(const char *FileName);
	void CloseAllFile();
private:
	// �����ļ���ӳ����ڴ��ļ�
	MemFile* GetMemFile(const char *fileName);
private:
	std::vector<MemFile*> memFiles;  // �����Ѿ��򿪵��ļ��б�
};

