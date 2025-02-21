#include "CInline_Hook_Manager.h"

CInline_Hook_Manager* CInline_Hook_Manager::instance;

bool CInline_Hook_Manager::inline_hook_remove(void* ori_func_addr)
{
    return false;
}

bool CInline_Hook_Manager::inline_hook(void** ori_func_addr, void* target_func)
{
    if (m_cur_hook_count >= MAX_HOOK_COUNT) return false;
    
    //�����Ĵ��ڴ�
    UINT64 break_byte_count = 0;     //�ƻ��ֽڵ�����

    char* ori_func = (char*)*ori_func_addr; //ȡ��ԭ�������ĵ�ַ

    hde64s hde{ 0 };                 //��ʼ��hde����������ȡʵ���ƻ����ֽ�

    //x64��jmpָ��������Ҫ14���ֽڣ������hde���жϵ��׻��ƻ�����ָ���С��14�ֽ�
    while (break_byte_count < 14)
    {
        hde64_disasm(ori_func + break_byte_count, &hde);

        break_byte_count += hde.len;
    }

    auto& info = instance->info;

    info[m_cur_hook_count].ori_func_addr = ori_func;
    memcpy(info[m_cur_hook_count].SavedCode, ori_func, break_byte_count);

    //����push 0  mov ret�ķ�ʽ�����ص�ַ
    *ori_func_addr = create_tramp_line(ori_func, break_byte_count, ori_func);

    //jmpָ��
    char jmp_code[14] = { 0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    *((ULONG*)(&jmp_code[6])) = (ULONG64)target_func;

    ULONG64 cr0;
    // ��ȡ CR0
    cr0 = __readcr0();
    // �ر� WP λ (λ16)
    cr0 &= ~(1UL << 16);
    // д�� CR0
    __writecr0(cr0);
    ///
    memcpy(ori_func, jmp_code, 14);
    ///
    ULONG64 cr0;
    // ��ȡ CR0
    cr0 = __readcr0();
    // ���� WP λ (λ16)
    cr0 |= (1UL << 16);
    // д�� CR0
    __writecr0(cr0);

    return false;
}

void* CInline_Hook_Manager::create_tramp_line(char* target_func, UINT64 break_bytes_count, char* break_bytes)
{
    const ULONG TrampLineBreakBytes = 20; //push mov retָ���

    unsigned char TrampLineCode[TrampLineBreakBytes] = {
    0x6A, 0x00, 0x3E, 0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00,
    0x3E, 0xC7, 0x44, 0x24, 0x04, 0x00, 0x00, 0x00, 0x00, 0xC3
    };

    //��Ҫ���ص���ԭ������ַ���浽shellcode��
    *((PUINT32)&TrampLineCode[6]) = (UINT32)(((uint64_t)target_func + break_bytes_count) & 0xFFFFFFFF);
    *((PUINT32)&TrampLineCode[6]) = (UINT32)((((uint64_t)target_func + break_bytes_count) >> 32) & 0xFFFFFFFF);

    auto& used = instance->m_tramp_line_used;
    auto& tramp_line_base = instance->m_tramp_line;

    //����ԭ���ƻ����ֽ�, ÿһ���ƻ����ֽڶ��浽����
    RtlCopyMemory(tramp_line_base + used, break_bytes, break_bytes_count);
    RtlCopyMemory(tramp_line_base + used + break_bytes_count, TrampLineCode, sizeof(TrampLineCode));

    auto ret = tramp_line_base + used;
    used += TrampLineBreakBytes + break_bytes_count;

    return ret;
}

CInline_Hook_Manager* CInline_Hook_Manager::fn_get_install()
{
    if (instance == 0)
    {
        instance = (CInline_Hook_Manager*)ExAllocatePoolWithTag(NonPagedPool, sizeof(CInline_Hook_Manager), 'HOOK');
    }

    instance->m_cur_hook_count = 0;
    RtlSecureZeroMemory(instance->info, sizeof(HOOK_INFO) * MAX_HOOK_COUNT);
    instance->m_tramp_line = (unsigned char*)ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 'Line');
    instance->m_tramp_line_used = 0;

    return instance;
}
