#ifndef __BPTREE_H__
#define __BPTREE_H__
#include "Buffer.h"
#include "Record.h"
#include <queue>


// 索引文件头信息
class IndexHeadNode
{
public:
	FileAddr    root;                                       // the address of the root
	FileAddr    MostLeftNode;                               // the address of the most left node
	int         KeyTypeIndex;                               // 关键字字段的位置
	char        RecordTypeInfo[RecordColumnCount];          // 记录字段类型信息，
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
	// 参数：索引文件名称， 关键字类型， 记录各个类型信息数组， 记录各个字段名称信息数组
	BTree(std::string idx_name);
	BTree(const std::string idx_name, int KeyTypeIndex, char(&_RecordTypeInfo)[RecordColumnCount], char(&_RecordColumnName)[RecordColumnCount / 4 * ColumnNameLength]);          // 创建索引文件的B+树
	~BTree() { }
	FileAddr Search(KeyAttr search_key);                                        // 查找关键字是否已经存在
	bool Insert(KeyAttr k, FileAddr k_fd);                                      // 插入关键字k
	FileAddr UpdateKey(KeyAttr k, KeyAttr k_new);                               // 返回关键字对应的记录地址
	FileAddr Delete(KeyAttr k);                                                 // 返回该关键字记录在数据文件中的地址
	void PrintBTreeStruct();                                                    // 层序打印所有结点信息
	void PrintAllLeafNode();
	IndexHeadNode *GetPtrIndexHeadNode();
	BTNode *FileAddrToMemPtr(FileAddr node_fd);                                 // 文件地址转换为内存指针

private:
	FileAddr Delete_InnerNode(FileAddr x, int i, KeyAttr key);              // x的下标为i的结点为叶子结点
	FileAddr Delete_LeafNode(FileAddr x, int i, KeyAttr key);               // x的下标为i的结点为叶子结点
	void Insert_NotFull(FileAddr x, KeyAttr k, FileAddr k_fd);
	void SplitChild(FileAddr x, int i, FileAddr y);                             // 分裂x的孩子结点x.children[i] , y
	FileAddr Search(KeyAttr search_key, FileAddr node_fd);                      // 判断关键字是否存在
	FileAddr Search_InnerNode(KeyAttr search_key, FileAddr node_fd);             // 在内部节点查找
	FileAddr Search_LeafNode(KeyAttr search_key, FileAddr node_fd);              // 在叶子结点查找


private:
	int file_id;
	std::string str_idx_name;
	IndexHeadNode idx_head;
};

#endif
