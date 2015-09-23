#ifndef _LIB_SHMT_H_
#define _LIB_SHMT_H_

#include <stdbool.h>

#define MAX_LEN_KEY_FORMAT  32
#define SEM_NUM_FOR_RWLOCK  2
#define READ_LOCK           0
#define WRITE_LOCK          1

struct ShmDescriptor{
    int shm_size;
    /*No need to save sem number, it shoule be always 2 for a rwlock*/
    bool lock_flag; /*use lock or not*/
    int lock_counter;
    int num_of_hash_key;
    int num_of_sort_key;
    int size_of_record;
    int size_of_table;
    int capacity;
    int load_count;
    int hash_prime_number;
};

enum SearchMethod{
    kHash = 0,
    kSort = 1
};
enum KeyType{
    kKeyTypeInt,
    kKeyTypeUnsignedInt,
    kKeyTypeLong,
    kKeyTypeUnsignedLong,
    kKeyTypeString
};
struct KeyComponent {
    KeyType type;
    int offset;
    int length;
};
// There are some rules for create key.
// I will try to document it latter.
// TODO: Documentation about how to use Key.
struct Key {
    KeyType type;
    SearchMethod method;
    int numOfKeyComponent; // Available range: 1-2
    struct KeyComponent components[2];
    char format[MAX_LEN_KEY_FORMAT];
};

// Key data which saved in shm
struct KeyInShm {
    Key key;
    int load_count;
};

struct DoublyLinkedListNode {
    int prev;
    int next;
};

int get_hash_prime_number(int);
int calculate_shm_size(int table_capacity, int num_of_hashkey, int num_of_sortkey);
bool shm_existed(const char *pathname, int proj_id, int shmflag = 0600);
int create_shm(const char *pathname, int proj_id, int size, int shmflag = 0600);
void* connect_shm(int shm_id);
bool release_shm(int shm_id);
int create_sem(const char *pathname, int proj_id, int shmflag = 0600);
bool release_sem(int sem_id);

// A set of functions for RW lock, use two semaphore and a counter
// help function lock() and unlock() are not declared in .h file
// NOTE: I use the almost most simple way to implement the RW lock
// No need to describe more here.
bool read_lock(struct ShmDescriptor* shm_descriptor, int sem_id);
bool read_unlock(struct ShmDescriptor* shm_descriptor, int sem_id);
bool write_lock(struct ShmDescriptor* shm_descriptor, int sem_id);
bool write_unlock(struct ShmDescriptor* shm_descriptor, int sem_id);

#endif // _LIB_SHMT_H_
