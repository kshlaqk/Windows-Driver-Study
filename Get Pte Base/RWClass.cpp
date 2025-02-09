#include "RWClass.h"

// 读取寄存器
UINT64 RWClass::ReadRegister()
{
    UINT64 result = 0;

    switch (target)
    {
    case Register::CR0:
        result = __readcr0();
        break;
    case Register::CR2:
        result = __readcr2();
        break;
    case Register::CR3:
        result = __readcr3();
        break;
    case Register::CR4:
        result = __readcr4();
        break;
    case Register::IA32_LSTAR:
        result = __readmsr(0xC0000082); // IA32_LSTAR MSR
        break;
    default:
        DbgPrint("[+]Unsupported register read request\r\n");
        break;
    }

    return result;
}

// 写入寄存器
NTSTATUS RWClass::WriteRegister(UINT64 input_data)
{
    KIRQL oldIrql = KeGetCurrentIrql();
    if (oldIrql > DISPATCH_LEVEL) {
        DbgPrint("[+]Reg access at wrong IRQL! IRQL=%d", oldIrql);
        return STATUS_UNSUCCESSFUL;
    }

    DisableCR0WP();
    switch (target)
    {
    case Register::CR0:
        __writecr0(input_data);
        break;
    case Register::CR2:
        DbgPrint("[+]CR2 is read-only!\r\n");
        break;
    case Register::CR3:
        __writecr3(input_data);
        break;
    case Register::CR4:
        __writecr4(input_data);
        break;
    case Register::IA32_LSTAR:
        __writemsr(0xC0000082, input_data); // IA32_LSTAR MSR
        break;
    default:
        DbgPrint("Unsupported register write request\n");
        break;
    }
    EnableCR0WP();

    return STATUS_SUCCESS;
}

VOID RWClass::DisableCR0WP() 
{
    ULONG64 cr0;
    // 读取 CR0
    cr0 = __readcr0();
    // 关闭 WP 位 (位16)
    cr0 &= ~(1UL << 16);
    // 写回 CR0
    __writecr0(cr0);
}

VOID RWClass::EnableCR0WP() 
{
    ULONG64 cr0;
    // 读取 CR0
    cr0 = __readcr0();
    // 设置 WP 位 (位16)
    cr0 |= (1UL << 16);
    // 写回 CR0
    __writecr0(cr0);
}

BOOLEAN RWClass::ReplaceReg(Register param)
{
    target = param;

    return STATUS_SUCCESS;
}
