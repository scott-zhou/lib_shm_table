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
        kStrKeyFromStrPlusInt = 0,//(str+int)
        kStrKeyFromIntPlusStr,//(int+str)
        kStrKeyFromStrPlusStr,//(str+str)
        kStrKeyFromIntPlusInt,//(int+int)
        kStrKeyFromOnlyStr,   //(str), only have first key field
        kStrKeyFromOnlyInt,   //(int), only have first key field
        kIntKeyFromOnlyInt,   //(int), only have first key field
        kBitKeyOneField,      //Only use first field bits
        kBitKeyTwoField       //Use two fields bits
    };
#consts
public:
    Table(const char *ipc_path_name,int ipcid,int operator_flag = 0666);
    ~Table();

    //
    CreateResult create(int table_capacity,int num_of_hashkey = 0,int num_of_sortkey = 0);

    //
    void addKey(int keyid,
                int field_1_offset,
                int field_1_length,
                int field_2_offset,
                int field_2_length,
                SearchMethod search_method,
                KeyType key_type,
                const char *key_format="");

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
