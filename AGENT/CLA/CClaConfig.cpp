#include "CLA.h"
#include "CGlobal.hpp"
#include "CClaConfig.hpp"
#include "FetchMaria.hpp"

CClaConfig::CClaConfig(DB *a_cDb, int &nRet)
{
	if(a_cDb == NULL){
		CLA_LOG(CLA_ERR,false,"DB infomation not exist\n");
		nRet = CLA_NOK;
		return;
	}

	m_cDb = a_cDb;
	nRet = CLA_OK;

}

CClaConfig::~CClaConfig()
{
}

int CClaConfig::DBLoadCmdDest(CCliRoute *cCliRoute)
{
	int nRet = 0;
	int nQueryLen = 0;
	char chQuery[1024];
	FetchMaria cData;
	char szCmdCode[STRING_NUMBER_LEN];
	char szCmdName[CMD_NAME_LEN];
	char szPkgName[PKG_NAME_LEN];
	char szDest[CMD_DEST_LEN];

	nQueryLen = snprintf(chQuery, sizeof(chQuery),
			"SELECT TAT_CMD_DEST.CMD_CODE, CMD_NAME, TAT_CMD_DEST.PKG_NAME, CMD_EMS_DESTINATION\
			FROM TAT_CMD_DEST, TAT_CMD WHERE TAT_CMD.CMD_CODE = TAT_CMD_DEST.CMD_CODE AND TAT_CMD.PKG_NAME = TAT_CMD_DEST.PKG_NAME");
	nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
	if(nRet < 0){
		CLA_LOG(CLA_ERR,false,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
		return CLA_NOK;
	}

	//cData.Clear();

	cData.Set(szCmdCode, sizeof(szCmdCode));
	cData.Set(szCmdName, sizeof(szCmdName));
	cData.Set(szPkgName, sizeof(szPkgName));
	cData.Set(szDest, sizeof(szDest));

	while(1){
		if(cData.Fetch() == false){
			break;
		}

		nRet =cCliRoute->Insert(atoi(cData.Get(0)), cData.Get(1), cData.Get(2), (unsigned int)atoi(cData.Get(3)));
		if(nRet != CLA_OK){
			CLA_LOG(CLA_ERR,false,"Route informaiton insert failed(nRet=%d)\n",nRet);
		}
	} /* end of while(1) */


	return CLA_OK;
}

