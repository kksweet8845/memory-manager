#ifndef PRA
#define PRA
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "pra.h"
#include "typedefine.h"
#include "pagetable.h"
#include "disk.h"
#include "phytable.h"


#define rd(ref, dir) ((ref << 1) | dir)

extern struct list_head **page_lookupT, **phy_lookupT;


pra_ptr_t new_praItem(int vpi, int pfi, char ref, char dir, struct list_head *head, int type)
{
    pra_ptr_t node = NULL;
    switch(type)
    {
    case FIFO:
        node = (pra_ptr_t) malloc(sizeof(praItem_t));
        if(node == NULL)
            return NULL;
        INIT_LIST_HEAD(&node->list);
        node->vpi = vpi;
        node->pfi = pfi;
        list_add(&node->list, head);
        return node;
    case SLRU:
        node = (pra_ptr_t) malloc(sizeof(praItem_t));
        if(node == NULL)
            return NULL;
        INIT_LIST_HEAD(&node->list);
        node->vpi = vpi;
        node->pfi = pfi;
        node->ref_bit = ref;
        list_add(&node->list, head);
        return node;
    case ESCA:
        node = (pra_ptr_t) malloc(sizeof(praItem_t));
        if(node == NULL)
            return NULL;
        INIT_LIST_HEAD(&node->list);
        node->ref_bit = -1;
        node->dirty_bit = -1;
        node->vpi = vpi;
        node->pfi = pfi;
        list_add_tail(&node->list, head);
        return node;
    default:
        printf("Unknown policy\n");
        return NULL;
    }
}


bool create_PRATable(char type, int pfn, struct list_head *head1, struct list_head *head2)
{
    bool ok = true;
    switch(type)
    {
    case FIFO:
        INIT_LIST_HEAD(head1);
        return ok;
    case ESCA:
        INIT_LIST_HEAD(head1);
        for(int i=0; i<pfn; i++)
        {
            ok = ok && new_praItem(-1, i, 0, 0, head1, type);
        }
        return ok;
    case SLRU:
        INIT_LIST_HEAD(head1);
        INIT_LIST_HEAD(head2);
        return ok;
    default:
        printf("Error in create pra table\n");
        return false;
    }
}


// ====================================================== //
// =================== FIFO Algorithm =================== //
// ====================================================== //
swap_ptr_t fifo_algo(
    int vpi,
    int pfi,
    int pfn,
    struct list_head *head
)
{
    pra_ptr_t item = NULL, safe= NULL;
    // hit
    list_for_each_entry_safe(item, safe, head, list)
    {
        if(item->vpi == vpi)
        {
            return NULL;
        }
    }
    // miss
    page_ptr_t pageItem = list_entry(page_lookupT[vpi], pageItem_t, list);
    swap_ptr_t info = (swap_ptr_t) malloc(sizeof(swapItem_t));
    if(pfi == -1)
    {
        pra_ptr_t lastEntry = list_last_entry(head, praItem_t, list);
        list_del_init(&lastEntry->list);
        // Update the page-out page tabel entry
        int swapped_dbi = -1;
        disk_update(&swapped_dbi);
        pageTable_update(lastEntry->vpi, swapped_dbi, -1, -1, USED, NOTINMEM, NULL, page_lookupT, FIFO);
        phyTable_update(lastEntry->pfi, -1, phy_lookupT);
        // Update the page-in page table entry
        pra_ptr_t newNode = new_praItem(vpi, lastEntry->pfi, -1, -1, head, FIFO);
        if(newNode == NULL)
        {
            printf("Error when new a pra in fifo\n");
        }
        info->evicvpi = lastEntry->vpi;
        info->vpi = vpi;
        info->pfi = lastEntry->pfi;
        info->des = swapped_dbi;
        if(pageItem->in_use_bit == NOT_USED)
        {
            info->src = -1;
        }
        else
        {
            int dbi = pageItem->pfi_dbi;
            disk_update(&dbi);
            info->src = pageItem->pfi_dbi;
        }
        pageTable_update(vpi, lastEntry->pfi, -1, -1, USED, INMEM, NULL, page_lookupT, FIFO);
        phyTable_update(lastEntry->pfi, vpi, phy_lookupT);
        return info;
    }
    else
    {
        pra_ptr_t newNode = new_praItem(vpi, pfi, -1, -1, head, FIFO);
        if(newNode == NULL)
        {
            printf("Error when new a pra in fifo\n");
        }
        pageTable_update(vpi, pfi, -1, -1, USED, INMEM, NULL, page_lookupT, FIFO);
        phyTable_update(pfi, vpi, phy_lookupT);
        return NULL;
    }
}

// ====================================================== //
// =================== ESCA Algorithm =================== //
// ====================================================== //




void pageUpdata_via_wrtype(int vpi, int pfi, struct list_head *head, char w_r_type)
{
    switch(w_r_type)
    {
    case READTYPE:
        pageTable_update(vpi, pfi, 1, 0, USED, INMEM, head, page_lookupT, ESCA);
        break;
    case WRITETYPE:
        pageTable_update(vpi, pfi, 1, 1, USED, INMEM, head, page_lookupT, ESCA);
        break;
    }
    phyTable_update(pfi, vpi, phy_lookupT);
}

swap_ptr_t swap_phy_pra(
    int vpi,
    int evicvpi,
    int pfi,
    char w_r_type,
    struct list_head *head
)
{
    int swapped_dbi = -1;
    disk_update(&swapped_dbi);
    pageTable_update(evicvpi, swapped_dbi, -1, -1, USED, NOTINMEM, NULL, page_lookupT, ESCA);
    phyTable_update(pfi, -1, phy_lookupT);
    swap_ptr_t info = (swap_ptr_t) malloc(sizeof(swapItem_t));
    page_ptr_t pageItem = list_entry(page_lookupT[vpi], pageItem_t, list);
    info->pfi = pfi;
    info->evicvpi = evicvpi;
    info->vpi = vpi;
    info->des = swapped_dbi;
    if(pageItem->in_use_bit == USED)
    {
        int dbi = pageItem->pfi_dbi;
        disk_update(&dbi);
        info->src = pageItem->pfi_dbi;
    }
    else
    {
        info->src = -1;
    }
    pageUpdata_via_wrtype(vpi, pfi, head, w_r_type);
    return info;
}


swap_ptr_t esca_algo(
    int vpi,
    int pfi,
    char w_r_type,
    struct list_head *head
)
{
    pra_ptr_t item = NULL;
    if(pfi != -1)
    {
        item = find_entry_with_pfi(head, pfi);
        switch(w_r_type)
        {
        case READTYPE:
            pageTable_update(vpi, pfi, 1, 0, USED, INMEM, head, page_lookupT, ESCA);
            phyTable_update(pfi, vpi, phy_lookupT);
            break;
        case WRITETYPE:
            pageTable_update(vpi, pfi, 1, 1, USED, INMEM, head, page_lookupT, ESCA);
            phyTable_update(pfi, vpi, phy_lookupT);
            break;
        }
        return NULL;
    }
    else
    {
        int state = 1;
        while(state != 0)
        {
            list_for_each_entry(item, head, list)
            {
                switch(rd(item->ref_bit, item->dirty_bit))
                {
                case 0:
                    if(state == 1)
                        return swap_phy_pra(vpi, item->vpi, item->pfi, w_r_type, head);
                    break;
                case 1:
                    if(state == 2)
                        return swap_phy_pra(vpi, item->vpi, item->pfi, w_r_type, head);
                    break;
                case 2:
                case 3:
                    if(state == 2)
                        item->ref_bit = 0;
                    break;
                }
            }
            if(state == 1)
                state <<= 1;
            else
                state >>= 1;
        }
        printf("Error when esca algorithm\n");
        return NULL;
    }
}


// ====================================================== //
// =================== SLRU Algorithm =================== //
// ====================================================== //


swap_ptr_t replace_inactive(int vpi, int pfi, struct list_head* in_head)
{
    // printf("%d\n", vpi);
    while(1)
    {
        pra_ptr_t lastEntry = list_last_entry(in_head, praItem_t, list);
        page_ptr_t pageItem = list_entry(page_lookupT[vpi], pageItem_t, list);
        // printf("replace in vpi : %d, ref: %d\n", lastEntry->vpi, lastEntry->ref_bit);
        if(lastEntry->ref_bit == 0)
        {
            //swapped out
            int swapped_dbi = -1;
            disk_update(&swapped_dbi);
            pageTable_update(lastEntry->vpi, swapped_dbi, -1, -1, USED, NOTINMEM, NULL, page_lookupT, SLRU);
            phyTable_update(lastEntry->pfi, -1, phy_lookupT);
            list_del_init(&lastEntry->list);
            swap_ptr_t info = (swap_ptr_t) malloc(sizeof(swapItem_t));
            info->pfi = lastEntry->pfi;
            info->vpi = vpi;
            info->evicvpi = lastEntry->vpi;
            info->des = swapped_dbi;
            if(pageItem->in_use_bit == USED)
            {
                int dbi = pageItem->pfi_dbi;
                disk_update(&dbi);
                info->src = pageItem->pfi_dbi;
            }
            else
            {
                info->src = -1;
            }
            pageTable_update(vpi, info->pfi, 1, 0, USED, INMEM, in_head, page_lookupT, SLRU);
            phyTable_update(info->pfi, vpi, phy_lookupT);
            lastEntry->vpi = vpi;
            lastEntry->ref_bit = 1;
            lastEntry->type = INACTIVE;
            list_add(&lastEntry->list, in_head);
            return info;
        }
        else
        {
            lastEntry->ref_bit = 0;
            list_del_init(&lastEntry->list);
            list_add(&lastEntry->list, in_head);
        }
    }
}


void replace_active(
    int vpi,
    int in_prn,
    int ac_prn,
    struct list_head *in_head,
    struct list_head *ac_head
)
{
    struct list_head *node = NULL;
    int i = 0;
    while(1)
    {
        pra_ptr_t lastEntry = list_last_entry(ac_head, praItem_t, list);
        page_ptr_t pageItem = list_entry(page_lookupT[vpi], pageItem_t, list);
        if(lastEntry->ref_bit == 0)
        {
            list_del_init(&lastEntry->list);
            list_add_tail(&lastEntry->list, in_head);
            return;
        }
        else
        {
            lastEntry->ref_bit = 0;
            list_del_init(&lastEntry->list);
            list_add(&lastEntry->list, ac_head);
        }
    }
}


swap_ptr_t slru_algo(
    int vpi,
    int pfi,
    int in_prn,
    int ac_prn,
    struct list_head *in_head,
    struct list_head *ac_head
)
{
    pra_ptr_t item = NULL, safe=NULL;
    struct list_head *node;
    int i=0;
    // inactive head
    list_for_each_entry_safe(item, safe, in_head, list)
    {
        if(item->vpi == vpi && item->ref_bit == 0)
        {
            item->ref_bit = 1;
            list_del_init(&item->list);
            list_add(&item->list, in_head);
            return NULL;
        }
        else if(item->vpi == vpi && item->ref_bit == 1)
        {
            list_for_each(node, ac_head) i++;
            if(ac_prn == 0)
            {
                list_del_init(&item->list);
                list_add(&item->list, in_head);
                return NULL;
            }
            if(i < ac_prn)
            {
                item->ref_bit = 0;
            }
            else
            {
                replace_active(item->vpi, in_prn, ac_prn, in_head, ac_head);
            }
            list_del_init(&item->list);
            list_add(&item->list, ac_head);
            return NULL;
        }
    }
    // active head
    list_for_each_entry_safe(item, safe, ac_head, list)
    {
        if(item->vpi == vpi)
        {
            item->ref_bit = 1;
            list_del_init(&item->list);
            list_add(&item->list, ac_head);
            return NULL;
        }
    }
    if(pfi != -1)
    {
        i = 0;
        list_for_each(node, in_head) i++;
        if( i < in_prn )
        {
            pra_ptr_t newNode = new_praItem(vpi, pfi, 1, 0, in_head, SLRU);
            if(newNode == NULL)
                printf("Error in slru in pfi != -1\n");
            pageTable_update(vpi, pfi, 1, 0, USED, INMEM, in_head, page_lookupT, SLRU);
            phyTable_update(pfi, vpi, phy_lookupT);
            return NULL;
        }
        else
        {
            swap_ptr_t info = replace_inactive(vpi, pfi, in_head);
            return info;
        }
    }
    else
    {
        swap_ptr_t info = replace_inactive(vpi, pfi, in_head);
        return info;
    }
}


#endif