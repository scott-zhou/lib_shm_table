#ifndef _LIB_SHMT_TEST_DEF_H_
#define _LIB_SHMT_TEST_DEF_H_

#define IPC_PATH   "/tmp/inmdb_test_ipc"
#define IPC_ID     160
#define RECORD_NUM 100*1000

struct TestDataElement {
    int id;
    int number;
    char desc[32];
    char x;
    char y;
    char i_dont_know[32];
};

#endif //_LIB_SHMT_TEST_DEF_H_
