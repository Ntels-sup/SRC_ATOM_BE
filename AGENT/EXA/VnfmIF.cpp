#include <cstdio>
#include <string>

#include "VnfmIF.hpp"
#include "ExaCommonDef.hpp"
#include "VnaCFG.hpp"
#include "CFileLog.hpp"

extern CFileLog *   gLog;
extern VnaCFG       gVnaCFG;

VnfmIF::
VnfmIF()
{
    m_bInitialized  = false;

    // m_timerTryInit.Update();
    m_timerForReportPerf.Update();

    m_pHttpClient   = NULL;
    m_pHttpServer   = NULL;
}

VnfmIF::
~VnfmIF()
{
    if(m_pHttpClient != NULL)
    {
        delete m_pHttpClient;
        m_pHttpClient = NULL;
    }

    if(m_pHttpServer != NULL)
    {
        delete m_pHttpServer;
        m_pHttpServer = NULL;
    }

}

bool VnfmIF::
Init(void * a_pArg)
{
    if(m_bInitialized == true)
        return true;

    if(m_timerTryInit.TimeOut(10) != true)
        return false;
    else
        m_timerTryInit.Update();

    CModule::ST_MODULE_OPTIONS stOption =
        *static_cast<CModule::ST_MODULE_OPTIONS*>(a_pArg);

    setProfile(m_profile, stOption);

    if(gVnaCFG.Init(stOption.m_szCfgFile,
                    stOption.m_pclsDB,
                    m_profile.m_strPkgName,
                    m_profile.m_strNodeType) == false)
        return false;

    if(m_board.Init() != true)
    {
        gLog->WARNING("%-24s| Init - Dash Board Init fail",
            "VnfmIF");
        return false;
    }

    if(m_pHttpServer != NULL)
    {
        delete m_pHttpServer;
        m_pHttpServer = NULL;
    }

    m_pHttpServer = new (std::nothrow) HttpServer();
    if(m_pHttpServer == NULL ||
       //m_pHttpServer->Init(m_board.GetMyIP(),
       m_pHttpServer->Init("0.0.0.0",
                           VNF_LISTEN_PORT) != true)
    {
        gLog->WARNING("%-24s| Init - http server IF Init fail [%p]",
            "VnfmIF",
            m_pHttpServer);
        return false;
    }

    if(m_pHttpClient != NULL)
    {
        delete m_pHttpClient;
        m_pHttpClient = NULL;
    }

    m_pHttpClient = new (std::nothrow) HttpClient();
    if(m_pHttpClient == NULL || m_pHttpClient->Init() != true)
    {
        gLog->WARNING("%-24s| Init - http client IF Init fail [%p]",
            "VnfmIF",
            m_pHttpClient);
        return false;
    }

    if(m_nodeInterface.Init(m_profile,
                         stOption.m_pclsModIpc,
                         stOption.m_pclsAddress) == false)
    {
        gLog->WARNING("%-24s| Init - node interface init fail",
            "VnfmIF");
        return false;
    }

    // 5 초 단위로 수집 요청
    // TO DO : 5 를 설정으로 변경합시다.
    if(m_resourceAndTpsProvider.Init(m_profile.m_nNodeNo,
                                     m_profile.m_nProcNo,
                                    stOption.m_pclsModIpc,
                                    gVnaCFG.SVC.gathering_period_) == false)
    {
        gLog->WARNING("%-24s| Init - can't init for provider",
            "VnfmIF");
        return false;
    }

    int     node_no;
    int     rsa_no;

    if(stOption.m_pclsAddress->LookupAtom("ATOM_NA_RSA", node_no, rsa_no) <= 0)
    {
        gLog->ERROR("%-24s| Init - LookupAtom fail [ATOM_NA_RSA]",
            "VnfmIF");
        return false;
    }

    std::string body_for_rsa_request =
        "{ \"BODY\":{ \"period\":0, \"timeout\":0 } }";

    m_resourceAndTpsProvider.Register(CMD_RSA_PERF_REPORT,
                                      rsa_no,
                                      body_for_rsa_request);

    if(m_nodeToVnfmEvent.Init(&m_resourceAndTpsProvider) == false)
    {
        gLog->WARNING("%-24s| Init - nodeToVnfm Init fail",
            "VnfmIF");
        return false;
    }

    if(m_vnfmToNodeEvent.Init() == false)
    {
        gLog->WARNING("%-24s| vnfmToNode Init fail",
            "VnfmIF");
        return false;
    }

    NodeData & sNodeData = m_vnfmToNodeEvent.Request(m_profile, m_board);

    gLog->DEBUG("%-24s| Init - make ready msg [%s]",
        "VnfmIF",
        sNodeData.GetBody().c_str());

    if(m_nodeInterface.SendRequest(CMD_VNF_PRA_READY,
                                   sNodeData.GetBody()) == false)
    {
        gLog->WARNING("%-24s| Init - can't send to NA for ready request message",
            "VnfmIF");
        return false;
    }

    gLog->INFO("%-24s| INIT SUCCESS",
        "VnfmIF");

    m_bInitialized = true;
    return true;
}

void VnfmIF::
setProfile(ST_Profile                 & a_stProfile,
           CModule::ST_MODULE_OPTIONS & a_stOption)
{
    m_profile.m_strPkgName    = a_stOption.m_szPkgName;

    m_profile.m_strNodeName   = a_stOption.m_szNodeName;
    m_profile.m_strNodeType   = a_stOption.m_szNodeType;
    m_profile.m_strNodeVersion= a_stOption.m_szNodeVersion;

    m_profile.m_nNodeNo       = a_stOption.m_nNodeNo;
    m_profile.m_nProcNo       = a_stOption.m_nProcNo;
}

bool VnfmIF::
Do()
{
    NodeData & rNodeData = m_nodeInterface.GetData();

    if(rNodeData.IsFill() == true)
    {
        HttpData & sHttpData = m_nodeToVnfmEvent.Receive(rNodeData, m_board);

        if(sHttpData.IsFill() == true)
        {
            m_pHttpClient->Send(sHttpData);
            HttpData & rHttpData = m_pHttpClient->GetData();
            m_vnfmToNodeEvent.ReceiveResponse(rHttpData, m_board);

            m_pHttpClient->Close();
        }

        // Install 을 받으면, start 명령을 자동으로 연계 시켜줘야 해요.
        if(rNodeData.GetCommand() == CMD_VNF_PRA_INSTALL &&
           rNodeData.IsError() == false)
        {
            gLog->DEBUG("%-24s| Try To make start msg",
                "VnfmIF");
            NodeData & sNodeData = m_vnfmToNodeEvent.MakeStart(m_board);
            m_nodeInterface.Send(sNodeData);
        }
    }

    HttpData & rHttpData = m_pHttpServer->GetData();

    if(rHttpData.IsFill() == true)
    {
        NodeData & sNodeData = m_vnfmToNodeEvent.Receive(rHttpData, m_board);

        if(sNodeData.IsFill() == true)
        {
            if(m_nodeInterface.Send(sNodeData) == true)
                m_pHttpServer->Send(vnfm::eOk, vnfm::Ok);
            else
                m_pHttpServer->Send(vnfm::eInternal, vnfm::Internal);
        }
        else
        {
            m_pHttpServer->Send(m_vnfmToNodeEvent.GetStatusCode(),
                                m_vnfmToNodeEvent.GetStatusMsg());
        }

    }

    m_resourceAndTpsProvider.RequestAll();

    // TO DO : 5 초를 설정으로 변경합시다.
    if(strlen(m_board.GetUUID()) > 0 &&
       m_timerForReportPerf.TimeOut(gVnaCFG.SVC.gathering_period_))
    {
        m_timerForReportPerf.Update();

        HttpData & sHttpData = m_nodeToVnfmEvent.ReportPerf(m_board);
        if(sHttpData.IsFill() == true)
        {
            if(m_pHttpClient->Send(sHttpData) == true)
                m_pHttpClient->GetData();

            m_pHttpClient->Close();
        }
    }

    return true;
}

void VnfmIF::
Final()
{
    // Empty
}
