#include "CommandFormat.hpp"
#include "CGlobal.hpp"
#include "CConfig.hpp"
#include "CMain.hpp"
#include "CModule.hpp"
#include "CModuleIPC.hpp"
#include "CAddress.hpp"

CMain::CMain(const char *a_szPackageName, const char *a_szCfgFile, int &nRet)
{
    const char *szTmpCfg = NULL;
    CGlobal *cGlob = NULL;
    CFileLog *cLog = NULL;
    string strPackageName = a_szPackageName;

    nRet = m_cConfig.Initialize((char*)a_szCfgFile);
    if(nRet != 0){
        STA_LOG(STA_ERR,"Config init failed\n");
        nRet = STA_NOK;
        return;
    }

    cGlob = CGlobal::GetInstance();

    cLog = cGlob->GetLogP();

    szTmpCfg = m_cConfig.GetConfigValue("GLOBAL","LOG_PATH");
    if(szTmpCfg == NULL){
        STA_LOG(STA_ERR,"LOG_PATH not exist\n");
        nRet = STA_NOK;
        return;
    }

    nRet = cLog->Initialize(szTmpCfg , NULL, (char*)"STA", 0, LV_DEBUG);
    if(nRet != 0){
        STA_LOG(STA_ERR,"Log init failed(nRet=%d)\n", nRet);
        nRet = STA_NOK;
        return;
    }

    cLog->SetThreadLock();


    m_strStsDumpLogPath = m_cConfig.GetConfigValue("STA","STS_DUMP_LOG_PATH");
    if(m_strStsDumpLogPath.size() == 0){
        STA_LOG(STA_ERR,"STS_DUMP_LOG_PATH not exist\n");
        nRet = STA_NOK;
        return ;
    }

	/* create direcory */
	cGlob->ForceDir(m_strStsDumpLogPath.c_str());

    m_cCollectTableList = new CTableList();
    m_cEmsInfo = new CEmsInfo(m_cCollectTableList, strPackageName, nRet);
    if(nRet != STA_OK){
        nRet = STA_NOK;
        return;
    }

    szTmpCfg = m_cConfig.GetConfigValue("STA","STS_SEND_PERIOD");
    if(szTmpCfg == NULL){
        STA_LOG(STA_ERR,"STS_SEND_PERIOD not exist\n");
        nRet = STA_NOK;
        return;
    }
    m_cAppInfo = new CAppInfo(m_cEmsInfo, strPackageName, m_cCollectTableList, atoi(szTmpCfg));

}

CMain::~CMain()
{
    delete m_cCollectTableList;
    delete m_cAppInfo;
}

int CMain::ReceiveProcess()
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
        STA_LOG(STA_ERR,"Message receive failed(ERR:%s)\n", cIpc->m_strErrorMsg.c_str());
        return STA_NOK;
    }
    else if(nRet == 0){
        return STA_OK;
    }

    chFlag = cProto.GetFlag();

    if((chFlag != CProtocol::FLAG_REQUEST) && 
            (chFlag != CProtocol::FLAG_RESPONSE) && 
            (chFlag != CProtocol::FLAG_NOTIFY) ){
        STA_LOG(STA_ERR,"Invalid Flag(flag=%d)\n",chFlag);
        return STA_NOK;
    }

    strCmdCode = cProto.GetCommand();

    cmdCode = CGlobal::GetCmdCode(strCmdCode);

    switch(cmdCode){
        case CMD_STS_STA: /* REQUEST */
            {
                STA_LOG(STA_ERR,"RECEIVE STA REQUEST\n");
                nRet = m_cEmsInfo->ReceiveStaRequestProcess(cProto);
                if(nRet != STA_OK){
                    STA_LOG(STA_ERR,"Sta Request process failed(nRet=%d)\n",nRet);
                    return STA_NOK;
                }
            }
            break;
        case CMD_STS_STA_RESOURCE: /* REQUEST */
            {
                DEBUG_LOG("Request Resource\n");
                m_cAppInfo->SendResourceMessage();
            }
            break;

        case CMD_STS_APP: /* RESPONSE */
        case CMD_STS_APP_HIST: /* NOTIFY */
            {
                nRet = m_cAppInfo->ReceiveProcess(cProto, cmdCode);
                if(nRet != STA_OK){
                    STA_LOG(STA_ERR,"Application process failed(nRet=%d)\n",nRet);
                    return STA_NOK;
                }
            }
            break;
        case CMD_STS_RSA: /* RESPONSE */
            {
                DEBUG_LOG("Answer Resource\n");
                m_cEmsInfo->ReceiveResourceProcess(cProto);
            }
            break;
        case CMD_STS_STA_HEARTBEAT:
            {
                m_cEmsInfo->SendHeartBeatRsp();
            }
            break;
        default :
            STA_LOG(STA_ERR<"Invalid cmdCode(%s)\n",cProto.GetCommand().c_str());
            return STA_NOK;

    };

    return STA_OK;
}

int CMain::Run()
{
    int nRet = 0;
    time_t currentTime = 0;
    time_t lastAggregationTime = 0;
    bool aggregationFlag = false;

    currentTime = lastAggregationTime = time(NULL);

    //m_cCollectTableList->LoadTableFromFile("./STS_LOG");
    m_cCollectTableList->LoadTableFromFile(m_strStsDumpLogPath);

    while(1){
        nRet = ReceiveProcess();
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"Receive process failed(nRet =%d\n",nRet);
        }

        /* application process */
        m_cAppInfo->SendCollectMessage();

        /* ems process */
        m_cEmsInfo->CollectProcess();

        /* aggr */
        if((currentTime % 60) == 0){
            if(aggregationFlag != true){

#if 0
                std::string strPath(getenv("HOME"));
                strPath += "/LOG/STS_COLLECT";
#endif
                m_cCollectTableList->Aggregation(currentTime, (char*)m_strStsDumpLogPath.c_str());
                lastAggregationTime = currentTime;
                aggregationFlag = true;
            }
        }
        else {
            aggregationFlag = false;
        }

        currentTime = time(NULL);

		usleep(1000);
    }

    return 0;
}

void* STAStart(void * pArg)
{
    int nRet = STA_OK;
    CGlobal *cGlob = NULL;
    CModule::ST_MODULE_OPTIONS stOption = *static_cast<CModule::ST_MODULE_OPTIONS*>(pArg);
    CMain *cMain = NULL;

    cGlob = CGlobal::GetInstance();

    cGlob->SetModuleOption(&stOption);

    cMain = new CMain(stOption.m_szPkgName, stOption.m_szCfgFile, nRet);
    if(nRet != STA_OK){
        DEBUG_LOG("CMain init failed(nRet=%d)\n",nRet);
        return NULL;
    }
    cMain->Run();

    return NULL;
}

MODULE_OP = {
    "ATOM_NA_STA",
    true,
    NULL,
    STAStart,
    NULL
};
