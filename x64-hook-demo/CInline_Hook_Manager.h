#pragma once
#define MAX_HOOK_COUNT 0X100
#include <ntifs.h>
#include <ntddk.h>
#include <intrin.h>
#include <windows.h>
#include "hde64.h"

typedef struct _HOOK_INFO {
    void* ori_func_addr;
    unsigned char SavedCode[14];
}HOOK_INFO, *PHOOK_INFO;

class CInline_Hook_Manager
{
public:
    bool inline_hook_remove(void* ori_func_addr);
    bool inline_hook(void** ori_func_addr, void* target_func);
    void* create_tramp_line(char* target_func, UINT64 break_bytes_count, char* break_bytes); //�����Ĵ�
    static CInline_Hook_Manager* fn_get_install();

private:
    static CInline_Hook_Manager* instance; //����ָ��
    UINT64 m_cur_hook_count;                //hook�ĺ�������
    HOOK_INFO info[MAX_HOOK_COUNT];         //�Ѿ�hook���ĺ�����Ϣ����
    unsigned char* m_tramp_line;            //�Ĵ�������ָ��
    UINT64 m_tramp_line_used;               //�Ĵ�����
};

