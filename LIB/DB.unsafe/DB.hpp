#ifndef __ATOM_BASE_DB_HPP__
#define __ATOM_BASE_DB_HPP__

#include <sys/types.h>

#include "FetchData.hpp"
#include "DebugMacro.hpp"

class DB
{
public:
    DB() {}
    DB(int _auto_retry_cnt, int _retry_connect_period) {}
    virtual ~DB() {}

    // Default Functions
    virtual bool Connect(const char *_ip,
                         int         _port,
                         const char *_user,
                         const char *_passwd,
                         const char * _dbname = "",
                         const char * _domain_path = "") = 0;
    virtual bool Connect() = 0;
    virtual bool IsConnect() = 0;
    virtual void Close() = 0;

    virtual int Execute(const char *_sql, size_t _le) = 0;
    virtual int Query(const char *_sql, size_t _len, bool _is_stored = true) = 0;
    virtual void * MoveResultOwnership() = 0;

    // Sub Functions
    virtual const char * GetErrorMsg() = 0;
    virtual unsigned int GetError() = 0;

    virtual void SetAutoCommit(bool _flag = true) = 0;
    virtual bool Commit() = 0;
    virtual bool Rollback() = 0;
    virtual void SetAutoRetryCnt(int _cnt) = 0;
    virtual void SetConnectPeroid(int _period) = 0;

};

#endif