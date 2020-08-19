#include "B+Tree.h"

BTree::BTree(const std::string idx_name, int KeyTypeIndex, char(&_RecordTypeInfo)[RecordColumnCount],
	char(&_RecordColumnName)[RecordColumnCount / 4 * ColumnNameLength])
	:str_idx_name(idx_name)
{
	auto &buffer = GetGlobalFileBuffer();
	auto pMemFile = buffer[str_idx_name.c_str()];

	// ��������ļ��������򴴽�
	if (!pMemFile)
	{
		// ���������ļ�
		buffer.CreateFile(str_idx_name.c_str());
		pMemFile = buffer[str_idx_name.c_str()];

		// ��ʼ�������ļ�������һ�������
		BTNode root_node;
		assert(sizeof(BTNode) < (FILE_PAGESIZE - sizeof(PAGEHEAD)));
		root_node.node_type = NodeType::ROOT;
		root_node.count_valid_key = 0;
		root_node.next = FileAddr{ 0,0 };
		FileAddr root_node_fd = buffer[str_idx_name.c_str()]->AddRecord(&root_node, sizeof(root_node));

		// ��ʼ�����������ļ�ͷ��Ϣ
		idx_head.root = root_node_fd;
		idx_head.MostLeftNode = root_node_fd;
		idx_head.KeyTypeIndex = KeyTypeIndex;
		//strcpy(idx_head.RecordTypeInfo, _RecordTypeInfo.c_str());
		memcpy(idx_head.RecordTypeInfo, _RecordTypeInfo, RecordColumnCount);
		//strcpy(idx_head.RecordColumnName, _RecordColumnName.c_str());
		memcpy(idx_head.RecordColumnName, _RecordColumnName, RecordColumnCount / 4 * ColumnNameLength);


		// �����ĵ�ַд���ļ�ͷ��Ԥ���ռ���
		memcpy(buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve, &idx_head, sizeof(idx_head));

	}
	file_id = pMemFile->fileId;
}

BTree::BTree(std::string idx_name)
{
	str_idx_name = idx_name;
	file_id = GetGlobalFileBuffer()[idx_name.c_str()]->fileId;
}

FileAddr BTree::DeleteKeyAtInnerNode(FileAddr x, int i, KeyAttr key)
{
	auto px = FileAddrToMemPtr(x);
	auto py = FileAddrToMemPtr(px->children[i]);
	FileAddr fd_res;
	if (py->node_type == NodeType::LEAF)
	{
		fd_res = DeleteKeyAtLeafNode(x, i, key);
	}
	else
	{
		int j = py->count_valid_key - 1;
		while (py->key[j] > key)j--;
		assert(j >= 0);
		fd_res = DeleteKeyAtInnerNode(px->children[i], j, key);
	}

	// �ж�ɾ����Ľ�����
	if (py->count_valid_key >= MaxKeyCount / 2)
		return fd_res;

	// ���ɾ����Ĺؼ��ָ���������B+���Ĺ涨�����ֵܽ�����key

	// ������ֵܴ������и���ؼ���
	if ((i <= px->count_valid_key - 2) && (FileAddrToMemPtr(px->children[i + 1])->count_valid_key > MaxKeyCount / 2))
	{
		auto RBrother = FileAddrToMemPtr(px->children[i + 1]);
		// �����Ĺؼ���
		auto key_bro = RBrother->key[0];
		auto fd_bro = RBrother->children[0];


		// �������ֵܵ��������
		px->key[i + 1] = RBrother->key[1];
		// �������ֵܽ��
		for (int j = 1; j <= RBrother->count_valid_key - 1; j++)
		{
			RBrother->key[j - 1] = RBrother->key[j];
			RBrother->children[j - 1] = RBrother->children[j];
		}
		RBrother->count_valid_key -= 1;

		// ���±�Ҷ�ӽ��
		py->key[py->count_valid_key] = key_bro;
		py->children[py->count_valid_key] = fd_bro;
		py->count_valid_key += 1;

		return fd_res;
	}

	// ������ֵܴ������и���ؼ���
	if (i > 0 && FileAddrToMemPtr(px->children[i - 1])->count_valid_key > MaxKeyCount / 2)
	{
		auto LBrother = FileAddrToMemPtr(px->children[i - 1]);
		// �����Ĺؼ���
		auto key_bro = LBrother->key[LBrother->count_valid_key - 1];
		auto fd_bro = LBrother->children[LBrother->count_valid_key - 1];

		// �������ֵܽ��
		LBrother->count_valid_key -= 1;

		// ���±����
		px->key[i] = key_bro;
		for (int j = py->count_valid_key - 1; j >= 0; j--)
		{
			py->key[j + 1] = py->key[j];
			py->children[j + 1] = py->children[j];
		}
		py->key[0] = key_bro;
		py->children[0] = fd_bro;

		py->count_valid_key += 1;

		return fd_res;
	}

	// ���ֵܽ����û�и����key,��ǰ�����ֵܽ��ϲ���һ���µ�Ҷ�ӽ�㣬��ɾ��������е�key

	// �����ֵܴ��ڽ���ϲ�
	if (i < px->count_valid_key - 1)
	{
		auto RBrother = FileAddrToMemPtr(px->children[i + 1]);
		for (int j = 0; j < RBrother->count_valid_key; j++)
		{
			py->key[py->count_valid_key] = RBrother->key[j];
			py->children[py->count_valid_key] = RBrother->children[j];
			py->count_valid_key++;
		}

		// ����next
		py->next = RBrother->next;
		// ɾ���ҽ��
		GetGlobalFileBuffer()[str_idx_name.c_str()]->DeleteRecord(&px->children[i + 1], sizeof(BTNode));
		// ���¸��ڵ�����
		for (int j = i + 2; j < px->count_valid_key; j++)
		{
			px->key[j - 1] = px->key[j];
			px->children[j - 1] = px->children[j];
		}
		px->count_valid_key--;
	}
	else
	{// ������ϲ�
		auto LBrother = FileAddrToMemPtr(px->children[i - 1]);
		for (int j = 0; j < py->count_valid_key; j++)
		{
			LBrother->key[LBrother->count_valid_key] = py->key[j];
			LBrother->children[LBrother->count_valid_key] = py->children[j];
			LBrother->count_valid_key++;
		}

		// ����next
		LBrother->next = py->next;

		// ɾ�������
		GetGlobalFileBuffer()[str_idx_name.c_str()]->DeleteRecord(&px->children[i], sizeof(BTNode));
		// ���¸��ڵ�����
		for (int j = i + 1; j < px->count_valid_key; j++)
		{
			px->key[j - 1] = px->key[j];
			px->children[j - 1] = px->children[j];
		}
		px->count_valid_key--;
	}
	return fd_res;
}

// �����ɾ���Ĺؼ����Ѿ�����
FileAddr BTree::DeleteKeyAtLeafNode(FileAddr x, int i, KeyAttr key)
{
	auto px = FileAddrToMemPtr(x);
	auto py = FileAddrToMemPtr(px->children[i]);
	FileAddr fd_res;
	int j = py->count_valid_key - 1;
	while (py->key[j] != key)j--;
	assert(j >= 0);
	fd_res = py->children[j];
	// ɾ��Ҷ�ڵ�����С�Ĺؼ��֣����¸��ڵ�
	if (j == 0)
	{
		px->key[i] = py->key[j + 1];
	}

	j++;
	while (j <= py->count_valid_key - 1)
	{
		py->children[j - 1] = py->children[j];
		py->key[j - 1] = py->key[j];
		j++;
	}
	py->count_valid_key -= 1;
	return fd_res;
}

// ��һ��������� x, ����ؼ��� k, k�����ݵ�ַΪ k_fd
void BTree::InsertNotFull(FileAddr x, KeyAttr k, FileAddr k_fd)
{
	auto px = FileAddrToMemPtr(x);
	int i = px->count_valid_key - 1;

	// ����ý����Ҷ�ӽ�㣬ֱ�Ӳ���
	if (px->node_type == NodeType::LEAF || px->node_type == NodeType::ROOT)
	{
		while (i >= 0 && k < px->key[i])
		{
			px->key[i + 1] = px->key[i];
			px->children[i + 1] = px->children[i];
			i--;
		}
		px->key[i + 1] = k;
		px->children[i + 1] = k_fd;
		px->count_valid_key += 1;
	}
	else
	{
		while (i >= 0 && k < px->key[i])  i = i - 1;

		// ��������ֵ���ڽڵ��ֵ��С
		if (i < 0) {
			i = 0;
			px->key[i] = k;
		}
		assert(i >= 0);

		FileAddr ci = px->children[i];
		auto pci = FileAddrToMemPtr(ci);
		if (pci->count_valid_key == MaxKeyCount)
		{
			SplitChild(x, i, ci);
			if (k >= px->key[i + 1])
				i += 1;
		}
		InsertNotFull(px->children[i], k, k_fd);
	}
}

// ��x�±�Ϊi�ĺ�����������
void BTree::SplitChild(FileAddr x, int i, FileAddr y)
{
	auto pMemPageX = GetGlobalClock()->GetMemAddr(file_id, x.filePageID);
	auto pMemPageY = GetGlobalClock()->GetMemAddr(file_id, y.filePageID);
	pMemPageX->isModified = true;
	pMemPageY->isModified = true;

	BTNode*px = FileAddrToMemPtr(x);
	BTNode*py = FileAddrToMemPtr(y);
	BTNode z;         // ���ѳ������½��
	FileAddr z_fd;    // �½����ļ��ڵ�ַ

	z.node_type = py->node_type;
	z.count_valid_key = MaxKeyCount / 2;

	// ��y����һ������ת�Ƶ��½��
	for (int k = MaxKeyCount / 2; k < MaxKeyCount; k++)
	{
		z.key[k - MaxKeyCount / 2] = py->key[k];
		z.children[k - MaxKeyCount / 2] = py->children[k];
	}
	py->count_valid_key = MaxKeyCount / 2;

	// ��y�ĸ��ڵ�x������´������ӽ�� z
	int j;
	for (j = px->count_valid_key - 1; j > i; j--)
	{
		px->key[j + 1] = px->key[j];
		px->children[j + 1] = px->children[j];
	}

	j++; // after j++, j should be i+1;
	px->key[j] = z.key[0];

	if (py->node_type == NodeType::LEAF)
	{
		z.next = py->next;
		z_fd = GetGlobalFileBuffer()[str_idx_name.c_str()]->AddRecord(&z, sizeof(z));
		py->next = z_fd;
	}
	else
		z_fd = GetGlobalFileBuffer()[str_idx_name.c_str()]->AddRecord(&z, sizeof(z));

	px->children[j] = z_fd;
	px->count_valid_key++;
}

FileAddr BTree::Search(KeyAttr search_key)
{
	auto pMemPage = GetGlobalClock()->GetMemAddr(file_id, 0);
	auto pfilefd = (FileAddr*)pMemPage->GetFileCond()->reserve;  // �ҵ������ĵ�ַ
	return Search(search_key, *pfilefd);
}

FileAddr BTree::Search(KeyAttr search_key, FileAddr node_fd)
{
	BTNode* pNode = FileAddrToMemPtr(node_fd);

	if (pNode->node_type == NodeType::LEAF || pNode->node_type == NodeType::ROOT)
	{
		return SearchLeafNode(search_key, node_fd);
	}
	else
	{
		return SearchInnerNode(search_key, node_fd);
	}
}

bool BTree::Insert(KeyAttr k, FileAddr k_fd)
{
	// ����ùؼ����Ѿ����������ʧ��
	try
	{
		auto key_fd = Search(k);
		if (key_fd != FileAddr{ 0,0 })
			throw SQLError::KEY_INSERT_ERROR();
	}
	catch (const SQLError::BaseError &error)
	{
		SQLError::DispatchError(error);
		std::cout << std::endl;
		return false;
	}

	// �õ�������fd
	FileAddr root_fd = *(FileAddr*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve;
	auto proot = FileAddrToMemPtr(root_fd);
	if (proot->count_valid_key == MaxKeyCount)
	{
		// �����µĽ�� s ,��Ϊ�����
		BTNode s;
		s.node_type = NodeType::INNER;  // ֻ�г�ʼ����ʹ�� NodeType::ROOT
		s.count_valid_key = 1;
		s.key[0] = proot->key[0];
		s.children[0] = root_fd;
		FileAddr s_fd = GetGlobalFileBuffer()[str_idx_name.c_str()]->AddRecord(&s, sizeof(BTNode));

		// ���µĸ��ڵ��ļ���ַд��
		*(FileAddr*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve = s_fd;
		GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->isModified = true;

		//���ɵĸ��������ΪҶ�ӽ��
		auto pOldRoot = FileAddrToMemPtr(root_fd);
		if (pOldRoot->node_type == NodeType::ROOT)
			pOldRoot->node_type = NodeType::LEAF;

		// �ȷ����ٲ���
		SplitChild(s_fd, 0, s.children[0]);
		InsertNotFull(s_fd, k, k_fd);
	}
	else
	{
		InsertNotFull(root_fd, k, k_fd);
	}
	return true;
}

// ���¹ؼ���
FileAddr BTree::UpdateKey(KeyAttr k, KeyAttr k_new)
{
	// ��������ɾ���ɵĹؼ���
	auto data_fd = Delete(k);  // ����ؼ��ֶ�Ӧ�ļ�¼��ַ

	//�����µĹؼ���
	Insert(k_new, data_fd);
	return data_fd;
}

FileAddr BTree::Delete(KeyAttr key)
{
	auto search_res = Search(key);
	if (search_res.offSet == 0)
		return FileAddr{ 0,0 };

	// �õ�������fd
	FileAddr root_fd = *(FileAddr*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve;
	auto proot = FileAddrToMemPtr(root_fd);


	// ���ڵ�ΪROOT ���� LEAF ֱ��ɾ��
	if (proot->node_type == NodeType::ROOT || proot->node_type == NodeType::LEAF)
	{
		// ֱ��ɾ��
		int j = proot->count_valid_key - 1;
		while (proot->key[j] != key)j--;
		assert(j >= 0);
		FileAddr fd_res = proot->children[j];
		for (j++; j < proot->count_valid_key; j++)
		{
			proot->key[j - 1] = proot->key[j];
			proot->children[j - 1] = proot->children[j];
		}
		proot->count_valid_key--;
		return fd_res;
	}

	int i = proot->count_valid_key - 1;
	while (proot->key[i] > key)i--;
	assert(i >= 0);
	//auto px = FileAddrToMemPtr(root_fd);
	//auto py = FileAddrToMemPtr(px->children[i]);

	auto fd_delete = DeleteKeyAtInnerNode(root_fd, i, key);


	if (proot->count_valid_key == 1)
	{
		// ���µĸ��ڵ��ļ���ַд��
		*(FileAddr*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve = proot->children[0];
		GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->isModified = true;

		GetGlobalFileBuffer()[str_idx_name.c_str()]->DeleteRecord(&root_fd, sizeof(BTNode));
	}

	return fd_delete;
}

void BTree::PrintBTreeStruct()
{
	std::queue<FileAddr> fds;
	//int n = 0;
	// �õ�������fd
	FileAddr root_fd = *(FileAddr*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve;
	auto pRoot = FileAddrToMemPtr(root_fd);
	if (pRoot->node_type == NodeType::ROOT || pRoot->node_type == NodeType::LEAF)
	{
		if (pRoot->count_valid_key > 0)
		{
			pRoot->PrintSelf();
		}
		return;
	}

	fds.push(root_fd);
	while (!fds.empty())
	{
		// ��ӡ�ý������еĹؼ���
		FileAddr tmp = fds.front();
		fds.pop();
		auto pNode = FileAddrToMemPtr(tmp);
		std::cout << "Node File Address:" << tmp.filePageID << " " << tmp.offSet << std::endl;
		pNode->PrintSelf();
		std::cout << std::endl;

		if (pNode->node_type != NodeType::LEAF)
		{
			for (int i = 0; i < pNode->count_valid_key; i++)
			{
				fds.push(pNode->children[i]);
			}
		}

	}
	//std::cout << "total nodes: " << n << std::endl;
}

void BTree::PrintAllLeafNode()
{
	auto phead = (IndexHeadNode*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve;
	auto pRoot = FileAddrToMemPtr(phead->root);
	if (pRoot->count_valid_key <= 0)
	{
		std::cout << "��¼Ϊ��" << std::endl;
		return;
	}

	auto pNode = FileAddrToMemPtr(phead->MostLeftNode);


	static int n = 0;
	while (pNode->next.offSet != 0)
	{
		for (int i = 0; i < pNode->count_valid_key; i++)
		{
			n++;
			std::cout << pNode->key[i];
		}

		pNode = FileAddrToMemPtr(pNode->next);
	}
	for (int i = 0; i < pNode->count_valid_key; i++)
	{
		n++;
		std::cout << pNode->key[i];
	}
	std::cout << std::endl << n << std::endl;
}

IndexHeadNode * BTree::GetPtrIndexHeadNode()
{
	auto phead = (IndexHeadNode*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve;
	return phead;
}

FileAddr BTree::SearchInnerNode(KeyAttr search_key, FileAddr node_fd)
{
	FileAddr fd_res{ 0,0 };

	BTNode* pNode = FileAddrToMemPtr(node_fd);
	for (int i = pNode->count_valid_key - 1; i >= 0; i--)
	{
		if (pNode->key[i] <= search_key)
		{
			fd_res = pNode->children[i];
			break;
		}
	}

	if (fd_res == FileAddr{ 0,0 })
	{
		return fd_res;
	}
	else
	{
		BTNode* pNextNode = FileAddrToMemPtr(fd_res);
		if (pNextNode->node_type == NodeType::LEAF)
			return SearchLeafNode(search_key, fd_res);
		else
			return SearchInnerNode(search_key, fd_res);
	}
	//return fd_res;
}

FileAddr BTree::SearchLeafNode(KeyAttr search_key, FileAddr node_fd)
{

	BTNode* pNode = FileAddrToMemPtr(node_fd);
	for (int i = 0; i < pNode->count_valid_key; i++)
	{
		if (pNode->key[i] == search_key)
		{
			return pNode->children[i];
		}
	}
	return FileAddr{ 0,0 };
}

BTNode * BTree::FileAddrToMemPtr(FileAddr node_fd)
{
	auto pMemPage = GetGlobalClock()->GetMemAddr(file_id, node_fd.filePageID);
	pMemPage->isModified = true;
	return (BTNode*)((char*)pMemPage->Ptr2PageBegin + node_fd.offSet + sizeof(FileAddr));
}

void BTNode::PrintSelf()
{
	using std::cout;
	using std::endl;
	cout << "Node Type: ";
	switch (node_type)
	{
	case NodeType::ROOT:
		cout << "ROOT";
		break;
	case NodeType::INNER:
		cout << "INNER";
		break;
	case NodeType::LEAF:
		cout << "LEAF";
		break;
	default:
		break;
	}
	cout << "\tcount_valid_key: " << count_valid_key << endl;

	for (int i = 0; i < count_valid_key; i++)
	{
		cout << "index: " << i << " key: " << key[i] << "\t" << "child addr: " << children[i].filePageID << " " << children[i].offSet << endl;
	}

}

