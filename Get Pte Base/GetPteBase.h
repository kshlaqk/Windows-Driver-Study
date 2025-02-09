#pragma once
#include "main.h"
#include "RWClass.h"

typedef UINT64 pte_base;
typedef UINT64 pde_base;
typedef UINT64 pdpte_base;
typedef UINT64 plm4e_base;

typedef struct ALL_PAGE_TABLE_BASE {
    pte_base _pte_base;
    pde_base _pde_base;
    pdpte_base _pdpte_base;
    plm4e_base _plm4e_base;
} All_Page_Table_Base, * PAll_Page_Table_Base;

UINT64 Get_PXT_Index();
All_Page_Table_Base Get_All_Base(UINT64 param);