#include "GetPteBase.h"

//用于获取PXT序号
UINT64 Get_PXT_Index() {
    RWClass cr3(RWClass::Register::CR3);
    UINT64 cr3_physical = cr3.ReadRegister();

    cr3_physical &= 0xfffffffffffff000;

    LARGE_INTEGER cr3_pa_addr = { 0 };
    cr3_pa_addr.QuadPart = cr3_physical;

    UINT64* cr3_virtual = (UINT64*)MmGetVirtualForPhysical(cr3_pa_addr);

    //DbgPrintEx(77, 0, "[+]cr3:%llx\r\n", cr3_physical);
    //DbgPrintEx(77, 0, "[+]cr3 lineaddress Base:%llx\r\n", cr3_virtual);

    for (UINT64 i = 0; i < 512; i++) {
        if (cr3_physical == ((cr3_virtual[i]) & 0x000ffffffffff000)) {
            return i;//见[*]
        }
    }

    return 0xffffffffffffffff;
}

All_Page_Table_Base Get_All_Base(UINT64 param) {

    All_Page_Table_Base result = { 0 };

    result._pte_base = 0xffff000000000000 | (param << 39);

    result._pde_base = 0xffff000000000000 | ((param << 39) + (param << 30));

    result._pdpte_base = 0xffff000000000000 | ((param << 39) + (param << 30) + (param << 21));

    result._plm4e_base = 0xffff000000000000 | ((param << 39) + (param << 30) + (param << 21) + (param << 12));

    return result;

}
