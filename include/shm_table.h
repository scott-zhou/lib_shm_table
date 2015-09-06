// Copyright 2015 Scott Zhou
// The MIT License (MIT)
// Scott Zhou (www.scottzhou.me)
//
// This is the head file for table in shared memory.
// You can use template class lib_shm_table::table to save your
// own data struct elements into the table.
//
#ifndef _LIB_SHM_TABLE_H_
#define _LIB_SHM_TABLE_H_

#include <string>

namespace lib_shm_table {

// Template class for table in shared memory. Sample usage:
//    struct OwnStruct {
//        int index;
//        char key[8];
//        ...other members
//    };
//    lib_shm_table::Table<OwnStruct> OwnTable;
//    OwnTable myTable("/tmp/file", 0);
//    myTable.create(100000, 1, 0)
//    myTable.connect();
//    OwnStruct element;
//    element.??? = ???
//    myTable.insert(element);
//    myTable.detach()
template<class T>
class Table{
#typedefs and enums
    enum CreateResult {
        kFail    = 0,
        kSuccess = 1,
        kExisted = 2
    };
    enum SearchMethod{
        kHash = 0,
        kSort = 1
    };
    enum KeyType{
        kKeyTypeInt,
        kKeyTypeLong,
        kKeyTypeString,
    };
#consts
public:
    // Construct
    // Parameters will be passed to POSIX ftok() and shmget() function.
    // They have exact same meaning with POSIX definition.
    // Please refer to POSIX api help page for details.
    Table(const char *ipc_pathname,int ipc_proj_id,int shmflag = 0600);

    // Destruct
    ~Table();

    // Create shared memory and table data struct.
    // Parameters:
    //     table_capacity, the max number of elements in table
    //     num_of_hashkey, How many hash keys will be created in table, default 0
    //     num_of_sortkey, How many sort keys will be created in table, default 0
    // NOTES: Keys (if exist) must be created (use function addKey) immediately
    //        after table creatation.
    CreateResult create(int table_capacity,int num_of_hashkey = 0,int num_of_sortkey = 0);

    // The following addXxxKey functions are almost same function.
    // The only different is they are for different key types.
    // They will add a key to table and return true when success.
    // For each type of key (hash and sort), ID range is 0 ~ (max-1)
    // NOTES: Again, Keys must be created immediately after table creatation.
    bool addIntKey(SearchMethod search_method, int key_id,
                   int (*key_generator_func) (const T&));
    bool addLongKey(SearchMethod search_method, int key_id,
                    long (*key_generator_func) (const T&));
    bool addStringKey(SearchMethod search_method, int key_id,
                      std::string (*key_generator_func) (const T&));

    // Connect table to shared memory
    // Return true if succeed
    bool connect();

    // Detach table from shared memory
    // Return true if succeed
    bool detach();

    // Set if the table use lock to protect data save for multi thread(process)
    void useLock(bool use);

    // Returns true if the table use lock to protect data save for multi thread(process)
    bool isUseLock();

private:
};

} //namespace lib_shm_table
#endif // LIB_SHM_TABLE_H_
