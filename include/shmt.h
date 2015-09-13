#ifndef _LIB_SHMT_H_
#define _LIB_SHMT_H_

int create_shm(const char *pathname, int proj_id, int size, int shmflag = 0600);
int create_sem(const char *pathname, int proj_id, int semNum, int shmflag = 0600);

#endif // _LIB_SHMT_H_
