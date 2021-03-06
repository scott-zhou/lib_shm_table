#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "shmt.h"
#include "shmt_log.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

int get_hash_prime_number(int table_capacity)
{
    // TODO: not implemented
    return table_capacity;
}

size_t calculate_shm_size(int table_capacity,
                       int element_size,
                       int num_of_hashkey,
                       int num_of_sortkey)
{
    return sizeof(struct ShmDescriptor) +
           sizeof(struct KeyInShm) * (num_of_hashkey + num_of_sortkey) +
           2*sizeof(int)*get_hash_prime_number(table_capacity)*num_of_hashkey+
           sizeof(int)*(table_capacity)*num_of_sortkey +
           sizeof(struct DoublyLinkedListNode)*(table_capacity + 2) +
           sizeof(time_t)*(table_capacity) /*time stamp for elements*/ +
           element_size*(table_capacity)
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
        shmt_log(LOGDEBUG,"ftok(pathname(%s),proj_id(%d)) error.",
                 pathname,proj_id);
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
               size_t size,
               int shmflag/* = 0600*/)
{
    assert(proj_id >=0);
    assert(proj_id <= 255);
    assert(pathname != NULL);
    assert(strlen(pathname) > 0);
    assert(size > 0);

    key_t ipckey = ftok(pathname,proj_id);
    if(ipckey == -1){
        shmt_log(LOGDEBUG,"ftok(pathname(%s),proj_id(%d)) error.",
                 pathname,proj_id);
        return -1;
    }

    int shm_id = shmget(ipckey,size,IPC_CREAT|shmflag);
    if(shm_id == -1){
        shmt_log(LOGDEBUG,"shmget error: %d. ipckey = %d shmSize=%zu",
                 errno,ipckey,size);
        return -1;
    }
    return shm_id;
}

int get_shm_id(const char *pathname, int proj_id, int shmflag/*= 0600*/)
{
    assert(proj_id >=0);
    assert(proj_id <= 255);
    assert(pathname != NULL);
    assert(strlen(pathname) > 0);

    key_t ipckey = ftok(pathname,proj_id);
    if(ipckey == -1){
        shmt_log(LOGDEBUG,"ftok(pathname(%s),proj_id(%d)) error.",
                 pathname,proj_id);
        return -1;
    }

    int shm_id = shmget(ipckey,0,IPC_CREAT|shmflag);
    if(shm_id == -1){
        shmt_log(LOGDEBUG,"shmget error: %d. ipckey = %d",
                 errno,ipckey);
        return -1;
    }
    return shm_id;
}

int get_sem_id(const char *pathname, int proj_id, int shmflag/*= 0600*/)
{
    assert(proj_id >=0);
    assert(proj_id <= 255);
    assert(pathname != NULL);
    assert(strlen(pathname) > 0);

    key_t ipckey = ftok(pathname,proj_id);
    if(ipckey == -1){
        shmt_log(LOGDEBUG,"ftok(pathname(%s),proj_id(%d)) error.",
                 pathname,proj_id);
        return -1;
    }

    int sem_id = semget(ipckey,0 ,IPC_CREAT|shmflag);
    if(sem_id == -1){
        shmt_log(LOGDEBUG,"semget error: %d, ipckey = %d",
                  errno, ipckey);
        return 0;
    }
    return sem_id;
}

void* connect_shm(int shm_id)
{
    void* p = shmat(shm_id,NULL,0);
    if(p == (void *)-1){
        shmt_log(LOGDEBUG,"shmat fail, errno: %d.", errno);
        return NULL;
    }
    return p;
}

bool release_shm(int shm_id)
{
    assert(shm_id >= 0);
    if (shmctl(shm_id, IPC_RMID, 0) < 0){
        shmt_log(LOGDEBUG,"semctl at set value error: %d", errno);
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

    key_t ipckey = ftok(pathname,proj_id);
    if(ipckey == -1){
        shmt_log(LOGDEBUG,"ftok(pathname(%s),proj_id(%d)) error.",pathname,proj_id);
        return -1;
    }

    int sem_id = semget(ipckey,SEM_NUM_FOR_RWLOCK ,IPC_CREAT|shmflag);
    if(sem_id == -1){
        shmt_log(LOGDEBUG,"semget error: %d. ipckey = %d", errno, ipckey);
        return -1;
    }

    union semum {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
        struct seminfo  *__buf;
    }arg;
    unsigned short ar[SEM_NUM_FOR_RWLOCK];
    memset(&arg,0,sizeof(arg));
    memset(ar, 0, SEM_NUM_FOR_RWLOCK*sizeof(unsigned short));
    for (int i=0; i<SEM_NUM_FOR_RWLOCK; i++) {ar[i] = 1;}
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

bool init_shm(void *p, int table_capacity, size_t size, int num_of_hash_key, int num_of_sort_key)
{
    struct ShmDescriptor* descriptor = getp_shm_descriptor(p);
    //struct ShmDescriptor{
    descriptor->shm_size = size;
    descriptor->lock_flag = true;
    descriptor->lock_counter = 0;
    descriptor->num_of_hash_key = num_of_hash_key;
    descriptor->num_of_sort_key = num_of_sort_key;
    descriptor->size_of_record = 0;
    //descriptor->size_of_table = ?;
    descriptor->capacity = table_capacity;
    descriptor->load_count = 0;
    descriptor->hash_prime_number = get_hash_prime_number(table_capacity);
    //};
    dl_list_init(p);
    return true;
}

bool detach_shm(void *p)
{
    if (NULL == p) {
        return true;
    }
    return (shmdt(p)==0);
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

bool read_lock(void* p, int sem_id)
{
    assert(p != NULL);
    assert(sem_id >= 0);
    struct ShmDescriptor* shm_descriptor = getp_shm_descriptor(p);
    if(!shm_descriptor->lock_flag) return true;
    if(!lock(sem_id, READ_LOCK)) { return false; }
    if(0 == shm_descriptor->lock_counter++) {
        lock(sem_id, WRITE_LOCK);
    }
    if(!unlock(sem_id, READ_LOCK)) { return false; }
    return true;
}

bool read_unlock(void* p, int sem_id)
{
    assert(p != NULL);
    assert(sem_id >= 0);
    struct ShmDescriptor* shm_descriptor = getp_shm_descriptor(p);
    if(!shm_descriptor->lock_flag) return true;
    if(!lock(sem_id, READ_LOCK)) { return false; }
    if(0 == --shm_descriptor->lock_counter) {
        unlock(sem_id, WRITE_LOCK);
    }
    if(!unlock(sem_id, READ_LOCK)) { return false; }
    return true;
}

bool write_lock(void* p, int sem_id)
{
    assert(p != NULL);
    assert(sem_id >= 0);
    struct ShmDescriptor* shm_descriptor = getp_shm_descriptor(p);
    if(!shm_descriptor->lock_flag) return true;
    return lock(sem_id, WRITE_LOCK);
}

bool write_unlock(void* p, int sem_id)
{
    assert(p != NULL);
    assert(sem_id >= 0);
    struct ShmDescriptor* shm_descriptor = getp_shm_descriptor(p);
    if(!shm_descriptor->lock_flag) return true;
    return unlock(sem_id, WRITE_LOCK);
}

struct ShmDescriptor* getp_shm_descriptor(void *p)
{
    return (struct ShmDescriptor*)p;
}

struct KeyInShm* getp_hashkey(void *p, int key_id)
{
    struct ShmDescriptor* descriptor = getp_shm_descriptor(p);
    assert(key_id < descriptor->num_of_hash_key);
    struct KeyInShm* p_hashkey = (struct KeyInShm*)(p+sizeof(struct ShmDescriptor));
    p_hashkey += key_id;
    return p_hashkey;
}

struct KeyInShm* getp_sortkey(void *p, int key_id)
{
    struct ShmDescriptor* descriptor = getp_shm_descriptor(p);
    assert(key_id < descriptor->num_of_sort_key);
    struct KeyInShm* p_sortkey = (struct KeyInShm*)(
                p+
                sizeof(struct ShmDescriptor)+
                sizeof(struct KeyInShm)*(descriptor->num_of_hash_key)
                );
    p_sortkey += key_id;
    return p_sortkey;
}

int * getp_hash_table(void *p, int key_id)
{
    struct ShmDescriptor* descriptor = getp_shm_descriptor(p);
    assert(key_id < descriptor->num_of_hash_key);
    return (int *) (p +
                    sizeof(struct ShmDescriptor) +
                    sizeof(struct KeyInShm) * (descriptor->num_of_hash_key + descriptor->num_of_sort_key) +
                    2*sizeof(int)*descriptor->hash_prime_number*key_id
                    );
}

int * getp_sort_table(void *p, int key_id)
{
    struct ShmDescriptor* descriptor = getp_shm_descriptor(p);
    assert(key_id < descriptor->num_of_sort_key);
    return (int *) (p +
                    sizeof(struct ShmDescriptor) +
                    sizeof(struct KeyInShm) * (descriptor->num_of_hash_key + descriptor->num_of_sort_key) +
                    2*sizeof(int)*descriptor->hash_prime_number*descriptor->num_of_hash_key +
                    sizeof(int)*(descriptor->capacity)*key_id
                    );
}

void* getp_data(void *p)
{
    struct ShmDescriptor* descriptor = getp_shm_descriptor(p);
    return p +
           sizeof(struct ShmDescriptor) +
           sizeof(struct KeyInShm) * (descriptor->num_of_hash_key + descriptor->num_of_sort_key) +
           2*sizeof(int)*descriptor->hash_prime_number*descriptor->num_of_hash_key +
           sizeof(int)*(descriptor->capacity)*descriptor->num_of_sort_key +
           sizeof(struct DoublyLinkedListNode)*(descriptor->capacity + 2) +
           sizeof(time_t)*(descriptor->capacity);
}

bool add_hashkey(void* p, unsigned int id, const struct Key* key)
{
    struct KeyInShm* key_point = getp_hashkey(p, id);
    key_point->load_count = 0;
    key_point->key = *key;
    return true;
}

bool add_sortkey(void* p, unsigned int id, const struct Key* key)
{
    struct KeyInShm* key_point = getp_sortkey(p, id);
    key_point->load_count = 0;
    key_point->key = *key;
    return true;
}

void set_lock_flag(void* p, bool use_lock)
{
    struct ShmDescriptor* descriptor = getp_shm_descriptor(p);
    descriptor->lock_flag = use_lock;
}

bool get_lock_flag(void* p)
{
    struct ShmDescriptor* descriptor = getp_shm_descriptor(p);
    return descriptor->lock_flag;
}

struct DoublyLinkedListNode* dl_getp_head(void* p)
{
    struct ShmDescriptor* descriptor = getp_shm_descriptor(p);
    struct DoublyLinkedListNode* dlist = (struct DoublyLinkedListNode*)(
                p +
                sizeof(struct ShmDescriptor) +
                sizeof(struct KeyInShm)*(descriptor->num_of_hash_key + descriptor->num_of_sort_key) +
                2*sizeof(int)* descriptor->hash_prime_number * descriptor->num_of_hash_key +
                sizeof(int)*(descriptor->capacity)*descriptor->num_of_sort_key
                );
    return dlist;
}

struct DoublyLinkedListNode* dl_getp_used_head(void* p)
{
    return dl_getp_head(p)+kUsedHead;
}
struct DoublyLinkedListNode* dl_getp_unuse_head(void* p)
{
    return dl_getp_head(p)+kUnuseHead;
}

bool dl_list_init(void *p)
{
    struct ShmDescriptor* descriptor = getp_shm_descriptor(p);
    struct DoublyLinkedListNode* dlist = dl_getp_head(p);
    struct DoublyLinkedListNode* used_head = dlist+kUsedHead; // dlist[0]
    used_head->next = 0;
    used_head->prev = 0;
    for(int i=1; i<descriptor->capacity + 2; i++) {
        struct DoublyLinkedListNode* cur_node = dlist+i;
        cur_node->next = i+1;
        cur_node->prev = i-1;
    }
    struct DoublyLinkedListNode* unuse_head = dlist+kUnuseHead; // dlist[1]
    unuse_head->prev = (descriptor->capacity + 2) - 1;
    struct DoublyLinkedListNode* unuse_end = dlist+(descriptor->capacity + 2)-1;
    unuse_end->next = 1;
    return true;
}

int dl_add_element(void* p)
{
    struct DoublyLinkedListNode* used_head = dl_getp_used_head(p);
    struct DoublyLinkedListNode* unuse_head = dl_getp_unuse_head(p);
    int position = unuse_head->next;
    if(position == kUnuseHead) {
        return -1;
    }
    struct DoublyLinkedListNode* cur_node = dl_getp_head(p) + position;
    struct DoublyLinkedListNode* prev = dl_getp_head(p) + cur_node->prev;
    struct DoublyLinkedListNode* next = dl_getp_head(p) + cur_node->next;
    prev->next = cur_node->next;
    next->prev = cur_node->prev;

    prev = dl_getp_head(p) + used_head->prev;
    cur_node->prev = used_head->prev;
    cur_node->next = kUsedHead;
    used_head->prev = position;
    prev->next = position;
    return position;
}

bool add_hashkey_for_element(void* p)
{
    //TODO: Insert hash data
    return true;
}

bool insert_to_sorttable(struct KeyInShm *key, int *table, const void* data, int element_pos, int element_size)
{
    if(key->load_count == 0) {
        table[0] = element_pos;
        key->load_count ++;
        return true;
    }
    int begin = 0;
    int end = key->load_count - 1;
    int cur = (end - begin) / 2;
}

bool add_sortkey_for_element(void* p, int element_pos, int element_size)
{
    //TODO: Insert sort data
    struct ShmDescriptor* descriptor = getp_shm_descriptor(p);
    void* p_data = getp_data(p);
    for(int i=0; i<descriptor->num_of_sort_key; i++) {
        struct KeyInShm *p_sort_key = getp_sortkey(p, i);
        int * p_sort_table = getp_sort_table(p, i);
        if(!insert_to_sorttable(p_sort_key, p_sort_table, p_data, element_pos, element_size)) {
            return false;
        }
    }
    return true;
}

int add_element(void* p, void* data, size_t size)
{
    struct ShmDescriptor* descriptor = getp_shm_descriptor(p);
    int position = dl_add_element(p);
    if(-1 == position) {
        shmt_log(LOGCRITICAL,"Add element to shm fail, shm table full. Table capacity %d",
                 descriptor->capacity);
        return -1;
    }
    //Doubly Linked List use two element for used_head and unused_head from position 0
    //So data position shoule be dl_list position-2
    int data_position = position - 2;
    descriptor->load_count ++;
    void* p_data = getp_data(p);
    void* p_to = p_data + (size * data_position);
    memcpy(p_to, data, size);

    if(!(add_hashkey_for_element(p) && add_sortkey_for_element(p, data_position, size))) {
        //TODO:
        //If fail on crate key for element, we have to remove it from doubly linked list,
        //clear data(if we want clean memory), and clear already added keys before return.
        return -1;
    }
    return position;
}
