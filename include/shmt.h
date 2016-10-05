#ifndef _LIB_SHMT_H_
#define _LIB_SHMT_H_

#include <stdbool.h>
#include <stddef.h>

#define MAX_LEN_KEY_FORMAT  32
#define SEM_NUM_FOR_RWLOCK  2
#define READ_LOCK           0
#define WRITE_LOCK          1

struct ShmDescriptor{
    size_t shm_size;
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
    kKeyTypeInt8,
    kKeyTypeInt16,
    kKeyTypeInt32,
    kKeyTypeInt64,
    kKeyTypeString
};
struct KeyComponent {
    enum KeyType type;
    int offset;
    int length;
};
// There are some rules for create key.
// I will try to document it latter.
// TODO: Documentation about how to use Key.
struct Key {
    enum KeyType type;
    enum SearchMethod method;
    int numOfKeyComponent; // Available range: 1-2
    struct KeyComponent components[2];
    char format[MAX_LEN_KEY_FORMAT];
};

// Key data which saved in shm
struct KeyInShm {
    struct Key key;
    int load_count;
};

enum DoublyLinkedListDefine{
    kUsedHead = 0,
    kUnuseHead = 1
};

struct DoublyLinkedListNode {
    int prev;
    int next;
};

int get_hash_prime_number(int table_capacity);
size_t calculate_shm_size(int table_capacity, int element_size, int num_of_hashkey, int num_of_sortkey);
bool shm_existed(const char *pathname, int proj_id, int shmflag/*= 0600*/);
int create_shm(const char *pathname, int proj_id, size_t size, int shmflag/*= 0600*/);
int get_shm_id(const char *pathname, int proj_id, int shmflag/*= 0600*/);
int get_sem_id(const char *pathname, int proj_id, int shmflag/*= 0600*/);
void* connect_shm(int shm_id);
bool release_shm(int shm_id);
int create_sem(const char *pathname, int proj_id, int shmflag/*= 0600*/);
bool release_sem(int sem_id);
bool init_shm(void *p, int table_capacity, size_t size, int num_of_hash_key, int num_of_sort_key);
bool detach_shm(void *p);
bool add_hashkey(void* p, unsigned int id, const struct Key* key);
bool add_sortkey(void* p, unsigned int id, const struct Key* key);
void set_lock_flag(void* p, bool use_lock);
bool get_lock_flag(void* p);

// Get all components points in shm
struct ShmDescriptor* getp_shm_descriptor(void *p);
struct KeyInShm* getp_hashkey(void *p, int key_id);
struct KeyInShm* getp_sortkey(void *p, int key_id);
int * getp_hash_table(void *p, int key_id);
int * getp_sort_table(void *p, int key_id);
void* getp_data(void *p);

// Doubly Linked List function family
struct DoublyLinkedListNode* dl_getp_head(void* p);
struct DoublyLinkedListNode* dl_getp_used_head(void* p);
struct DoublyLinkedListNode* dl_getp_unuse_head(void* p);
bool dl_list_init(void *p);
int dl_add_element(void* p); //Add one element to dl, return the position for new element, -1 for fail.

// A set of functions for RW lock, use two semaphore and a counter
// help function lock() and unlock() are not declared in .h file
// NOTE: I use the almost most simple way to implement the RW lock
// No need to describe more here.
bool read_lock(void* p, int sem_id);
bool read_unlock(void* p, int sem_id);
bool write_lock(void* p, int sem_id);
bool write_unlock(void* p, int sem_id);

// add / remove
int add_element(void* p, void* data, size_t size); // Return element position, -1 for fail.

#endif // _LIB_SHMT_H_
