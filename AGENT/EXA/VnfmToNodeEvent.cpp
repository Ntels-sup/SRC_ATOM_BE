
#include "rabbit.hpp"

#include "VnfmToNodeEvent.hpp"
#include "CTimeUtil.hpp"
#include "CFileLog.hpp"

extern CFileLog * gLog;

VnfmToNodeEvent::
VnfmToNodeEvent()
{
    // Empty
}

VnfmToNodeEvent::
~VnfmToNodeEvent()
{
    // Empty
}

bool VnfmToNodeEvent::
Init()
{
    gLog->INFO("Init");

    return true;
}

NodeData & VnfmToNodeEvent::
Receive(HttpData & a_rHttpData, DashBoard & a_board)
{
    gLog->DEBUG("%-24s| Receive",
        "VnfmToNodeEvent");

    m_sNodeData.Clear();

    char    tempUri[URI_MAX_SIZE];
    memset(tempUri, 0, sizeof(tempUri));

    std::string     uri(a_rHttpData.GetUri(tempUri, sizeof(tempUri)));

    if(strncasecmp("/vnf/notification/lifecycle", uri.c_str(), uri.size()) == 0)
        m_eStatusCode = lifecycle(a_rHttpData, a_board);
    else if(strncasecmp("/vnf/action/start", uri.c_str(), uri.size()) == 0)
        m_eStatusCode = start(a_rHttpData, a_board);
    else if(strncasecmp("/vnf/action/stop", uri.c_str(), uri.size()) == 0)
        m_eStatusCode = stop(a_rHttpData, a_board);
    else if(strncasecmp("/vnf/action/install", uri.c_str(), uri.size()) == 0)
        m_eStatusCode = install(a_rHttpData, a_board);
    else if(strncasecmp("/alive", uri.c_str(), uri.size()) == 0)
        m_eStatusCode = keepAlive(a_rHttpData);
    else
        m_eStatusCode = unknown(a_rHttpData);

    return m_sNodeData;
}

void VnfmToNodeEvent::
ReceiveResponse(HttpData & a_rHttpData, DashBoard & a_board)
{
    gLog->DEBUG("%-24s| ReceiveResponse",
        "VnfmToNodeEvent");

    if(strlen(a_rHttpData.GetBody()) == 0)
        return ;

    gLog->DEBUG("%-24s| ReceiveResponse - [%s]",
        "VnfmToNodeEvent",
        a_rHttpData.GetBody());

    try {
        rabbit::document    doc;
        doc.parse(a_rHttpData.GetBody());

        if(doc["server"].is_null() == false)
        {
            std::string     uuid = doc["server"]["server-uuid"].as_string();
            a_board.SetUUID(uuid);
        }

        if(doc["subscriber"].is_null() == false)
        {
            std::string     id = doc["subscriber"]["id"].as_string();
            a_board.SetSubscriberId(id);
        }

    } catch(rabbit::type_mismatch & e) {

        gLog->WARNING("%-24s| ReceiveResponse - type mismatch [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return ;
    } catch(rabbit::parse_error & e) {

        gLog->WARNING("%-24s| ReceiveResponse - parse error [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return ;
    } catch(...) {
        gLog->WARNING("%-24s| ReceiveResponse - [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return ;
    }
}

NodeData & VnfmToNodeEvent::
Request(ST_Profile & a_profile, DashBoard & a_board)
{
    gLog->DEBUG("%-24s| Request",
        "VnfmToNodeEvent");

    m_sNodeData.Clear();

    rabbit::object      root;
    rabbit::object      body = root["BODY"];

    body["ip"]       =  a_board.GetInternalInterfaceIP();
    body["pkg_name"] =  a_profile.m_strPkgName;
    body["node_type"]=  a_profile.m_strNodeType;

    std::string         prc_date;
    std::string         dst_yn;

    getPrcDateAndDstYn(prc_date, dst_yn);

    body["prc_date"] =  prc_date;
    body["dst_yn"]   =  dst_yn;

    m_sNodeData.GetBody() = root.str();

    gLog->DEBUG("---- size [%d]", m_sNodeData.GetBody().size());

    return m_sNodeData;
}

const vnfm::eStatusCode VnfmToNodeEvent::
GetStatusCode()
{
    return  m_eStatusCode;
}

const std::string & VnfmToNodeEvent::
GetStatusMsg()
{
    switch(m_eStatusCode)
    {
    case vnfm::eOk:           return vnfm::Ok;
    case vnfm::eBadRequest:   return vnfm::BadRequest;
    case vnfm::eForbidden:    return vnfm::Forbidden;
    case vnfm::eNotFound:     return vnfm::NotFound;
    case vnfm::eInternal:     return vnfm::Internal;
    default:
        break;
    }

    return vnfm::Unknown;
}

/* Sample Data
{
    "notification": {
        "subscriber_id": "dea5a2f7-3ec7-4496-adab-0abb5a42d635",
        "event_type": "Model Change",
        "timestamp": "2014-03-12 17:00:24.156710",
        "event_id": "00004e00-8da5-4c39-8ffb-c94ed0b5278c",
        "perceived_severity": "Information",
        "probable_cause": "Information Modification",
        "specific_problem": "vnf.instance.scale_out.end",
        "ha_status": "Active",
        "additional_text": "more detail descriptions",
        "notification_id": "1f370095-98f6-4079-be64-6d3d4a6adcc6"
    }
}
*/

vnfm::eStatusCode VnfmToNodeEvent::
lifecycle(HttpData & a_rHttpData, DashBoard & a_board)
{

    gLog->DEBUG("lifecycle");

    m_sNodeData.Clear();

    std::string     subscriber_id;
    std::string     event_type;
    std::string     timestamp;
    std::string     event_id;
    std::string     perceived_severity;
    std::string     probable_cause;
    std::string     specific_problem;
    std::string     ha_status;
    std::string     additional_text;
    std::string     notification_id;

    try {
        rabbit::document    doc;
        doc.parse(a_rHttpData.GetBody());

        rabbit::object      root = doc["notification"];

        subscriber_id       = root["subscriber_id"].as_string();
        event_type          = root["event_type"].as_string();
        timestamp           = root["timestamp"].as_string();
        event_id            = root["event_id"].as_string();
        perceived_severity  = root["perceived_severity"].as_string();
        probable_cause      = root["probable_cause"].as_string();
        specific_problem    = root["specific_problem"].as_string();
        ha_status           = root["ha_status"].as_string();
        additional_text     = root["additional_text"].as_string();
        notification_id     = root["notification_id"].as_string();
    } catch(rabbit::type_mismatch & e) {

        gLog->WARNING("%-24s| lifecycle - type mismatch [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eBadRequest;
    } catch(rabbit::parse_error & e) {

        gLog->WARNING("%-24s| lifecycle - parse error [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eBadRequest;
    } catch(...) {
        gLog->WARNING("%-24s| lifecycle - [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eBadRequest;
    }

    if(a_board.IsValidSubscriberId(subscriber_id.c_str()) != true)
    {
        gLog->WARNING("%-24s| lifecycle - mismatch my:[%s] received:[%s]",
            "VnfmToNodeEvent",
            a_board.GetSubscriberId(),
            subscriber_id.c_str());
        return vnfm::eForbidden;
    }

    // TO DO : NODE 메시지를 만들어서 PRM 으로 Send 해야 합니다.

    return vnfm::eOk;
}

/* Sample Data
{
 "start": {
            "server-uuid": "bd98f997-d325-4582-8788-3f894786e0da",
            "timestamp": "2014-03-12 17:00:24.156710"
          }

}
*/

vnfm::eStatusCode VnfmToNodeEvent::
start(HttpData & a_rHttpData, DashBoard & a_board)
{
    gLog->DEBUG("%-24s| start",
        "VnfmToNodeEvent");

    m_sNodeData.Clear();

    std::string     server_uuid;
    std::string     timestamp;

    try {
        rabbit::document    doc;
        doc.parse(a_rHttpData.GetBody());

        rabbit::object      root = doc["start"];

        server_uuid         = root["server-uuid"].as_string();
        timestamp           = root["timestamp"].as_string();
    } catch(rabbit::type_mismatch & e) {

        gLog->WARNING("%-24s| start - type mismatch [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eBadRequest;
    } catch(rabbit::parse_error & e) {

        gLog->WARNING("%-24s| start - parse error [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eBadRequest;
    } catch(...) {
        gLog->WARNING("%-24s| start - [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eBadRequest;
    }

    if(a_board.IsValidUUID(server_uuid.c_str()) == false)
    {
        gLog->WARNING("%-24s| start - [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eForbidden;
    }

    m_sNodeData.SetCommand(CMD_VNF_PRA_START);

    rabbit::object      oRoot;
    rabbit::object      oBody   = oRoot["BODY"];

    oBody["uuid"]          = a_board.GetUUID();
    oBody["all"]           = true;

    m_sNodeData.GetBody() = oRoot.str();

    return vnfm::eOk;
}

/* Sample Data
{
       "stop":
        {
            "server-uuid": "aa079dc9-b9ec-4e15-a2ee-b753c2d02397",
            "timestamp": "2016-03-12 17:00:24.156710"
         }

*/

vnfm::eStatusCode VnfmToNodeEvent::
stop(HttpData & a_rHttpData, DashBoard & a_board)
{
    gLog->DEBUG("%-24s| stop",
        "VnfmToNodeEvent");

    m_sNodeData.Clear();

    std::string     server_uuid;
    std::string     timestamp;
    std::string     cause;

    try {
        rabbit::document    doc;
        doc.parse(a_rHttpData.GetBody());

        rabbit::object      root = doc["stop"];

        server_uuid         = root["server-uuid"].as_string();
        timestamp           = root["timestamp"].as_string();
        cause               = root["cause"].as_string();
    } catch(rabbit::type_mismatch & e) {

        gLog->WARNING("%-24s| stop - type mismatch [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eBadRequest;
    } catch(rabbit::parse_error & e) {

        gLog->WARNING("%-24s| stop - parse error [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eBadRequest;
    } catch(...) {
        gLog->WARNING("%-24s| stop - [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eBadRequest;
    }

    if(a_board.IsValidUUID(server_uuid.c_str()) == false)
    {
        gLog->WARNING("%-24s| start - [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eForbidden;
    }

    m_sNodeData.SetCommand(CMD_VNF_PRA_STOP);

    rabbit::object      oRoot;
    rabbit::object      oBody   = oRoot["BODY"];

    oBody["uuid"]          = a_board.GetUUID();
    oBody["all"]           = true;
    oBody["scalein"]       = (cause.compare("reboot") == 0)?false:true;

    m_sNodeData.GetBody() = oRoot.str();

    return vnfm::eOk;
}

/* Sample Data
{
    "install":
        {
            "server-uuid ": "bd98f997-d325-4582-8788-3f894786e0da",
            "timestamp": "2016-03-12 17:00:24.156710",
            "details" : {
                [== To Be Define more details  ==]
                1. service version ( package version )
                2. package download url
                3. service up timeout
                4.
             }
        }
}
*/

vnfm::eStatusCode VnfmToNodeEvent::
install(HttpData & a_rHttpData, DashBoard & a_board)
{
    gLog->DEBUG("%-24s| install",
        "VnfmToNodeEvent");

    m_sNodeData.Clear();

    std::string     server_uuid;
    std::string     timestamp;

    std::string     version;
    std::string     download_path;
    std::string     checksum;

    try {
        rabbit::document    doc;
        doc.parse(a_rHttpData.GetBody());

        rabbit::object      root = doc["install"];

        server_uuid         = root["server-uuid"].as_string();
        timestamp           = root["timestamp"].as_string();

        version             = root["details"]["version"].as_string();
        download_path       = root["details"]["download_path"].as_string();
        checksum            = root["details"]["checksum"].as_string();

    } catch(rabbit::type_mismatch & e) {

        gLog->WARNING("%-24s| install - type mismatch [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eBadRequest;
    } catch(rabbit::parse_error & e) {

        gLog->WARNING("%-24s| install - parse error [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eBadRequest;
    } catch(...) {
        gLog->WARNING("%-24s| install - [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eBadRequest;
    }

    if(a_board.IsValidUUID(server_uuid.c_str()) == false)
    {
        gLog->WARNING("%-24s| install - [%s]",
            "VnfmToNodeEvent",
            a_rHttpData.GetBody());
        return vnfm::eForbidden;
    }

    m_sNodeData.SetCommand(CMD_VNF_PRA_INSTALL);

    rabbit::object      oRoot;
    rabbit::object      oBody   = oRoot["BODY"];

    oBody["uuid"]               = server_uuid;
    oBody["version"]            = version;
    oBody["download_path"]      = download_path;
    oBody["checksum"]           = checksum;
    oBody["prc_date"]           = timestamp;

    std::string     dstYn;
    CTimeUtil::SetDstYn(dstYn);

    oBody["dst_yn"]             = dstYn;

    m_sNodeData.GetBody() = oRoot.str();

    return vnfm::eOk;
}

vnfm::eStatusCode VnfmToNodeEvent::
unknown(HttpData & a_rHttpData)
{
    gLog->WARNING("%-24s| unknown",
        "VnfmToNodeEvent");

    m_sNodeData.Clear();

    return vnfm::eNotFound;
}

vnfm::eStatusCode VnfmToNodeEvent::
keepAlive(HttpData & a_rHttpData)
{
    gLog->WARNING("%-24s| unknown",
        "VnfmToNodeEvent");

    m_sNodeData.Clear();

    return vnfm::eOk;
}

void VnfmToNodeEvent::
getPrcDateAndDstYn(std::string & a_prcDate,
                   std::string & a_dstYn)
{
    char    date[DB_DATETIME_SIZE+1];

    struct tm   stT;
    time_t      now = time(NULL);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", localtime_r(&now, &stT));

    a_prcDate = date;

    if(stT.tm_isdst > 0)
        a_dstYn = "Y";
    else
        a_dstYn = "N";
}

NodeData & VnfmToNodeEvent::
MakeStart(DashBoard & a_board)
{
    m_sNodeData.Clear();

    m_sNodeData.SetCommand(CMD_VNF_PRA_START);

    rabbit::object      oRoot;
    rabbit::object      oBody   = oRoot["BODY"];

    oBody["uuid"]          = a_board.GetUUID();
    oBody["all"]           = true;

    m_sNodeData.GetBody() = oRoot.str();

    return m_sNodeData;
}