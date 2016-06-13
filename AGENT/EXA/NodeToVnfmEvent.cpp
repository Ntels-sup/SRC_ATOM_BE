
#include "NodeToVnfmEvent.hpp"
#include "rabbit.hpp"

#include "CFileLog.hpp"
#include "VnaCFG.hpp"

// #include <vector>

extern CFileLog * gLog;
extern VnaCFG     gVnaCFG;

NodeToVnfmEvent::
NodeToVnfmEvent()
{
    m_pResourceAndTpsProvider = NULL;
}

NodeToVnfmEvent::
~NodeToVnfmEvent()
{
    // Empty
}

bool NodeToVnfmEvent::
Init(ContentsProvider * a_pResourceAndTpsProvider)
{
    gLog->DEBUG("Init");

    m_pResourceAndTpsProvider = a_pResourceAndTpsProvider;

    return true;
}

HttpData & NodeToVnfmEvent::
ReportPerf(DashBoard & a_board)
{
    gLog->DEBUG("%-24s| ReportPerf ",
        "NodeToVnfmEvent");

    gLog->DEBUG("%-24s| ReportPerf - Gathering Time [%s]",
        "NodeToVnfmEvent",
        a_board.GetResourceGatheringTime().c_str());

    m_sHttpData.Clear();

    // Time 에 대한 유효성 검증
    if(isValidReportPerf(a_board.GetResourceGatheringTime()) == false)
        return m_sHttpData;

    rabbit::object      root;
    rabbit::object      performance = root["performance"];
    performance["server-uuid"]  = a_board.GetUUID();

    rabbit::array       performance_metrics = performance["performance_metrics"];

    // 1) TPS
	rabbit::object      tps;
    if(m_timerForReportTPS.TimeOut(gVnaCFG.SVC.tps_period_))
    {
        m_timerForReportTPS.Update();

        std::vector<std::string>    vec_tps_titles;
        a_board.GetTPSTitles(vec_tps_titles);

        for(auto iter = vec_tps_titles.begin(); iter != vec_tps_titles.end(); ++iter)
        {
            tps["id"]           = "tps." + *iter;
            tps["timestamp"]    = a_board.GetResourceGatheringTime();
            tps["value"]        = a_board.GetTPS(*iter);
            tps["data_type"]    = "int";
            tps["unit"]         = "tps";
            tps["calculated"]   = "cur";
            tps["metric_type"]  = "Data Traffic";

            performance_metrics.push_back(tps);
        }
    }

    // 2) CPU
    rabbit::object      cpu;
    if(m_timerForReportCPU.TimeOut(gVnaCFG.SVC.cpu_period_))
    {
        m_timerForReportCPU.Update();

        cpu["id"]           = "cpu.load";
        cpu["timestamp"]    = a_board.GetResourceGatheringTime();
        cpu["value"]        = a_board.GetCPU(0);
        cpu["data_type"]    = "int";
        cpu["unit"]         = "%";
        cpu["calculated"]   = "cur";
        cpu["metric_type"]  = "cpu.load";

        performance_metrics.push_back(cpu);
    }

    // 3) MEM
    rabbit::object      mem;
    if(m_timerForReportMEM.TimeOut(gVnaCFG.SVC.mem_period_))
    {
        m_timerForReportMEM.Update();

        mem["id"]           = "memory.load";
        mem["timestamp"]    = a_board.GetResourceGatheringTime();
        mem["value"]        = a_board.GetMEM(0);
        mem["data_type"]    = "int";
        mem["calculated"]   = "cur";
        mem["metric_type"]  = "memory.usage";

        performance_metrics.push_back(mem);
    }

    // 4) DISK
    rabbit::object      disk;
    if(m_timerForReportDISK.TimeOut(gVnaCFG.SVC.disk_period_))
    {
        m_timerForReportDISK.Update();

        std::vector<std::string>    vec_disk_names;
        a_board.GetDiskNames(vec_disk_names);

		// SetDISK() 에서 total 이라는 이름으로 하나만 넣으니까, 하나일꺼예요.
        for(auto iter=vec_disk_names.begin(); iter != vec_disk_names.end(); ++iter)
        {
        	std::string         usage;
            a_board.GetDISK(*iter, usage);

            disk["id"]        = "disk.usage:<partition>";
            disk["timestamp"] = a_board.GetResourceGatheringTime();
            disk["value"]     = usage;
            disk["data_type"] = "int";
            disk["unit"]      = "%";
            disk["calculated"]= "cur";
            disk["metric_type"]= "disk.usage:<partition>";

            performance_metrics.push_back(disk);
        }
    }

    // 5) NET
    rabbit::object      net_rx;
    rabbit::object      net_tx;

    std::string         name;

    if(m_timerForReportNET.TimeOut(gVnaCFG.SVC.net_period_))
    {
        m_timerForReportNET.Update();

        std::vector<std::string> vec_interface_names;
        a_board.GetNetInterfaceNames(vec_interface_names);

        for(auto iter = vec_interface_names.begin();
                 iter != vec_interface_names.end();
                 ++iter)
        {
            name = "network." + *iter + ".rx";
            net_rx["id"]         = name;
            net_rx["timestamp"]  = a_board.GetResourceGatheringTime();
            net_rx["value"]      = a_board.GetRX(*iter);
            net_rx["data_type"]  = "int";
            net_rx["unit"]       = "CPS";
            net_rx["calculated"] = "cur";
            net_rx["metric_type"]= "Signal Traffic";

            performance_metrics.push_back(net_rx);

            name = "network." + *iter + ".tx";
            net_tx["id"]         = name;
            net_tx["timestamp"]  = a_board.GetResourceGatheringTime();
            net_tx["value"]      = a_board.GetTX(*iter);
            net_tx["data_type"]  = "int";
            net_tx["unit"]       = "CPS";
            net_tx["calculated"] = "cur";
            net_tx["metric_type"]= "Signal Traffic";

            performance_metrics.push_back(net_tx);
        }

    }

    m_sHttpData.Clear();

    std::string     body = root.str();

    if(body.size() <= 0)
        return m_sHttpData;

    char uriBuf[URI_MAX_SIZE];

    sprintf(uriBuf, "%s/vnf/performance", a_board.GetVnfmPerformanceURL());

    m_sHttpData.AddLine("POST", uriBuf, "HTTP/1.1");
    m_sHttpData.AddHeader("Content-Type", "application/json");
    m_sHttpData.AddBody(body.c_str(), body.size());


    return m_sHttpData;
}

bool NodeToVnfmEvent::
isValidReportPerf(std::string & a_time)
{
    if(a_time.size() <= 0)
        return false;

    struct tm stT;

    if(strptime(a_time.c_str(), "%Y-%m-%d %H:%M:%S", &stT) == NULL)
    {
        gLog->WARNING("%-24s| isValidReportPerf - strptime fail [%s] [%d:%s]",
            "NodeToVnfmEvent",
            a_time.c_str(),
            errno,
            strerror(errno));

        return false;
    }

    if(time(NULL) - mktime(&stT) > 180)
    {
        gLog->DEBUG("%-24s| this is old updated data [%s]",
            "NodeToVnfmEvent",
            a_time.c_str());
        return false;
    }

    return true;
}

HttpData & NodeToVnfmEvent::
Receive(NodeData & a_rNodeData, DashBoard & a_board)
{
    gLog->DEBUG("%-24s| Receive",
        "NodeToVnfmEvent");

    if(a_rNodeData.IsError())
    {
        gLog->DEBUG("%-24s| Received Msg is Error, message is going to discard",
            "NodeToVnfmEvent");
        m_sHttpData.Clear();
    }

    // PRA -> VNF 는 모두 Response  입니다.
    switch(a_rNodeData.GetCommand())
    {
    case CMD_VNF_PRA_READY:
        ready(a_rNodeData,      a_board);
        break;
    case CMD_VNF_PRA_START:
        started(a_rNodeData,    a_board);
        break;
    case CMD_VNF_PRA_STOP:
        stopped(a_rNodeData,    a_board);
        break;
    case CMD_VNF_SUBSCRIBER:
        subscriber(a_rNodeData, a_board);
        break;
    case CMD_VNF_EVENT:
        event(a_rNodeData,      a_board);
        break;
    case CMD_RSA_PERF_REPORT:
        perf(a_rNodeData,       a_board);
        break;
    // From EventAPI - SendTps()
    case CMD_VNF_PERF_TPS:
        perfTps(a_rNodeData,    a_board);
        break;
    case CMD_REGISTER_PROVIDER:
        m_pResourceAndTpsProvider->Register(a_rNodeData.GetBody());
        break;
    case CMD_VNF_PRA_INSTALL:
        break;
    default:
        unknown(a_rNodeData,    a_board);
        break;
    }

    return m_sHttpData;
}

bool NodeToVnfmEvent::
ready(NodeData & a_rNodeData, DashBoard & a_board)
{
    gLog->DEBUG("ready");

    m_sHttpData.Clear();

    std::string     result;
    std::string     reason;
    std::string     prc_date;
    std::string     dst_yn;

    try {
        rabbit::document    doc;
        doc.parse(a_rNodeData.GetBody());

        rabbit::object      root = doc["BODY"];

        result  =   root["result"].as_string();
        reason  =   root["reason"].as_string();
        prc_date=   root["prc_date"].as_string();
        dst_yn  =   root["dst_yn"].as_string();

    } catch(rabbit::type_mismatch & e) {

        gLog->WARNING("%-24s| ready - type mismatch [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    } catch(rabbit::parse_error & e) {

        gLog->WARNING("%-24s| ready - parse error [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    } catch(...) {
        gLog->WARNING("%-24s| ready - [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    }

    if(result.compare("OK") != 0)
    {
        gLog->WARNING("%-24s| ready - recevied response, but is not success [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());

        return false;
    }

    // Encoding

    char actionUri[URI_MAX_SIZE];
    sprintf(actionUri, "%s/vnf/action", a_board.GetMyURL());

    rabbit::object      oRoot;
    rabbit::object      oBody = oRoot["ready"];

    oBody["server-uuid"]    = "Unknown";
    oBody["timestamp"]      = prc_date;
    oBody["action-uri"]     = actionUri;

    std::string        body = oRoot.str();

    char uriBuf[URI_MAX_SIZE];

    sprintf(uriBuf,
            "%s/vnf/status/ready",
            a_board.GetVnfmServiceURL());

    m_sHttpData.AddLine("POST", uriBuf, "HTTP/1.1");
    m_sHttpData.AddHeader("Content-Type", "application/json");
    m_sHttpData.AddBody(body.c_str(), body.size());

    return true;
}

bool NodeToVnfmEvent::
started(NodeData & a_rNodeData, DashBoard & a_board)
{
    gLog->INFO("%-24s| started",
        "NodeToVnfmEvent");

    m_sHttpData.Clear();

    std::string     result;
    std::string     reason;
    std::string     prc_date;
    std::string     dst_yn;

    try {
        rabbit::document    doc;
        doc.parse(a_rNodeData.GetBody());

        rabbit::object      root = doc["BODY"];

        result  =   root["result"].as_string();
        reason  =   root["reason"].as_string();
        prc_date=   root["prc_date"].as_string();
        dst_yn  =   root["dst_yn"].as_string();

    } catch(rabbit::type_mismatch & e) {

        gLog->WARNING("%-24s| started - type mismatch [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    } catch(rabbit::parse_error & e) {

        gLog->WARNING("%-24s| started - parse error [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    } catch(...) {
        gLog->WARNING("%-24s| started - [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    }

    if(result.compare("OK") != 0)
    {
        gLog->WARNING("%-24s| started- recevied response, but is not success [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());

        return false;
    }

    // Encoding
    rabbit::object      oRoot;
    rabbit::object      oBody = oRoot["started"];

    oBody["server-uuid"]  = a_board.GetUUID();
    oBody["timestamp"]    = prc_date;

    std::string body      = oRoot.str();

    // Send 준비
    char uriBuf[URI_MAX_SIZE];

    sprintf(uriBuf,
            "%s/vnf/status/started",
            a_board.GetVnfmServiceURL());

    // Send Data 생성
    m_sHttpData.AddLine("POST", uriBuf, "HTTP/1.1");
    m_sHttpData.AddHeader("Content-Type", "application/json");
    m_sHttpData.AddBody(body.c_str(), body.size());

    return true;
}

bool NodeToVnfmEvent::
stopped(NodeData & a_rNodeData, DashBoard & a_board)
{
    gLog->INFO("%-24s| stopped",
        "NodeToVnfmEvent");

    m_sHttpData.Clear();

    std::string     result;
    std::string     reason;
    std::string     prc_date;
    std::string     dst_yn;

    try {
        rabbit::document    doc;
        doc.parse(a_rNodeData.GetBody());

        rabbit::object      root = doc["BODY"];

        result  =   root["result"].as_string();
        reason  =   root["reason"].as_string();
        prc_date=   root["prc_date"].as_string();
        dst_yn  =   root["dst_yn"].as_string();

    } catch(rabbit::type_mismatch & e) {

        gLog->WARNING("%-24s| stopped - type mismatch [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    } catch(rabbit::parse_error & e) {

        gLog->WARNING("%-24s| stopped - parse error [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    } catch(...) {
        gLog->WARNING("%-24s| stopped - [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    }

    if(result.compare("OK") != 0)
    {
        gLog->WARNING("%-24s| stopped - recevied response, but is not success [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());

        return false;
    }

    // Encoding
    rabbit::object      oRoot;
    rabbit::object      oBody   = oRoot["stopped"];

    oBody["server-uuid"]        = a_board.GetUUID();
    oBody["timestamp"]          = prc_date;

    std::string body            = oRoot.str();

    char uriBuf[256];

    sprintf(uriBuf, "%s/vnf/status/stopped", a_board.GetVnfmServiceURL());

    m_sHttpData.AddLine("POST", uriBuf, "HTTP/1.1");
    m_sHttpData.AddHeader("Content-Type", "application/json");
    m_sHttpData.AddBody(body.c_str(), body.size());

    return true;
}

bool NodeToVnfmEvent::
subscriber(NodeData & a_rNodeData, DashBoard & a_board)
{
    gLog->DEBUG("%-24s| subscriber",
        "NodeToVnfmEvent");

    m_sHttpData.Clear();

    std::string     prc_date;
    std::string     dst_yn;

    try {
        rabbit::document    doc;
        doc.parse(a_rNodeData.GetBody());

        rabbit::object      root = doc["BODY"];

        prc_date=   root["prc_date"].as_string();
        dst_yn  =   root["dst_yn"].as_string();

    } catch(rabbit::type_mismatch & e) {

        gLog->WARNING("%-24s| stopped - type mismatch [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    } catch(rabbit::parse_error & e) {

        gLog->WARNING("%-24s| stopped - parse error [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    } catch(...) {
        gLog->WARNING("%-24s| stopped - [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    }

    // Encoding
    char callbackUri[URI_MAX_SIZE];
    sprintf(callbackUri, "%s/vnf/notification/lifecycle", a_board.GetMyURL());

    rabbit::object      oRoot;
    rabbit::object      oBody   = oRoot["subscriber"];

    oBody["server-uuid"]        = a_board.GetUUID();
    oBody["name"]               = "VNF";
    oBody["notification_type"]  = "VNF Lifecycle Change";
    oBody["callback_uri"]       = callbackUri;

    std::string body    = oRoot.str();


    char uriBuf[URI_MAX_SIZE];
    sprintf(uriBuf, "%s/vnf/subscriber", a_board.GetVnfmServiceURL());

    m_sHttpData.AddLine("POST", uriBuf, "HTTP/1.1");
    m_sHttpData.AddHeader("Content-Type", "application/json");
    m_sHttpData.AddBody(body.c_str(), body.size());

    return true;
}

bool NodeToVnfmEvent::
event(NodeData & a_rNodeData, DashBoard & a_board)
{
    gLog->DEBUG("%-24s| event",
        "NodeToVnfmEvent");

    m_sHttpData.Clear();

    std::string     event_type;
    std::string     timestamp;
    std::string     code;
    std::string     severity;
    std::string     probable_cause;
    std::string     additional_text;
    int             coresponse_action_cnt = 0;
    std::string     coresponse_action;

    try {
        rabbit::document    doc;
        doc.parse(a_rNodeData.GetBody());

        rabbit::object      root = doc["BODY"];

        event_type      = root["event_type"].as_string();
        timestamp       = root["prc_date"].as_string();
        code            = root["code"].as_string();
        severity        = root["perceived_severity"].as_string();
        probable_cause  = root["probable_cause"].as_string();
        additional_text = root["additional_text"].as_string();

        if(root["coresponse_action"].is_null() == false)
        {
            coresponse_action_cnt = root["coresponse_action_cnt"].as_int();
            coresponse_action     = root["coresponse_action"].as_string();
        }

    } catch(rabbit::type_mismatch & e) {

        gLog->WARNING("%-24s| event - type mismatch [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    } catch(rabbit::parse_error & e) {

        gLog->WARNING("%-24s| event - parse error [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    } catch(...) {
        gLog->WARNING("%-24s| event - [%s]",
            "NodeToVnfmEvent",
            a_rNodeData.GetBody().c_str());
        return false;
    }

    rabbit::object      oRoot;
    rabbit::object      oBody   = oRoot["event"];

    oBody["server-uuid"]        = a_board.GetUUID();
    oBody["event_type"]         = event_type;
    oBody["timestamp"]          = timestamp;
    oBody["perceived_severity"] = severity;
    oBody["additional_text"]    = additional_text;
    oBody["alarm_code"]         = code;

    if(coresponse_action.size() > 0)
    {
        if(coresponse_action_cnt == 0)
            oBody["coresponse_action_cnt"] = 1;
        else
            oBody["coresponse_action_cnt"] = coresponse_action_cnt;

        oBody["coresponse_action"] = coresponse_action;
    }

    std::string body    = oRoot.str();

    char uriBuf[URI_MAX_SIZE];
    sprintf(uriBuf, "%s/vnf/event", a_board.GetVnfmServiceURL());

    m_sHttpData.AddLine("POST", uriBuf, "HTTP/1.1");
    m_sHttpData.AddHeader("Content-Type", "application/json");
    m_sHttpData.AddBody(body.c_str(), body.size());

    return true;
}

bool NodeToVnfmEvent::
perf(NodeData & a_rNodeData, DashBoard & a_board)
{
    m_sHttpData.Clear();

    gLog->DEBUG("%-24s| perf",
        "NodeToVnfmEvent");

    std::string  &  body = a_rNodeData.GetBody();

    if(body.size() <= 0)
    {
        gLog->WARNING("%-24s| perf - no body",
            "NodeToVnfmEvent");

        return false;
    }

    gLog->DEBUG("%-24s| perf - [%s]",
        "NodeToVnfmEvent",
        body.c_str());

    try
    {
        rabbit::document    doc;
        doc.parse(body);


        // 1) CPU
        rabbit::array       cpu_list = doc["BODY"]["RSC_GRP_04"]["LIST"];

        std::string         key =
            (cpu_list.at(0)).as_string();
        std::string         cpu_usage =
            doc["BODY"]["RSC_GRP_04"][key.c_str()]["usage"].as_string();

        a_board.SetCPU(cpu_usage);


        // 2) MEM
        rabbit::array       mem_list = doc["BODY"]["RSC_GRP_02"]["LIST"];
        key =
            (mem_list.at(0)).as_string();
        std::string         mem_usage =
            doc["BODY"]["RSC_GRP_02"][key.c_str()]["usage"].as_string();

        a_board.SetMEM(mem_usage);


        // 4) NET
        std::string     net_rx;
        std::string     net_tx;
        rabbit::array   net_list = doc["BODY"]["RSC_GRP_11"]["LIST"];
        for(auto nLoop = 0u; nLoop < net_list.size(); ++nLoop)
        {
            key        = net_list.at(nLoop).as_string();
            net_rx     = doc["BODY"]["RSC_GRP_11"][key.c_str()]["RX"].as_string();
            net_tx     = doc["BODY"]["RSC_GRP_11"][key.c_str()]["TX"].as_string();

            a_board.SetNET(key, net_rx, net_tx);
        }

        // 5) TIME
        std::string timestamp   = doc["BODY"]["time"].as_string();
        std::string dst_yn      = doc["BODY"]["dst_yn"].as_string();

        a_board.SetTIME(timestamp, dst_yn);


        // 3) DISK
        unsigned long long  disk_total = 0;
        unsigned long long  disk_used  = 0;

        rabbit::array       disk_list = doc["BODY"]["RSC_GRP_05"]["LIST"];
        for(auto nLoop = 0u; nLoop < disk_list.size(); ++nLoop)
        {
            key        = disk_list.at(nLoop).as_string();
            disk_total += atoll(doc["BODY"]["RSC_GRP_05"][key.c_str()]["total"].as_string().c_str());
            disk_used  += atoll(doc["BODY"]["RSC_GRP_05"][key.c_str()]["used"].as_string().c_str());
        }

        unsigned long long disk_used_percent = 0;

        if(disk_total == 0)
            disk_used_percent = 0;
        else
            disk_used_percent = (disk_used / disk_total) * 100;

        key = "total";
        std::string disk_usage = std::to_string(disk_used_percent);
        a_board.SetDISK(key, disk_usage);


    } catch(rabbit::type_mismatch & e) {

        gLog->WARNING("%-24s| perf - type mismatch [%s]",
            "NodeToVnfmEvent",
            body.c_str());
        return false;
    } catch(rabbit::parse_error & e) {

        gLog->WARNING("%-24s| perf - parse error [%s]",
            "NodeToVnfmEvent",
            body.c_str());
        return false;
    } catch(...) {
        gLog->WARNING("%-24s| perf - [%s]",
            "NodeToVnfmEvent",
            body.c_str());
        return false;
    }

    return true;
}

bool NodeToVnfmEvent::
perfTps(NodeData & a_rNodeData, DashBoard & a_board)
{
    m_sHttpData.Clear();

    gLog->DEBUG("%-24s| perf tps",
        "NodeToVnfmEvent");

    // vector 로 전달 된다고 합시다.
    // timestamp, Tps_Title, Tps_Cnt
    // Tps_Name 으로 된 값에 Sum 을 해서 보내야 하나요??
    // 받아서 Board 에 Sum 만 하면 되요..
    std::string     timestamp;
    std::string     tpsTitle;
    int             cnt;

    std::string &   apiMsg = a_rNodeData.GetBody();

    size_t  first   = 0;
    size_t  end     = apiMsg.find('\0', first);

    try
    {
        timestamp       = apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = apiMsg.find('\0', first);
        tpsTitle= apiMsg.substr(first).c_str();

        first   = end + 1;
        end     = apiMsg.find('\0', first);
        cnt     = atoi(apiMsg.substr(first).c_str());

    } catch(std::exception & e) {

        gLog->WARNING("%-24s| perfTps - pasing fail",
            "NodeToVnfmEvent");
        return false;
    }

    a_board.SetTPS(timestamp, tpsTitle,     cnt);

    gLog->DEBUG("---- TPS : [%s] [%d]", tpsTitle.c_str(), cnt);

    return true;
}


bool NodeToVnfmEvent::
unknown(NodeData & a_rNodeData, DashBoard & a_board)
{
    gLog->WARNING("unknown");
    gLog->WARNING("%-24s| unknown - [%d] [%s]",
        "NodeToVnfmEvent",
        a_rNodeData.GetCommand(),
        a_rNodeData.GetBody().c_str());
    return true;
}

HttpData & NodeToVnfmEvent::
MakeKeepAliveForEvent(DashBoard & a_board)
{
    m_sHttpData.Clear();

    std::string     body;

    char uriBuf[256];

    sprintf(uriBuf, "%s/alive", a_board.GetVnfmServiceURL());

    m_sHttpData.AddLine("POST", uriBuf, "HTTP/1.1");
    m_sHttpData.AddHeader("Content-Type", "application/json");
    m_sHttpData.AddBody(body.c_str(), body.size());

    return m_sHttpData;
}
