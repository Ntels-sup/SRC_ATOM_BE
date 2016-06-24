#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "CommandFormat.hpp"
#include "CLC.h"
#include "CProtocol.hpp"
#include "CThreadQueue.hpp"
#include "CGlobal.hpp"
#include "CConsoleThread.hpp"
#include "CMain.hpp"

CMain::CMain()
{
	m_nMessageTimeOut = 5;
	m_cConsoleRcvQueue = NULL;
	m_cConsoleSndQueue = NULL;
	m_blnLoginSuccessFlg = false;
}

CMain::~CMain()
{
	delete m_cConsoleRcvQueue;
	delete m_cConsoleSndQueue;

	while(m_lstConsoleSession.size()){
		delete m_lstConsoleSession.front();
		m_lstConsoleSession.pop_front();
	}
}

int CMain::Init()
{
    int nRet = 0;
    uid_t stUserId;
    struct passwd *stUserPw;
    const char *szTmpCfg[5];
	char szTmpPath[1024];
	unsigned int nEndOfPath = 0;
    CGlobal *cGlob = NULL;
    CFileLog *cLog = NULL;
    string strLocalPath;
    string strLocalCfg;
    CClaInterface *cInter = NULL;
    CConsoleThread *cConsoleThread = NULL;
    CThreadQueue<CConsoleSession*> *cSndQueue = NULL;
    CThreadQueue<CConsoleSession*> *cRcvQueue = NULL;

    cSndQueue = new CThreadQueue<CConsoleSession*>();
    cRcvQueue = new CThreadQueue<CConsoleSession*>();

    stUserId = getuid();

    stUserPw = getpwuid(stUserId);

    strLocalCfg = stUserPw->pw_dir;

    strLocalCfg.append("/.atomclirc");

    /* Get local path */
    nRet = m_cConfig.Initialize((char*)strLocalCfg.c_str());
    if(nRet != 0){
        CLC_LOG(CLC_ERR,true,"Local Config Load failed(check \"~/.atomclirc\")\n");
        return CLC_NOK;
    }

    strLocalPath = m_cConfig.GetConfigValue("GLOBAL","LOCAL_PATH");
    if(strLocalPath.size() == 0){
        CLC_LOG(CLC_ERR,true,"LOCAL_PATH config not exist\n");
        return CLC_NOK;
    }

    strLocalCfg = strLocalPath;
    strLocalCfg.append("/CFG/ATOM.cfg");

    //m_cConfig.Initialize();
    nRet = m_cConfig.Initialize((char*)strLocalCfg.c_str());
    if(nRet != 0){
        CLC_LOG(CLC_ERR,true ,"ATOM Config init failed\n");
        return CLC_NOK;
    }

    cGlob = CGlobal::GetInstance();

    cLog = cGlob->GetLogP();

    szTmpCfg[0] = m_cConfig.GetConfigValue("GLOBAL","LOG_PATH");
    if(szTmpCfg[0] == NULL){
        CLC_LOG(CLC_ERR,true,"LOG_PATH config not exist\n")
            return CLC_NOK;
    }
    nRet = cLog->Initialize(szTmpCfg[0], NULL, (char*)"CLC", 0, LV_ERROR);
    if(nRet != 0){
        CLC_LOG(CLC_ERR,true,"Log init failed(nRet=%d)\n", nRet);
        return CLC_NOK;
    }

    cInter = cGlob->GetClaInterfaceP();

    szTmpCfg[0] = m_cConfig.GetConfigValue("CLA","LISTEN_IP");
    if(szTmpCfg[0] == NULL){
        CLC_LOG(CLC_ERR,true,"LISTEN_IP not exist\n");
        return CLC_NOK;
    }
    szTmpCfg[1] = m_cConfig.GetConfigValue("CLA","LISTEN_PORT");
    if(szTmpCfg[1] == NULL){
        CLC_LOG(CLC_ERR,true,"LISTEN_PORT not exist\n");
        return CLC_NOK;
    }

    szTmpCfg[2] = m_cConfig.GetConfigValue("CLC","CONN_RETRY_TIME");
    if(szTmpCfg[2] == NULL){
        CLC_LOG(CLC_ERR,true,"CONN_RETRY_TIME not exist\n");
        return CLC_NOK;
    }

    cInter->SetTransportAddr( szTmpCfg[0], atoi(szTmpCfg[1]), atoi(szTmpCfg[2]));


    szTmpCfg[0] = m_cConfig.GetConfigValue("CLC","MESSAGE_TIMEOUT");
    if(szTmpCfg[0] == NULL){
        CLC_LOG(CLC_ERR,true,"MESSAGE_TIMEOUT not exist\n");
        return CLC_NOK;
    }

    m_nMessageTimeOut = atoi(szTmpCfg[0]);

    cConsoleThread = new CConsoleThread(cRcvQueue, cSndQueue);

    szTmpCfg[0] = m_cConfig.GetConfigValue("GLOBAL","DB_DATABASE");
    if(szTmpCfg[0] == NULL){
        CLC_LOG(CLC_ERR,true,"DB_DATABASE not exist\n");
        return CLC_NOK;
    }

    szTmpCfg[1] = m_cConfig.GetConfigValue("GLOBAL","DB_HOST");
    if(szTmpCfg[1] == NULL){
        CLC_LOG(CLC_ERR,true,"DB_HOST not exist\n");
        return CLC_NOK;
    }

    szTmpCfg[2] = m_cConfig.GetConfigValue("GLOBAL","DB_PORT");
    if(szTmpCfg[2] == NULL){
        CLC_LOG(CLC_ERR,true,"DB_PORT not exist\n");
        return CLC_NOK;
    }

    szTmpCfg[3] = m_cConfig.GetConfigValue("GLOBAL","DB_USER");
    if(szTmpCfg[3] == NULL){
        CLC_LOG(CLC_ERR,true,"DB_USER not exist\n");
        return CLC_NOK;
    }

    szTmpCfg[4] = m_cConfig.GetConfigValue("GLOBAL","DB_PASS");
    if(szTmpCfg[4] == NULL){
        CLC_LOG(CLC_ERR,true,"DB_PATH not exist\n");
        return CLC_NOK;
    }

    cConsoleThread->InitDBCfg( szTmpCfg[0], szTmpCfg[1],
            atoi(szTmpCfg[2]), szTmpCfg[3], szTmpCfg[4]);


    szTmpCfg[0] = m_cConfig.GetConfigValue("CLC","USER_DUMP_FILE");
    if(szTmpCfg[0] == NULL){
        CLC_LOG(CLC_ERR,true,"USER_DUMP_FILE not exist\n");
        return CLC_NOK;
    }

	cGlob->GetPathLen(szTmpCfg[0], &nEndOfPath);
	if(sizeof(szTmpPath) <= nEndOfPath){
        CLC_LOG(CLC_ERR,true,"USER_DUMP_FILE too long\n");
        return CLC_NOK;
	}
	strncpy(szTmpPath, szTmpCfg[0], nEndOfPath);
	szTmpPath[nEndOfPath] = '\0';

	cGlob->ForceDir(szTmpPath);

    szTmpCfg[1] = m_cConfig.GetConfigValue("CLC","CMD_DUMP_FILE");
    if(szTmpCfg[1] == NULL){
        CLC_LOG(CLC_ERR,true,"CMD_DUMP_FILE not exist\n");
        return CLC_NOK;
    }

	cGlob->GetPathLen(szTmpCfg[0], &nEndOfPath);
	if(sizeof(szTmpPath) <= nEndOfPath){
        CLC_LOG(CLC_ERR,true,"CMD_DUMP_FILE too long\n");
        return CLC_NOK;
	}
	strncpy(szTmpPath, szTmpCfg[0], nEndOfPath);
	szTmpPath[nEndOfPath] = '\0';

	fprintf(stderr,"PATH=%s\n",szTmpPath);
	cGlob->ForceDir(szTmpPath);


    cConsoleThread->InitPathCfg( strLocalPath,
            szTmpCfg[0], szTmpCfg[1]);

    szTmpCfg[0] = m_cConfig.GetConfigValue("CLC","LOGIN_RETRY_COUNT");
    if(szTmpCfg[0] == NULL){
        CLC_LOG(CLC_ERR,true,"LOGIN_RETYR_COUNT not exist\n");
        return CLC_NOK;
    }
    cConsoleThread->LoginRetryCntCfg(atoi(szTmpCfg[0]));

    cConsoleThread->Start();

    m_cConsoleRcvQueue = cSndQueue;
    m_cConsoleSndQueue = cRcvQueue;

    return CLC_OK;
}

int CMain::SessionTimeOutHandler(time_t curTime)
{
	CConsoleSession *cSession = NULL;
	time_t sesTime = 0;
	list<CConsoleSession*>::iterator iter; 

	while(m_lstConsoleSession.size()){
		cSession = m_lstConsoleSession.front();
		sesTime = cSession->GetTime();
		if((curTime - sesTime) > m_nMessageTimeOut){
			m_lstConsoleSession.pop_front();

			cSession->SetResultCode(CLC_RSLT_CODE_TIME_OUT,"COMMAND TIMEOUT");
			m_cConsoleSndQueue->Push(cSession);
		}
		else {
			break;
		}
	}

	return CLC_OK;
}

int CMain::ConsoleHandler()
{
	int nRet = 0;
	unsigned int nType = 0;
	CGlobal *cGlob = NULL;
	CClaInterface *cInter = NULL;
	CConsoleSession *cSession = NULL;
	CProtocol cProto; 

	cGlob = CGlobal::GetInstance();

	cInter = cGlob->GetClaInterfaceP();

	nRet = m_cConsoleRcvQueue->Pop(&cSession);
	if(nRet != CThreadQueue<CConsoleSession*>::CTHRD_Q_OK){
		if(nRet == CThreadQueue<CConsoleSession*>::CTHRD_Q_EMPTY){
			return CLC_OK;
		}
		CLC_LOG(CLC_ERR,false,"Queue pop failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	nType = cSession->GetType();
	if(nType == CLC_SESSION_TYPE_LOGIN){
		string strUserId;

		strUserId =cSession->GetUserId();
		cInter->SetUserId(strUserId);

		m_blnLoginSuccessFlg = true;

		delete cSession;
	}
	else if(nType == CLC_SESSION_TYPE_DATA){
		cProto.SetCommand("0000080001");
		cProto.SetFlagRequest();
		cProto.SetSequence(cSession->GetSessionId());
		cProto.SetSource(0, 0);
		cProto.SetDestination(0, 0);

		cProto.SetPayload(*cSession->GetRequestP());

		nRet = cInter->Send(cProto);
		if(nRet != CClaInterface::RESULT_OK){
			CLC_LOG(CLC_ERR,false,"CLA Send failed(ret=%d, %s)\n", nRet, cInter->errBuffer);

			cSession->SetResultCode(CLC_RSLT_CODE_SEND_ERROR ,"MESSAGE SEND ERORR");
			m_cConsoleSndQueue->Push(cSession);

			return CLC_NOK;
		}

		m_lstConsoleSession.push_back(cSession);
	}

	return CLC_OK;
}

CConsoleSession* CMain::FindConsoleSession(unsigned int nSessionId)
{
	CConsoleSession *cSession = NULL;
	list<CConsoleSession*>::iterator iter; 

	for(iter = m_lstConsoleSession.begin();iter != m_lstConsoleSession.end();iter++){
		cSession = *iter;

		if(cSession->GetSessionId() == nSessionId){
			m_lstConsoleSession.erase(iter);
			return cSession;
		}
	}

	return NULL;
}

int CMain::CmdResponseProcess(CCliRsp &a_cDecRsp)
{
	CConsoleSession *cSession = NULL;
	unsigned int nSessionId = 0;
	unsigned int nResultCode = 0;
	string strResult;
	string strText;

	nSessionId = a_cDecRsp.GetSessionId();

	cSession = FindConsoleSession(nSessionId);
	if(cSession == NULL) {
		CLC_LOG(CLC_ERR,false,"Console session not found(id=%d)\n", nSessionId);
		return CLC_NOK;
	}

	a_cDecRsp.GetResultCode(nResultCode, strResult);

	cSession->SetResultCode(nResultCode ,strResult);

	strText = a_cDecRsp.GetText();

	cSession->SetResponse(strText);

	m_cConsoleSndQueue->Push(cSession);

	return CLC_OK;
}

int CMain::ExternalHandler()
{
	int nRet = 0;
	CCliRsp cDecRsp;
	unsigned int nCmdCode = 0;
	string strCmdCode;
	CGlobal *cGlob = NULL;
	CClaInterface *cInter = NULL;
	CProtocol *cProto = NULL;
	vector<char> vecPayload;
	char chFlag = 0;
	cGlob = CGlobal::GetInstance();

	cInter = cGlob->GetClaInterfaceP();

	while(1){
		cProto = cInter->Receive();
		if(cProto == NULL){
			return CLC_OK;
		}

		chFlag = cProto->GetFlag();

		if((chFlag != CProtocol::FLAG_RESPONSE)){
			CLC_LOG(CLC_ERR,false,"Invalid Flag(flag=%d)\n",chFlag);
			delete cProto;
			continue;
		}

		strCmdCode = cProto->GetCommand();

		nCmdCode = CGlobal::GetCmdCode(strCmdCode);

		switch(nCmdCode){
			case CMD_CLI_COMMAND:
				{
					cProto->GetPayload(vecPayload);

					string strPayload;

					strPayload = &vecPayload[0];

					nRet = cDecRsp.DecodeMessage(vecPayload);
					if(nRet != CCliApi::RESULT_OK){
						CLC_LOG(CLC_ERR,false,"Response message decoding failed(ret=%d)\n",nRet);
						delete cProto;
						continue;
					}

					nRet = CmdResponseProcess(cDecRsp);
					if(nRet != CLC_OK){
						CLC_LOG(CLC_ERR,false,"Comamnd response process failed(ret=%d)\n",nRet);
						delete cProto;
						continue;
					}
				}
				break;
			default:
				{
					CLC_LOG(CLC_ERR,false,"Invalid command code(%d)\n",nCmdCode);
					delete cProto;
					continue;
				}
		};

	}

	return CLC_OK;
}

int CMain::Run()
{
	int nRet = 0;
	time_t curTime = 0;
	CGlobal *cGlob = NULL;
	CClaInterface *cInter = NULL;

	nRet = Init();
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,false,"Init failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}


	cGlob = CGlobal::GetInstance();

	cInter = cGlob->GetClaInterfaceP();

	cInter->SetLog(cGlob->GetLogP());

	while(1){
		curTime = time(NULL);

		ConsoleHandler();

		ExternalHandler();

		if(m_blnLoginSuccessFlg == true){
			cInter->Handler();
		}

		SessionTimeOutHandler(curTime);

		usleep(300);
	}

	return CLC_OK;
}

int main()
{
	CMain cMain;

	cMain.Run();
}
