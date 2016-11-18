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
#include <string>
extern "C" {
#include "shmt.h"
}

namespace lib_shm_table {

    enum CreateResult {
        kFail    = 0,
        kSuccess = 1,
        kExisted = 2
    };

    class Locker{
        enum Type{
            kNone, // Don't use kNone to init a locker!!
            kRead,
            kWrite
        };
    public:
        Locker(bool use_locker, Type type, void* shm_p, int sem_id)
            : use_locker_(use_locker), cur_locker_type_(type), p(shm_p), id(sem_id)
        {
            if(use_locker && kWrite == type) {
                if(!write_lock(shm_p, sem_id)) {
                    throw kNone;
                }
            }
            else if(use_locker && kRead == type) {
                if(!read_lock(shm_p, sem_id)) {
                    throw kNone;
                }
            }
            else if(use_locker) {
                throw kNone;
            }
        }
        ~Locker() {
            if(use_locker_ && kWrite == cur_locker_type_) {
                write_unlock(p, id);
            }
            else if(use_locker_ && kRead == cur_locker_type_) {
                read_unlock(p, id);
            }
        }
    private:
        bool use_locker_;
        Type cur_locker_type_;
        void* p;
        int id;
    };


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
//typedefs and enums

//consts

public:
    // Construct
    // Parameters will be passed to POSIX ftok() and shmget() function.
    // They have exact same meaning with POSIX definition.
    // Please refer to POSIX api help page for details.
    Table(const char *ipc_pathname, int ipc_proj_id, int shmflag = 0600);

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
                        const std::vector<struct Key> & hashKeys = std::vector<struct Key>(),
                        const std::vector<struct Key> & sortKeys = std::vector<struct Key>());

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

    // Return true if the shm and sem is successfully destroied.
    bool destroy();

    // Return the size for shared memory
    static size_t calculateSize(int table_capacity, int num_of_hashkey, int num_of_sortkey);

    // Insert data into table, return data index, -1 means fail.
    int insert(T& data);

private:
    static CreateResult convert_create_shm_result(int result);
    std::string ipc_pathname_;
    int ipc_proj_id_;
    int shmflag_;
    int shm_id_;
    int sem_id_;
    void* shm_p_;
};

template<class T>
Table<T>::Table(const char *ipc_pathname,int ipc_proj_id,int shmflag /*= 0600*/) :
    ipc_pathname_(ipc_pathname),
    ipc_proj_id_(ipc_proj_id),
    shmflag_(shmflag),
    shm_id_(-1),
    sem_id_(-1),
    shm_p_(NULL)
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
                              const std::vector<struct Key> & hashKeys /*= std::vector<struct Key>()*/,
                              const std::vector<struct Key> & sortKeys /*= std::vector<struct Key>()*/)
{
    if( shm_existed(ipc_pathname_.c_str(), ipc_proj_id_, shmflag_) ) {
        return kExisted;
    }
    size_t size = calculateSize(table_capacity, hashKeys.size(), sortKeys.size());
    shm_id_ = create_shm(ipc_pathname_.c_str(), ipc_proj_id_, size, shmflag_);
    if (-1 == shm_id_) {
        return kFail;
    }
    shm_p_ = connect_shm(shm_id_);
    if(NULL == shm_p_) {
        release_shm(shm_id_);
        return kFail;
    }
    sem_id_ = create_sem(ipc_pathname_.c_str(), ipc_proj_id_, shmflag_);
    if(sem_id_ == -1) {
        release_shm(shm_id_);
        shm_id_ = -1;
        shm_p_ = NULL;
        return kFail;
    }
    if(!init_shm(shm_p_, table_capacity, size, hashKeys.size(), sortKeys.size())) {
        release_shm(shm_id_);
        release_sem(sem_id_);
        shm_id_ = -1;
        sem_id_ = -1;
        shm_p_ = NULL;
        return kFail;
    }
    for(std::vector<struct Key>::size_type i = 0; i != hashKeys.size(); i++) {
        add_hashkey(shm_p_, i, &hashKeys[i]);
    }
    for(std::vector<struct Key>::size_type i = 0; i != sortKeys.size(); i++) {
        add_sortkey(shm_p_, i, &sortKeys[i]);
    }
    return kSuccess;
}

template<class T>
size_t Table<T>::calculateSize(int table_capacity, int num_of_hashkey, int num_of_sortkey)
{
    return calculate_shm_size(table_capacity, sizeof(T), num_of_hashkey, num_of_sortkey);
}

template<class T>
CreateResult Table<T>::convert_create_shm_result(int result)
{
    switch (result) {
        case 0:
            return kFail;
        case 1:
            return kSuccess;
        case 2:
            return kExisted;
        default:
            return kFail;
    }
}

template<class T>
bool Table<T>::connect()
{
    if( !shm_existed(ipc_pathname_.c_str(), ipc_proj_id_, shmflag_) ) {
        return false;
    }
    if(shm_id_ == -1) {
        shm_id_ = get_shm_id(ipc_pathname_.c_str(), ipc_proj_id_, shmflag_);
    }
    if(sem_id_ == -1) {
        sem_id_ = get_sem_id(ipc_pathname_.c_str(), ipc_proj_id_, shmflag_);
    }
    shm_p_ = connect_shm(shm_id_);
    if(NULL == shm_p_) {
        return false;
    }
    return true;
}

template<class T>
bool Table<T>::isConnected()
{
    if (NULL == shm_p_ && shm_id_ != -1 && sem_id_ != -1) {
        return false;
    }
    return true;
}

template<class T>
bool Table<T>::detach()
{
    bool rt = detach_shm(shm_p_);
    shm_p_ = NULL;
    return rt;
}

template<class T>
bool Table<T>::destroy()
{
    if(!isConnected()) {
        return false;
    }
    detach();
    if(release_shm(shm_id_)) {
        shm_id_ = -1;
    }
    if(release_sem(sem_id_)) {
        sem_id_ = -1;
    }
    return shm_id_ == -1 && sem_id_ == -1;
}

template<class T>
void Table<T>::useLock(bool use)
{
    set_lock_flag(shm_p_, use);
}

template<class T>
bool Table<T>::isUseLock()
{
    return get_lock_flag(shm_p_);
}

template <class T>
int Table<T>::insert(T& data){
    Locker locker(isUseLock(), Locker::kWrite, shm_p_, sem_id_);
    return add_element(shm_p_, &data, sizeof(T));
}

} //namespace lib_shm_table
#endif // LIB_SHM_TABLE_H_
