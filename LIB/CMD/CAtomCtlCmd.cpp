
#include "CAtomCtlCmd.hpp"

CAtomCtlCmd::
CAtomCtlCmd()
{
    m_vecCls.clear();
}

CAtomCtlCmd::
~CAtomCtlCmd()
{
    m_vecCls.clear();
}

void CAtomCtlCmd::
Register(CAtomCtlInterface * a_pCls)
{
    m_vecCls.push_back(a_pCls);
}

bool CAtomCtlCmd::
work(CProtocol * a_pCls)
{
    if(a_pCls->GetCommand() != CMD_ATOM_PROC_CTL)
        return false;

    std::string     action;
    int             level = 0;

    // Parsing 하고, interface 를 호출하고, 결과를 보내고.. 끝!
    a_pCls->GetPayload(m_vec);
    m_str.assign(m_vec.begin(), m_vec.end());

    try {
        rabbit::document    doc;
        doc.parse(m_str);

        action = doc["BODY"]["action"].as_string();

        if(action == "LOGLEVEL")
            level   = doc["BODY"]["loglevel"].as_int();

    } catch(...) {

        response(CProtocol, false, "parse error");
        return true;
    }

    if(action == "RECONFIG")
        reponseMsg(a_pCls, reconfig());
    else if(action == "STOP")
        responseMsg(a_pCls, stop());
    else if(action == "INIT")
        responseMsg(a_pCls, init());
    else if(action == "LOGLEVEL")
        responseMsg(a_pCls, loglevel(level));
    else
        responseMsg(a_pCls, false, "unknown action");

    return true;
}

bool CAtomCtlCmd::
Do(CProtocol * a_pCls, CModuleIPC * _ipc)
{
    if(work(a_pCls) == false)
        return false;

    _ipc->SendMesg(*a_pCls);
}

bool CAtomCtlCmd::
Do(CProtocol * a_pCls, CMesgExchSocket * _sock)
{
    if(work(a_pCls) == false)
        return false;

    _sock->SendMesg(*a_pCls);
}

void CAtomCtlCmd::
responseMsg(CProtocol *  a_pCls,
            bool         a_bResult,
            const char * a_text)
{
    rabbit::object  oRoot;
    rabbit::object  oBody = oRoot["BODY"];

    oBody["code"] = (a_bResult)?0:100;

    if(a_bResult)
        oBody["text"] = "ok";
    else
        oBody["text"] = a_text;

    m_str = oRoot.str();

    int     src_no   = 0;
    int     src_proc = 0;
    int     dest_no  = 0;
    int     dest_proc= 0;

    a_pCls->GetSource(src_no, src_proc);
    a_pCls->GetDestination(dest_no, dest_proc);

    a_pCls->SetFlagResponse();
    a_pCls->SetSource(dest_no, dest_proc);
    a_pCls->SetDestination(src_no, src_proc);
    a_pCls->SetPayload(m_str);

}