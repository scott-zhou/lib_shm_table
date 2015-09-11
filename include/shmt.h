#ifndef _LIB_SHMT_H_
#define _LIB_SHMT_H_

int create_shm(const char *ipcPathName, int ipcid, int shmSize, int operatorFlag = 0600);
int create_sem(const char *ipcPathName, int ipcid, int semNum, int operatorFlag = 0600);

#endif // _LIB_SHMT_H_
