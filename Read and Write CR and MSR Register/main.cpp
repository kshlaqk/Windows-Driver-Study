#include "main.h"
#include "RWClass.h"

// 提供一个 Unload 函数只是为了让这个程序能动态卸载，方便调试
VOID DriverUnload(PDRIVER_OBJECT driver)
{
    //用于消除为使用参数的警告
    UNREFERENCED_PARAMETER(driver);

    DbgPrintEx(77, 0, "[+]unloading..\r\n");
}
// DriverEntry，入口函数，相当于main
EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path)
{
    UNREFERENCED_PARAMETER(reg_path);

    DbgPrintEx(77, 0, "[+]load success\r\n");

    RWClass rw_reg(RWClass::Register::CR0);

    LARGE_INTEGER now_cr0 = rw_reg.ReadRegister();

    DbgPrintEx(77, 0, "[+]%016llx\r\n", now_cr0);

    rw_reg.ReplaceReg(RWClass::Register::CR3);

    LARGE_INTEGER now_cr3 = rw_reg.ReadRegister();

    DbgPrintEx(77, 0, "[+]%016llx\r\n", now_cr3);

    LARGE_INTEGER write_cr3;
    write_cr3.QuadPart = 0x00000000001aa000;

    rw_reg.WriteRegister(write_cr3);

    DbgPrintEx(77, 0, "[+]%016llx\r\n", now_cr3);

    write_cr3.QuadPart = now_cr3.QuadPart;

    rw_reg.WriteRegister(write_cr3);

    driver->DriverUnload = DriverUnload;

    return STATUS_SUCCESS;
}
