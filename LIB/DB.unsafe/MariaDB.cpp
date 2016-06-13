
#include <poll.h>
#include <cstring>

#include "MariaDB.hpp"

MariaDB::
MariaDB()
{
    pH_                     = NULL;
    Res_                    = NULL;

    bConnected_             = false;

    port_                   = 0;

    auto_retry_cnt_         = 3;
    now_retry_cnt_          = 0;
    retry_connect_period_   = 5;
}

MariaDB::
MariaDB(int _auto_retry_cnt, int _retry_connect_period)
{
    pH_                     = NULL;
    Res_                    = NULL;

    bConnected_             = false;

    port_                   = 0;

    auto_retry_cnt_         = _auto_retry_cnt;
    now_retry_cnt_          = 0;
    retry_connect_period_   = _retry_connect_period;
}

MariaDB::
~MariaDB()
{
    Close();
}

bool MariaDB::
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

void MariaDB::
Close()
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

void MariaDB::
freeResult()
{
    if(Res_ != NULL)
    {
        mysql_free_result(Res_);
        Res_ = NULL;
    }
}

void *  MariaDB::
MoveResultOwnership()
{
    MYSQL_RES * res = Res_;

    Res_  = NULL;

    return (void *)res;
}

bool MariaDB::
Connect(const char * _ip,
        int          _port,
        const char * _user,
        const char * _passwd,
        const char * _dbname,
        const char * _domain_path)
{
    if(IsConnect() == true)
        return true;

    if(init() == false)
        return false;

    if(auto_retry_cnt_ < now_retry_cnt_)
    {
        DEBUG_PRINT("before TimeOut Check\n");
        if(timer_.TimeOut(retry_connect_period_) == false)
            return true;

        timer_.Update();
    }

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
        Close();

        if(auto_retry_cnt_ > now_retry_cnt_)
        {
            poll(NULL, 0, 30);
            Connect();
        }

        return false;
    }

    bConnected_     = true;
    now_retry_cnt_  = 0;

    SetAutoCommit(true);

    DEBUG_PRINT("mysql real connect success\n");
    return true;
}

int MariaDB::
Execute(const char *_sql, size_t _len)
{
    int rc = execute(_sql, _len);

    if(rc < 0)
        return rc;

    return int(mysql_affected_rows(pH_));
}

int MariaDB::
Query(const char *_sql, size_t _len, bool _is_stored)
{
    freeResult();

    int rc = execute(_sql, _len);

    if(rc < 0)
        return rc;

    if(_is_stored)
    {
        if((Res_ = mysql_store_result(pH_)) == NULL)
        {
            DEBUG_PRINT("Query - store result Fail [%s]\n", mysql_error(pH_));
            return -1;
        }

        return int(mysql_num_rows(Res_));
    }

    if((Res_ = mysql_use_result(pH_)) == NULL)
    {
        DEBUG_PRINT("Query - use result Fail [%s]\n", mysql_error(pH_));
        return -1;
    }

    return 1;
}

int MariaDB::
execute(const char *_sql, size_t _len)
{
    if(_len == 0)
    {
        DEBUG_PRINT("SQL stmt is NULL\n");
        return -1;
    }

    TRY_EXECUTE:
    int rc = mysql_real_query(pH_, _sql, _len);

    if(rc != 0)
    {
        switch(mysql_errno(pH_))
        {
            case CR_COMMANDS_OUT_OF_SYNC:
                DEBUG_PRINT("execute fail - [CR_COMMANDS_OUT_OF_SYNC]\n");
                return -1;

            case CR_SERVER_GONE_ERROR:
            case CR_SERVER_LOST:
            case CR_UNKNOWN_ERROR:
                DEBUG_PRINT("execute fail - [%s]\n", mysql_error(pH_));

                Close();

                if(Connect() == true)
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


bool MariaDB::
Commit()
{
    if(mysql_commit(pH_) != 0)
    {
        // DEBUG_PRINT("Commit fail [%s]\n", mysql_error(pH_));
        return false;
    }

    return true;
}

bool MariaDB::
Rollback()
{
    if(mysql_rollback(pH_) != 0)
    {
        DEBUG_PRINT("Rollback fail [%s]\n", mysql_error(pH_));
        return false;
    }

    DEBUG_PRINT("Rollback success [%s]\n", mysql_error(pH_));
    return true;
}

void MariaDB::
SetAutoCommit(bool _flag)
{
    if(IsConnect())
    {
        mysql_autocommit(pH_, _flag);
    }
}


bool MariaDB::
Connect()
{
    return Connect(ip_.c_str(),
                   port_,
                   user_.c_str(),
                   passwd_.c_str(),
                   dbname_.c_str(),
                   domain_path_.c_str());
}
