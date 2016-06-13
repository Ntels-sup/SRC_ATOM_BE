#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "lnkLst.h"
#include "cli.h"
#include "cliInt.h"

int cli_pkgFind(CliCb *cliCb, const char *pkgName, unsigned int pkgNameLen, CliPkgCb **rt_pkgCb)
{
    unsigned int i = 0;
    unsigned int nodeCnt = 0;
    LnkLstDblNode *lnkNode = NULL;
    CliPkgCb *pkgCb = NULL;

    nodeCnt = cliCb->pkgCbLL.nodeCnt;
    if(nodeCnt == 0){
        return CLI_ERR_NOT_EXIST;
    }

    LNKLST_DBL_GET_FIRST(&cliCb->pkgCbLL, lnkNode);
    for(i=0;i<nodeCnt;i++){
        pkgCb = (CliPkgCb*)lnkNode->data;

        if(pkgCb->pkgNameLen != pkgNameLen){
            LNKLST_DBL_GET_NEXT_NODE(lnkNode);
            continue;
        }

        if(strncasecmp(pkgCb->pkgName, pkgName, pkgNameLen) == 0){
            *rt_pkgCb = pkgCb;

            return CLI_OK;
        }

        LNKLST_DBL_GET_NEXT_NODE(lnkNode);
    }/* end of for(i=0;i<nodeCnt;i++) */

    return CLI_ERR_NOT_EXIST;
}

CliPkgCb *cli_pkgGetDfltPkg(CliCb *cliCb)
{
    return cliCb->dfltPkgCb;
}

int cli_pkgAdd(CliCb *cliCb, unsigned int dfltFlg, const char *pkgName, unsigned int pkgNameLen, 
		void *usrVal,  CliPkgCb **rt_pkgCb)
{
    int ret = 0;
    CliPkgCb *pkgCb = NULL;

    if(pkgNameLen > CLI_PKG_NAME_LEN){
        CLI_LOG(CLI_ERR,"Invalid package name length(ret=%d)\n",ret);
        return CLI_ERR_INVALID_PKG_NAME_LEN;
    }

    ret = cli_pkgFind(cliCb, pkgName, pkgNameLen, &pkgCb);
    if(ret == CLI_OK){
        CLI_LOG(CLI_ERR,"Package already exist(%.*s)\n", pkgNameLen, pkgName);
        return CLI_ERR_PKG_ALREADY_EXIST;
    }

    pkgCb = (CliPkgCb*)malloc(sizeof(CliPkgCb));

    strncpy(pkgCb->pkgName, pkgName, pkgNameLen);

    pkgCb->pkgNameLen = pkgNameLen;

    pkgCb->lnkNode.data = pkgCb;

	if(usrVal != NULL){
		pkgCb->usrVal = usrVal;
	}
	else {
		pkgCb->usrVal = NULL;
	}

	pkgCb->enbFlg = CLI_TRUE;

    ret = lnkLst_dblInit(&pkgCb->cmdCbLL, ~0);
    if(ret != LNKLST_OK){
        CLI_LOG(CLI_ERR,"arguemnt linked list init failed(ret=%d)\n",ret);
        free(pkgCb);
        return CLI_ERR_LNKLST_INIT_FAILED;
    }

    ret = lnkLst_dblInsertTail(&cliCb->pkgCbLL, &pkgCb->lnkNode);
    if(ret != LNKLST_OK){
        CLI_LOG(CLI_ERR,"package insert failed(ret=%d)\n",ret);
        free(pkgCb);
        return CLI_ERR_PKG_MAKE_FAILED;
    }

	if(dfltFlg == CLI_TRUE){
		cliCb->dfltPkgCb = pkgCb;
	}

	if(rt_pkgCb != NULL){
		*rt_pkgCb = pkgCb;
	}

    return CLI_OK;
}

int cli_pkgSetEnable(CliPkgCb *pkgCb)
{
	pkgCb->enbFlg = CLI_TRUE;

	return CLI_OK;
}

int cli_pkgSetDisable(CliPkgCb *pkgCb)
{
	pkgCb->enbFlg = CLI_FALSE;

	return CLI_OK;
}

int cli_pkgSetDflt(CliCb *cliCb, CliPkgCb *pkgCb)
{
	cliCb->dfltPkgCb = pkgCb;

	return CLI_OK;
}

int cli_pkgSetDfltByName(CliCb *cliCb, const char *pkgName, unsigned int pkgNameLen)
{
	int ret = 0;
	CliPkgCb *pkgCb = NULL;

	ret = cli_pkgFind(cliCb, pkgName, pkgNameLen , &pkgCb);
	if(ret != CLI_OK){
		CLI_LOG(CLI_ERR,"Package find failed(ret=%d)\n",ret);
		return CLI_NOK;
	}

	ret = cli_pkgSetDflt(cliCb, pkgCb);
	if(ret != CLI_OK){
		CLI_LOG(CLI_ERR,"Default package setting failed(ret=%d, pkg=%.*s)\n",ret, pkgNameLen, pkgName);
		return CLI_NOK;
	}

	return CLI_OK;
}

int cli_pkgDstry(CliPkgCb *pkgCb, unsigned int usrValFreeFlg)
{
	int ret = 0;
	LnkLstDblNode *lnkNode = NULL;
	CliCmdCb *cmdCb = NULL;

	pkgCb->pkgNameLen = 0;

	if(usrValFreeFlg == CLI_TRUE){
		if(pkgCb->usrVal != NULL){
			free(pkgCb->usrVal);
		}
	}

	while(1){
		if(pkgCb->cmdCbLL.nodeCnt == 0){
			break;
		}

		lnkNode = lnkLst_dblGetFirst(&pkgCb->cmdCbLL);
		if(lnkNode == NULL){
			break;
		}

		cmdCb = (CliCmdCb*)lnkNode->data;

		ret = cli_cmdDstry(cmdCb, usrValFreeFlg);
		if(ret != CLI_OK){
			CLI_LOG(CLI_ERR,"Comamnd destroy failed(ret=%d)\n",ret);
		}

		free(cmdCb);
	}/* end of while(1) */

	return CLI_OK;
}

int cli_pkgDispCmdLst(CliPkgCb *pkgCb)
{
    int ret = 0;
    int i = 0;
    int nodeCnt = 0;
    LnkLstDblNode *lnkNode = NULL;
    CliCmdCb *cmdCb = NULL;

    CLI_DISP("PACKAGE = \"%.*s\"\n", pkgCb->pkgNameLen, pkgCb->pkgName);

    nodeCnt = pkgCb->cmdCbLL.nodeCnt;
    if(nodeCnt == 0){
        return CLI_OK;
    }

    CLI_DISP(" CODE  COMMAND          ARGUMENT                                  DESCRIPTION\n");
    CLI_DISP("---------------------------------------------------------------------------------------------------------------------\n");

    LNKLST_DBL_GET_FIRST(&pkgCb->cmdCbLL, lnkNode);
    for(i=0;i<nodeCnt;i++){
        cmdCb = (CliCmdCb*)lnkNode->data;

        ret = cli_cmdDispCmdDesc(cmdCb);
        if(ret != CLI_OK){
            CLI_LOG(CLI_ERR,"Command list display failed(ret=%d)\n",ret);
        }

        CLI_DISP("\n");

        LNKLST_DBL_GET_NEXT_NODE(lnkNode);
    }/* end of for(i=0;i<nodeCnt;i++) */

    return CLI_OK;
}

