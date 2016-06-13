#include <unistd.h>
#include <openssl/sha.h>
#include <termio.h>
#include "CLC.h"
#include "CGlobal.hpp"
#include "CConsole.hpp"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

CConsole::CConsole(CThreadQueue<CConsoleSession*> *a_cRcvQueue, CThreadQueue<CConsoleSession*> *a_cSndQueue)
{
	int nRet = 0;

    m_cRcvQueue = a_cRcvQueue;
    m_cSndQueue = a_cSndQueue;

	m_nSessionSeq = 0;

	m_cCli = new CCli(nRet);
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,"Cli init failed\n");
		return;
	}

	m_nLoginRetryCount = 5;

	m_strUserDumpFile= "./USER_INFO.JSON";

	m_strDBName = "ATOM";
	m_strDBIp = "127.0.0.1";

	m_nDBPort = 3306;
	m_strDBUser = "atom";
	m_strDBPassword = "atom";

	nRet = m_cCli->InitDbConfig(m_strDBName.c_str(), m_strDBIp.c_str(), m_nDBPort, m_strDBUser.c_str(), m_strDBPassword.c_str());
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,"Db Config init failed(ret=%d)\n",nRet);
	}

	m_cDb = new MariaDB();
}

CConsole::~CConsole()
{
	delete m_cCli;
	delete m_cDb;

	DropAllUser();
}

int CConsole::InitDBCfg(string a_strDBName, string a_strDBIp, int a_nDBPort, string a_strDBUser, string a_strDBPassword)
{
	int nRet = 0;

	m_strDBName = a_strDBName;
	m_strDBIp = a_strDBIp;
	m_nDBPort = a_nDBPort;
	m_strDBUser = a_strDBUser;
	m_strDBPassword = a_strDBPassword;

	nRet = m_cCli->InitDbConfig(m_strDBName.c_str(), m_strDBIp.c_str(), m_nDBPort, m_strDBUser.c_str(), m_strDBPassword.c_str());
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,"Db Config init failed(ret=%d)\n",nRet);
	}

	return CLC_OK;
}

int CConsole::InitPathCfg(string &a_strLocalPath, string &a_strUsrDumpFile, string &a_strCmdDumpFile)
{
	m_strCmdDumpFile = a_strCmdDumpFile;
	m_strUserDumpFile = a_strUsrDumpFile;

	m_cCli->SetLocalPath(a_strLocalPath);
	m_cCli->SetDumpFile(a_strCmdDumpFile);

	return CLC_OK;
}

int CConsole::LoadAllUserFromDb()
{
	int blnRet = 0;
	int nRet = 0;
	int nQueryLen = 0;
	char chQuery[1024];
	char szUserId[CLC_USER_ID_LEN];
	char szPasswd[CLC_PASSWD_LEN];
	CUser *cUser = NULL;
	FetchMaria cData;

	blnRet = m_cDb->Connect(m_strDBIp.c_str(), m_nDBPort, m_strDBUser.c_str(), m_strDBPassword.c_str(), m_strDBName.c_str());
	if(blnRet == false){
		CLC_LOG(CLC_ERR,"DB Connect failed(%s, %d, %s, %s, %s)\n",
				m_strDBIp.c_str(), m_nDBPort, m_strDBUser.c_str(), m_strDBPassword.c_str(), m_strDBName.c_str());
		return CLC_NOK;
	}

	/* load Command */
	nQueryLen = snprintf(chQuery, sizeof(chQuery), "SELECT USER_ID, PASSWD FROM TAT_USER");

	nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
	if(nRet < 0){
		CLC_LOG(CLC_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
		return CLC_NOK;
	}

	//cData.Clear();

	cData.Set(szUserId, sizeof(szUserId));
	cData.Set(szPasswd, sizeof(szPasswd));

	while(1){
		if(cData.Fetch() == false){
			break;
		}

		cUser = new CUser(szUserId, szPasswd);

		lstUser.push_back(cUser);
	}

	m_cDb->Close();

	return CLC_OK;
}

int CConsole::DropAllUser()
{
	while(lstUser.size()){
		delete lstUser.front();
		lstUser.pop_front();
	}

	return CLC_OK;
}

int CConsole::LoadAllUserFromCfg(string &a_strPath)
{
	unsigned int i = 0;
	char szBuffer[1024];
	FILE *fp = NULL;
	string strUserId;
	string strPasswd;
	CRabbitAlloc cAlloc;
	list<CUser*>::iterator iter;
	CUser *cUser = NULL;
	string strJson;
	string strTmp;

	DropAllUser();

	fp = fopen(a_strPath.c_str(),"r");
	if(fp == NULL){
		CLC_LOG(CLC_ERR,"Config file open error(%s)\n", a_strPath.c_str());
		return CLC_NOK;
	}

	size_t len = 0;

	while(!feof(fp)){
		len = fread(szBuffer,sizeof(szBuffer[0]), sizeof(szBuffer) , fp);
		strJson.append(szBuffer,len);
	}

	fclose(fp);

	try {
		rabbit::document doc;
		rabbit::array cUserArray;

		doc.parse(strJson);

		cUserArray = doc["USER_LIST"];

		for(i=0;i<cUserArray.size();i++){
			rabbit::object cObject;

			cObject = cUserArray[i];

			strUserId = cObject["USER"].as_string();
			strPasswd = cObject["PASSWD"].as_string();

			cUser = new CUser(strUserId.c_str(), strPasswd.c_str());

			lstUser.push_back(cUser);
		}/* end of for(i=0;i<cUserArray.size();i++) */

	} catch(rabbit::type_mismatch   e) {
		CLC_LOG(CLC_ERR,e.what());
		return CLC_NOK;
	} catch(rabbit::parse_error e) {
		CLC_LOG(CLC_ERR,e.what());
		return CLC_NOK;
	} catch(...) {
		CLC_LOG(CLC_ERR,"Unknown error\n");
		return CLC_NOK;
	}

	return CLC_OK;
}

int CConsole::DumpAllUserToCfg()
{
	CRabbitAlloc cAlloc;
	list<CUser*>::iterator iter;
	CUser *cUser = NULL;
	string strTmp;

	try {
		rabbit::object cBody;
		rabbit::array cArgArray;
		rabbit::object *cRecord = NULL;

		cArgArray = cBody["USER_LIST"];

		for(iter = lstUser.begin(); iter != lstUser.end(); iter++){
			cUser = *iter;

			cRecord = cAlloc.GetRabbitObject();

			strTmp = cUser->GetUserId();
			(*cRecord)["USER"] = strTmp;
			strTmp = cUser->GetPasswd();
			(*cRecord)["PASSWD"] = strTmp;

			cArgArray.push_back(*cRecord);
		}/* end of for(iter = lstUser.begin(); iter != listUser.end(); iter++) */

		{
			FILE *fp = NULL;
			string strBuffer;

			fp = fopen(m_strUserDumpFile.c_str(),"w");
			if(fp == NULL){
				CLC_LOG(CLC_ERR,"File open failed(file=%s, errno=%d(%s)\n",
						m_strUserDumpFile.c_str(), errno, strerror(errno));
				return CLC_NOK;
			}
			rapidjson::Document doc;

			doc.Parse(cBody.str().c_str());

			rapidjson::GenericStringBuffer<rapidjson::UTF8<>> buffer;
			rapidjson::PrettyWriter<
				rapidjson::GenericStringBuffer<rapidjson::UTF8<>>
				> writer(buffer);

			doc.Accept(writer);

			strBuffer = buffer.GetString();
			fprintf(fp,"%s",strBuffer.c_str());

			fclose(fp);

		}
	} catch(rabbit::type_mismatch   e) {
		CLC_LOG(CLC_ERR,e.what());
		return CLC_NOK;
	} catch(rabbit::parse_error e) {
		CLC_LOG(CLC_ERR,e.what());
		return CLC_NOK;
	} catch(...) {
		CLC_LOG(CLC_ERR,"Unknown error\n");
		return CLC_NOK;
	}

	return CLC_OK;
}

int CConsole::CheckUser(const char *a_szUserId, const char *a_szPasswd)
{
	int nRet = 0;
	list<CUser*>::iterator iter;
	CUser *cUser = NULL;

	for(iter = lstUser.begin(); iter != lstUser.end(); iter++){
		cUser = *iter;

		nRet = cUser->CheckUserId(a_szUserId);
		if(nRet != CLC_OK){
			continue;
		}

		nRet = cUser->CheckPasswd(a_szPasswd);
		if(nRet == CLC_OK){
			return CLC_OK;
		}
	}/* end of for(iter = lstUser.begin(); iter != listUser.end(); iter++) */

	return CLC_NOK;
}

CConsoleSession *CConsole::MakeSession(unsigned int a_nType)
{
	time_t sndTime = 0;
	CConsoleSession *cSession = NULL;

	sndTime = time(NULL);
	cSession = new CConsoleSession(a_nType, m_nSessionSeq++, sndTime);

	return cSession;
}

int CConsole::Display(CConsoleSession *a_cSession)
{
	unsigned int nResultCode = 0;
	unsigned int nCmdCode = 0;
	string *strCmd;
	string *strCmdDesc;
	string strResult;
	string *strResponse = NULL;

	a_cSession->GetResultCode(&nResultCode, &strResult);

	strResponse = a_cSession->GetResponseP();

	a_cSession->GetCmdP(&nCmdCode, &strCmd, &strCmdDesc); 
	m_cCli->DisplayHeader(a_cSession->GetTime(), nCmdCode, *strCmd, *strCmdDesc);
	m_cCli->Display(nResultCode, strResult, *strResponse);

	return CLC_OK;
}

int CConsole::Login(unsigned int a_nRetryCnt)
{
	int nRet = 0;
	unsigned int nLen = 0;
	char szInputUserId[CLC_USER_ID_LEN];
	char szInputPasswd[CLC_PASSWD_LEN];
	unsigned char szHash[SHA256_DIGEST_LENGTH];
	char szPasswd[SHA256_DIGEST_LENGTH*2+1];
	struct termio orgTermIo;
	struct termio newTermIo;

	while(a_nRetryCnt--){
		fprintf(stderr,"Enter user-name: ");
		fgets(szInputUserId, CLC_USER_ID_LEN, stdin);

		ioctl(0,TCGETA, &orgTermIo);
		ioctl(0,TCGETA, &newTermIo);

		newTermIo.c_lflag &= ~(ECHO);

		ioctl(0,TCSETA, &newTermIo);

		fprintf(stderr,"Enter password: ");
		fgets(szInputPasswd, CLC_PASSWD_LEN, stdin);
		ioctl(0,TCSETA, &orgTermIo);

		fprintf(stderr,"\n");

		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, szInputPasswd, strlen(szInputPasswd)-1);
		SHA256_Final(&szHash[0], &sha256);

		for(int i=0;i<SHA256_DIGEST_LENGTH;i++){
			sprintf(&szPasswd[i*2], "%02x", szHash[i]);
		}

		nLen = strlen(szInputUserId);
		if(nLen != 0){
			szInputUserId[nLen-1] = '\0';
		}
		nRet = CheckUser(szInputUserId, szPasswd);
		if(nRet == CLC_OK){
			m_strUserId = szInputUserId;
			return CLC_OK;
		}

		fprintf(stderr,"Login Incorrent\n\n");
	}

	fprintf(stderr, "Login failed. Exit CLI...\n");

	return CLC_NOK;
}

int CConsole::Run()
{
	int nRet = 0;
	string pkgName = "ATOM";
	CConsoleSession *cSession = NULL;
	string strMessage;
	unsigned int nCmdCode = 0;
	string strCmd;
	string strCmdDesc;
	string strResult;

	nRet = LoadAllUserFromDb();
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,"db load failed(user information)\n");
		nRet = LoadAllUserFromCfg(m_strUserDumpFile);
		if(nRet != CLC_OK){
			CLC_LOG(CLC_ERR,"User Config load failed(nRet=%d)\n",nRet);
		}
	}
	else {
		DumpAllUserToCfg();
	}

	nRet = Login(m_nLoginRetryCount);
	if(nRet != CLC_OK){
		exit(0);
	}

	cSession = MakeSession(CLC_SESSION_TYPE_LOGIN);

	cSession->SetUserId(m_strUserId);

	/* send to sesson */
	m_cSndQueue->Push(cSession);

	nRet = m_cCli->LoadFromDb();
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,"DB load failed(ret=%d, load local configuration)\n",nRet);

		nRet = m_cCli->LoadFromCfg(m_strCmdDumpFile);
		if(nRet != CLC_OK){
			CLC_LOG(CLC_ERR,"load Configuration failed(ret=%d)\n",nRet);
		}
	}
	else {
		nRet = m_cCli->Dump();
		if(nRet != CLC_OK){
			CLC_LOG(CLC_ERR,"Command list backup failed(ret=%d)\n",nRet);
		}
	}

	nRet = m_cCli->SetDefaultPkg(pkgName);
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,"ATOM Package Default setting failed(ret=%d)\n",nRet);
	}

	while(1){
		nRet = m_cCli->Read();
		if(nRet != CLC_OK){
			if(nRet == CLC_RSLT_QUIT){
				delete m_cCli;
				exit(0);
				break;
			}
			continue;
		}

		cSession = MakeSession(CLC_SESSION_TYPE_DATA);

		m_cCli->GetJsonMessage(cSession->GetSessionId(), strMessage);

		cSession->SetRequest(strMessage);

		m_cCli->GetCmd(&nCmdCode, strCmd, strCmdDesc);

		cSession->SetCmd(nCmdCode, strCmd, strCmdDesc);

		/* send to sesson */
		m_cSndQueue->Push(cSession);

		/* wait receiive */
		nRet = m_cRcvQueue->PopWait(&cSession);
		if(nRet != CThreadQueue<CConsoleSession*>::CTHRD_Q_OK){
			CLC_LOG(CLC_ERR,"Console thread queue pop failed(ret=%d)\n",nRet);
			continue;
		}

		nRet = Display(cSession);
		if(nRet != CLC_OK){
			CLC_LOG(CLC_ERR,"Display error(ret=%d)\n",nRet);
		}

		delete cSession;
	}/* end of while (1) */

	return CLC_OK;
}

