#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <fstream>
#include "CLC.h"
#include "CGlobal.hpp"
#include "CCliDoc.hpp"
#include "CCli.hpp"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

void ErrLogFunc(unsigned int level, char *file, unsigned int line, char *buf)
{
	CLC_LOG(CLC_ERR,"%s", buf);
}

void DispFunc(char *buf)
{
	fprintf(stderr,"%s",buf);
}

CCli::CCli(int &nRet)
{
	nRet = cli_mainInit(&m_stCliCb);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Cli init failed(ret=%d)\n",nRet);
		nRet = CLC_NOK;
		return;
	}

	m_strDefaultPkgName = "DFLT";

	m_strDumpFile = "./CMD_DUMP.JSON";

	cli_globSetLogFunc(CLI_ERR, ErrLogFunc);
	cli_globSetDispFunc( DispFunc);
	cli_globSetParseLogFunc( DispFunc);

	cli_rlSetPrmt("ATOM> ", 7);

	m_blnDbInitFlag = false;
	m_stCliDoc = NULL;
	m_cDb = NULL;

	m_cDb = new MariaDB();

	nRet = LoadDfltCmd();
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,"Default command load failed(nRet=%d)\n",nRet);
	}

	nRet = CLC_OK;
}

CCli::~CCli()
{
	int nRet = 0;
	if(m_stCliDoc != NULL){
		nRet = cli_docDstry(m_stCliDoc);
		if(nRet != CLI_OK){
			CLC_LOG(CLC_ERR,"Cli doc destory failed(ret=%d)\n",nRet);
		}
		m_stCliDoc = NULL;
	}

	nRet = cli_mainDstry(&m_stCliCb, CLI_TRUE);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Cli doc destory failed(ret=%d)\n",nRet);
	}

	if(m_cDb != NULL){
		delete m_cDb;
	}
}

int CCli::InitDbConfig(const char *a_szDBName, const char *a_szDBIp, int a_nDBPort,
		                const char *a_szDBUser, const char *a_szDBPassword)
{
	m_strDBName = a_szDBName;
	m_strDBIp = a_szDBIp;
	m_nDBPort = a_nDBPort;
	m_strDBUser = a_szDBUser;
	m_strDBPassword = a_szDBPassword;

	m_blnDbInitFlag = true;

	return CLC_OK;
}

int CCli::LoadDfltCmdDispNa(CliPkgCb *pkgCb)
{
	int nRet = 0;
	CliCmdCfg cmdCfg;

	CLI_CMD_CFG_INIT(&cmdCfg);

	cmdCfg.cmdCode = CMD_CODE_DISP_NA;
	cmdCfg.cmdName = "DISP-NA";
	cmdCfg.cmdNameLen = strlen(cmdCfg.cmdName);
	cmdCfg.desc = "Display NA Status";
	cmdCfg.descLen = strlen(cmdCfg.desc);
	cmdCfg.help = "DISP-NA;";
	cmdCfg.helpLen = strlen(cmdCfg.help);
	
	nRet = cli_cmdAdd(pkgCb, &cmdCfg, NULL);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"edit cfg command add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	return CLC_OK;
}

int CCli::LoadDfltCmdStartNa(CliPkgCb *pkgCb)
{
	int nRet = 0;
	CliCmdCfg cmdCfg;

	CLI_CMD_CFG_INIT(&cmdCfg);

	cmdCfg.cmdCode = CMD_CODE_START_NA;
	cmdCfg.cmdName = "START-NA";
	cmdCfg.cmdNameLen = strlen(cmdCfg.cmdName);
	cmdCfg.desc = "START NA Status";
	cmdCfg.descLen = strlen(cmdCfg.desc);
	cmdCfg.help = "START-NA;";
	cmdCfg.helpLen = strlen(cmdCfg.help);
	
	nRet = cli_cmdAdd(pkgCb, &cmdCfg, NULL);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"edit cfg command add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	return CLC_OK;
}

int CCli::LoadDfltCmdStopNa(CliPkgCb *pkgCb)
{
	int nRet = 0;
	CliCmdCfg cmdCfg;
	CliArgCfg argCfg;
    CliCmdCb *cmdCb = NULL;
    CliArgCb *argCb = NULL;
	CliArgEnumCfg argEnumCfg;

	CLI_CMD_CFG_INIT(&cmdCfg);

	cmdCfg.cmdCode = CMD_CODE_STOP_NA;
	cmdCfg.cmdName = "STOP-NA";
	cmdCfg.cmdNameLen = strlen(cmdCfg.cmdName);
	cmdCfg.desc = "STOP NA Status";
	cmdCfg.descLen = strlen(cmdCfg.desc);
	cmdCfg.help = "STOP-NA;";
	cmdCfg.helpLen = strlen(cmdCfg.help);
	
	nRet = cli_cmdAdd(pkgCb, &cmdCfg, &cmdCb);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"stop na command add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}
    
	CLI_ARG_CFG_INIT(&argCfg);
	argCfg.argName = "PID";
	argCfg.argNameLen = strlen(argCfg.argName);
	argCfg.argType = CLI_ARG_TYPE_NUMBER;
	argCfg.optFlg = CLI_TRUE;
	argCfg.multiFlg = CLI_FALSE;
	argCfg.rngMin = 0;
	argCfg.rngMax = 10;

	nRet = cli_argAdd(cmdCb, &argCfg, &argCb);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Load command argument add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	CLI_ARG_CFG_INIT(&argCfg);
	argCfg.argName = "SIG";
	argCfg.argNameLen = strlen(argCfg.argName);
	argCfg.argType = CLI_ARG_TYPE_ENUM;
	argCfg.optFlg = CLI_TRUE;
	argCfg.multiFlg = CLI_FALSE;
	argCfg.rngMin = 0;
	argCfg.rngMax = 10;

	nRet = cli_argAdd(cmdCb, &argCfg, &argCb);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Load command argument add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	CLI_ARG_ENUM_CFG_INIT(&argEnumCfg);
	argEnumCfg.enumName = "TERM";
	argEnumCfg.enumNameLen = strlen(argEnumCfg.enumName);
	argEnumCfg.enbFlg = CLI_TRUE;
	argEnumCfg.enumValFlg = CLI_TRUE;
    argEnumCfg.enumValType = CLI_ARG_TYPE_NUMBER;
    argEnumCfg.u.num.enumVal = SIGTERM;

	nRet = cli_argEnumAdd(argCb, &argEnumCfg, NULL);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Load command argument enum value add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	CLI_ARG_ENUM_CFG_INIT(&argEnumCfg);
	argEnumCfg.enumName = "SEGV";
	argEnumCfg.enumNameLen = strlen(argEnumCfg.enumName);
	argEnumCfg.enbFlg = CLI_TRUE;
	argEnumCfg.enumValFlg = CLI_TRUE;
    argEnumCfg.enumValType = CLI_ARG_TYPE_NUMBER;
    argEnumCfg.u.num.enumVal = SIGSEGV;

	nRet = cli_argEnumAdd(argCb, &argEnumCfg, NULL);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Load command argument enum value add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	return CLC_OK;
}

int CCli::LoadDfltCmdEditCfg(CliPkgCb *pkgCb)
{
	int nRet = 0;
	CliCmdCfg cmdCfg;

	CLI_CMD_CFG_INIT(&cmdCfg);

	cmdCfg.cmdCode = CMD_CODE_EDIT_CFG;
	cmdCfg.cmdName = "EDIT-CFG";
	cmdCfg.cmdNameLen = strlen(cmdCfg.cmdName);
	cmdCfg.desc = "Edit configuration";
	cmdCfg.descLen = strlen(cmdCfg.desc);
	cmdCfg.help = "EDIT-CFG;";
	cmdCfg.helpLen = strlen(cmdCfg.help);
	
	nRet = cli_cmdAdd(pkgCb, &cmdCfg, NULL);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"edit cfg command add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	return CLC_OK;
}

int CCli::LoadDfltCmdDump(CliPkgCb *pkgCb)
{
	int nRet = 0;
	CliCmdCfg cmdCfg;

	CLI_CMD_CFG_INIT(&cmdCfg);

	cmdCfg.cmdCode = CMD_CODE_DUMP;
	cmdCfg.cmdName = "DUMP";
	cmdCfg.cmdNameLen = strlen(cmdCfg.cmdName);
	cmdCfg.desc = "Backup all commands";
	cmdCfg.descLen = strlen(cmdCfg.desc);
	cmdCfg.help = "DUMP;";
	cmdCfg.helpLen = strlen(cmdCfg.help);
	
	nRet = cli_cmdAdd(pkgCb, &cmdCfg, NULL);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Dump command add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	return CLC_OK;
}

int CCli::LoadDfltCmdLoad(CliPkgCb *pkgCb)
{
	int nRet = 0;
	CliCmdCb *cmdCb = NULL;
	CliArgCb *argCb = NULL;
	CliCmdCfg cmdCfg;
	CliArgCfg argCfg;
	CliArgEnumCfg argEnumCfg;

	CLI_CMD_CFG_INIT(&cmdCfg);
	CLI_ARG_CFG_INIT(&argCfg);

	cmdCfg.cmdCode = CMD_CODE_LOAD;
	cmdCfg.cmdName = "LOAD";
	cmdCfg.cmdNameLen = strlen(cmdCfg.cmdName);
	cmdCfg.desc = "Load all commands";
	cmdCfg.descLen = strlen(cmdCfg.desc);
	cmdCfg.help = "LOAD:LOCATION(DB or CONFIG);";
	cmdCfg.helpLen = strlen(cmdCfg.desc);
	
	nRet = cli_cmdAdd(pkgCb, &cmdCfg, &cmdCb);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Dump command add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	argCfg.argName = "LOCATION";
	argCfg.argNameLen = strlen(argCfg.argName);
	argCfg.argType = CLI_ARG_TYPE_ENUM;
	argCfg.optFlg = CLI_FALSE;
	argCfg.multiFlg = CLI_FALSE;
	argCfg.rngMin = 0;
	argCfg.rngMax = 10;

	nRet = cli_argAdd(cmdCb, &argCfg, &argCb);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Load command argument add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	CLI_ARG_ENUM_CFG_INIT(&argEnumCfg);
	argEnumCfg.enumName = "DB";
	argEnumCfg.enumNameLen = strlen(argEnumCfg.enumName);
	argEnumCfg.enbFlg = CLI_TRUE;
	argEnumCfg.enumValFlg = CLI_FALSE;

	nRet = cli_argEnumAdd(argCb, &argEnumCfg, NULL);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Load command argument enum value add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	CLI_ARG_ENUM_CFG_INIT(&argEnumCfg);
	argEnumCfg.enumName = "CFG";
	argEnumCfg.enumNameLen = strlen(argEnumCfg.enumName);
	argEnumCfg.enbFlg = CLI_TRUE;
	argEnumCfg.enumValFlg = CLI_FALSE;

	nRet = cli_argEnumAdd(argCb, &argEnumCfg, NULL);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Load command argument enum value add failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}



	return CLC_OK;
}

int CCli::LoadDfltCmd()
{
    int nRet = 0;
    CliPkgCb *stPkgCb = NULL;

    nRet = cli_pkgAdd(&m_stCliCb, CLI_FALSE, m_strDefaultPkgName.c_str(), m_strDefaultPkgName.size(), NULL, &stPkgCb);
    if(nRet != CLI_OK){
        CLC_LOG(CLC_ERR,"Package add failed(%d)\n",nRet);
        return CLC_NOK;
    }

    nRet = LoadDfltCmdDump(stPkgCb);
    if(nRet != CLC_OK){
        CLC_LOG(CLC_ERR,"dump command load failed\n");
        return CLC_NOK;
    }

    nRet = LoadDfltCmdLoad(stPkgCb);
    if(nRet != CLC_OK){
        CLC_LOG(CLC_ERR,"load command load failed\n");
        return CLC_NOK;
    }

    nRet = LoadDfltCmdEditCfg(stPkgCb);
    if(nRet != CLC_OK){
        CLC_LOG(CLC_ERR,"edit config command load failed\n");
        return CLC_NOK;
    }

    nRet = LoadDfltCmdDispNa(stPkgCb);
    if(nRet != CLC_OK){
        CLC_LOG(CLC_ERR,"displaly na command load failed\n");
        return CLC_NOK;
    }

    nRet = LoadDfltCmdStopNa(stPkgCb);
    if(nRet != CLC_OK){
        CLC_LOG(CLC_ERR,"stop na command load failed\n");
        return CLC_NOK;
    }

    nRet = LoadDfltCmdStartNa(stPkgCb);
    if(nRet != CLC_OK){
        CLC_LOG(CLC_ERR,"start na command load failed\n");
        return CLC_NOK;
    }

    return CLC_OK;
}

int CCli::LoadArgEnumParaFromDb(CliArgCb *a_stArgCb, const char *a_szTableName, const char *a_szColumnName, int a_nColumnType)
{
	int nRet = 0;
	int nQueryLen = 0;
	char chQuery[1024];
	char szArgEnumValue[CLI_ENUM_VALUE_LEN+1];
	CliArgEnumCfg argEnumCfg;
	FetchMaria cData;

	CLI_ARG_ENUM_CFG_INIT(&argEnumCfg);

	/* load Command */
	nQueryLen = snprintf(chQuery, sizeof(chQuery),
			"SELECT %s FROM %s\n",
			a_szColumnName, a_szTableName);

	nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
	if(nRet < 0){
		CLC_LOG(CLC_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
		return CLC_NOK;
	}

	//cData.Clear();

	cData.Set(szArgEnumValue, sizeof(szArgEnumValue));

	while(1){
		if(cData.Fetch() == false){
			break;
		}

		argEnumCfg.enumNameLen = strlen(cData.Get(0));
		argEnumCfg.enumName = (char*)cData.Get(0);
		argEnumCfg.enbFlg = CLI_TRUE;
		argEnumCfg.enumValFlg = CLI_TRUE;
		if(a_nColumnType == CLI_ARG_TYPE_NUMBER){
			argEnumCfg.enumValType = CLI_ARG_TYPE_NUMBER;
			argEnumCfg.u.num.enumVal = atoi(cData.Get(0));
		}
		else {
			argEnumCfg.enumValType = CLI_ARG_TYPE_STRING;
			argEnumCfg.u.str.enumValLen = strlen(cData.Get(0));
			argEnumCfg.u.str.enumVal = (char*)cData.Get(0);
		}

		nRet = cli_argEnumAdd(a_stArgCb, &argEnumCfg, NULL);
		if(nRet != CLI_OK){
			CLC_LOG(CLC_ERR,"argument enum add failed(ret=%d)\n",nRet);
			continue;
		}
	}/* end of while(1) */

	return CLC_OK;
}

int CCli::LoadArgEnumFromDb(string &a_strPkgName, CliCmdCb *a_stCmdCb, CliArgCb *a_stArgCb)
{
	int nRet = 0;
	int nQueryLen = 0;
	char chQuery[1024];
	int nColumnType = 0;
	bool blnDbUpdateFlag = false;
	CliArgEnumCb *argEnumCb = NULL;
	char szArgEnumName[CLI_ENUM_NAME_LEN+1];
	char szArgEnumValueType[CLC_ARG_TYPE_LEN];
	char szArgEnumValue[CLI_ENUM_VALUE_LEN+1];
	char szArgEnumTable[CLC_ARG_ENUM_TABLE_LEN];
	char szArgEnumColumn[CLC_ARG_ENUM_COLUMN_LEN];
	int tmpLen = 0;
	char tmp[CLC_ARG_ENUM_TABLE_LEN + CLC_ARG_ENUM_COLUMN_LEN];
	CliArgEnumCfg argEnumCfg;
	FetchMaria cData;

	CLI_ARG_ENUM_CFG_INIT(&argEnumCfg);

	/* load Command */
	nQueryLen = snprintf(chQuery, sizeof(chQuery),
			"SELECT ARG_ENUM_NAME, ARG_ENUM_VALUE_TYPE, ARG_ENUM_VALUE , ARG_ENUM_TABLE, ARG_ENUM_COLUMN FROM TAT_CMD_ARG_ENUM WHERE CMD_CODE=%d AND PKG_NAME=\"%s\" AND ARG_NAME=\"%.*s\"", 
			a_stCmdCb->cmdCode, a_strPkgName.c_str(), a_stArgCb->argNameLen, a_stArgCb->argName);

	nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
	if(nRet < 0){
		CLC_LOG(CLC_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
		return CLC_NOK;
	}

	//cData.Clear();

	cData.Set(szArgEnumName, sizeof(szArgEnumName));
	cData.Set(szArgEnumValueType, sizeof(szArgEnumValueType));
	cData.Set(szArgEnumValue, sizeof(szArgEnumValue));
	cData.Set(szArgEnumTable, sizeof(szArgEnumTable));
	cData.Set(szArgEnumColumn, sizeof(szArgEnumColumn));

	while(1){
		if(cData.Fetch() == false){
			break;
		}

		argEnumCfg.enumNameLen = strlen(cData.Get(0));
		argEnumCfg.enumName = (char*)cData.Get(0);
		if(cData.Get(1)[0] == '\0'){
			argEnumCfg.enumValFlg = CLI_FALSE;
		}
		else {
			argEnumCfg.enumValFlg = CLI_TRUE;
		}

		argEnumCfg.enbFlg = CLI_TRUE;

		if(argEnumCfg.enumValFlg == CLI_TRUE){
			if(strcasecmp(cData.Get(1),"STRING") == 0){
				argEnumCfg.enumValType = CLI_ARG_TYPE_STRING;
				argEnumCfg.u.str.enumValLen = strlen(cData.Get(2));
				argEnumCfg.u.str.enumVal = (char*)cData.Get(2);
			}
			else if(strcasecmp(cData.Get(1),"NUMBER") == 0){
				argEnumCfg.enumValType = CLI_ARG_TYPE_NUMBER;
				argEnumCfg.u.num.enumVal = atoi(cData.Get(2));
			}
			else if(strcasecmp(cData.Get(1),"DB_STRING") == 0){
				nColumnType = CLI_ARG_TYPE_STRING;
				blnDbUpdateFlag = true;
			}
			else if(strcasecmp(cData.Get(1),"DB_NUMBER") == 0){
				nColumnType = CLI_ARG_TYPE_NUMBER;

				blnDbUpdateFlag = true;
			}

			if(blnDbUpdateFlag == true){
				struct EnumArgValue *stEnumArgValue = NULL;	

				/* find db */
				argEnumCfg.enumValType = CLI_ARG_TYPE_STRING;
				if((szArgEnumTable[0] == '\0') || 
						(szArgEnumColumn[0] == '\0')){
					CLC_LOG(CLC_ERR,"enum table not exit (table=%s, column=%s)\n",
							szArgEnumTable, szArgEnumColumn);
					continue;
				}
				tmpLen = snprintf(tmp, CLC_ARG_ENUM_TABLE_LEN + CLC_ARG_ENUM_COLUMN_LEN
						,"%s/%s",szArgEnumTable, szArgEnumColumn);
				argEnumCfg.u.str.enumValLen = tmpLen;
				argEnumCfg.u.str.enumVal = tmp;

				stEnumArgValue = new struct EnumArgValue;

				stEnumArgValue->strTable = szArgEnumTable;
				stEnumArgValue->strColumn = szArgEnumColumn;

				argEnumCfg.usrVal = (void*)stEnumArgValue;
				argEnumCfg.enbFlg = CLI_FALSE;
			}
		}

		nRet = cli_argEnumAdd(a_stArgCb, &argEnumCfg, &argEnumCb);
		if(nRet != CLI_OK){
			CLC_LOG(CLC_ERR,"argument enum add failed(ret=%d)\n",nRet);
			continue;
		}

		if(blnDbUpdateFlag == true){
			cli_argEnumDisable(argEnumCb);
			nRet = LoadArgEnumParaFromDb(a_stArgCb, szArgEnumTable, szArgEnumColumn, nColumnType);
			if(nRet != CLC_OK){
				CLC_LOG(CLC_ERR,"Enum db paremeter load failed(ret=%d)\n",nRet);
			}
			blnDbUpdateFlag = false;
		}
	}/* end of while(1) */

	return CLC_OK;
}

int CCli::LoadArgFromDb(string &a_strPkgName, CliCmdCb *a_stCmdCb)
{
	int nRet = 0;
	int nQueryLen = 0;
	char chQuery[1024];
	char szArgName[CLI_ARG_NAME_LEN+1];
	char szArgType[CLC_ARG_TYPE_LEN+1];
	char szArgDfltVal[CLI_ARG_VAL_LEN+1];
	char szArgOptFlag[2];
	char szArgRangeMin[CLC_NUMBER_STRING_LEN];
	char szArgRangeMax[CLC_NUMBER_STRING_LEN];
	char szArgMultiFlag[2];
	char szArgMultiMaxCount[CLC_NUMBER_STRING_LEN];
	FetchMaria cData;
	CliArgCfg argCfg;
	CliArgCb *stArgCb = NULL;

	CLI_ARG_CFG_INIT(&argCfg);

	/* load Command */
	nQueryLen = snprintf(chQuery, sizeof(chQuery),
			"SELECT ARG_NAME, ARG_TYPE, ARG_DEFAULT_VAL,  ARG_OPT_FLAG, ARG_RANGE_MIN, ARG_RANGE_MAX, ARG_MULTI_FLAG, ARG_MULTI_MAX_COUNT FROM TAT_CMD_ARG WHERE PKG_NAME=\"%s\" AND CMD_CODE=%d ORDER BY ARG_SEQUENCE asc", 
			a_strPkgName.c_str(),
			a_stCmdCb->cmdCode);

	nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
	if(nRet < 0){
		CLC_LOG(CLC_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
		return CLC_NOK;
	}

	//cData.Clear();

	cData.Set(szArgName, sizeof(szArgName));
	cData.Set(szArgType, sizeof(szArgType));
	cData.Set(szArgDfltVal, sizeof(szArgDfltVal));
	cData.Set(szArgOptFlag, sizeof(szArgOptFlag));
	cData.Set(szArgRangeMin, sizeof(szArgRangeMin));
	cData.Set(szArgRangeMax, sizeof(szArgRangeMax));
	cData.Set(szArgMultiFlag, sizeof(szArgMultiFlag));
	cData.Set(szArgMultiMaxCount, sizeof(szArgMultiMaxCount));

	while(1){
		if(cData.Fetch() == false){
			break;
		}

		argCfg.argNameLen = strlen(cData.Get(0));
		argCfg.argName = (char*)cData.Get(0);
		if(strcasecmp(cData.Get(1),"STRING") == 0){
			argCfg.argType = CLI_ARG_TYPE_STRING;
		}
		else if(strcasecmp(cData.Get(1),"NUMBER") == 0){
			argCfg.argType = CLI_ARG_TYPE_NUMBER;
		}
		else {
			argCfg.argType = CLI_ARG_TYPE_ENUM;
		}
		if(strlen(cData.Get(2)) != 0){
			if(argCfg.argType == CLI_ARG_TYPE_STRING){
				argCfg.u.str.dfltArgValLen = strlen(cData.Get(2));
				argCfg.u.str.dfltArgVal = cData.Get(2);
			}
			else {
				argCfg.u.num.dfltArgVal = atoi(cData.Get(2));
			}
		}
		if(cData.Get(3)[0] == 'Y'){
			argCfg.optFlg = CLI_TRUE;
		}
		else {
			argCfg.optFlg = CLI_FALSE;
		}
		argCfg.rngMin = atoi(cData.Get(4));
		argCfg.rngMax = atoi(cData.Get(5));
		if(cData.Get(6)[0] == 'Y'){
			argCfg.multiFlg = CLI_TRUE;
		}
		else {
			argCfg.multiFlg = CLI_FALSE;
		}
		argCfg.multiMaxCnt = atoi(cData.Get(7));

		nRet = cli_argAdd(a_stCmdCb, &argCfg, &stArgCb);
		if(nRet != CLI_OK){
			CLC_LOG(CLC_ERR,"Arguemnt add failed(ret=%d)\n",nRet);
			continue;
		}

		if(argCfg.argType == CLI_ARG_TYPE_ENUM){
			nRet = LoadArgEnumFromDb(a_strPkgName, a_stCmdCb, stArgCb);
			if(nRet != CLC_OK){
				CLC_LOG(CLC_ERR,"argument enum load failed(ret=%d)\n",nRet);
			}
		}
	}

	return CLC_OK;
}

int CCli::LoadPkgFromDb(string &a_strPkgName)
{
	//int blnRet = 0;
	int nRet = 0;
	int nQueryLen = 0;
	char chQuery[1024];
	char szCmdCode[CLC_NUMBER_STRING_LEN];
	char szCmdName[CLI_CMD_NAME_LEN+1];
	char szCmdDesc[CLI_CMD_DESC_LEN+1];
	char szCmdHelp[CLI_CMD_HELP_LEN+1];
	FetchMaria cData;
	CliCmdCfg cmdCfg;
	CliPkgCb *stPkgCb = NULL;
	CliCmdCb *stCmdCb = NULL;

	CLI_CMD_CFG_INIT(&cmdCfg);

	if(m_blnDbInitFlag != true){
		CLC_LOG(CLC_ERR,"DB config init first\n");
		return CLC_NOK;
	}

	nRet = m_cDb->Connect(m_strDBIp.c_str(), m_nDBPort, m_strDBUser.c_str(), m_strDBPassword.c_str(), m_strDBName.c_str());
	if(nRet < 0){
		CLC_LOG(CLC_ERR,"DB Connect failed(%s, %d, %s, %s, %s)\n",
				m_strDBIp.c_str(), m_nDBPort, m_strDBUser.c_str(), m_strDBPassword.c_str(), m_strDBName.c_str());
		return CLC_NOK;
	}

	/* load Command */
	nQueryLen = snprintf(chQuery, sizeof(chQuery),
			"SELECT CMD_CODE, CMD_NAME, CMD_DESCRIPTION,  CMD_HELP FROM TAT_CMD WHERE PKG_NAME=\"%s\"", a_strPkgName.c_str());

	nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
	if(nRet < 0){
		CLC_LOG(CLC_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
		return CLC_NOK;
	}

	//cData.Clear();

	cData.Set(szCmdCode, sizeof(szCmdCode));
	cData.Set(szCmdName, sizeof(szCmdName));
	cData.Set(szCmdDesc, sizeof(szCmdDesc));
	cData.Set(szCmdHelp, sizeof(szCmdHelp));

	nRet = cli_pkgAdd(&m_stCliCb, CLI_FALSE, a_strPkgName.c_str(), a_strPkgName.size(), NULL, &stPkgCb);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Package add failed(%d)\n",nRet);
		return CLC_NOK;
	}

	while(1){
		if(cData.Fetch() == false){
			break;
		}

		cmdCfg.cmdCode = atoi(cData.Get(0));
		cmdCfg.cmdNameLen = strlen(cData.Get(1));
		cmdCfg.cmdName = (char*)cData.Get(1);
		cmdCfg.descLen = strlen(cData.Get(2));
		cmdCfg.desc = (char*)cData.Get(2);
		cmdCfg.helpLen = strlen(cData.Get(3));
		cmdCfg.help = (char*)cData.Get(3);

		nRet = cli_cmdAdd(stPkgCb, &cmdCfg, &stCmdCb);
		if(nRet != CLI_OK){
			CLC_LOG(CLC_ERR,"Command add failed(ret=%d)\n",nRet);
			continue;
		}

		nRet = LoadArgFromDb(a_strPkgName, stCmdCb);
		if(nRet != CLC_OK){
			CLC_LOG(CLC_ERR,"Command arguemnt load failed(nRet=%d, cmdName=%.*s)\n",nRet, cmdCfg.cmdNameLen, cmdCfg.cmdName);
			continue;
		}
	}

	m_cDb->Close();

	return CLC_OK;
}

int CCli::LoadFromDb()
{
	int blnRet = 0;
	int nRet = 0;
	int nQueryLen = 0;
	char chQuery[1024];
	char szCmdCode[CLC_NUMBER_STRING_LEN];
	char szCmdName[CLI_CMD_NAME_LEN+1];
	char szCmdDesc[CLI_CMD_DESC_LEN+1];
	char szCmdHelp[CLI_CMD_HELP_LEN+1];
	char szPkgName[CLI_PKG_NAME_LEN+1];
	string strPkgName;
	FetchMaria cData;
	CliCmdCfg cmdCfg;
	CliPkgCb *stPkgCb = NULL;
	CliCmdCb *stCmdCb = NULL;

	CLI_CMD_CFG_INIT(&cmdCfg);

	nRet = cli_mainDstry(&m_stCliCb, CLI_TRUE);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Cli control block destory failed(ret=%d)\n",nRet);
		return CLC_NOK;
	}

	nRet = LoadDfltCmd();
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,"Default command load failed(nRet=%d)\n",nRet);
	}

	if(m_blnDbInitFlag != true){
		CLC_LOG(CLC_ERR,"DB config init first\n");
		return CLC_NOK;
	}

	blnRet = m_cDb->Connect(m_strDBIp.c_str(), m_nDBPort, m_strDBUser.c_str(), m_strDBPassword.c_str(), m_strDBName.c_str());
	if(blnRet == false){
		CLC_LOG(CLC_ERR,"DB Connect failed(%s, %d, %s, %s, %s)\n",
				m_strDBIp.c_str(), m_nDBPort, m_strDBUser.c_str(), m_strDBPassword.c_str(), m_strDBName.c_str());
		return CLC_NOK;
	}

	/* load Command */
	nQueryLen = snprintf(chQuery, sizeof(chQuery),
			"SELECT CMD_CODE, CMD_NAME, CMD_DESCRIPTION,  CMD_HELP, PKG_NAME FROM TAT_CMD ");

	nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
	if(nRet < 0){
		CLC_LOG(CLC_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
		return CLC_NOK;
	}

	//cData.Clear();

	cData.Set(szCmdCode, sizeof(szCmdCode));
	cData.Set(szCmdName, sizeof(szCmdName));
	cData.Set(szCmdDesc, sizeof(szCmdDesc));
	cData.Set(szCmdHelp, sizeof(szCmdHelp));
	cData.Set(szPkgName, sizeof(szPkgName));

	while(1){
		if(cData.Fetch() == false){
			break;
		}

		cmdCfg.cmdCode = atoi(cData.Get(0));
		cmdCfg.cmdNameLen = strlen(cData.Get(1));
		cmdCfg.cmdName = (char*)cData.Get(1);
		cmdCfg.descLen = strlen(cData.Get(2));
		cmdCfg.desc = (char*)cData.Get(2);
		cmdCfg.helpLen = strlen(cData.Get(3));
		cmdCfg.help = (char*)cData.Get(3);

		strPkgName = szPkgName;

		nRet = cli_pkgFind(&m_stCliCb, strPkgName.c_str(), strPkgName.size(), &stPkgCb);
		if(nRet != CLI_OK){
			nRet = cli_pkgAdd(&m_stCliCb, CLI_FALSE, strPkgName.c_str(), strPkgName.size(), NULL, &stPkgCb);
			if(nRet != CLI_OK){
				CLC_LOG(CLC_ERR,"Package add failed(%d)\n",nRet);
				return CLC_NOK;
			}
		}

		nRet = cli_cmdAdd(stPkgCb, &cmdCfg, &stCmdCb);
		if(nRet != CLI_OK){
			CLC_LOG(CLC_ERR,"Command add failed(ret=%d)\n",nRet);
			continue;
		}

		nRet = LoadArgFromDb(strPkgName, stCmdCb);
		if(nRet != CLC_OK){
			CLC_LOG(CLC_ERR,"Command arguemnt load failed(nRet=%d, cmdName=%.*s)\n",nRet, cmdCfg.cmdNameLen, cmdCfg.cmdName);
			continue;
		}
	}/* end of while(1) */

	m_cDb->Close();

	return CLC_OK;
}

int CCli::LoadArgEnumFromCfg(CliArgCb *a_stArgCb, rabbit::object &a_cRecord)
{
	int nRet = 0;
	unsigned int nEnumValueType = CLI_ARG_TYPE_STRING;
	unsigned int nEnableFlag = CLI_TRUE;
	string strArgEnumName;
	string strEnumValue;
	unsigned int nEnumValue = 0;
	string strTmp;
	CliArgEnumCfg stArgEnumCfg;

	CLI_ARG_ENUM_CFG_INIT(&stArgEnumCfg);

	try{
		rabbit::array cArgEnumArray;
		rabbit::object cObject;

		strArgEnumName = a_cRecord["NAME"].as_string();

		strTmp = a_cRecord["ENABLE_FLAG"].as_string();

		if((strTmp.size() == 4) &&
				(strncasecmp(strTmp.c_str(),"TRUE", 4) == 0)){
			nEnableFlag = CLI_TRUE;
		}
		else {
			nEnableFlag = CLI_FALSE;
		}

		if((a_cRecord.has("DB_TABLE") > 0) &&
				(a_cRecord.has("DB_COLUMN") > 0)){
			struct EnumArgValue *stEnumArgValue = NULL;

			stEnumArgValue = new struct EnumArgValue;

			stEnumArgValue->strTable = a_cRecord["DB_TABLE"].as_string();
			stEnumArgValue->strColumn = a_cRecord["DB_COLUMN"].as_string();

			stArgEnumCfg.usrVal = (void*)stEnumArgValue;
		}

		stArgEnumCfg.enumValFlg = CLI_FALSE;
		if(a_cRecord.has("ENUM_VALUE_TYPE")){
			if(a_cRecord.has("ENUM_VAUE")){
				strTmp = a_cRecord["ENUM_VALUE_TYPE"].as_string();
				if((strTmp.size() == 6 ) &&
						(strncasecmp(strTmp.c_str(), "STRING", 6) == 0)){
					nEnumValueType = CLI_ARG_TYPE_STRING;
				}
				else if((strTmp.size() == 6 ) &&
						(strncasecmp(strTmp.c_str(), "NUMBER", 6) == 0)){
					nEnumValueType = CLI_ARG_TYPE_NUMBER;
				}

				if(nEnumValueType == CLI_ARG_TYPE_STRING){
					strEnumValue = a_cRecord["ENUM_VALUE"].as_string();

					stArgEnumCfg.enumValType = nEnumValueType;
					stArgEnumCfg.enumValFlg = CLI_TRUE;
					stArgEnumCfg.u.str.enumValLen = strEnumValue.size();
					stArgEnumCfg.u.str.enumVal = (char*)strEnumValue.c_str();
				}
				else {
					nEnumValue = a_cRecord["ENUM_VALUE"].as_uint();

					stArgEnumCfg.enumValType = nEnumValueType;
					stArgEnumCfg.enumValFlg = CLI_TRUE;
					stArgEnumCfg.u.num.enumVal = nEnumValue;
				}
			}/* end of if(a_cRecord.has("ENUM_VAUE")) */
		}/* end of if(a_cRecord.has("ENUM_VALUE_TYPE")) */

		stArgEnumCfg.enumNameLen = strArgEnumName.size();
		stArgEnumCfg.enumName = (char*)strArgEnumName.c_str();
		stArgEnumCfg.enbFlg = nEnableFlag;

		nRet = cli_argEnumAdd(a_stArgCb, &stArgEnumCfg, NULL);
		if(nRet != CLI_OK){
			CLC_LOG(CLC_ERR,"Enum value add failed(ret=%d)\n",nRet);
			return CLC_NOK;
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

int CCli::LoadArgFromCfg(CliCmdCb *a_stCmdCb, rabbit::object &a_cRecord)
{
	int nRet = 0;
	unsigned int i = 0;
	unsigned int nDefaultValueFlag = CLI_FALSE;
	string strArgName;
	string strDefaultValue;
	unsigned int nDefaultValue;
	string strTmp;
	CliArgCfg stArgCfg;
	CliArgCb *argCb = NULL;
	unsigned int nOptFlag = CLI_FALSE;
	unsigned int nType = CLI_ARG_TYPE_STRING;
	unsigned int nMultiFlag = CLI_FALSE;
	unsigned int nMultiMaxCnt = 0;
	unsigned int nRangeMin = 0;
	unsigned int nRangeMax = 0;

	CLI_ARG_CFG_INIT(&stArgCfg);

	try{
		rabbit::array cArgEnumArray;
		rabbit::object cObject;

		strArgName = a_cRecord["NAME"].as_string();

		strTmp = a_cRecord["TYPE"].as_string();

		if((strTmp.size() == 6) &&
				(strncasecmp(strTmp.c_str(), "STRING", 6) == 0)){
			nType = CLI_ARG_TYPE_STRING;
		}
		else if((strTmp.size() == 6) &&
				(strncasecmp(strTmp.c_str(), "NUMBER", 6) == 0)){
			nType = CLI_ARG_TYPE_NUMBER;
		}
		else if((strTmp.size() == 4) &&
				(strncasecmp(strTmp.c_str(), "ENUM", 4) == 0)){
			nType = CLI_ARG_TYPE_ENUM;
		}

		strTmp = a_cRecord["OPTION_FLAG"].as_string();

		if((strTmp.size() == 4) &&
				(strncasecmp(strTmp.c_str(),"TRUE", 4) == 0)){
			nOptFlag = CLI_TRUE;
		}
		else {
			nOptFlag = CLI_FALSE;

			if(a_cRecord.has("DEFAULT_VALUE")){
				if(nType == CLI_ARG_TYPE_STRING){
					strDefaultValue = a_cRecord["DEFAULT_VALUE"].as_string();
				}
				else {
					nDefaultValue = a_cRecord["DEFAULT_VALUE"].as_uint();
				}

				nDefaultValueFlag = CLI_TRUE;
			}	
		}

		strTmp = a_cRecord["MULTI_FLAG"].as_string();

		if((strTmp.size() == 4) &&
				(strncasecmp(strTmp.c_str(),"TRUE",4) == 0)){
			nMultiFlag = CLI_TRUE;
			if(a_cRecord.has("MULTI_MAX_CNT")){
				nMultiMaxCnt = a_cRecord["MULTI_MAX_CNT"].as_uint();
			}
			else {
				nMultiMaxCnt = 10;
			}
		}
		else {
			nMultiFlag = CLI_FALSE;
		}

		nRangeMin = a_cRecord["RANGE_MIN"].as_uint();
		nRangeMax = a_cRecord["RANGE_MAX"].as_uint();

		stArgCfg.argNameLen = strArgName.size();
		stArgCfg.argName= (char*)strArgName.c_str();
		if(nDefaultValueFlag == CLI_TRUE){
			if(nType == CLI_ARG_TYPE_STRING){
				stArgCfg.u.str.dfltArgValLen = strDefaultValue.size();
				stArgCfg.u.str.dfltArgVal= strDefaultValue.c_str();
			}
			else {
				stArgCfg.u.num.dfltArgVal= nDefaultValue;
			}
		}
		else {
			stArgCfg.dfltArgValFlg = CLI_FALSE;
		}
		stArgCfg.argType = nType;
		stArgCfg.optFlg = nOptFlag;
		stArgCfg.multiFlg = nMultiFlag;
		stArgCfg.multiMaxCnt = nMultiMaxCnt;
		stArgCfg.rngMin = nRangeMin;
		stArgCfg.rngMax = nRangeMax;

		nRet = cli_argAdd(a_stCmdCb, &stArgCfg, &argCb);
		if(nRet != CLI_OK){
			CLC_LOG(CLC_ERR,"Argument add failed(nRet=%d)\n",nRet);
			return CLC_NOK;
		}

		if(stArgCfg.argType == CLI_ARG_TYPE_ENUM){
			if(a_cRecord.has("ENUM_LIST")){
				cArgEnumArray = a_cRecord["ENUM_LIST"];
				for(i=0;i<cArgEnumArray.size();i++){
					cObject = cArgEnumArray[i];

					nRet = LoadArgEnumFromCfg(argCb, cObject);
					if(nRet != CLC_OK){
						CLC_LOG(CLC_ERR,"Argument Enum load failed(ret=%d)\n",nRet);
						return CLC_NOK;
					}
				}
			}
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

int CCli::LoadCmdFromCfg(CliPkgCb *a_cPkgCb, rabbit::object &a_cRecord)
{
	int nRet = 0;
	unsigned int i = 0;
	string strCmdName;
	string strDesc;
	string strHelp;
	string strTmp;
	unsigned int nEnableFlag = CLI_FALSE;
	CliCmdCb *cmdCb = NULL;
	CliCmdCfg cmdCfg;

	CLI_CMD_CFG_INIT(&cmdCfg);

	try{
		rabbit::array cArgArray;
		rabbit::object cObject;

		strCmdName = a_cRecord["NAME"].as_string();
		strTmp = a_cRecord["ENABLE_FLAG"].as_string();
		if((strTmp.size() == 4) &&
				strncasecmp(strTmp.c_str(), "TRUE", 4) == 0){
			nEnableFlag = CLI_TRUE;
		}
		else {
			nEnableFlag = CLI_FALSE;
		}


		strDesc = a_cRecord["DESCRIPTION"].as_string();
		strHelp = a_cRecord["HELP"].as_string();

		cmdCfg.cmdName = (char*)strCmdName.c_str();
		cmdCfg.cmdNameLen = strCmdName.size();
		cmdCfg.cmdCode = a_cRecord["CODE"].as_uint();
		cmdCfg.desc = (char*)strDesc.c_str();
		cmdCfg.descLen = strDesc.size();
		cmdCfg.help = (char*)strHelp.c_str();
		cmdCfg.helpLen = strHelp.size();
		cmdCfg.enbFlg = nEnableFlag;

		nRet = cli_cmdAdd(a_cPkgCb, &cmdCfg, &cmdCb);
		if(nRet != CLI_OK){
			CLC_LOG(CLC_ERR,"Command add faeild(ret=%d)\n",nRet);
			return CLC_NOK;
		}

		cArgArray = a_cRecord["ARGUMENT_LIST"];

		for(i=0;i<cArgArray.size();i++){
			cObject = cArgArray[i];

			nRet = LoadArgFromCfg(cmdCb, cObject);
			if(nRet != CLC_OK){
				CLC_LOG(CLC_ERR,"Argument load failed(ret=%d)\n",nRet);
				return CLC_NOK;
			}
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

int CCli::LoadPkgFromCfg(rabbit::object &a_cRecord)
{
	int nRet = 0;
	unsigned int i = 0;
	string strPkgName;
	string strTmp;
	unsigned int nDefaultFlag = CLI_FALSE;
	CliPkgCb *pkgCb = NULL;

	try{
		rabbit::array cCmdArray;
		rabbit::object cObject;

		strPkgName = a_cRecord["NAME"].as_string();

		strTmp = a_cRecord["DEFAULT_FLAG"].as_string();
		if((strTmp.size() == 4) &&
				strncasecmp(strTmp.c_str(), "TRUE", 4) == 0){
			nDefaultFlag = CLI_TRUE;
		}
		else {
			nDefaultFlag = CLI_FALSE;
		}

		nRet = cli_pkgAdd(&m_stCliCb, nDefaultFlag, strPkgName.c_str(), strPkgName.size(), NULL, &pkgCb);
		if(nRet != CLI_OK){
			CLC_LOG(CLC_ERR,"Package add failed(ret=%d)\n",nRet);
			return CLC_NOK;
		}

		cCmdArray = a_cRecord["COMMAND_LIST"];

		for(i=0;i<cCmdArray.size();i++){
			cObject = cCmdArray[i];

			nRet = LoadCmdFromCfg(pkgCb, cObject);
			if(nRet != CLC_OK){
				CLC_LOG(CLC_ERR,"Command load failed(ret=%d)\n",nRet);
				return CLC_NOK;
			}
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

int CCli::LoadFromCfg(string &a_strPath)
{
	int nRet = 0;
	unsigned int i = 0;
	string strJson;
	char szBuffer[1024];
	FILE *fp = NULL;

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

	nRet = cli_mainDstry(&m_stCliCb, CLI_TRUE);
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Cli control block destory failed(ret=%d)\n",nRet);
		return CLC_NOK;
	}

	nRet = LoadDfltCmd();
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,"Default command load failed(nRet=%d)\n",nRet);
	}

	try{
		rabbit::document doc;
		rabbit::array cPkgArray;

		doc.parse(strJson);

		cPkgArray = doc["package_list"];

		for(i=0;i<cPkgArray.size();i++){
			rabbit::object cObject;

			cObject = cPkgArray[i];

			nRet = LoadPkgFromCfg(cObject);
			if(nRet  != CLC_OK){
				CLC_LOG(CLC_ERR,"Package config load failed(ret=%d)\n",nRet);
				return nRet;
			}
		}/* end of for(i=0;i<cPkgArray.size();i++) */

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

int CCli::SetDefaultPkg(string &a_strPkgName)
{
	int nRet = 0;

	nRet = cli_pkgSetDfltByName(&m_stCliCb, (char*)a_strPkgName.c_str(), a_strPkgName.size());
	if(nRet != CLI_OK){
		CLC_LOG(CLC_ERR,"Default setting failed(nRet=%d)\n",nRet);
		return CLC_NOK;
	}

	return CLC_OK;

}

int CCli::DumpArgEnum(CRabbitAlloc &alloc, rabbit::object &a_cRecord, CliArgEnumCb &argEnumCb)
{
	try {
		string strTmp;

		a_cRecord["NAME"] = strTmp.append(argEnumCb.enumName, argEnumCb.enumNameLen);
		if(argEnumCb.enbFlg == CLI_TRUE){
			a_cRecord["ENABLE_FLAG"] = "TRUE";
		}
		else {
			a_cRecord["ENABLE_FLAG"] = "FALSE";
		}

		if(argEnumCb.usrVal != NULL){
			struct EnumArgValue *stEnumArgValue = NULL;

			stEnumArgValue = (struct EnumArgValue*)argEnumCb.usrVal;
			a_cRecord["DB_TABLE"] = stEnumArgValue->strTable;
			a_cRecord["DB_COLUMN"] = stEnumArgValue->strColumn;
		}

		if(argEnumCb.enumValFlg == CLI_TRUE){
			if(argEnumCb.enumValType == CLI_ARG_TYPE_STRING){
				a_cRecord["ENUM_VALUE_TYPE"] = "STRING";
				strTmp.clear();
				a_cRecord["ENUM_VALUE"] = strTmp.append(argEnumCb.u.str.enumVal, argEnumCb.u.str.enumValLen); 
			}
			else {
				a_cRecord["ENUM_VALUE_TYPE"] = "NUMBER";
				a_cRecord["ENUM_VALUE"] = argEnumCb.u.num.enumVal;
			}
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

int CCli::DumpArg(CRabbitAlloc &alloc, rabbit::object &a_cRecord, CliArgCb &argCb)
{
	LnkLstDblNode *lnkNode = NULL;
	CliArgEnumCb *argEnumCb = NULL;

	try {
		string strTmp;
		rabbit::document doc;
		rabbit::object *cRecord = NULL;
		rabbit::array *cArgArray;

		a_cRecord["NAME"] = strTmp.append(argCb.argName, argCb.argNameLen);
		if(argCb.argType == CLI_ARG_TYPE_STRING){
			a_cRecord["TYPE"] = "STRING";
		}
		else if(argCb.argType == CLI_ARG_TYPE_NUMBER){
			a_cRecord["TYPE"] = "NUMBER";
		}
		else {
			a_cRecord["TYPE"] = "ENUM";
		}

		if(argCb.optFlg == CLI_TRUE){
			a_cRecord["OPTION_FLAG"] = "TRUE";
		}
		else {
			a_cRecord["OPTION_FLAG"] = "FALSE";
			if(argCb.dfltArgValFlg == CLI_TRUE){
				if(argCb.argType == CLI_ARG_TYPE_STRING){
					strTmp.clear();
					a_cRecord["DEFAULT_VALUE"] = strTmp.append(argCb.u.str.dfltArgVal, argCb.u.str.dfltArgValLen);
				}
				else {
					a_cRecord["DEFAULT_VALUE"] = argCb.u.num.dfltArgVal;
				}
			}
		}

		if(argCb.multiFlg == CLI_TRUE){
			a_cRecord["MULTI_FLAG"] = "TRUE";
			a_cRecord["MULTI_MAX_CNT"] = argCb.multiMaxCnt;
		}
		else {
			a_cRecord["MULTI_FLAG"] = "FALSE";
		}

		a_cRecord["RANGE_MIN"] = argCb.rngMin;
		a_cRecord["RANGE_MAX"] = argCb.rngMax;


		LNKLST_DBL_GET_FIRST(&argCb.argEnumCbLL, lnkNode);
		if(lnkNode == NULL){
			return CLC_OK;
		}

		cArgArray = alloc.GetRabbitArray();

		*cArgArray = a_cRecord["ENUM_LIST"];

		while(1){
			argEnumCb = (CliArgEnumCb*)lnkNode->data;

			cRecord = alloc.GetRabbitObject();

			DumpArgEnum(alloc, *cRecord, *argEnumCb);

			cArgArray->push_back(*cRecord);

			LNKLST_DBL_GET_NEXT_NODE(lnkNode);
			if(lnkNode == NULL){
				break;
			}
		}/* end of while(1) */

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

int CCli::DumpCmd(CRabbitAlloc &alloc, rabbit::object &a_cRecord, CliCmdCb &cmdCb)
{
	LnkLstDblNode *lnkNode = NULL;
	CliArgCb *argCb = NULL;

	try {
		string strTmp;
		rabbit::document doc;
		rabbit::object *cRecord = NULL;
		rabbit::array *cArgArray;

		a_cRecord["CODE"] = cmdCb.cmdCode;
		a_cRecord["NAME"] = strTmp.append(cmdCb.cmdName, cmdCb.cmdNameLen);
		if(cmdCb.enbFlg == CLI_TRUE){
			a_cRecord["ENABLE_FLAG"] = "TRUE";
		}
		else {
			a_cRecord["ENABLE_FLAG"] = "FALSE";
		}

		strTmp.clear();
		a_cRecord["DESCRIPTION"] = strTmp.append(cmdCb.desc, cmdCb.descLen);
		strTmp.clear();
		a_cRecord["HELP"] = strTmp.append(cmdCb.help, cmdCb.helpLen);

		cArgArray = alloc.GetRabbitArray();

		LNKLST_DBL_GET_FIRST(&cmdCb.argCbLL, lnkNode);
		if(lnkNode == NULL){
			return CLC_OK;
		}

		*cArgArray = a_cRecord["ARGUMENT_LIST"];

		while(1){
			argCb = (CliArgCb*)lnkNode->data;

			cRecord = alloc.GetRabbitObject();

			DumpArg(alloc, *cRecord, *argCb);

			cArgArray->push_back(*cRecord);

			LNKLST_DBL_GET_NEXT_NODE(lnkNode);
			if(lnkNode == NULL){
				break;
			}

		}/* end of while(1) */

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

int CCli::DumpPkg(CRabbitAlloc &alloc, rabbit::object &a_cRecord, CliPkgCb &pkgCb)
{
	LnkLstDblNode *lnkNode = NULL;
	CliCmdCb *cmdCb = NULL;

	try {
		string strTmp;
		rabbit::document doc;
		rabbit::array *cArgArray;
		rabbit::object *cRecord = NULL;

		a_cRecord["NAME"] = strTmp.append(pkgCb.pkgName, pkgCb.pkgNameLen);
		if(pkgCb.enbFlg == CLI_TRUE){
			a_cRecord["DEFAULT_FLAG"] = "TRUE";
		}
		else {
			a_cRecord["DEFAULT_FLAG"] = "FALSE";
		}

		cArgArray = alloc.GetRabbitArray();

		*cArgArray = a_cRecord["COMMAND_LIST"];

		LNKLST_DBL_GET_FIRST(&pkgCb.cmdCbLL, lnkNode);
		if(lnkNode == NULL){
			return CLC_OK;
		}

		while(1){
			cmdCb = (CliCmdCb*)lnkNode->data;

			cRecord = alloc.GetRabbitObject();

			DumpCmd(alloc, *cRecord, *cmdCb);
			cArgArray->push_back(*cRecord);

			LNKLST_DBL_GET_NEXT_NODE(lnkNode);
			if(lnkNode == NULL){
				break;
			}

		}/* end of while(1) */

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

int CCli::Dump()
{
	CliCb *cliCb = NULL;
	CliPkgCb *pkgCb = NULL;
	CRabbitAlloc alloc;
	LnkLstDblNode *lnkNode = NULL;
	FILE *fp = NULL;

	cliCb = &m_stCliCb;

	LNKLST_DBL_GET_FIRST(&cliCb->pkgCbLL, lnkNode);
	if(lnkNode == NULL){
		return CLC_OK;
	}


	try {
		rabbit::object cBody;
		rabbit::object *cRecord = NULL;
		rabbit::array *cArgArray;

		cArgArray = alloc.GetRabbitArray();

		*cArgArray = cBody["package_list"];

		while(1){
			pkgCb = (CliPkgCb*)lnkNode->data;

			if((pkgCb->pkgNameLen == m_strDefaultPkgName.size()) && 
					(strncasecmp(pkgCb->pkgName,m_strDefaultPkgName.c_str(), pkgCb->pkgNameLen) == 0)){
				LNKLST_DBL_GET_NEXT_NODE(lnkNode);
				if(lnkNode == NULL){
					break;
				}
				continue;
			}

			cRecord = alloc.GetRabbitObject();

			DumpPkg(alloc, *cRecord, *pkgCb);

			cArgArray->push_back(*cRecord);

			LNKLST_DBL_GET_NEXT_NODE(lnkNode);
			if(lnkNode == NULL){
				break;
			}
		}/* end of while(1) */

		//cout << cBody.str() << endl;

		{
			string strBuffer;
			//fp = fopen(a_strPath.c_str(),"w");
			fp = fopen(m_strDumpFile.c_str(),"w");
			if(fp == NULL){
				CLC_LOG(CLC_ERR,"File open failed(file=%s, errno=%d(%s)\n",
						m_strDumpFile.c_str(), errno, strerror(errno));
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

int CCli::LoadConfig(string &cfg)
{
	return CLC_OK;
}

int CCli::Read(char *a_szLine)
{
	return CLC_OK;
}

int CCli::Read(string &a_strLine)
{
	return CLC_OK;
}

int CCli::CheckDefaultCmd(CliDoc *a_stDoc)
{
	int nRet = 0;
	CliArgDoc *argDoc = NULL;
	CliArgParaDoc *argParaDoc = NULL;
	//LnkLstDblNode *lnkNode = NULL;

	if(m_strDefaultPkgName.size() != a_stDoc->pkgNameLen){
		return CLC_NOK;
	}

	if(strncasecmp(m_strDefaultPkgName.c_str(), a_stDoc->pkgName, a_stDoc->pkgNameLen) != 0){
		return CLC_NOK;
	}

	switch(a_stDoc->cmdCode){
		case CMD_CODE_DUMP:
			{
				Dump();
			}
			break;
		case CMD_CODE_LOAD:
			{
				if(a_stDoc->argDocLL.nodeCnt == 0){
					CLC_LOG(CLC_ERR,"Argument not exist\n");
					return CLC_OK;
				}

				nRet = cli_docFindArg(a_stDoc, "LOCATION", 8, &argDoc);
				if(nRet != CLI_OK){
					CLC_LOG(CLC_ERR,"LOCATION argument not exist(ret=%d)\n",nRet);
					return CLC_OK;
				}

				CLI_ARG_GET_FIRST_ARG_PARA(argDoc, argParaDoc);
				if(argParaDoc == NULL){
					CLC_LOG(CLC_ERR,"argument paramter not exist\n");
					return CLC_OK;
				}

				nRet = cli_docArgParaIsStr(argParaDoc);
				if(nRet != CLI_TRUE){
					CLC_LOG(CLC_ERR,"Invalid argument paramter type\n");
					return CLC_OK;
				}

				nRet = cli_docArgParaStrCmp(argParaDoc, "DB", 2);
				if(nRet == CLI_TRUE){
					LoadFromDb();
					return CLC_OK;
				}
				nRet = cli_docArgParaStrCmp(argParaDoc, "CFG", 3);
				if(nRet == CLI_TRUE){
					LoadFromCfg(m_strDumpFile);
					return CLC_OK;
				}

				CLC_LOG(CLC_ERR,"unknown argument(%.*s)\n",argDoc->argNameLen, argDoc->argName);
				return CLC_OK;
			}
			break;
		case CMD_CODE_EDIT_CFG:
			{
				pid_t pid;
				int status;
				string strLocalCfg;
				pid_t pidChild;

				strLocalCfg = m_strLocalPath;
				strLocalCfg.append("/CFG/ATOM.cfg");
				pid = fork();
				if(pid < 0){
					CLC_LOG(CLC_ERR,"Edit config fork failed(%d)\n",pid);
					return CLC_OK;
				}

				if(pid == 0){
					char *argv[] =  {(char*)"/usr/bin/vim", (char*)strLocalCfg.c_str(), NULL};
					execv("/usr/bin/vim", argv);
				}

				pidChild = waitpid(pid, &status, 0);
				if( 0 != (status & 0xff)){
					fprintf(stderr,"EDIT Abnormal termination(%d)\n", status >> 8);
				}
			}
			break;
		case CMD_CODE_START_NA:
			{
				string strLocalBin;

				strLocalBin = m_strLocalPath;

				strLocalBin.append("/BIN/");
				chdir(strLocalBin.c_str());

				system("./run.sh&");
			}
			break;
		case CMD_CODE_DISP_NA:
			{
				FILE *fp = NULL;
				uid_t stUserId;
				string strCmd;
				char szBuff[CLC_BUFFER_LEN];
				struct passwd *stUserPw;

				stUserId = getuid();
				stUserPw = getpwuid(stUserId);

				strCmd = "ps -U";
				strCmd.append(stUserPw->pw_name);
				strCmd.append("|grep ATOM_NA");

				fp = popen(strCmd.c_str(),"r");
				if(fp == NULL){
					CLC_LOG(CLC_ERR,"Popen failed(err=%d(%s)\n",errno,strerror(errno));
					return CLC_OK;
				}

				fprintf(stderr,"  USER=%s\n",stUserPw->pw_name);
				fprintf(stderr,"  PID TTY          TIME CMD\n");
				fprintf(stderr,"-------------------------------\n");
				while(fgets(szBuff, CLC_BUFFER_LEN-1, fp)){
					DispFunc(szBuff);
				}

				pclose(fp);
			}
			break;
        case CMD_CODE_STOP_NA:
            {
                FILE *fp = NULL;
                int nPid = 0;
                int nRcvPid = 0;
                int nSig = SIGTERM;
                uid_t stUserId;
                string strCmd;
                char szBuff[CLC_BUFFER_LEN];
                struct passwd *stUserPw;

				nRet = cli_docFindArg(a_stDoc, "PID", 3, &argDoc);
                if(nRet == CLI_OK){
                    CLI_ARG_GET_FIRST_ARG_PARA(argDoc, argParaDoc);
                    if(argParaDoc == NULL){
                        CLC_LOG(CLC_ERR,"argument paramter not exist\n");
                        return CLC_OK;
                    }

                    GET_ARG_PARA_NUM(argParaDoc, nPid, nRet);
                    if(nRet != CLI_OK){
                        CLC_LOG(CLC_ERR,"Invalid argument paramter type\n");
                        return CLC_OK;
                    }
                }

				nRet = cli_docFindArg(a_stDoc, "SIG", 3, &argDoc);
                if(nRet == CLI_OK){
                    CLI_ARG_GET_FIRST_ARG_PARA(argDoc, argParaDoc);
                    if(argParaDoc != NULL){
                        GET_ARG_PARA_NUM(argParaDoc, nSig, nRet);
                        if(nRet != CLI_OK){
                            CLC_LOG(CLC_ERR,"Invalid argument paramter type\n");
                            nSig = SIGTERM;
                        }
                    }
                }

                stUserId = getuid();
                stUserPw = getpwuid(stUserId);

                strCmd = "ps -U";
                strCmd.append(stUserPw->pw_name);
                strCmd.append("|grep ATOM_NA");

                fp = popen(strCmd.c_str(),"r");
                if(fp == NULL){
                    CLC_LOG(CLC_ERR,"Popen failed(err=%d(%s)\n",errno,strerror(errno));
                    return CLC_OK;
                }

                while(fgets(szBuff, CLC_BUFFER_LEN-1, fp)){
                    sscanf(szBuff,"%d",&nRcvPid );

                    if((nPid != 0) && (nPid == nRcvPid)){
                        nRet = kill(nPid, nSig);
                        if(nRet != 0){
                            CLC_LOG(CLC_ERR,"process(%d) kill failed(err=%d(%s))\n",nPid, errno,strerror(errno));
                        }
                    }
                    else if (nPid == 0){
                        nRet = kill(nRcvPid, nSig);
                        if(nRet != 0){
                            CLC_LOG(CLC_ERR,"process(%d) kill failed(err=%d(%s))\n",nPid, errno,strerror(errno));
                        }
                    }
                }

                pclose(fp);
            }
            break;
		default:
			CLC_LOG(CLC_ERR,"Unkonwn command code(%d, %.*s)\n",a_stDoc->cmdCode, a_stDoc->cmdNameLen, a_stDoc->cmdName);
	}

	return CLC_OK;
}

int CCli::Read()
{
	int nRet = 0;

	if(m_stCliDoc != NULL){
		//	delete m_stCliDoc;
		//	m_stCliDoc = NULL;
	}

	nRet = cli_rlRead(&m_stCliCb, &m_stCliDoc);
	if(nRet != CLI_OK){
		if(nRet != CLI_ERR_NOT_EXIST){
			if(nRet == CLI_RSLT_QUIT){
				return CLC_RSLT_QUIT;
			}
			CLC_LOG(CLC_ERR,"Read failed(ret=%d)\n",nRet);
			return CLC_NOK;
		}
		return CLC_RSLT_END;
	}

	/* Check default command */
	nRet = CheckDefaultCmd(m_stCliDoc);
	if(nRet == CLC_OK){
		cli_docDstry(m_stCliDoc);
		m_stCliDoc = NULL;
		return CLC_RSLT_END;
	}

	return CLC_OK;
}

int CCli::GetCmd(unsigned int *a_nCmdCode, string &a_strCmd, string &a_strCmdDesc)
{
	if(m_stCliDoc == NULL){
		CLC_LOG(CLC_ERR,"Command document not exist\n");
		return CLC_NOK;
	}

	*a_nCmdCode = m_stCliDoc->cmdCode;
	a_strCmd.clear();
	a_strCmd.append(m_stCliDoc->cmdName, m_stCliDoc->cmdNameLen); 

	a_strCmdDesc.clear();
	a_strCmdDesc.append(m_stCliDoc->cmdDesc, m_stCliDoc->cmdDescLen);

	return CLC_OK;
}

int CCli::GetJsonMessage(unsigned int a_nSessionId, string &a_strMessage )
{
	int nRet = 0;
	CCliDoc cDoc;
	CCliReq cEncReq(a_nSessionId);

	if(m_stCliDoc == NULL){
		CLC_LOG(CLC_ERR,"Command document not exist\n");
		return CLC_NOK;
	}

	nRet = cDoc.Encode(m_stCliDoc, cEncReq);
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,"Command document encoding failed(%d)\n", nRet);
		return CLC_NOK;
	}

	cEncReq.EncodeMessage(a_strMessage);	

	return CLC_OK;
}

int CCli::DisplayHeader(time_t a_time, unsigned int a_nCmdCode, string &a_strCmd, string &a_strCmdDesc)
{
	struct tm *stTm = NULL;
	char szTime[63];

	stTm = localtime(&a_time);

	strftime(szTime, 62, "%04Y-%02m-%02d %02H:%02M:%02S",stTm);

	fprintf(stderr,"[LOCAL] %s\n",szTime);
	fprintf(stderr,"C%04d  %s\n", a_nCmdCode, a_strCmdDesc.c_str());
	fprintf(stderr,"%s\n", a_strCmd.c_str());

	return CLC_OK;
}

int CCli::Display(unsigned int a_nResultCode, string &a_strResult, string &a_strText)
{
	if(a_strText.size() != 0){
		fprintf(stderr,"%s\n", a_strText.c_str());
	}
	fprintf(stderr,"   RESULT = %d (%s)\n", a_nResultCode, a_strResult.c_str());
	fprintf(stderr,"COMPLETED\n");

	return CLC_OK;
}

