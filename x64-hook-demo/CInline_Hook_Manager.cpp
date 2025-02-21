#include "CInline_Hook_Manager.h"

CInline_Hook_Manager* CInline_Hook_Manager::instance;

bool CInline_Hook_Manager::inline_hook_remove(void* ori_func_addr)
{
    return false;
}

bool CInline_Hook_Manager::inline_hook(void** ori_func_addr, void* target_func)
{
    if (m_cur_hook_count >= MAX_HOOK_COUNT) return false;
    
    //构建蹦床内存
    UINT64 break_byte_count = 0;     //破坏字节的数量

    char* ori_func = (char*)*ori_func_addr; //取得原来函数的地址

    hde64s hde{ 0 };                 //初始化hde反汇编引擎获取实际破坏的字节

    //x64下jmp指令最少需要14个字节，因此用hde来判断到底会破坏几条指令，不小于14字节
    while (break_byte_count < 14)
    {
        hde64_disasm(ori_func + break_byte_count, &hde);

        break_byte_count += hde.len;
    }

    auto& info = instance->info;

    info[m_cur_hook_count].ori_func_addr = ori_func;
    memcpy(info[m_cur_hook_count].SavedCode, ori_func, break_byte_count);

    //采用push 0  mov ret的方式来返回地址
    *ori_func_addr = create_tramp_line(ori_func, break_byte_count, ori_func);

    //jmp指令
    char jmp_code[14] = { 0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    *((ULONG*)(&jmp_code[6])) = (ULONG64)target_func;

    ULONG64 cr0;
    // 读取 CR0
    cr0 = __readcr0();
    // 关闭 WP 位 (位16)
    cr0 &= ~(1UL << 16);
    // 写回 CR0
    __writecr0(cr0);
    ///
    memcpy(ori_func, jmp_code, 14);
    ///
    ULONG64 cr0;
    // 读取 CR0
    cr0 = __readcr0();
    // 设置 WP 位 (位16)
    cr0 |= (1UL << 16);
    // 写回 CR0
    __writecr0(cr0);

    return false;
}

void* CInline_Hook_Manager::create_tramp_line(char* target_func, UINT64 break_bytes_count, char* break_bytes)
{
    const ULONG TrampLineBreakBytes = 20; //push mov ret指令长度

    unsigned char TrampLineCode[TrampLineBreakBytes] = {
    0x6A, 0x00, 0x3E, 0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00,
    0x3E, 0xC7, 0x44, 0x24, 0x04, 0x00, 0x00, 0x00, 0x00, 0xC3
    };

    //将要返回到的原函数地址保存到shellcode中
    *((PUINT32)&TrampLineCode[6]) = (UINT32)(((uint64_t)target_func + break_bytes_count) & 0xFFFFFFFF);
    *((PUINT32)&TrampLineCode[6]) = (UINT32)((((uint64_t)target_func + break_bytes_count) >> 32) & 0xFFFFFFFF);

    auto& used = instance->m_tramp_line_used;
    auto& tramp_line_base = instance->m_tramp_line;

    //保存原先破坏的字节, 每一次破坏的字节都存到这里
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
