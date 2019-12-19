#ifndef PAGETABLE
#define PAGETABLE

#include <stdbool.h>
#include <stdio.h>
#include "typedefine.h"

bool new_pageItem(struct list_head *head, struct list_head **entry)
{
    bool ok = true;
    page_ptr_t pptr = (page_ptr_t) malloc(sizeof(pageItem_t));
    if(pptr == NULL)
    {
        return false;
    }
    INIT_LIST_HEAD(&pptr->list);
    pptr->in_use_bit = NOT_USED;
    pptr->present_bit = NOTINMEM;
    pptr->pfi_dbi = -1;
    list_add_tail(&pptr->list, head);
    *entry = &pptr->list;
    return ok;
}

bool create_PT(int vpn, struct list_head *head, struct list_head ***vp_arr)
{
    bool ok = true;
    (*vp_arr) = (struct list_head**) malloc(sizeof(struct list_head*)*vpn);
    if(*vp_arr == NULL)
    {
        printf("Error in malloc space to vp_arr in create_PT\n");
        return false;
    }
    for(int i=0; i<vpn; i++)
    {
        ok = ok && new_pageItem(head, &((*vp_arr)[i]));
    }
    if(!ok)
    {
        printf("Error in create_PT\n");
    }
    return ok;
}




pra_ptr_t find_entry_with_pfi(struct list_head *head, int pfi)
{
    pra_ptr_t item = NULL;
    list_for_each_entry(item, head, list)
    {
        if(item->pfi == pfi)
        {
            return item;
        }
    }
    return item;
}

void fill_rdbits(int vpi, char w_r_type, pra_ptr_t item)
{
    item->vpi = vpi;
    switch(w_r_type)
    {
    case READTYPE:
        item->ref_bit = 1;
        item->dirty_bit = 0;
        break;
    case WRITETYPE:
        item->ref_bit = 1;
        item->dirty_bit = 1;
        break;
    }
}



void pageTable_update(
    int vpi,
    int pfi_dbi,
    char ref,
    char dir,
    char in_use_bit,
    char present_bit,
    struct list_head *head,
    struct list_head **lookup,
    char type
)
{
    page_ptr_t item = NULL;
    pra_ptr_t praItem = NULL;
    switch(type)
    {
    case FIFO:
        item = list_entry(lookup[vpi], pageItem_t, list);
        item->in_use_bit = in_use_bit;
        item->present_bit = present_bit;
        item->pfi_dbi = pfi_dbi;
        break;
    case ESCA:
        item = list_entry(lookup[vpi], pageItem_t, list);
        item->in_use_bit = in_use_bit;
        item->present_bit = present_bit;
        item->pfi_dbi = pfi_dbi;
        if(head == NULL )
            break;
        praItem = find_entry_with_pfi(head, pfi_dbi);
        praItem->ref_bit = ref;
        praItem->dirty_bit = praItem->dirty_bit == -1 ? dir : praItem->dirty_bit | dir;
        praItem->vpi = vpi;
        break;
    case SLRU:
        item = list_entry(lookup[vpi], pageItem_t, list);
        item->in_use_bit = in_use_bit;
        item->present_bit = present_bit;
        item->pfi_dbi = pfi_dbi;
        if(head == NULL)
            break;
        praItem = find_entry_with_pfi(head, pfi_dbi);
        praItem->ref_bit = ref;
        break;
    }
}

#endif