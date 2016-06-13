
#include <poll.h>
#include <cstring>

#include "MariaDB.hpp"

MariaDB::
MariaDB(bool _use_thread, int _lock_timeout)
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

    use_thread_             = false;
    lock_timeout_           = 5;

    pthread_mutex_init(&mutex_, NULL);
    clock_gettime(CLOCK_REALTIME, &tp_);
}

MariaDB::
~MariaDB()
{
    Close();

    pthread_mutex_destroy(&mutex_);
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
    if(lock() == false)
        return ;

    close();
    unlock();
}

void MariaDB::
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


void MariaDB::
freeResult()
{
    if(Res_ != NULL)
    {
        mysql_free_result(Res_);
        Res_ = NULL;
    }
}

int MariaDB::
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

    int ret = connect();
    unlock();

    return ret;
}

int MariaDB::
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

            if(connect() > 0)
                return 1;
        }

        return -ret;
    }

    bConnected_     = true;
    now_retry_cnt_  = 0;

    SetAutoCommit(true);

    DEBUG_PRINT("mysql real connect success\n");

    return 1;
}

int MariaDB::
Execute(const char   * _sql,
        size_t         _len)
{
    if(lock() == false)
        return eLOCK_FAIL;

    int ret = eNO_ERROR;

    ret = execute(_sql, _len);

    if(ret != 0)
    {
        unlock();
        return -ret;
    }

    ret = int(mysql_affected_rows(pH_));
    unlock();

    return ret;
}

int MariaDB::
Query(FetchData  * _f,
      const char * _sql,
      size_t       _len)
{
    if(lock() == false)
        return eLOCK_FAIL;

    int ret = 0;

    freeResult();
    ret = execute(_sql, _len);

    if(ret != 0)
    {
        unlock();
        return -ret;
    }

    if((Res_ = mysql_store_result(pH_)) == NULL)
    {
        DEBUG_PRINT("Query - store result Fail [%s]\n", mysql_error(pH_));
        ret = int(mysql_errno(pH_));

        unlock();
        return -ret;
    }

    ret = int(mysql_num_rows(Res_));

    _f->SetResult(ret, (void *)Res_);
    Res_ = NULL;

    unlock();

    return ret;
}

int MariaDB::
execute(const char *_sql, size_t _len)
{
    if(_len == 0)
    {
        DEBUG_PRINT("SQL stmt is NULL\n");
        return eNO_STMT;
    }

    int rc = 0;

	if(IsConnect() == false)
	{
		if((rc = connect()) < 0)
			return rc;
	}

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

                if(connect() >= 0)
                    goto TRY_EXECUTE;

                break;

            default :
                DEBUG_PRINT("execute fail - [%s]\n", mysql_error(pH_));
                break;
        }
    }

    return rc;
}

const char * MariaDB::
GetErrorMsg(int _ret)
{
    // https://dev.mysql.com/doc/refman/5.6/en/error-messages-client.html

	if(use_thread_ == false)
		return mysql_error(&H_);

    if(_ret >= 0)
        return "no error";

    switch(_ret)
    {
    case MariaDB::eLOCK_FAIL:  return "lock fail";
    case MariaDB::eNEW_FAIL:   return "new operator fail";
    case MariaDB::eNULL_PTR:   return "db handler is null";
    case MariaDB::eCONN_TRY_BUSY: return "too short period to reconnect";
    case MariaDB::eNO_STMT:    return "sql stmt is null";
    case MariaDB::eNO_ERROR:   return "no error";
    }

    switch(-_ret)
    {
    case CR_UNKNOWN_ERROR:       return "CR_UNKNOWN_ERROR";
    case CR_SOCKET_CREATE_ERROR: return "CR_SOCKET_CREATE_ERROR";
    case CR_CONNECTION_ERROR:    return "CR_CONNECTION_ERROR";
    case CR_CONN_HOST_ERROR:     return "CR_CONN_HOST_ERROR";
    case CR_IPSOCK_ERROR:        return "CR_IPSOCK_ERROR";
    case CR_UNKNOWN_HOST:        return "CR_UNKNOWN_HOST";
    case CR_SERVER_GONE_ERROR:   return "CR_SERVER_GONE_ERROR";
    case CR_VERSION_ERROR:       return "CR_VERSION_ERROR";
    case CR_OUT_OF_MEMORY:       return "CR_OUT_OF_MEMORY";
    case CR_WRONG_HOST_INFO:     return "CR_WRONG_HOST_INFO";
    case CR_LOCALHOST_CONNECTION: return "CR_LOCALHOST_CONNECTION";
    case CR_TCP_CONNECTION:      return "CR_TCP_CONNECTION";
    case CR_SERVER_HANDSHAKE_ERR: return "CR_SERVER_HANDSHAKE_ERR";
    case CR_SERVER_LOST:         return "CR_SERVER_LOST";
    case CR_COMMANDS_OUT_OF_SYNC: return "CR_COMMANDS_OUT_OF_SYNC";
    case CR_NAMEDPIPE_CONNECTION: return "CR_NAMEDPIPE_CONNECTION";
    case CR_NAMEDPIPEWAIT_ERROR: return "CR_NAMEDPIPEWAIT_ERROR";
    case CR_NAMEDPIPEOPEN_ERROR: return "CR_NAMEDPIPEOPEN_ERROR";
    case CR_NAMEDPIPESETSTATE_ERROR: return "CR_NAMEDPIPESETSTATE_ERROR";
    case CR_CANT_READ_CHARSET:   return "CR_CANT_READ_CHARSET";
    case CR_NET_PACKET_TOO_LARGE: return "CR_NET_PACKET_TOO_LARGE";
    case CR_EMBEDDED_CONNECTION:  return "CR_EMBEDDED_CONNECTION";
    case CR_PROBE_SLAVE_STATUS:   return "CR_PROBE_SLAVE_STATUS";
    case CR_PROBE_SLAVE_HOSTS:    return "CR_PROBE_SLAVE_HOSTS";
    case CR_PROBE_SLAVE_CONNECT:  return "CR_PROBE_SLAVE_CONNECT";
    case CR_PROBE_MASTER_CONNECT: return "CR_PROBE_MASTER_CONNECT";
    case CR_SSL_CONNECTION_ERROR: return "CR_SSL_CONNECTION_ERROR";
    case CR_MALFORMED_PACKET:     return "CR_MALFORMED_PACKET";
    case CR_WRONG_LICENSE:        return "CR_WRONG_LICENSE";

    case CR_NULL_POINTER:         return "CR_NULL_POINTER";
    case CR_NO_PREPARE_STMT:      return "CR_NO_PREPARE_STMT";
    case CR_PARAMS_NOT_BOUND:     return "CR_PARAMS_NOT_BOUND";
    case CR_NO_PARAMETERS_EXISTS: return "CR_NO_PARAMETERS_EXISTS";
    case CR_INVALID_PARAMETER_NO: return "CR_INVALID_PARAMETER_NO";
    case CR_INVALID_BUFFER_USE:   return "CR_INVALID_BUFFER_USE";
    case CR_UNSUPPORTED_PARAM_TYPE:   return "CR_UNSUPPORTED_PARAM_TYPE";
    case CR_SHARED_MEMORY_CONNECTION: return "CR_SHARED_MEMORY_CONNECTION";
    case CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR:
        return "CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR";
    case CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR:
        return "CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR";
    case CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR:
        return "CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR";
    case CR_SHARED_MEMORY_CONNECT_MAP_ERROR:
        return "CR_SHARED_MEMORY_CONNECT_MAP_ERROR";
    case CR_SHARED_MEMORY_FILE_MAP_ERROR:
        return "CR_SHARED_MEMORY_FILE_MAP_ERROR";
    case CR_SHARED_MEMORY_MAP_ERROR:
        return "CR_SHARED_MEMORY_MAP_ERROR";
    case CR_SHARED_MEMORY_EVENT_ERROR:
        return "CR_SHARED_MEMORY_EVENT_ERROR";
    case CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR:
        return "CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR";
    case CR_SHARED_MEMORY_CONNECT_SET_ERROR:
        return "CR_SHARED_MEMORY_CONNECT_SET_ERROR";
    case CR_CONN_UNKNOW_PROTOCOL:
        return "CR_CONN_UNKNOW_PROTOCOL";
    case CR_INVALID_CONN_HANDLE: return "CR_INVALID_CONN_HANDLE";
    case CR_SECURE_AUTH:         return "CR_SECURE_AUTH";
    case CR_FETCH_CANCELED:      return "CR_FETCH_CANCELED";
    case CR_NO_DATA:             return "CR_NO_DATA";
    case CR_NO_STMT_METADATA:    return "CR_NO_STMT_METADATA";
    case CR_NO_RESULT_SET:       return "CR_NO_RESULT_SET";
    case CR_NOT_IMPLEMENTED:     return "CR_NOT_IMPLEMENTED";
    case CR_SERVER_LOST_EXTENDED: return "CR_SERVER_LOST_EXTENDED";
    case CR_STMT_CLOSED:         return "CR_STMT_CLOSED";
    case CR_NEW_STMT_METADATA:   return "CR_NEW_STMT_METADATA";
    case CR_ALREADY_CONNECTED:   return "CR_ALREADY_CONNECTED";
    case CR_AUTH_PLUGIN_CANNOT_LOAD:   return "CR_AUTH_PLUGIN_CANNOT_LOAD";
    case CR_DUPLICATE_CONNECTION_ATTR: return "CR_DUPLICATE_CONNECTION_ATTR";
    case CR_AUTH_PLUGIN_ERR:     return "CR_AUTH_PLUGIN_ERR";
    default:
        return "you should check mariaDB error code [this code is actually positive code]";
    }
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

bool MariaDB::
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


bool MariaDB::
lock()
{
    if(use_thread_ == false)
        return true;

    clock_gettime(CLOCK_REALTIME, &tp_);
    tp_.tv_sec += lock_timeout_;

    if(pthread_mutex_timedlock(&mutex_, &tp_) == 0)
        return true;

    return false;
}

void MariaDB::
unlock()
{
    if(use_thread_ == false)
        return ;

    pthread_mutex_unlock(&mutex_);
}

