#ifndef __ATOM_BASE_DB_R_HPP__
#define __ATOM_BASE_DB_R_HPP__

#include <sys/types.h>

#include "DebugMacro.hpp"
#include "FetchData.hpp"

class DB_r
{
public:
    DB_r(bool _use_thread = false,
         int  _timeout = 5) {}
    DB_r(int  _auto_retry_cnt,
         int  _retry_connect_period,
         bool _use_thread = false,
         int  _timeout = 5) {}
    virtual ~DB_r() {}

    // Default Functions
    virtual bool Connect(const char *_ip,
                         int         _port,
                         const char *_user,
                         const char *_passwd,
                         const char * _dbname = "",
                         const char * _domain_path = "") = 0;
    virtual bool IsConnect() = 0;
    virtual void Close() = 0;

    virtual int Execute(const char  *_sql,
                        size_t       _len) = 0;
    virtual int Query(FetchData    & _f,
                      const char   *_sql,
                      size_t        _len) = 0;
    virtual const char * GetErrString(int _ret);

    // Sub Functions
    virtual void SetAutoRetryCnt(int _cnt) = 0;
    virtual void SetConnectPeroid(int _period) = 0;
    virtual void SetAutoCommit(bool _flag);
    virtual void UsedThread() { use_thread_ = true; }
    virtual bool Commit();
    virtual bool Rollback();

};

#endif