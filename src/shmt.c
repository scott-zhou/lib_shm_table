#include "shmt.h"
#include "shmt_log.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int get_hash_prime_number(int table_capacity)
{
    // TODO: not implemented
    return table_capacity;
}

int calculate_shm_size(int table_capacity, int num_of_hashkey, int num_of_sortkey)
{
    return sizeof(struct ShmDescriptor) +
           sizeof(struct KeyInShm) * (num_of_hashkey + num_of_sortkey) +
           2*sizeof(int)*getHashPrimeNumber(table_capacity + 2)*num_of_hashkey +
           sizeof(int)*(table_capacity + 2)*num_of_sortkey +
           sizeof(struct DoublyLinkedListNode)*(table_capacity + 2) +
           sizeof(time_t)*(table_capacity + 2) /*time stamp for elements*/ +
           sizeof(T)*(tablecapacity + 2)
           ;
}

int create_shm(const char *pathname,
               int proj_id,
               int size,
               int shmflag/* = 0600*/)
{
    assert(ipcid >=0);
    assert(ipcid <= 255);
    assert(ipcPathName != NULL);
    assert(strlen(ipcPathName) > 0);
    assert(shmSize > 0);

    key_t ipckey = ftok(pathname,proj_id);
    if(ipckey == -1){
        shmt_log(LOGDEBUG,"ftok(ipcPathName(%s),ipcid(%d)) error.",ipcPathName,ipcid);
        return 0;
    }

    int shm_id = shmget(ipckey,0,IPC_CREAT|shmflag);
    if(shmID > -1){
        //The shared memory is already existed.
        return 2;
    }

    shm_id = shmget(ipckey,size,IPC_CREAT|shmflag);
    if(shmID == -1){
        shmt_log(LOGDEBUG,"shmget error: %d. ipckey = %d shmSize=%d", errno,ipckey,size);
        return 0;
    }
    return 1;
}

int create_sem(const char *ipcPathName, int ipcid, int semNum, int operatorFlag/* = 0600*/)
{}
