#ifndef __ATOM_BASE_DB_R_HPP__
#define __ATOM_BASE_DB_R_HPP__

#include <sys/types.h>

#include "DebugMacro.hpp"
#include "FetchData.hpp"

class DB
{
public:
    DB() {}
    DB(bool _use_thread, int _lock_timeout) {}
    virtual ~DB() {}

    // Default Functions
    virtual int Connect(const char *_ip,
                         int         _port,
                         const char *_user,
                         const char *_passwd,
                         const char * _dbname = "",
                         const char * _domain_path = "") = 0;
    virtual bool IsConnect() = 0;
    virtual void Close() = 0;

    virtual int Execute(const char  *_sql,
                        size_t       _len) = 0;
    virtual int Query(FetchData    * _f,
                      const char   *_sql,
                      size_t        _len) = 0;

    virtual const char * GetErrorMsg(int _ret) = 0;

    // Sub Functions
    virtual void SetAutoRetryCnt(int _cnt) = 0;
    virtual void SetConnectPeroid(int _period) = 0;
    virtual void SetAutoCommit(bool _flag) = 0;
    virtual void UsedThread() = 0;
    virtual bool Commit() = 0;
    virtual bool Rollback() = 0;

};

#endif
