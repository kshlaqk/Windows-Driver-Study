//查找PTE表的基地址
//重要API: MmGetVirtualForPhysical
//原理：Windows自带的cr3页表自映射
//线性地址其实就是索引，多一级索引，那么就少算一级基地址, 多一次映射，就少一级[*]

#include "main.h"
#include "RWClass.h"
#include "GetPteBase.h"


// 提供一个 Unload 函数只是为了让这个程序能动态卸载，方便调试
VOID DriverUnload(PDRIVER_OBJECT driver) {
    //用于消除为使用参数的警告
    UNREFERENCED_PARAMETER(driver);

    //用于在windbg preview中打印
    DbgPrintEx(77, 0, "[+]unloading..\r\n");
}
// DriverEntry，入口函数，相当于main
EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path) {
    UNREFERENCED_PARAMETER(reg_path);

    DbgPrintEx(77, 0, "[+]load success\r\n");

    All_Page_Table_Base all_base = Get_All_Base(Get_PXT_Index());

    DbgPrintEx(77, 0, "[+]PTE Base:%llx\r\n", all_base._pte_base);
    DbgPrintEx(77, 0, "[+]PDE Base:%llx\r\n", all_base._pde_base);
    DbgPrintEx(77, 0, "[+]PDPTE Base:%llx\r\n", all_base._pdpte_base);
    DbgPrintEx(77, 0, "[+]PLM4E Base:%llx\r\n", all_base._plm4e_base);

    driver->DriverUnload = DriverUnload;
    return STATUS_SUCCESS;
}