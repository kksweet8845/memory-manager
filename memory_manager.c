#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "list.h"
#include "sim.h"
#include "phytable.h"
#include "pagetable.h"
#include "disk.h"


struct list_head trace_head, page_head, phy_head;
struct list_head **page_lookupT, **phy_lookupT;
int disk[MAX_DISK_SIZE];
struct list_head pra_in_head, pra_ac_head;

void init_head()
{
    INIT_LIST_HEAD(&trace_head);
    INIT_LIST_HEAD(&page_head);
    INIT_LIST_HEAD(&phy_head);
    INIT_LIST_HEAD(&pra_in_head);
    INIT_LIST_HEAD(&pra_ac_head);
}

int main()
{
    bool ok = true;
    int policy, vpn, pfn;
    trace_ptr_t item = NULL;
    init_head();
    read_traces(&policy, &vpn, &pfn, &trace_head);
    // list_for_each_entry(item, &trace_head, list){
    //     printf("%s %d\n", item->type == READTYPE ? "Read" : "Write", item->index);
    // }
    ok = ok && init_disk();
    for(int i=0; i<MAX_DISK_SIZE; i++)
    {
        assert(disk[i] == 0);
    }
    ok = ok && create_PT(vpn, &page_head, &page_lookupT);
    for(int i=0; i<vpn; i++)
    {
        page_ptr_t pItem = list_entry(page_lookupT[i], pageItem_t, list);
        assert(pItem->pfi_dbi == -1);
    }
    ok = ok && create_PhyFrame(pfn, &phy_head, &phy_lookupT);
    phy_ptr_t ppItem = NULL;
    int i = 0;
    // printf("%p\n", (&phy_head)->next);
    list_for_each_entry(ppItem, &phy_head, list)
    {
        assert(&ppItem->list == phy_lookupT[i++]);
        // printf("%p %p\n", &ppItem->list, phy_lookupT[i++]);
    }
    ok = ok && create_PRATable(policy, pfn, &pra_in_head, &pra_ac_head);
    if(policy == FIFO)
        assert(list_empty(&pra_in_head));
    else if(policy == ESCA )
    {
        assert(!list_empty(&pra_in_head));
    }
    else if(policy == SLRU)
    {
        assert(list_empty(&pra_in_head));
        assert(list_empty(&pra_ac_head));
    }
    else
    {
        printf("Unknow policy\n");
    }
    run_traces(&trace_head, &page_head, &phy_head, page_lookupT, phy_lookupT, vpn, pfn, policy);

    return 0;
}