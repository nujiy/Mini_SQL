#pragma once
#include "Global.h"
#include "API.h"

struct TB_Create_Info;
struct TB_Insert_Info;
struct TB_Update_Info;
struct TB_Delete_Info;
struct TB_Select_Info;

// �����±���Ϣ���
void Check_TB_Create_Info(const TB_Create_Info &tb_create_info);

// �������Ϣ���
void Check_TB_Insert_Info(const TB_Insert_Info &tb_insert_info);

// �������Ϣ���
void Check_TB_Update_Info(const TB_Update_Info &tb_update_info);

// ɾ����Ϣ���
void Check_TB_Delete_Info(const TB_Delete_Info &tb_delete_info);

// ������Ϣ���
void Check_TB_Select_Info(const TB_Select_Info &tb_select_info);