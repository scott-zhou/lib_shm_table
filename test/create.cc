#include "test_def.h"
#include <shm_table.h>
#include <stdio.h>

int main(int, char* [])
{
    lib_shm_table::Table<TestDataElement> table(IPC_PATH, IPC_ID, 0600);
    /*
    std::vector<Key> hashKeys;
    std::vector<Key> sortKeys;
    */
    lib_shm_table::CreateResult result =
            table.create(RECORD_NUM/*, hashKeys, sortKeys*/);
    if(lib_shm_table::kSuccess == result) {
        printf("Create shared memory success.\n");
    }
    else if(lib_shm_table::kExisted == result) {
        printf("Create shared memory but it is existed.\n");
    }
    else if(lib_shm_table::kFail == result) {
        printf("Create shared memory fail.\n");
        return 1;
    }
    return 0;
}
