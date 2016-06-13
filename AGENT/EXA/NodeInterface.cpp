
#include "NodeInterface.hpp"
#include "CFileLog.hpp"

extern CFileLog * gLog;

NodeInterface::
NodeInterface()
{
    m_seq   = 0;
}

NodeInterface::
~NodeInterface()
{
    // Empty
}

bool NodeInterface::
Init(ST_Profile & a_profile,
     CModuleIPC * a_pIpc,
     CAddress   * a_pAddr)
{
    m_pIpc      = a_pIpc;
    m_pAddr     = a_pAddr;

    m_node.m_nNo      = a_profile.m_nNodeNo;
    m_myProc.m_nNo    = a_profile.m_nProcNo;

    if(m_pAddr->LookupAtom("ATOM_NA_PRA", m_node.m_nNo, m_praProc.m_nNo) <= 0)
    {
        gLog->ERROR("%-24s| Init - LookupAtom fail [ATOM_PRA]",
            "NodeInterface");
        return false;
    } else {
        m_praProc.m_strName    = "PRA";
    }

    if(m_pAddr->LookupAtom("ATOM_NA", m_node.m_nNo, m_naProc.m_nNo) <= 0)
    {
        gLog->ERROR("%-24s| Init - LookupAtom fail [ATOM_NA]",
            "NodeInterface");
        return false;
    } else {
        m_naProc.m_strName    = "NA";
    }

    /*--
    if(m_pAddr->LookupAtom("ATOM_NA_RSA", m_node.m_nNo, m_rsaProc.m_nNo) <= 0)
    {
        gLog->ERROR("%-24s| Init - LookupAtom fail [NM_RSA]",
            "NodeInterface");
        return false;
    } else {
        m_rsaProc.m_strName    = "RSA";
    }
    --*/

    return true;
}

NodeData & NodeInterface::
GetData()
{
    m_rProtocol.Clear();

    int ret = 0;
    if((ret = m_pIpc->RecvMesg(m_myProc.m_nNo, m_rProtocol, -1)) < 0)
    {
        gLog->WARNING("%-24s| GetData - RecvMesg Error [%s]",
            "NodeInterface",
            m_pIpc->m_strErrorMsg.c_str());

        // Init 이 될 수 있게끔 해야 합니다.
    }

    m_nodeData.Clear();

    m_rProtocol.GetPayload(m_vec);
    m_nodeData.SetCommand(atoi(m_rProtocol.GetCommand().c_str()));
    m_nodeData.GetBody() = std::string(m_vec.begin(), m_vec.end());

    if(m_rProtocol.IsFlagError() == true)
    {
        gLog->WARNING("%-24s| GetData - RecvMesg, but error flag set [%d] [%s]",
            "NodeInterface",
            m_nodeData.GetCommand(),
            m_nodeData.GetBody().c_str());

        m_nodeData.SetError();
    }

    if(m_nodeData.GetBody().size() > 0)
    {
        gLog->DEBUG("%-24s| GetData - [%s] [%s]",
            "NodeInterface",
            m_rProtocol.GetCommand().c_str(),
            m_nodeData.GetBody().c_str());
    }

    return m_nodeData;
}


bool    NodeInterface::
SendRequest(int a_nCommandCode, std::string & a_strB)
{
    m_sProtocol.Clear();

    m_sProtocol.SetFlagRequest();
    m_sProtocol.SetSequence(generateSequenceId());
    setHeader(a_nCommandCode);

    m_vec.assign(a_strB.begin(), a_strB.end());
    m_sProtocol.SetPayload(m_vec);

    if(m_pIpc->SendMesg(m_sProtocol) == false)
    {
        gLog->WARNING("%-24s| SendRequest - SendMesg Error [%s] command [%d]",
            "NodeInterface",
            m_pIpc->m_strErrorMsg.c_str(),
            a_nCommandCode);
		m_sProtocol.Print(gLog, LV_WARNING, true);
        return false;
    }

    gLog->DEBUG("%-24s| SendRequest - [%d] [%s]",
        "NodeInterface",
        a_nCommandCode,
        a_strB.c_str());

    return true;
}

int  NodeInterface::
generateSequenceId()
{
    return ++m_seq;
}

void NodeInterface::
setHeader(int a_nCommandCode)
{
    char    buffer[32];
    sprintf(buffer, "%010d", a_nCommandCode);

    m_sProtocol.SetCommand(buffer);
    m_sProtocol.SetSource(m_node.m_nNo, m_myProc.m_nNo);
    m_sProtocol.SetDestination(m_node.m_nNo, m_naProc.m_nNo);

    gLog->DEBUG("%-24s| setHeader - [%d] [%u]",
        "NodeInterface",
        a_nCommandCode,
        m_naProc.m_nNo);

    /*--
    switch(a_nCommandCode)
    {
    // case CMD_VNF_PERF_REQ:
    //    m_sProtocol.SetDestination(m_node.m_nNo, m_rsaProc.m_nNo);
    //     break;
    case CMD_VNF_PRA_INSTALL:
    case CMD_VNF_PRA_READY:
        m_sProtocol.SetDestination(m_node.m_nNo, m_naProc.m_nNo);
        break;
    default:
        m_sProtocol.SetDestination(m_node.m_nNo, m_praProc.m_nNo);
    }
    --*/
}


bool NodeInterface::
SendNotify(int a_nCommandCode, std::string & a_strB)
{
    m_sProtocol.Clear();

    m_sProtocol.SetFlagNotify();
    m_sProtocol.SetSequence(0);
    setHeader(a_nCommandCode);

    m_vec.assign(a_strB.begin(), a_strB.end());
    m_sProtocol.SetPayload(m_vec);

    if(m_pIpc->SendMesg(m_sProtocol) == false)
    {
        gLog->WARNING("%-24s| SendNotify - SendMesg Error [%s] command [%d]",
            "NodeInterface",
            m_pIpc->m_strErrorMsg.c_str(),
            a_nCommandCode);
        return false;
    }

    gLog->DEBUG("%-24s| SendNotify - [%d] [%s]",
        "NodeInterface",
        a_nCommandCode,
        a_strB.c_str());

    return true;
}

bool NodeInterface::
Send(NodeData & a_sNodeData)
{
    switch(a_sNodeData.GetCommand())
    {
    case CMD_VNF_NOTIFY:
        return SendNotify(a_sNodeData.GetCommand(), a_sNodeData.GetBody());
    default:
        ;
    }

    return SendRequest(a_sNodeData.GetCommand(), a_sNodeData.GetBody());
}
