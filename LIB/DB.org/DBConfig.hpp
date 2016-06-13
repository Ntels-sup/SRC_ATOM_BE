
#ifndef __DB_CONFIG_HPP__
#define __DB_CONFIG_HPP__

#include <pthread.h>
#include <time.h>

#include "DB.hpp"
#include "MariaDB.hpp"
#include "FetchMaria.hpp"

class DBConfig
{
public:
    explicit DBConfig(int _timeout = 5);
    ~DBConfig();

    bool Init(const char * _ip,
              const int    _port,
              const char * _user,
              const char * _pw,
              const char * _db);

    bool Get(FetchMaria & _f,
             int        & _errno,
             const char * _sql);

    bool Lock();
    void UnLock();

    const char * GetErrorMsg(const int _errno);

public:
    enum eErrorCode : int
    {
        eLOCK_FAIL  = 100,
        eNEW_FAIL   = 101,
        eNULL_PTR   = 102
    };

private:

    int                 timeout_;
    pthread_mutex_t     mutex_;
    struct timespec     tp_;

    DB                * db_;

};

#endif // __DB_CONFIG_HPP__
