//����PTE��Ļ���ַ
//��ҪAPI: MmGetVirtualForPhysical
//ԭ��Windows�Դ���cr3ҳ����ӳ��
//���Ե�ַ��ʵ������������һ����������ô������һ������ַ, ��һ��ӳ�䣬����һ��[*]

#include "main.h"
#include "RWClass.h"
#include "GetPteBase.h"


// �ṩһ�� Unload ����ֻ��Ϊ������������ܶ�̬ж�أ��������
VOID DriverUnload(PDRIVER_OBJECT driver) {
    //��������Ϊʹ�ò����ľ���
    UNREFERENCED_PARAMETER(driver);

    //������windbg preview�д�ӡ
    DbgPrintEx(77, 0, "[+]unloading..\r\n");
}
// DriverEntry����ں������൱��main
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