#ifndef __MARIA_DB_HPP__
#define __MARIA_DB_HPP__

#include <string>

#include <mysql.h>
#include <errmsg.h>

#include "TimeOutCheck.hpp"

#include "DB.hpp"

class MariaDB : public DB
{
public:
    MariaDB();
    MariaDB(int _auto_retry_cnt, int _retry_connect_period);
    ~MariaDB();

    // Default Functions
    bool Connect();
    bool Connect(const char *_ip,
                 int         _port,
                 const char *_user,
                 const char *_passwd,
                 const char *_dbname="",
                 const char *_domain_path="");
    bool IsConnect() { return bConnected_; }
    void Close();

    int Execute(const char *_sql, size_t _len);
    int Query(const char *_sql, size_t _len, bool _is_stored = true);

    void FreeResult();
    void * MoveResultOwnership();

    // Sub Functions
    const char * GetErrorMsg() { return mysql_error(&H_); }
    unsigned int GetError() { return mysql_errno(&H_); }

    void SetAutoCommit(bool _flag = true);
    bool Commit();
    bool Rollback();
    void SetAutoRetryCnt(int _cnt) { auto_retry_cnt_ = _cnt; }
    void SetConnectPeroid(int _period) { retry_connect_period_ = _period; }



private:
    bool init();
    int  execute(const char *_sql, size_t _len);

private:
    MYSQL *         pH_;
    MYSQL           H_;

    MYSQL_RES *     Res_;

    bool            bConnected_;

    TimeOutCheck    timer_;

    std::string     ip_;
    int             port_;
    std::string     user_;
    std::string     passwd_;
    std::string     dbname_;
    std::string     domain_path_;

    int             auto_retry_cnt_;
    int             now_retry_cnt_;
    int             retry_connect_period_;


};



#endif // __MARIA_DB_HPP__
