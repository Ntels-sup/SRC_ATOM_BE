#include <errno.h>

#include "CommandFormat.hpp"
#include "CGlobal.hpp"
#include "CLA.h"
#include "MariaDB.hpp"
#include "CModule.hpp"
#include "CClaConfig.hpp"
#include "CModuleIPC.hpp"
#include "CCliApi.hpp"
#include "CCliReq.hpp"
#include "CCliRsp.hpp"
#include "CMain.hpp"

CMain::CMain()
{
	m_blnInitFlag = false;

	m_nTimeOut = 5;
	m_nSessionSeq = 0;
    m_cHistLog = NULL;

	m_cDb = NULL;

}

CMain::~CMain()
{
    unsigned int i = 0;
    CCliPeer *cPeer = NULL;

    if(m_cHistLog != NULL){
        delete m_cHistLog;
    }

    for(i=0;i<m_lstPeer.size();i++){
        cPeer = m_lstPeer.front();
        m_lstPeer.pop_front();
        delete cPeer;
    }
}

int CMain::UpdateHistErr(CSession *a_cSession, unsigned int a_nResultCode)
{
	int nRet = 0;
	int fd = 0;
    time_t curTm = 0;
	unsigned int nQueryLen = 0;
	char szQuery[2096];
	string strIp;
	int nPort = 0;
	CCliPeer *cPeer = NULL;
	CCliReq *decReq = NULL;
	string strCmdName;
	string strResultCode; 
	CGlobal *cGlob = NULL;

	cGlob = CGlobal::GetInstance();

	fd = a_cSession->GetPeerFd();

	cPeer = FindPeerByFd(fd);
	if(cPeer == NULL){
		CLA_LOG(CLA_ERR,false,"Can not find peer(fd=%d)\n",fd);
		delete a_cSession;
		return CLA_NOK;
	}

	switch(a_nResultCode){
		case CLA_RSLT_CODE_TIME_OUT:
			strResultCode = "MESSAGE TIME OUT";
			break;
		case CLA_RSLT_CODE_SEND_ERROR:
			strResultCode = "MESSAGE SEND ERROR";
			break;
	}

	cPeer->GetPeerIpPort(&strIp , &nPort);

	decReq = a_cSession->GetDecReqP();
	strCmdName = decReq->GetCmdName();

    curTm = time(NULL);

    if(m_cHistLog != NULL){
        char szTime[62];
        struct tm *t = NULL;
        //unsigned int nResultCode = 0;
        string strResult;

        t = localtime(&curTm);

        strftime(szTime, 62, "%04Y-%02m-%02d %02H:%02M:%02S",t);

        m_cHistLog->INFO("[%s] %s\n",cPeer->GetUserId().c_str(), szTime); 
        m_cHistLog->INFO("C%04d %s\n",decReq->GetCmdCode(), decReq->GetCmdName().c_str());

        m_cHistLog->INFO("   RESULT = %d (%s)\n", a_nResultCode, strResultCode.c_str());
        m_cHistLog->INFO("COMPLETED\n");
    }

	nQueryLen = sprintf(szQuery, "INSERT INTO TAT_OPERATION_HIST(USER_ID, PRC_DATE, OPER_TARGET, MENU_ID, PKG_NAME, NODE_NO, OPER_MESSAGE, OPER_CMD, CMD_ARG, OPER_IP, RES_DATE, RESULT) VALUES(\"%s\", FROM_UNIXTIME(%lu), %d, %d, \"%s\", \"%d\", \"%s\", \"%s\", \"%s\", \"%s\", FROM_UNIXTIME(%lu), \"%s\")", 
			cPeer->GetUserId().c_str(),
			a_cSession->GetSendTime(),
			1,
			96,
			decReq->GetPkgName().c_str(),
			cGlob->GetLocalNodeNo(),
			"8",
			strCmdName.c_str(),
			&decReq->GetCmdLine().c_str()[strCmdName.size()+1],
			strIp.c_str(),
            curTm, 
			strResultCode.c_str());

	nRet = m_cDb->Execute(szQuery, nQueryLen);
	if(nRet < 0){
		CLA_LOG(CLA_ERR,false,"(UPDATE ERROR HIST) DB Query Failed(nRet=%d, err=%s)\n",
				nRet, m_cDb->GetErrorMsg(nRet));
		return CLA_NOK;
	}

	return CLA_OK;
}

int CMain::UpdateHist(CCliPeer *a_cPeer, CSession *a_cSession, CCliRsp *a_cDecRsp)
{
    int nRet = 0;
    time_t curTm = 0;
    CGlobal *cGlob = NULL;
    CCliReq *decReq = NULL;
    string strIp;
    int nPort = 0;
    unsigned int nQueryLen = 0;
    string strOperArg;
    unsigned int nResultCode = 0;
    string strResult;
    string strCmdName;
    char szQuery[2096];

    cGlob = CGlobal::GetInstance();

    decReq = a_cSession->GetDecReqP();

    strCmdName = decReq->GetCmdName();
    a_cDecRsp->GetResultCode(nResultCode, strResult);

    a_cPeer->GetPeerIpPort(&strIp , &nPort);

    curTm = time(NULL);

    if(m_cHistLog != NULL){
        char szTime[62];
        struct tm *t = NULL;
        unsigned int nResultCode = 0;
        string strResult;

        t = localtime(&curTm);

        strftime(szTime, 62, "%04Y-%02m-%02d %02H:%02M:%02S",t);

        m_cHistLog->INFO("[%s] %s\n",a_cPeer->GetUserId().c_str(), szTime); 
        m_cHistLog->INFO("C%04d %s\n",decReq->GetCmdCode(), decReq->GetCmdName().c_str());
        if(a_cDecRsp->GetText().size() != 0){
            m_cHistLog->INFO("\n%s\n",a_cDecRsp->GetText().c_str());
        }

        a_cDecRsp->GetResultCode(nResultCode, strResult);
        m_cHistLog->INFO("   RESULT = %d (%s)\n",nResultCode, strResult.c_str());
        m_cHistLog->INFO("COMPLETED\n");
    }

    nQueryLen = sprintf(szQuery, "INSERT INTO TAT_OPERATION_HIST(USER_ID, PRC_DATE, OPER_TARGET, MENU_ID, PKG_NAME, NODE_NO, OPER_MESSAGE, OPER_CMD, CMD_ARG, OPER_IP, RES_DATE, RESULT) VALUES(\"%s\", FROM_UNIXTIME(%lu), %d, %d, \"%s\", \"%d\", \"%s\", \"%s\", \"%s\", \"%s\", FROM_UNIXTIME(%lu), \"%s\")", 
            a_cPeer->GetUserId().c_str(),
            a_cSession->GetSendTime(),
            1,
            96,
            decReq->GetPkgName().c_str(),
            cGlob->GetLocalNodeNo(),
            "8",
            strCmdName.c_str(),
            &decReq->GetCmdLine().c_str()[strCmdName.size()+1],
            strIp.c_str(),
            curTm,
            strResult.c_str());

    nRet = m_cDb->Execute(szQuery, nQueryLen);
    if(nRet < 0){
        CLA_LOG(CLA_ERR,false,"(UPDATE HIST) DB Query Failed(nRet=%d, err=%s)\n",
                nRet, m_cDb->GetErrorMsg(nRet));
        return CLA_NOK;
    }

    return CLA_OK;
}

CCliPeer* CMain::FindPeerByFd(unsigned int a_unFd)
{
	map<unsigned int,list<CCliPeer*>::iterator>::iterator  iter;

	iter = m_mapPeer.find(a_unFd);
	if(iter != m_mapPeer.end()){
		return *iter->second;
	}

	return NULL;
}

CSession* CMain::FindSession(unsigned int a_nSessionId)
{
	CSession *cSession = NULL;
	list<CSession*>::iterator iter;

	for(iter = m_lstSession.begin();iter != m_lstSession.end(); iter++){
		cSession = *iter;
		if(cSession->GetSessionId() == a_nSessionId){

			m_lstSession.erase(iter);

			return cSession;
		}
	}

	return NULL;
}

int CMain::SessionTimeOutHandler()
{
	int nRet = 0;
	time_t curTm = 0;
	CSession *cSession = NULL;

	curTm = time(NULL);

	while(m_lstSession.size()){
		cSession = m_lstSession.front();

		if((curTm - cSession->GetSendTime()) <= m_nTimeOut){
			return CLA_OK;
		}

		CLA_LOG(CLA_ERR,false,"Session timeout(curTm=%lu, sendTime=%lu, diff=%lu sessionId=%d, timeOut=%d)\n",
				curTm, cSession->GetSendTime(), (curTm - cSession->GetSendTime()), cSession->GetSessionId(), m_nTimeOut);
		nRet = UpdateHistErr(cSession, CLA_RSLT_CODE_TIME_OUT);
		if(nRet != CLA_OK){
			CLA_LOG(CLA_ERR,false,"Command history update failed(nRet=%d)\n",nRet);
		}

		delete cSession;
		m_lstSession.pop_front();
	}

	return CLA_OK;
}

int CMain::InsertPeer(CSocket *a_cSock)
{
	time_t curTm;
	CCliPeer *cPeer = NULL;

	curTm = time(NULL);

	cPeer  = new CCliPeer(curTm, a_cSock);

	m_lstPeer.push_back(cPeer);

	m_mapPeer.insert(map<unsigned int,list<CCliPeer*>::iterator>::value_type(a_cSock->GetSocket(), --m_lstPeer.end()));

	return CLA_OK;
}

int CMain::DelPeer(CCliPeer *cPeer)
{
	map<unsigned int,list<CCliPeer*>::iterator>::iterator iter;
	unsigned int fd = 0;

	fd = cPeer->GetSocketFd();

	iter = m_mapPeer.find(fd);
	if(iter == m_mapPeer.end()){
		return CLA_NOK;
	}

	m_lstPeer.erase(iter->second);
	m_mapPeer.erase(iter);

	delete cPeer;

	return CLA_OK;
}

int CMain::RouteHandler(CCliPeer *a_cPeer, CProtocol *cProto)
{
	bool blnRet = 0;
	int nRet = 0;
	time_t curTm = 0;
	CCliReq *cDecReq;
	string strPkgName;
	string strCmdName;
	string strPayload;
	CSession *cSession = NULL;
	CGlobal *cGlob = NULL;
	CModuleIPC *cIpc = NULL;
	int unDstNodeNo = 0;
	int unDstNo = 0;
	unsigned int nClcSessionId = 0;

	strPayload = cProto->GetPayload();

	cDecReq = new CCliReq();

	nRet = cDecReq->DecodeMessage(strPayload);
	if(nRet != CCliApi::RESULT_OK){
		return CLA_NOK;
	}

	curTm = time(NULL);

	if(m_cHistLog != NULL){
		char szTime[62];
		struct tm *t = NULL;

		t = localtime(&curTm);

		strftime(szTime, 62, "%04Y-%02m-%02d %02H:%02M:%02S",t);

		m_cHistLog->INFO("[%s:%s] %s\n",a_cPeer->GetUserId().c_str(), szTime, cDecReq->GetCmdLine().c_str());
	}

	strPkgName = cDecReq->GetPkgName();
	strCmdName = cDecReq->GetCmdName();

	if(strPkgName.size() == 0){
		strPkgName = "ATOM";
	}

	nRet = m_cCliRoute.FindDestinationNo(strCmdName, strPkgName, &unDstNo, &unDstNodeNo);
	if(nRet != CLA_OK){
		CLA_LOG(CLA_ERR,false,"Can not find destination(cmd=%s, pkg=%s)\n", strCmdName.c_str(), strPkgName.c_str());
		return CLA_NOK;
	}

	/* reload message */
	nClcSessionId = cProto->GetSequence();

	cDecReq->SetSessionId(m_nSessionSeq);

	cDecReq->EncodeMessage(strPayload);

	/* make c protocol */
	cGlob = CGlobal::GetInstance();
	cProto->SetPayload(strPayload);
	cProto->SetSequence(m_nSessionSeq);
	cProto->SetSource(cGlob->GetLocalNodeNo(),cGlob->GetLocalProcNo());
	cProto->SetDestination(unDstNodeNo,unDstNo);
	cProto->SetFlagRequest();
	cProto->SetCommand("0000080001");

	cIpc = cGlob->GetModuleIPC();

	/* make session */
	cSession = new CSession();

	cSession->Init(m_nSessionSeq++, a_cPeer->GetSocketFd(),a_cPeer->GetTimestamp(), curTm);
	cSession->SetClcSessionId(nClcSessionId);
	cSession->SetData(unDstNo, unDstNodeNo, cProto, cDecReq);

	blnRet = cIpc->SendMesg(*cProto);
	if(blnRet != true){
		CLA_LOG(CLA_ERR,false,"IPC Send failed(CLI REQUEST)\n");
		a_cPeer->SendError(nClcSessionId, CLA_RSLT_CODE_SEND_ERROR);
		nRet = UpdateHistErr(cSession, CLA_RSLT_CODE_SEND_ERROR);
		delete cSession;
		return CLA_NOK;
	}

	m_lstSession.push_back(cSession);

	return CLA_OK;
}

int CMain::ReceiveCliHandler()
{
	int nRet = 0;
	CSocket *cClient = NULL;
	CCliPeer *cPeer = NULL;
	CProtocol *cProto = NULL;

	cClient = m_cSock.Select(0, 1);
	if(cClient == NULL){
		if(m_cSock.m_enError == CSocket::SOCK_TIMEOUT){
			return CLA_OK;
		}
		else {
			CLA_LOG(CLA_ERR,false,"Socket select failed(err=%d, msg=%s, errno=%d(%s)\n", 
					m_cSock.m_enError,
					m_cSock.m_strErrorMsg.c_str(), 
					errno, strerror(errno));
			return CLA_NOK;
		}
	}

	cPeer = FindPeerByFd(cClient->GetSocket());
	if(cPeer == NULL){
		CLA_LOG(CLA_ERR,false,"Accept peer(fd=%d)\n",cClient->GetSocket());
		nRet = InsertPeer(cClient);
		if(nRet != CLA_OK){
			CLA_LOG(CLA_ERR,false,"Peer insert failed(nRet=%d)\n",nRet);
			return CLA_NOK;
		}
		return CLA_OK;
	}

	nRet = cPeer->ReceiveProcess();
	if(nRet == CLA_RSLT_PROCESS_END){
		return CLA_OK;
	}
	else if(nRet == CLA_RSLT_PEER_CLOSED){
		CLA_LOG(CLA_ERR,false,"Delete peer(fd=%d)\n",cPeer->GetSocketFd());

		//m_cSock.DelPeer(cPeer->GetSocketFd());
		/* close Peer */
		DelPeer(cPeer);

		return CLA_OK;
	}
	else if(nRet != CLA_OK){
		CLA_LOG(CLA_ERR,false,"Peer message receive failed(ret=%d)\n",nRet);
		return CLA_NOK;
	}

	cProto = cPeer->Receive();
	if(cProto == NULL) {
		CLA_LOG(CLA_ERR,false,"Peer message not exist\n");
		return CLA_OK;
	}

	nRet = RouteHandler(cPeer, cProto);
	if(nRet != CLA_OK){
		CLA_LOG(CLA_ERR,false,"Message routing failed\n");
		return CLA_NOK;
	}

	return CLA_OK;
}

int CMain::ReceiveIpcHandler()
{
	int cmdCode = 0;
	string strCmdCode;
	CGlobal *cGlob = NULL;
	CModuleIPC *cIpc = NULL;
	CProtocol cProto;
	char chFlag = 0;
	int nRet = 0;

	cGlob = CGlobal::GetInstance();

	cIpc = cGlob->GetModuleIPC();

	nRet = cIpc->RecvMesg(cGlob->GetLocalProcNo(), cProto, -1);
	if(nRet < 0){
		CLA_LOG(CLA_ERR,false,"Message receive failed(ERR:%s)\n", cIpc->m_strErrorMsg.c_str());
		return CLA_NOK;
	}
	else if(nRet == 0){
		return CLA_OK;
	}

	chFlag = cProto.GetFlag();

	if((chFlag != CProtocol::FLAG_RESPONSE)){
		CLA_LOG(CLA_ERR,false,"Invalid Flag(flag=%d)\n",chFlag);
		return CLA_NOK;
	}

	strCmdCode = cProto.GetCommand();

	cmdCode = CGlobal::GetCmdCode(strCmdCode);

	switch(cmdCode){
		case CMD_CLI_COMMAND:
			{
				unsigned int fd = 0;
				unsigned int nSessionId = 0;
				CCliPeer *cPeer = NULL;
				CSession *cSession = NULL;
				CCliRsp decRsp;
				string strPayload;

				nSessionId = cProto.GetSequence();

				cSession = FindSession(nSessionId);
				if(cSession == NULL){
					CLA_LOG(CLA_ERR,false,"Can not find session(id=%d)\n",nSessionId);
					return CLA_NOK;
				}

				strPayload = cProto.GetPayload();

				decRsp.DecodeMessage(strPayload);

				decRsp.SetSessionId(cSession->GetClcSessionId());

				decRsp.EncodeMessage(strPayload);

				cProto.SetPayload(strPayload);
				cProto.SetSequence(cSession->GetClcSessionId());

				fd = cSession->GetPeerFd();

				cPeer = FindPeerByFd(fd);
				if(cPeer == NULL){
					CLA_LOG(CLA_ERR,false,"Can not find peer(fd=%d)\n",fd);
					delete cSession;
					return CLA_NOK;
				}

				if(cPeer->GetTimestamp() != cSession->GetPeerTimestamp()){
					CLA_LOG(CLA_ERR,false,"Can not find peer(timestamp=%lu, %lu)\n", 
							cPeer->GetTimestamp() ,cSession->GetPeerTimestamp());
					delete cSession;
					return CLA_NOK;
				}

				/* update hist */
				UpdateHist(cPeer, cSession, &decRsp);

				cPeer->Send(cProto);

				delete cSession;
			}
			break;
		default :
			CLA_LOG(CLA_ERR,false,"Invalid cmdCode(%s)\n",cProto.GetCommand().c_str());
			return CLA_NOK;

	};


	return CLA_OK;
}

int CMain::Init(DB *a_cDb, char *a_szCfgFile)
{
    bool blnRet = false;
    int nRet = 0;
    const char *szTmpCfg[2];
    CGlobal *cGlob = NULL;
    CFileLog *cLog = NULL;

    if(m_blnInitFlag == true){
        return CLA_OK;
    }

    m_cDb = a_cDb;

    nRet = m_cConfig.Initialize(a_szCfgFile);
    if(nRet != 0){
        CLA_LOG(CLA_ERR, true,"Config init failed\n");
        return CLA_NOK;
    }

    cGlob = CGlobal::GetInstance();

    cLog = cGlob->GetLogP();

    szTmpCfg[0] = m_cConfig.GetConfigValue("GLOBAL","LOG_PATH");
    if(szTmpCfg[0] == NULL){
        CLA_LOG(CLA_ERR, true,"LOG_PATH not exist\n");
        return CLA_NOK;
    }

    cLog->SetThreadLock();

    nRet = cLog->Initialize(szTmpCfg[0], NULL, (char*)"CLA", 0, LV_ERROR);
    if(nRet != 0){
        CLA_LOG(CLA_ERR, true,"Log init failed(nRet=%d)\n", nRet);
        return CLA_NOK;
    }


    szTmpCfg[0] = m_cConfig.GetConfigValue("CLC","CLI_HIST_FILE");
    if(szTmpCfg[0] != NULL){
        m_cHistLog = new CFileLog();
        m_cHistLog->Initialize(szTmpCfg[0], NULL, (char*)"CLILOG", 0, LV_INFO);
        if(nRet != 0){
            CLA_LOG(CLA_ERR, true,"Hist Log init failed(nRet=%d)\n", nRet);
            return CLA_NOK;

        }
        m_cHistLog->SetThreadLock();
    }


    szTmpCfg[0] = m_cConfig.GetConfigValue("CLA","MESSAGE_TIMEOUT");
    if(szTmpCfg[0] == NULL){
        CLA_LOG(CLA_ERR,true,"MESSAGE_TIMEOUT not exist\n");
        return CLA_NOK;
    }

    m_nTimeOut = atoi(szTmpCfg[0]);


    szTmpCfg[0] = m_cConfig.GetConfigValue("CLA","LISTEN_IP");
    if(szTmpCfg[0] == NULL){
        CLA_LOG(CLA_ERR,false,"LISTEN_IP not exist\n");
        return CLA_NOK;
    }

    szTmpCfg[1] = m_cConfig.GetConfigValue("CLA","LISTEN_PORT");
    if(szTmpCfg[1] == NULL){
        CLA_LOG(CLA_ERR,false,"LISTEN_PORT not exist\n");
        return CLA_NOK;
    }

    blnRet = m_cSock.Listen( szTmpCfg[0], atoi(szTmpCfg[1]));
    if(blnRet == false){
        CLA_LOG(CLI_ERR,false, "Socket listen failed\n");
		return CLA_NOK;
    }

    CClaConfig cConfig(m_cDb, nRet);

    cConfig.DBLoadCmdDest(&m_cCliRoute);

    m_blnInitFlag = true;

	CLA_LOG(CLA_ERR,true,"CLA MODULE START\n");

    return CLA_OK;
}

int CMain::Run()
{

	ReceiveCliHandler();

	SessionTimeOutHandler();
	ReceiveIpcHandler();

	return CLA_OK;
}

CMain g_cMain;
void* CLAStart(void *pArg)
{
	int nRet = 0;
	CGlobal *cGlob = NULL;
	CModule::ST_MODULE_OPTIONS stOption = *static_cast<CModule::ST_MODULE_OPTIONS*>(pArg);

	cGlob = CGlobal::GetInstance();

	cGlob->SetModuleOption(&stOption);

	nRet = g_cMain.Init(stOption.m_pclsDB, stOption.m_szCfgFile);
	if(nRet == CLA_NOK){
		CLA_LOG(CLA_ERR,false,"Init failed(nRet=%d)\n",nRet);
		return NULL;
	}

	g_cMain.Run();

	return NULL;
}

MODULE_OP = {
	"ATOM_NA_CLA",
	false,
	NULL,
	CLAStart,
	NULL
};

