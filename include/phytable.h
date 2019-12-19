#ifndef PHYTABLE
#define PHYTABLE
#include <stdbool.h>
#include <stdio.h>
#include "typedefine.h"

bool new_phyItem(int index, struct list_head *head, struct list_head **entry)
{

    phy_ptr_t phyptr = (phy_ptr_t) malloc(sizeof(phyItem_t));
    if(phyptr == NULL)
    {
        return false;
    }
    INIT_LIST_HEAD(&phyptr->list);
    list_add_tail(&phyptr->list, head);
    phyptr->vpi = -1;
    phyptr->pfi = index;
    *entry = &phyptr->list;
    return true;
}

bool create_PhyFrame(
    int pfn,
    struct list_head *head,
    struct list_head ***pf_arr
)
{
    bool ok = true;
    *pf_arr = (struct list_head**) malloc(sizeof(struct list_head*)*pfn);
    if(*pf_arr == NULL)
    {
        return false;
    }
    for(int i=0; i<pfn; i++)
    {
        ok = ok && new_phyItem(i, head, &((*pf_arr)[i]));
    }
    if(!ok)
    {
        printf("Error in new_phyItem\n");
    }
    return ok;
}

void phyTable_update(
    int pfi,
    int vpi,
    struct list_head **lookup
)
{
    phy_ptr_t item = list_entry(lookup[pfi], phyItem_t, list);
    item->vpi = vpi;
}


#endif