#ifndef _LIB_SHMT_H_
#define _LIB_SHMT_H_

#define MAX_LEN_KEY_FORMAT 32

struct ShmDescriptor{
    int shm_size;
    /*No need to save sem number, it shoule be always 2 for a rwlock*/
    bool lock_flag; /*use lock or not*/
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
int create_shm(const char *pathname, int proj_id, int size, int shmflag = 0600);
int create_sem(const char *pathname, int proj_id, int semNum, int shmflag = 0600);


#endif // _LIB_SHMT_H_
