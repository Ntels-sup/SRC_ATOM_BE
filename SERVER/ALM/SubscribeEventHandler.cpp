
#include "SubscribeEventHandler.hpp"
#include "AlarmCFG.hpp"

#include "FetchMaria.hpp"

#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;
extern AlarmCFG     gCFG;

SubscribeEventHandler::
SubscribeEventHandler()
{
    buffer_.resize(gCFG.DEF.buffer_size_);
}

SubscribeEventHandler::
~SubscribeEventHandler()
{
    // Empty
}

bool SubscribeEventHandler::
Init(DB * _db)
{
    char sql[128];
    sprintf(sql, "SELECT PKG_NAME, CODE, SEVERITY_CCD, REQ_NODE_NO, REQ_PROC_NO "
                 " FROM TAT_ALM_SUBSCRIBE");

    int ret = 0;
    FetchMaria  f;
    if((ret = _db->Query(&f, sql, strlen(sql))) < 0)
    {
        gAlmLog->ERROR("%-24s| Init - Query Fail [%d:%s] [%s]\n",
            "SubscribeEventHandler",
			ret,
			_db->GetErrorMsg(ret),
            sql);
        return false;
    }

    char    pkg_name[16];
    char    code[32];
    char    severity_id[4];
    char    node_no[32];
    char    proc_no[32];

    f.Clear();
    f.Set(pkg_name,     sizeof(pkg_name));
    f.Set(code,         sizeof(code));
    f.Set(severity_id,  sizeof(severity_id));
    f.Set(node_no,      sizeof(node_no));
    f.Set(proc_no,      sizeof(proc_no));

    while(true)
    {
        if(f.Fetch() != true)
            break;

        ST_Subscriber   stSubscriber;
        stSubscriber.pkg_name_    = pkg_name;
        stSubscriber.code_        = code;
        stSubscriber.severity_id_ = atoi(severity_id);
        stSubscriber.node_no_     = atoi(node_no);
        stSubscriber.proc_no_     = atoi(proc_no);

        addSubscriber(stSubscriber);
    }

    gAlmLog->INFO("%-24s| Init - SUCCESS",
        "SubscribeEventHandler");

    return true;
}

void SubscribeEventHandler::
addSubscriber(ST_Subscriber & _stSubscriber)
{
    auto iter = map_.find(_stSubscriber.code_);

    if(iter == map_.end())
    {
        std::list<ST_Subscriber>    list;
        list.push_back(_stSubscriber);

        map_[_stSubscriber.code_]     = list;
    }
    else
    {
        (iter->second).push_back(_stSubscriber);
    }
}

void SubscribeEventHandler::
delSubscriber(ST_Subscriber & _stSubscriber)
{
    auto iter = map_.find(_stSubscriber.code_);

    if(iter != map_.end())
    {
        std::list<ST_Subscriber> & list = iter->second;

        for(auto list_iter = list.begin(); list_iter != list.end(); )
        {
            ST_Subscriber & stRegSubscriber = *list_iter;

            // 주의 : Compare 에서는, severity 값은 확인하지 않습니다.
            if(_stSubscriber.Compare(stRegSubscriber) == true)
                list_iter = list.erase(list_iter);
            else
                ++list_iter;
        }
    }
}

void SubscribeEventHandler::
Clear()
{
    buffer_.clear();
}

bool SubscribeEventHandler::
DecodingJSON()
{

    using namespace rapidjson;


    /*--
    {
        "action"      : " ", // ADD, DEL
        "pkg_name"    : " ",
        "code"        : " ",
        "severity_id" : ,
        "node_no"     : ,
        "proc_no"     :
    }
    --*/

    try {

        rabbit::document    doc;

        doc.parse(buffer_);

        rabbit::object      body    = doc["BODY"];

        stSubscriber_.action_       =   body["action"].as_string();
        stSubscriber_.pkg_name_     =   body["pkg_name"].as_string();
        stSubscriber_.code_         =   body["code"].as_string();
        stSubscriber_.severity_id_  =   body["severity_id"].as_int();
        stSubscriber_.node_no_      =   body["node_no"].as_int();
        stSubscriber_.proc_no_      =   body["proc_no"].as_int();

    } catch(rabbit::type_mismatch   e) {

        gAlmLog->WARNING("%-24s| DecodingJSON - type mismatch [%s]",
            "SubscribeEventHandler",
            e.what());
        return false;
    } catch(rabbit::parse_error e) {

        gAlmLog->WARNING("%-24s| DecodingJSON - parse error [%s]",
            "SubscribeEventHandler",
            e.what());
        return false;
    } catch(...) {
        gAlmLog->WARNING("%-24s| DecodingJSON - can't decoding",
            "SubscribeEventHandler");
        return false;
    }

    return true;
}

bool SubscribeEventHandler::
IsValidSequenceId(SequenceByNode & _sequence_by_node)
{
    // 이건, Proc ID 에서 보내는 거예요.. 확인할 필요 없죠..
    return true;
}

bool SubscribeEventHandler::
Do(NodeManagerIF & _nm_if, DB * _db)
{
    DEBUG_PRINT("inside Do\n");

    char    sql[256];

    delSubscriber(stSubscriber_);
    sprintf(sql,
        "DELETE FROM TAT_ALM_SUBSCRIBE WHERE "
        " REQ_NODE_NO=%d AND REQ_PROC_NO=%d AND"
        " PKG_TYPE='%s' AND CODE='%s'",
        stSubscriber_.node_no_,
        stSubscriber_.proc_no_,
        stSubscriber_.pkg_name_.c_str(),
        stSubscriber_.code_.c_str());

    _db->Execute(sql, strlen(sql));

    // 중복 적용을 제거하기 위하여, Delete 를 모두 태우는 겁니다.
    // 어차피, 프로세스 시작때나 몇 개 올지 말지 하는거..

    if(stSubscriber_.action_.compare("add") == 0 ||
       stSubscriber_.action_.compare("ADD") == 0)
    {
        addSubscriber(stSubscriber_);
        sprintf(sql,
            "INSERT INTO TAT_ALM_SUBSCRIBE "
            " (PKG_TYPE, CODE, SEVERITY_CCD, REQ_NODE_NO, REQ_PROC_NO, PRC_DATE) "
            " VALUES ('%s', '%s', %d, %d, %d, NOW())",
            stSubscriber_.pkg_name_.c_str(),
            stSubscriber_.code_.c_str(),
            stSubscriber_.severity_id_,
            stSubscriber_.node_no_,
            stSubscriber_.proc_no_);
    }

	int ret = 0;
    if((ret = _db->Execute(sql, strlen(sql))) < 0)
    {
        gAlmLog->WARNING("%-24s| updateProcessStatus - fail [%d:%s] [%s]",
            "SubscribeEventHandler",
			ret,
            _db->GetErrorMsg(ret),
            sql);

        return false;
    }

    return true;

}

int SubscribeEventHandler::
Notify(NodeManagerIF  & _nm_if,
       ST_AlarmStatus & _stAlarmStatus,
       std::string    & _body)
{
    int cnt = 0;

    int command_code = CMD_ALM_NOTIFY_EVENT;

    if(_stAlarmStatus.manual_yn_.compare("Y") == 0)
        command_code = CMD_ALM_NOTIFY_MANUAL;

    // WSM 으로는 무조건 보내야 해요
    if(_nm_if.Notify(command_code,
                     gCFG.NM.wsm_node_no_,
                     gCFG.NM.wsm_proc_no_,
                     _body) == true)
        cnt++;


    // 다른 애들 찾아봅니다.
    auto iter_map = map_.find(_stAlarmStatus.code_);

    if(iter_map == map_.end())
        return true;

    std::list<ST_Subscriber> & list = iter_map->second;

    for(auto iter=list.begin(); iter != list.end(); ++iter)
    {
        ST_Subscriber & app = *(iter);

        // severity 값은, Major 등록시, Major, Critical 모두 알립니다.
        if(app.pkg_name_.compare(_stAlarmStatus.pkg_name_) == 0 &&
           app.severity_id_ >= _stAlarmStatus.severity_id_)
        {
            if(_nm_if.Notify(CMD_ALM_NOTIFY_EVENT,
                             app.node_no_,
                             app.proc_no_,
                             _body) == true)
                cnt++;
        }
    }

    return cnt;
}
