/**
  * Use printf to do function test and debug maybe okay, but it's not a good
  * idea in a serious product.
  * Here I defind shmt_log MACRO and use printf to output logs.
  * If you want to take my library in your product, use you own log finction
  * to replace my MACRO.
  * I recommend my another library tool-log for asynchronous log/trace:
  * https://github.com/scott-zhou/tool-log
  */

#ifndef _LIB_SHM_TABLE_H_
#define _LIB_SHM_TABLE_H_

enum InmdbLogLevel {
    LOGCRITICAL = 1,
    LOGMINOR    = 2,
    LOGMAJOR    = 3,
    LOGWARN     = 4,
    LOGDEBUG    = 5
};

#define shmt_log(level, format,...) printf("%s:%d:%s LEVEL %d: "format"\n",__FILE__,__LINE__,__FUNCTION__,level,__VA_ARGS__)

#endif  //_LIB_SHM_TABLE_H_
