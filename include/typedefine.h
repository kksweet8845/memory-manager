#ifndef TYPEDEFINE
#define TYPEDEFINE
#include "list.h"

#define FIFO 0
#define ESCA 1
#define SLRU 2

#define READTYPE 0
#define WRITETYPE 1



// ============================================================================================== //
// ========================================= Traces ele ========================================= //
// ============================================================================================== //
typedef struct TELE
{
    int index, type;
    struct list_head list;
} trace_ele_t;

typedef trace_ele_t traceItem_t, *trace_ptr_t;


// ====================================================== //
// =================== Physical frame =================== //
// ====================================================== //

typedef struct PHYELE
{
    int vpi, pfi;
    struct list_head list;
} phy_ele_t;

typedef phy_ele_t phyItem_t, *phy_ptr_t;


// ====================================================== //
// ======================== Disk ======================== //
// ====================================================== //

#define MAX_DISK_SIZE 1048576

#define FREE 0
#define INUSE 1



// ====================================================== //
// ============= Page Replacement Algorithm ============= //
// ====================================================== //

#define ACTIVE 1
#define INACTIVE 0

extern struct list_head pra_in_head, pra_ac_head;


// ~~~~~~~~~~~~~~~~~ RELE ~~~~~~~~~~~~~~~~ //

typedef struct RELE
{
    int vpi, pfi;
    char ref_bit, dirty_bit;
    char type;
    struct list_head list;
} pra_ele_t;

typedef pra_ele_t praItem_t, *pra_ptr_t;

typedef struct SWAPELE
{
    int vpi, evicvpi;
    int pfi;
    int des, src;
} swap_ele_t;
typedef swap_ele_t swapItem_t, *swap_ptr_t;


// ====================================================== //
// ======================== PEPE ======================== //
// ====================================================== //


#define NOT_USED -1
#define USED 0

#define INITIAL -1
#define NOTINMEM 0
#define INMEM 1


typedef struct PELE
{
    char in_use_bit, present_bit;
    unsigned int pfi_dbi;
    struct list_head list;
} page_ele_t;


typedef page_ele_t pageItem_t, *page_ptr_t;
#endif