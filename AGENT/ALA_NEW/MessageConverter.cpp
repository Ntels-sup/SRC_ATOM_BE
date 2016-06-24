
#include "MessageConverter.hpp"

#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;

MessageConverter::
MessageConverter()
{

}

MessageConverter::
~MessageConverter()
{

}

bool MessageConverter::
Alarm(ST_AlarmEventMsg & _stAlarmEventMsg,
      std::string      & _apiMsg)
{
    _stAlarmEventMsg.message_   = "alarm";

    size_t first    = 0;
    size_t end      = _apiMsg.find('\0', first);

    try
    {
        _stAlarmEventMsg.location_.append(_apiMsg.substr(first).c_str());

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.code_              = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.target_            = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.value_             = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.complement_        = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.additional_text_   = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.prc_date_          = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.dst_yn_            = _apiMsg.substr(first).c_str();

    } catch(std::exception & e) {

        gAlmLog->WARNING("%-24s| Alarm - pasing fail",
            "MessageConverter");
        return false;
    }

    return true;

}

bool MessageConverter::
Process(ST_AlarmEventMsg & _stAlarmEventMsg,
        std::string      & _apiMsg)
{
    _stAlarmEventMsg.message_   = "process";
    _stAlarmEventMsg.code_      = ALRM_PROCESS_STATUS;

    size_t first    = 0;
    size_t end      = _apiMsg.find('\0', first);

    try
    {
        // worst status
        _stAlarmEventMsg.complement_= _apiMsg.substr(first).c_str();

        // proc no
        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.proc_no_   = atoi(_apiMsg.substr(first).c_str());

        // proc name
        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.location_.append(_apiMsg.substr(first).c_str());

        // proc status
        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.value_     = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.prc_date_  = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.dst_yn_    = _apiMsg.substr(first).c_str();

    } catch(std::exception & e) {

        gAlmLog->WARNING("%-24s| Process - pasing fail",
            "MessageConverter");
        return false;
    }

    return true;
}

bool MessageConverter::
Connect(ST_AlarmEventMsg & _stAlarmEventMsg,
        std::string      & _apiMsg)
{
    _stAlarmEventMsg.message_   = "connect";
    _stAlarmEventMsg.code_      = ALRM_APP_CONNECT;

    size_t first    = 0;
    size_t end      = _apiMsg.find('\0', first);

    try
    {
        // my ip
        _stAlarmEventMsg.location_  = _apiMsg.substr(first).c_str();

        // peer ip
        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.target_    = _apiMsg.substr(first).c_str();

        // service name
        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.complement_= _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.value_     = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.prc_date_  = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.dst_yn_    = _apiMsg.substr(first).c_str();

    } catch(std::exception & e) {

        gAlmLog->WARNING("%-24s| convertToAlarmEventMsg - pasing fail",
            "ConnectMsgHandler");
        return false;
    }

    return true;
}

bool MessageConverter::
Ping(ST_AlarmEventMsg    & _stAlarmEventMsg,
        std::string      & _apiMsg)
{
    _stAlarmEventMsg.message_   = "ping";
    _stAlarmEventMsg.code_      = ALRM_NETWORK_FAIL;

    size_t first    = 0;
    size_t end      = _apiMsg.find('\0', first);

    try
    {
        // proc name
        first   = end + 1;
        end     = _apiMsg.find('\0', first);

        // peer no
        _stAlarmEventMsg.additional_text_ = atoi(_apiMsg.substr(first).c_str());

        // my ip
        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.location_ = _apiMsg.substr(first).c_str();

        // peer ip
        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.target_   = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.value_    = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.prc_date_ = _apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        _stAlarmEventMsg.dst_yn_   = _apiMsg.substr(first).c_str();

    } catch(std::exception & e) {

        gAlmLog->WARNING("%-24s| convertToAlarmMsg - pasing fail",
            "PingMsgHandler");
        return false;
    }

    return true;
}