
#include <string.h>

#include "DBConfig.hpp"

DBConfig::
DBConfig(int _timeout) : timeout_(_timeout)
{
    pthread_mutex_init(&mutex_, NULL);
    clock_gettime(CLOCK_REALTIME, &tp_);
    db_             = NULL;
}

DBConfig::
~DBConfig()
{
    Lock();
    if(db_ != NULL)
    {
        delete db_;
        db_ = NULL;
    }
    UnLock();

    pthread_mutex_destroy(&mutex_);
}

bool DBConfig::
Init(const char * _ip,
     const int    _port,
     const char * _user,
     const char * _pw,
     const char * _db)
{
    if(Lock() != true)
    {
        return false;
    }

    if(db_ != NULL)
    {
        delete db_;
        db_ = NULL;
    }

    db_ = new (std::nothrow) MariaDB();

    if(db_ == NULL)
    {
        UnLock();
        return false;
    }

    if(db_->Connect(_ip,
                    _port,
                    _user,
                    _pw,
                    _db) != true)
    {
        UnLock();
        return false;
    }

    UnLock();

    return true;
}

bool DBConfig::
Get(FetchMaria & _f,
    int        & _errno,
    const char * _sql)
{
    _errno = 0;

    if(Lock() != true)
    {
        _errno = DBConfig::eLOCK_FAIL;
        return false;
    }

    if(db_ == NULL)
    {
        _errno = DBConfig::eNULL_PTR;

        UnLock();
        return false;
    }

    if(db_->Query(_sql, strlen(_sql)) < 0)
    {
        _errno = db_->GetError();

        UnLock();
        return false;
    }

    _f.SetRes((MYSQL_RES *)db_->MoveResultOwnership());

    UnLock();
    return true;
}

bool DBConfig::
Lock()
{
    clock_gettime(CLOCK_REALTIME, &tp_);
    tp_.tv_sec += timeout_;

    if(pthread_mutex_timedlock(&mutex_, &tp_) == 0)
        return true;

    return false;
}

void DBConfig::
UnLock()
{
    pthread_mutex_unlock(&mutex_);
}

const char * DBConfig::
GetErrorMsg(const int _errno)
{
    switch(_errno)
    {
    case DBConfig::eLOCK_FAIL:  return "lock fail";
    case DBConfig::eNEW_FAIL:   return "new operator fail";
    case DBConfig::eNULL_PTR:   return "db handler is null";
    case 0:                     return "no error";
    default:                    return "you should check mariaDB error code";
    }
}

