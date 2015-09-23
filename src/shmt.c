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

bool shm_existed(const char *pathname, int proj_id, int shmflag /*= 0600)*/)
{
    assert(proj_id >=0);
    assert(proj_id <= 255);
    assert(pathname != NULL);
    assert(strlen(pathname) > 0);

    key_t ipckey = ftok(pathname,proj_id);
    if(ipckey == -1){
        shmt_log(LOGDEBUG,"ftok(pathname(%s),proj_id(%d)) error.",pathname,proj_id);
        return false;
    }

    int shm_id = shmget(ipckey,0,IPC_CREAT|shmflag);
    if(shm_id > -1){
        //The shared memory is already existed.
        return true;
    }
    return false;
}

int create_shm(const char *pathname,
               int proj_id,
               int size,
               int shmflag/* = 0600*/)
{
    assert(proj_id >=0);
    assert(proj_id <= 255);
    assert(pathname != NULL);
    assert(strlen(pathname) > 0);
    assert(size > 0);

    key_t ipckey = ftok(pathname,proj_id);
    if(ipckey == -1){
        shmt_log(LOGDEBUG,"ftok(pathname(%s),proj_id(%d)) error.",pathname,proj_id);
        return -1;
    }

    shm_id = shmget(ipckey,size,IPC_CREAT|shmflag);
    if(shmID == -1){
        shmt_log(LOGDEBUG,"shmget error: %d. ipckey = %d shmSize=%d", errno,ipckey,size);
        return -1;
    }
    return shm_id;
}

void* connect_shm(int shm_id)
{
    void* p = shmat(shm_id,NULL,0);
    if((int)p == -1){
        shmt_log(LOGDEBUG,"shmat fail, errno: %d.", errno);
        return NULL;
    }
    return p;
}

bool release_shm(int shm_id)
{
    if ( shm_id >= 0 ){
        if (shmctl(shm_id, IPC_RMID, 0) < 0){
            shmt_log(LOGDEBUG,"semctl at set value error: %d", errno);
            return false;
        }
    }
    else{
        return false;
    }
    return true;
}


int create_sem(const char *pathname,
               int proj_id,
               int shmflag/* = 0600*/)
{
    assert(proj_id >=0);
    assert(proj_id <= 255);
    assert(pathname != NULL);
    assert(strlen(pathname) > 0);
    assert(sem_num > 0);

    key_t ipckey = ftok(pathname,proj_id);
    if(ipckey == -1){
        shmt_log(LOGDEBUG,"ftok(pathname(%s),proj_id(%d)) error.",pathname,proj_id);
        return false;
    }

    int sem_id = semget(ipckey,SEM_NUM_FOR_RWLOCK ,IPC_CREAT|shmflag);
    if(sem_id == -1){
        shmt_log(LOGDEBUG,"semget error: %d. ipckey = %d", errno, ipckey);
        return -1;
    }

    union semum {
        int val;
        struct semid_ds *buf;
        ushort *array;
    }arg;
    ushort ar[SEM_NUM_FOR_RWLOCK];
    memset(&arg,0,sizeof(arg));
    memset(ar, 0, SEM_NUM_FOR_RWLOCK*sizeof(ushort));
    for (int i=0; i<SEM_NUM_FOR_RWLOCK; i++) {ar[i] = 1;};
    arg.array = ar;

    if (semctl(sem_id, 0, SETALL, arg) == -1) {
        shmt_log(LOGDEBUG,"semctl SETALL fail, errno: %d", errno);
        release_sem(sem_id);
        return -1;
    }
    return sem_id;
}

bool release_sem(int sem_id)
{
    assert(sem_id >= 0);
    if (semctl(sem_id, 0, IPC_RMID) < 0) {
        return false;
    }
    return true;
}

bool lock(int sem_id, int locker)
{
    struct sembuf sbuf;
    sbuf.sem_num= locker;
    sbuf.sem_op = -1;
    sbuf.sem_flg = SEM_UNDO;
    if(semop(sem_id, &sbuf, 1) == -1 ){
        shmt_log(LOGDEBUG,"semop operate error: %d, sem_id = %d locker = %d",
                  errno, sem_id, locker);
        return false;
    }
    shmt_log(LOGDEBUG,"lock succeed for sem_id %d locker %d.",sem_id, locker);
    return true;
}

bool unlock(int sem_id, int locker)
{
    struct sembuf sbuf;
    sbuf.sem_num= locker;
    sbuf.sem_op = 1;
    sbuf.sem_flg = SEM_UNDO;
    if(semop(sem_id, &sbuf, 1) == -1 ){
        shmt_log(LOGDEBUG,"semop operate error: %d, sem_id = %d locker = %d",
                  errno, sem_id, locker);
        return false;
    }
    shmt_log(LOGDEBUG,"unlock succeed for sem_id %d locker %d.",sem_id, locker);
    return true;
}

bool read_lock(struct ShmDescriptor* shm_descriptor, int sem_id)
{
    assert(shm_descriptor != NULL);
    assert(sem_id >= 0);
    lock(sem_id, READ_LOCK) || return false;
    if(0 == shm_descriptor->lock_counter++) {
        lock(sem_id, WRITE_LOCK);
    }
    unlock(sem_id, READ_LOCK) || return false;
    return true;
}

bool read_unlock(struct ShmDescriptor* shm_descriptor, int sem_id)
{
    assert(shm_descriptor != NULL);
    assert(sem_id >= 0);
    lock(sem_id, READ_LOCK) || return false;
    if(0 == --shm_descriptor->lock_counter) {
        unlock(sem_id, WRITE_LOCK);
    }
    unlock(sem_id, READ_LOCK) || return false;
    return true;
}

bool write_lock(struct ShmDescriptor* shm_descriptor, int sem_id)
{
    assert(shm_descriptor != NULL);
    assert(sem_id >= 0);
    return lock(sem_id, WRITE_LOCK);
}

bool write_unlock(struct ShmDescriptor* shm_descriptor, int sem_id)
{
    assert(shm_descriptor != NULL);
    assert(sem_id >= 0);
    return unlock(sem_id, WRITE_LOCK);
}

