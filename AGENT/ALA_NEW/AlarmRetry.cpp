
#include <poll.h>

#include "AlarmRetry.hpp"
#include "AlarmEncoding.hpp"
#include "CFileLog.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"


extern CFileLog * gAlmLog;

AlarmRetry::
AlarmRetry()
{
    is_on_          = false;
    my_node_no_     = -1;
    my_node_name_.clear();
    my_proc_no_     = -1;
    max_cnt_        = 30;

    is_on_          = false;
    req_seq_id_     = 0;
    now_seq_id_     = 0;
}

AlarmRetry::
~AlarmRetry()
{
    // Empty
}

bool AlarmRetry::
Init(int            _node_no,
     std::string &  _node_name,
     int            _proc_no)
{
    my_node_no_    = _node_no;
    my_node_name_  = _node_name;
    my_proc_no_    = _proc_no;

    return true;
}

bool AlarmRetry::
IsContinues()
{
    return is_on_;
}

bool AlarmRetry::
Request(std::string & _body, AuditData & _audit)
{
    // 현재 보내는 중인데 또 Retry 요청이 오면 어떻하죠?? 그럴리 없지만,
    // 만약에 온다면, 무시합니다. 또 보내겠죠.. 뭐..
    if(IsContinues() == true)
        return false;

    unsigned int id         = 0;
    int          node_no    = 0;

    // TO DO
    // JSON -> Start MSG ID
    try {

        rabbit::document    doc;
        doc.parse(_body);

        node_no     = doc["BODY"]["node_no"].as_int();
        id          = doc["BODY"]["req_seq_id"].as_uint();

    } catch(rabbit::type_mismatch & e) {

        gAlmLog->WARNING("%-24s| Request - type mismatch [%s]",
            "AlarmRetry",
            _body.c_str());
        return false;
    } catch(rabbit::parse_error & e) {

        gAlmLog->WARNING("%-24s| Request - parse error [%s]",
            "AlarmRetry",
            _body.c_str());
        return false;
    } catch(...) {
        gAlmLog->WARNING("%-24s| Request - [%s]",
            "AlarmRetry",
            _body.c_str());
        return false;
    }

    if(node_no != my_node_no_)
    {
        gAlmLog->WARNING("%-24s| Request - mismatch node no my[%d], recv[%d]",
            "AlarmRetry",
            my_node_no_,
            node_no);

        return false;
    }

    // 요청 값이 더 클 때는 어떻게 할지 고민해야 합니다.
    if(id == _audit.GetSequenceId())
    {
        gAlmLog->WARNING("%-24s| Request - already lasted sequence id [%u]",
            "AlarmRetry",
            id);

        return false;
    }

    retryOn(id);

    return true;
}

void AlarmRetry::
retryOn(unsigned int _id)
{
    is_on_          = true;

    req_seq_id_     = _id;
    now_seq_id_     = _id;
}

void AlarmRetry::
retryOff()
{
    is_on_ = false;

    req_seq_id_     = 0;
    now_seq_id_     = 0;
}


bool AlarmRetry::
Response(AlaInterface & _interface, AuditData & _audit)
{
    vec_msg_.clear();
    int cnt = _audit.ReadData(vec_msg_, now_seq_id_, max_cnt_);

    if(cnt == 0 && req_seq_id_ == now_seq_id_)
    {
        // 한번도 보내지도 못했는데, 찾지도 못했다는 뜻이네요..
        // 그러면, 닥치고 내가 가진 ID 로 작업하라고 합니다.

        std::string         body;
        ST_AlarmEventMsg    stAlarmEventMsg;

        stAlarmEventMsg.Clear();

        stAlarmEventMsg.message_        = "sequence id sync";
        stAlarmEventMsg.sequence_id_    = _audit.GetSequenceId();
        stAlarmEventMsg.node_no_        = my_node_no_;
        stAlarmEventMsg.node_name_      = my_node_name_;
        stAlarmEventMsg.proc_no_        = my_proc_no_;
        stAlarmEventMsg.severity_id_    = ALM::eCLEARED;

        AlarmEncoding::MakeBody(body, stAlarmEventMsg);
        _interface.SendRetryAlarm(body);
    }

    // 이미 가지고 있는 것이 최신일 수 있지 않을까요??
    // 그 때도 마찬가지로, cnt 는 0 일텐데요??
    // Request() 에서 막아요.

    if(cnt < max_cnt_)
        retryOff();

    for(auto iter = vec_msg_.begin(); iter != vec_msg_.end(); ++iter)
    {
        if(_interface.SendRetryAlarm(iter->second) == false)
        {
            gAlmLog->WARNING("%-24s| Response - can't send alarm [%s] seq id [%u]",
                "AlarmRetry",
                _interface.GetErrorMsg(),
                iter->first);

            // 살짝 쉽니다...
            poll(NULL, 0, 1);

            // Send 실패라는 것은, 아직 보내야 할 것이 있다는 뜻이죠.
            retryOn(iter->first);
            break;
        }

        gAlmLog->DEBUG("%-24s| Response - SUCCESS [%u] [%s]",
            "AlarmRetry",
            iter->first,
            (iter->second).c_str());

        now_seq_id_ = iter->first;
    }

    return true;
}



