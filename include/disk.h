#ifndef DISK
#define DISK
#include <stdbool.h>
#include "typedefine.h"

extern int disk[MAX_DISK_SIZE];
bool init_disk()
{
    bool ok = true;
    for(int i=0; i<MAX_DISK_SIZE; i++)
    {
        disk[i] = FREE;
    }
    return ok;
}


int find_ava_diskBlock()
{
    for(int i=0; i<MAX_DISK_SIZE; i++)
    {
        if(disk[i] == FREE)
            return i;
    }
    return -1;
}


void disk_update(
    int *dbi
)
{
    if(*dbi != -1)
    {
        disk[*dbi] = FREE;
        return;
    }
    int index = find_ava_diskBlock();
    disk[index] = INUSE;
    *dbi = index;
}

#endif