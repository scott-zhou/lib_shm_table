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
    if(table.connect()){
        printf("Connect shared memory success.\n");
    }
    else {
        printf("Connect shared memory fail.\n");
        return 1;
    }

    if(table.destroy()){
        printf("Destroy shared memory success.\n");
    }
    else {
        printf("Destroy shared memory fail.\n");
        return 1;
    }

    return 0;
}
