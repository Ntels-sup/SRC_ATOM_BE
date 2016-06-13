
#ifndef __MARIA_DB_R_HPP__
#define __MARIA_DB_R_HPP__

#include <sys/types.h>
#include <pthread.h>

#include <string>

#include <mysql.h>
#include <errmsg.h>

#include "DB.hpp"
#include "TimeOutCheck.hpp"

#include "DebugMacro.hpp"

class MariaDB : public DB
{
public:
    MariaDB();
    MariaDB(bool _use_thread, int _lock_timeout);
    ~MariaDB();

    // Default Functions
    int  Connect(const char * _ip,
                 int          _port,
                 const char * _user,
                 const char * _passwd,
                 const char * _dbname = "",
                 const char * _domain_path = "");

    bool IsConnect() { return bConnected_; }
    void Close();

    int Execute(const char  *_sql,
                size_t       _len);
    int Query(FetchData  * _f,
              const char *_sql,
              size_t      _len);

	unsigned int GetError();
    const char * GetErrorMsg(int _ret);

    // Sub Functions
    void SetAutoRetryCnt(int _cnt) { auto_retry_cnt_ = _cnt; }
    void SetConnectPeroid(int _period) { retry_connect_period_ = _period; }
    void SetAutoCommit(bool _flag);
    void UsedThread() { use_thread_ = true; }
    bool Commit();
    bool Rollback();

private:
    bool init();
    void close();
    void freeResult();
    int  connect();
    int  execute(const char *_sql, size_t _len);
    bool lock();
    void unlock();

private:
    bool                use_thread_;
    int                 lock_timeout_;
    pthread_mutex_t     mutex_;
    struct timespec     tp_;

    MYSQL *             pH_;
    MYSQL               H_;

    MYSQL_RES *         Res_;

    bool                bConnected_;

    TimeOutCheck        timer_;

    std::string         ip_;
    int                 port_;
    std::string         user_;
    std::string         passwd_;
    std::string         dbname_;
    std::string         domain_path_;

    int                 auto_retry_cnt_;
    int                 now_retry_cnt_;
    int                 retry_connect_period_;

    enum eErrorCode : int
    {
        eSUCCESS    = 1,
        eNO_ERROR   = 0,
        eLOCK_FAIL  = -100,
        eNEW_FAIL   = -101,
        eNULL_PTR   = -102,
        eCONN_FAIL  = -103,
        eCONN_TRY_BUSY = -104,
        eNO_STMT    = -105
    };
};

#endif // __MARIA_DB_R_HPP__
