#ifndef __BPTREE_H__
#define __BPTREE_H__
#include "Buffer.h"
#include "Record.h"
#include <queue>


// �����ļ�ͷ��Ϣ���,�������ļ�ͷԤ���ռ�
class IndexHeadNode
{
public:
	FileAddr    root;                                       // the address of the root
	FileAddr    MostLeftNode;                               // the address of the most left node
	int         KeyTypeIndex;                               // �ؼ����ֶε�λ��
	char        RecordTypeInfo[RecordColumnCount];          // ��¼�ֶ�������Ϣ��
	char        RecordColumnName[RecordColumnCount / 4 * ColumnNameLength];
};

// define B+tree Node
enum class NodeType { ROOT, INNER, LEAF };
class BTNode
{
public:
	NodeType node_type;                              // node type
	int count_valid_key;                             // the number of key has stored in the node

	KeyAttr key[MaxKeyCount];                        // array of keys
	FileAddr children[MaxChildCount];                // if the node is not a leaf node, children store the children pointer
													 // otherwise it store record address;

	FileAddr next;                                   // if leaf node
	void PrintSelf();
};

class BTree
{
	friend std::vector<RecordHead> ShowTable(std::string table_name, std::string path);
	friend RecordHead GetDbfRecord(std::string table_name, FileAddr fd, std::string path);
public:
	// �����������ļ����ƣ� �ؼ������ͣ� ��¼����������Ϣ���飬 ��¼�����ֶ�������Ϣ����
	BTree(std::string idx_name);
	BTree(const std::string idx_name, int KeyTypeIndex, char(&_RecordTypeInfo)[RecordColumnCount], char(&_RecordColumnName)[RecordColumnCount / 4 * ColumnNameLength]);          // ���������ļ���B+��
	~BTree() { }
	FileAddr Search(KeyAttr search_key);                                        // ���ҹؼ����Ƿ��Ѿ�����
	bool Insert(KeyAttr k, FileAddr k_fd);                                      // ����ؼ���k
	FileAddr UpdateKey(KeyAttr k, KeyAttr k_new);                               // ���عؼ��ֶ�Ӧ�ļ�¼��ַ
	FileAddr Delete(KeyAttr k);                                                 // ���ظùؼ��ּ�¼�������ļ��еĵ�ַ
	void PrintBTreeStruct();                                                    // �����ӡ���н����Ϣ
	void PrintAllLeafNode();
	IndexHeadNode *GetPtrIndexHeadNode();
	BTNode *FileAddrToMemPtr(FileAddr node_fd);                                 // �ļ���ַת��Ϊ�ڴ�ָ��

private:
	FileAddr DeleteKeyAtInnerNode(FileAddr x, int i, KeyAttr key);              // x���±�Ϊi�Ľ��ΪҶ�ӽ��
	FileAddr DeleteKeyAtLeafNode(FileAddr x, int i, KeyAttr key);               // x���±�Ϊi�Ľ��ΪҶ�ӽ��
	void InsertNotFull(FileAddr x, KeyAttr k, FileAddr k_fd);
	void SplitChild(FileAddr x, int i, FileAddr y);                             // ����x�ĺ��ӽ��x.children[i] , y
	FileAddr Search(KeyAttr search_key, FileAddr node_fd);                      // �жϹؼ����Ƿ����
	FileAddr SearchInnerNode(KeyAttr search_key, FileAddr node_fd);             // ���ڲ��ڵ����
	FileAddr SearchLeafNode(KeyAttr search_key, FileAddr node_fd);              // ��Ҷ�ӽ�����


private:
	int file_id;
	std::string str_idx_name;
	IndexHeadNode idx_head;
};

#endif
