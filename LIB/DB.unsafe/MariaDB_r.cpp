
#include <poll.h>
#include <cstring>

#include "MariaDB_r.hpp"

MariaDB_r::
MariaDB_r(bool _use_thread, int _lock_timeout)
{
    pH_                     = NULL;
    Res_                    = NULL;

    bConnected_             = false;

    port_                   = 0;

    auto_retry_cnt_         = 3;
    now_retry_cnt_          = 0;
    retry_connect_period_   = 5;

    use_thread_             = _use_thread;
    lock_timeout_           = _lock_timeout;

    pthread_mutex_init(&mutex_, NULL);
    clock_gettime(CLOCK_REALTIME, &tp_);
}

MariaDB_r::
MariaDB_r(int  _auto_retry_cnt,
          int  _retry_connect_period,
          bool _use_thread,
          int  _lock_timeout)
{
    pH_                     = NULL;
    Res_                    = NULL;

    bConnected_             = false;

    port_                   = 0;

    auto_retry_cnt_         = _auto_retry_cnt;
    now_retry_cnt_          = 0;
    retry_connect_period_   = _retry_connect_period;

    use_thread_             = _use_thread;
    lock_timeout_           = _lock_timeout;

    pthread_mutex_init(&mutex_, NULL);
    clock_gettime(CLOCK_REALTIME, &tp_);
}

MariaDB_r::
~MariaDB_r()
{
    Close();

    pthread_mutex_destroy(&mutex_);
}

bool MariaDB_r::
init()
{
    if(mysql_init(&H_) == NULL)
    {
        DEBUG_PRINT("DB Init Fail\n");
        return false;
    }

    if(mysql_options(&H_, MYSQL_SET_CHARSET_NAME, "utf8") != 0)
    {
        DEBUG_PRINT("can't set mysql-option [%s]\n", "utf8");
    }

    DEBUG_PRINT("Init Success\n");

    return true;
}

void MariaDB_r::
Close()
{
    if(lock() == false)
        return ;

    close();
    unlock();
}

void MariaDB_r::
close()
{
    if(pH_)
    {
        freeResult();
        mysql_close(pH_);

        pH_          = NULL;
        bConnected_  = false;

        DEBUG_PRINT("DB Close\n");
    }
}


void MariaDB_r::
freeResult()
{
    if(Res_ != NULL)
    {
        mysql_free_result(Res_);
        Res_ = NULL;
    }
}

int MariaDB_r::
Connect(const char * _ip,
        int          _port,
        const char * _user,
        const char * _passwd,
        const char * _dbname,
        const char * _domain_path)
{
    if(lock() == false)
        return eLOCK_FAIL;

    if(ip_.compare(_ip) != 0)
        ip_     = std::string(_ip);
    if(_port != 0 && port_ != _port)
        port_   = _port;
    if(user_.compare(_user) != 0)
        user_   = std::string(_user);
    if(passwd_.compare(_passwd) != 0)
        passwd_ = std::string(_passwd);
    if(dbname_.compare(_dbname) != 0)
        dbname_ = std::string(_dbname);
    if(domain_path_.compare(_domain_path) != 0)
        domain_path_ = std::string(_domain_path);

    ret = connect();
    unlock();

    return ret;
}

int MariaDB_r::
connect()
{
    if(IsConnect() == true)
        return 1;

    if(init() == false)
        return eCONN_FAIL;

    if(auto_retry_cnt_ < now_retry_cnt_)
    {
        DEBUG_PRINT("before TimeOut Check\n");
        if(timer_.TimeOut(retry_connect_period_) == false)
            return eCONN_TRY_BUSY;

        timer_.Update();
    }

    if((pH_ = mysql_real_connect(&H_,
                                  ip_.c_str(),
                                  user_.c_str(),
                                  passwd_.c_str(),
                                  dbname_.c_str(),
                                  port_,
                                  domain_path_.c_str(),
                                  0)) == NULL)
    {
        now_retry_cnt_++;

        DEBUG_PRINT("mysql real connect fail [%s]\n", mysql_error(&H_));
        int ret = int(mysql_errno(&H_));

        close();

        if(auto_retry_cnt_ > now_retry_cnt_)
        {
            poll(NULL, 0, 30);

            if(connect(_ret) == true)
                return 1;
        }

        return ret;
    }

    bConnected_     = true;
    now_retry_cnt_  = 0;
    _ret            = 0;

    SetAutoCommit(true);

    DEBUG_PRINT("mysql real connect success\n");

    return 1;
}

int MariaDB_r::
Execute(const char   * _sql,
        size_t         _len)
{
    if(lock() == false)
        return eLOCK_FAIL;

    int ret = eNO_ERROR;

    if(pH_ == NULL)
    {
        ret = eNULL_PTR;
        unlock();

        return ret;
    }

    ret = execute(_sql, _len);

    if(ret != 0)
    {
        unlock();
        return ret;
    }

    ret = int(mysql_affected_rows(pH_));
    unlock();

    return ret;
}

int MariaDB_r::
Query(FetchData  & _f,
      const char * _sql,
      size_t       _len)
{
    if(lock() == false)
        return eLOCK_FAIL;

    int ret = 0;
    if(pH_ == NULL)
    {
        ret = eNULL_PTR;
        unlock();

        return ret;
    }

    freeResult();
    ret = execute(_sql, _len);

    if(ret != 0)
    {
        unlock();
        return ret;
    }

    if((Res_ = mysql_store_result(pH_)) == NULL)
    {
        DEBUG_PRINT("Query - store result Fail [%s]\n", mysql_error(pH_));
        ret = int(mysql_errno(pH_));

        unlock();
        return -ret;
    }

    ret = int(mysql_num_rows(Res_));

    _f.SetResult(ret, (void *)Res_);
    Res_ = NULL;

    unlock();

    return ret;
}

int MariaDB_r::
execute(const char *_sql, size_t _len)
{
    if(_len == 0)
    {
        DEBUG_PRINT("SQL stmt is NULL\n");
        return eNO_STMT;
    }

    int rc = 0;
    int ret= 0;

    TRY_EXECUTE:
    rc = mysql_real_query(pH_, _sql, _len);

    if(rc != 0)
    {
        switch(mysql_errno(pH_))
        {
            case CR_COMMANDS_OUT_OF_SYNC:
                DEBUG_PRINT("execute fail - [CR_COMMANDS_OUT_OF_SYNC]\n");
                return CR_COMMANDS_OUT_OF_SYNC;

            case CR_SERVER_GONE_ERROR:
            case CR_SERVER_LOST:
            case CR_UNKNOWN_ERROR:
                DEBUG_PRINT("execute fail - [%s]\n", mysql_error(pH_));

                close();

                if(connect(ret) >= 0)
                {
                    goto TRY_EXECUTE;
                }

                break;

            default :
                DEBUG_PRINT("execute fail - [%s]\n", mysql_error(pH_));
                break;
        }
    }

    return rc;
}

const char * MariaDB_r::
GetErrString(int _ret)
{
    switch(_ret)
    {
    case MariaDB_r::eLOCK_FAIL:  return "lock fail";
    case MariaDB_r::eNEW_FAIL:   return "new operator fail";
    case MariaDB_r::eNULL_PTR:   return "db handler is null";
    case MariaDB_r::eNO_ERROR:   return "no error";
    default:                     return "you should check mariaDB error code";
    }
}

void MariaDB_r::
SetAutoCommit(bool _flag)
{
    if(IsConnect())
    {
        mysql_autocommit(pH_, _flag);
    }
}

bool MariaDB_r::
Commit()
{
    if(use_thread_)
        return true;

    if(mysql_commit(pH_) != 0)
    {
        // DEBUG_PRINT("Commit fail [%s]\n", mysql_error(pH_));
        return false;
    }

    return true;
}

bool MariaDB_r::
Rollback()
{
    if(use_thread_)
        return false;

    if(mysql_rollback(pH_) != 0)
    {
        DEBUG_PRINT("Rollback fail [%s]\n", mysql_error(pH_));
        return false;
    }

    DEBUG_PRINT("Rollback success [%s]\n", mysql_error(pH_));
    return true;
}


bool MariaDB_r::
lock()
{
    if(_use_thread == false)
        return true;

    clock_gettime(CLOCK_REALTIME, &tp_);
    tp_.tv_sec += lock_timeout_;

    if(pthread_mutex_timedlock(&mutex_, &tp_) == 0)
        return true;

    return false;
}

void MariaDB_r::
unlock()
{
    if(_use_thread == false)
        return ;

    pthread_mutex_unlock(&mutex_);
}

