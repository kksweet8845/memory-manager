#ifndef SIM
#define SIM
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "pagetable.h"
#include "disk.h"
#include "typedefine.h"
#include "pra.h"

#define MISS 1
#define HIT 0

extern struct list_head **page_lookupT, **phy_loopupT;


trace_ptr_t new_traceItem(int index, int type)
{
    trace_ptr_t tptr = (trace_ptr_t) malloc(sizeof(traceItem_t));
    tptr->type = type;
    tptr->index = index;
    INIT_LIST_HEAD(&tptr->list);
    return tptr;
}

void read_traces(int *policy, int* vpn, int*pfn, struct list_head *head)
{

    char buf[30], temp[30];
    unsigned char i = 0, len;
    char* cur = NULL, *src, **ptr = NULL;
    int index;
    while( fgets(buf, 30, stdin) != NULL )
    {

        switch(i++)
        {
        case 0:
            cur = &(buf[8]);
            if(strncmp(cur, "FIFO", 4) == 0)
            {
                *policy = FIFO;
            }
            else if(strncmp(cur, "ESCA", 4) == 0)
            {
                *policy = ESCA;
            }
            else if(strncmp(cur, "SLRU", 4) == 0)
            {
                *policy = SLRU;
            }
            break;
        case 1:
            src = cur = &(buf[24]);
            len = 0;
            while( isdigit(*cur++) ) len++;
            memset(temp, '\0', strlen(temp));
            memcpy(temp, src, len);
            *vpn = (int) strtol(temp,  ptr, 10);
            break;
        case 2:
            src = cur = &(buf[26]);
            len = 0;
            while( isdigit(*cur++) ) len++;
            memset(temp, '\0', strlen(temp));
            memcpy(temp, src, len);
            *pfn = (int) strtol(temp, ptr, 10);
            break;
        case 3:
            break;
        default:
            src = cur = buf;
            if(strncmp(src, "Read", 4) == 0)
            {
                cur += 5;
                len = 0;
                while( isdigit(*cur++) ) len++;
                cur -= (len+1);
                memset(temp, '\0', strlen(temp));
                memcpy((void*)temp, (void*) cur, len);
                index = strtol(temp, ptr, 10);
                trace_ptr_t tptr = new_traceItem(index, READTYPE);
                list_add_tail(&tptr->list, head);
            }
            else if(strncmp(src, "Write", 5) == 0)
            {
                cur += 6;
                len = 0;
                while( isdigit(*cur++)) len++;
                cur -= (len+1);
                memset(temp, '\0', strlen(temp));
                memcpy(temp, cur, len);
                index = strtol(temp, ptr, 10);
                trace_ptr_t tptr = new_traceItem(index, WRITETYPE);
                list_add_tail(&tptr->list, head);
            }
            break;
        }
    }

    return;
}


void report_miss(int pfi, int evicvpi, int vpi, int des, int src )
{
    printf("Miss, %d, %d>>%d, %d<<%d\n", pfi, evicvpi, des, vpi, src);
}


void find_empty_frame(int pfn, int vpi, int *pfi, phy_ptr_t *item)
{
    for(int i=0; i<pfn; i++)
    {
        // printf("%p\n", phy_lookupT[i]->next->next);
        *item = list_entry(phy_lookupT[i], phyItem_t, list);
        if((*item)->vpi == -1)
        {
            (*item)->vpi = vpi;
            *pfi = (*item)->pfi;
            break;
        }
    }
}


void miss(
    struct list_head *page_node,
    struct list_head *phy_head,
    int vpi,
    char type,
    int pfn,
    char w_r_type
)
{
    phy_ptr_t item = NULL;
    int pfi = -1, i = 0;
    int in_use_bit, present_bit, pfi_dbi;
    double ac_prn, in_prn;
    in_prn = ceil((double)pfn/2);
    ac_prn = floor((double)pfn/2);
    page_ptr_t pageItem = list_entry(page_node, pageItem_t, list);
    pfi_dbi = pageItem->pfi_dbi;
    // list_for_each_entry(item , phy_head, list) i++;
    // printf("%d numbers\n", i);
    pra_ptr_t praItem = NULL;
    if(type == SLRU)
    {
        list_for_each_entry(praItem, &pra_in_head, list) i++;
        if( i < in_prn )
        {
            find_empty_frame(pfn, vpi, &pfi, &item);
            in_use_bit = pageItem->in_use_bit;
            present_bit = pageItem->present_bit;
        }
    }
    else
    {
        find_empty_frame(pfn, vpi, &pfi, &item);
        in_use_bit = pageItem->in_use_bit;
        present_bit = pageItem->present_bit;
        // for(int i=0;i<pfn;i++){
        //     // printf("%p\n", phy_lookupT[i]->next->next);
        //     item = list_entry(phy_lookupT[i], phyItem_t, list);
        //     if(item->vpi == -1){
        //         item->vpi = vpi;
        //         pfi = item->pfi;
        //         in_use_bit = pageItem->in_use_bit;
        //         present_bit = pageItem->present_bit;
        //         // if(pageItem->in_use_bit == NOT_USED ){
        //         //     report_miss(item->pfi, -1, item->vpi, -1, -1);
        //         // }else if(pageItem->in_use_bit == USED
        //         //     && pageItem->present_bit == NOTINMEM){
        //         //     report_miss(item->pfi, -1, item->vpi, -1, pageItem->pfi_dbi);
        //         // }
        //         break;
        //     }
        // }
    }
    // printf("%p\n", &list_entry(phy_head, phyItem_t, list)->list );
    // list_for_each_entry(item , phy_head, list){
    //     printf("Should go here here %d\n", i++);
    //     printf("%p\n", phy_head->next);
    //     printf("%p  %p\n", &item->list, phy_head);
    //     printf("next : %p", item->list.next);
    //     if(item->vpi == -1){
    //         item->vpi = vpi;
    //         pfi = item->pfi;
    //         if(pageItem->in_use_bit == NOT_USED){
    //             report_miss(item->pfi, -1, item->vpi, -1, -1);
    //         }else if(pageItem->in_use_bit == USED
    //             && pageItem->present_bit == NOTINMEM){
    //             report_miss(item->pfi, -1, item->vpi, -1, pageItem->pfi_dbi);
    //         }
    //         break;
    //     }
    // }

    //replacement algorithm
    swap_ptr_t info = NULL;
    switch(type)
    {
    case FIFO:
        info = fifo_algo(vpi, pfi, pfn, &pra_in_head);
        break;
    case ESCA:
        info = esca_algo(vpi, pfi, w_r_type, &pra_in_head);
        break;
    case SLRU:
        info = slru_algo(vpi, pfi, in_prn, ac_prn, &pra_in_head, &pra_ac_head);
        break;
    }
    if(info != NULL)
    {
        report_miss(info->pfi, info->evicvpi, info->vpi, info->des, info->src);
    }
    else
    {
        if(in_use_bit == NOT_USED )
        {
            report_miss(item->pfi, -1, item->vpi, -1, -1);
        }
        else if(in_use_bit == USED
                && present_bit == NOTINMEM)
        {
            report_miss(item->pfi, -1, item->vpi, -1, pfi_dbi);
        }
    }

}

void report_hit(int vpi, int pfi)
{
    printf("Hit, %d=>%d\n", vpi, pfi);
}


void hit(
    struct list_head *page_node,
    int vpi,
    char type,
    int pfn,
    char w_r_type
)
{
    page_ptr_t pageItem = list_entry(page_node, pageItem_t, list);
    report_hit(vpi, pageItem->pfi_dbi);
    double ac_prn, in_prn;
    switch(type)
    {
    case FIFO:
        fifo_algo(vpi, pageItem->pfi_dbi, 0, &pra_in_head);
        break;
    case ESCA:
        esca_algo(vpi, pageItem->pfi_dbi, w_r_type, &pra_in_head);
        break;
    case SLRU:
        in_prn = ceil((double)pfn/2);
        ac_prn = floor((double)pfn/2);
        slru_algo(vpi, pageItem->pfi_dbi, in_prn, ac_prn, &pra_in_head, &pra_ac_head);
        break;
    }
}


unsigned char execute_trace(
    trace_ptr_t traceItem,
    struct list_head *trace_head,
    struct list_head *page_head,
    struct list_head *phy_head,
    struct list_head **page_lookup,
    struct list_head **phy_lookup,
    int vpn,
    int pfn,
    char type
)
{
    page_ptr_t pageItem = NULL;

    pageItem = list_entry(page_lookup[traceItem->index], pageItem_t, list);
    if( pageItem->in_use_bit == NOT_USED )
    {
        miss(&pageItem->list, phy_head, traceItem->index, type, pfn, traceItem->type);
        return MISS;
    }
    else if( pageItem->present_bit == INMEM )
    {
        // hit
        hit(&pageItem->list, traceItem->index, type, pfn, traceItem->type);
        return HIT;
    }
    else if( pageItem->present_bit == NOTINMEM )
    {
        // miss
        miss( &pageItem->list, phy_head, traceItem->index, type, pfn, traceItem->type);
        return MISS;
    }
    return MISS;
}

void report_mr(float r)
{
    printf("Page Fault Rate: %.3f\n", r);
}

void run_traces(
    struct list_head *trace_head,
    struct list_head *page_head,
    struct list_head *phy_head,
    struct list_head **page_lookup,
    struct list_head **phy_lookup,
    int vpn,
    int pfn,
    char type
)
{

    trace_ptr_t item, safe;
    float total = 0;
    float miss_rate = 0;
    list_for_each_entry_safe(item, safe, trace_head, list)
    {
        total++;
        miss_rate += execute_trace(item, trace_head, page_head, phy_head, page_lookup, phy_lookup, vpn, pfn, type);
#define DE
#ifdef DEBUG
        printf("Page Table ==============\n");
        for(int i=0; i<vpn; i++)
        {
            page_ptr_t pi = list_entry(page_lookup[i], pageItem_t, list);
            printf("%d : pfi_dbi: %d, in_use : %d, present_bit :%d\n", i, pi->pfi_dbi, pi->in_use_bit, pi->present_bit);
        }
        printf("\nPhysical Table============\n");
        for(int i=0; i<pfn; i++)
        {
            phy_ptr_t phyi = list_entry(phy_lookup[i], phyItem_t, list);
            printf("%d : vpi : %d\n", phyi->pfi, phyi->vpi);
        }
        pra_ptr_t praItem = NULL;
        printf("\n");
        list_for_each_entry(praItem, &pra_in_head, list)
        {
            printf("in vpi : %d, ref : %d, dir : %d\n", praItem->vpi, praItem->ref_bit, praItem->dirty_bit);
        }
        if(type == SLRU)
        {
            printf("\n");
            list_for_each_entry(praItem, &pra_ac_head, list)
            {
                printf("ac vpi : %d, ref : %d, dir : %d\n", praItem->vpi, praItem->ref_bit, praItem->dirty_bit);
            }
        }
#endif
    }
    report_mr(miss_rate / total);

}
#endif