#ifndef _LIB_SHM_TABLE_H_
#define _LIB_SHM_TABLE_H_
namespace libshmt {

template<class T>
class table{
#typedefs and enums
    enum createResult {
        FAIL    = 0,
        SUCCESS = 1,
        EXISTED = 2
    };
#consts
public:
    table(const char *ipcPathName,int ipcid,int operatorFlag = 0666);
    ~table();
    createResult create(int tablecapacity,int numofhashkey = 0,int numofsortkey = 0);
    bool connect();
    bool detatchShm(void);
private:
};

}
#endif // LIB_SHM_TABLE_H_
