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

#include <vector>

namespace lib_shm_table {

// Template class for table in shared memory. Sample usage:
//    struct OwnStruct {
//        int index;
//        char key[8];
//        ...other members
//    };
//    lib_shm_table::Table<OwnStruct> OwnTable;
//    OwnTable myTable("/tmp/file", 0);
//    myTable.create(100000)
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
        KeyComponent components[2];
        char format[kMaxLenKeyFormat];
    };
#consts
    static const unsigned int kMaxLenKeyFormat = 32;
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
    //     hashKeys, A list of hash keys in table, default no hash key
    //     sortKeys, A list of sort keys in table, default no sort key
    // NOTES: Keys (if exist) must be created (use function addKey) immediately
    //        after table creatation.
    CreateResult create(int table_capacity,
                        const std::vector<Key> & hashKeys = std::vector<Key>(),
                        const std::vector<Key> & sortKeys = std::vector<Key>());

    // Connect table to shared memory
    // Return true if succeed
    bool connect();

    // Return if the instance is connected to shared memory or not.
    bool isConnected();

    // Detach table from shared memory
    // Return true if succeed
    bool detach();

    // Set if the table use lock to protect data save for multi thread(process)
    void useLock(bool use);

    // Returns true if the table use lock to protect data save for multi thread(process)
    bool isUseLock();

private:
    std::string ipc_pathname_;
    int ipc_proj_id_;
    int shmflag_;
};

template<class T>
Table<T>::Table(const char *ipc_pathname,int ipc_proj_id,int shmflag /*= 0600*/) :
    ipc_pathname_(ipc_pathname),
    ipc_proj_id_(ipc_proj_id),
    shmflag_(shmflag)
{
}

template<class T>
Table<T>::~Table()
{
    if(isConnected()) {
        detach();
    }
}

template<class T>
CreateResult Table<T>::create(int table_capacity,
                              const std::vector<Key> & hashKeys /*= std::vector<Key>()*/,
                              const std::vector<Key> & sortKeys /*= std::vector<Key>()*/)
{
    CreateResult result = kSuccess;
    return result;
}

} //namespace lib_shm_table
#endif // LIB_SHM_TABLE_H_
