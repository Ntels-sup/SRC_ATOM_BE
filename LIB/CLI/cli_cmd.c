#include <unistd.h>
//#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lnkLst.h"
#include "cli.h"
#include "cliInt.h"

int cli_cmdFind(CliPkgCb *pkgCb, const char *cmdName, unsigned int cmdNameLen, CliCmdCb **rt_cmdCb)
{
    unsigned int i = 0;
    unsigned int nodeCnt = 0;
    CliCmdCb *cmdCb = NULL;
    LnkLstDblNode *lnkNode = NULL;

    if(pkgCb == NULL){
        CLI_LOG(CLI_ERR,"Package control block not exist\n");
        return CLI_ERR_PKGCB_IS_NULL;
    }

    nodeCnt = pkgCb->cmdCbLL.nodeCnt;
    if(nodeCnt == 0){
        return CLI_ERR_NOT_EXIST;
    }

    LNKLST_DBL_GET_FIRST(&pkgCb->cmdCbLL, lnkNode);
    for(i=0;i<nodeCnt;i++){
        cmdCb = (CliCmdCb*)lnkNode->data;

        if(cmdCb->cmdNameLen != cmdNameLen){
            LNKLST_DBL_GET_NEXT_NODE(lnkNode);
            continue;
        }
        if(strncasecmp(cmdCb->cmdName, cmdName, cmdNameLen) == 0){
            *rt_cmdCb = cmdCb;
            return CLI_OK;
        }

        LNKLST_DBL_GET_NEXT_NODE(lnkNode);
    }/* end of for(i=0;i<nodeCnt;i++) */

    return CLI_ERR_NOT_EXIST;
}

int cli_cmdAdd(CliPkgCb *pkgCb, CliCmdCfg *cmdCfg, CliCmdCb **rt_cmdCb)
{
    int ret = 0;
    CliCmdCb *cmdCb = NULL;

    if(pkgCb == NULL){
        CLI_LOG(CLI_ERR,"Package control block not exist\n");
        return CLI_ERR_PKGCB_IS_NULL;
    }

    if(cmdCfg == NULL){
        CLI_LOG(CLI_ERR,"Cmd config not exist\n");
        return CLI_ERR_CMD_CONFIG_IS_NULL;
    }

    if(cmdCfg->cmdNameLen == 0){
        CLI_LOG(CLI_ERR,"Command name not exist\n");
        return CLI_ERR_CMD_NAME_NOT_EXIST;
    }

    ret = cli_cmdFind(pkgCb, cmdCfg->cmdName, cmdCfg->cmdNameLen, &cmdCb);
    if(ret == CLI_OK){
        CLI_LOG(CLI_ERR,"Command already exist(ret=%d)\n",ret);
        return CLI_ERR_CMD_ALREADY_EXIST;
    }

    cmdCb = (CliCmdCb*)malloc(sizeof(CliCmdCb));

    cmdCb->argMandCnt = 0;

	cmdCb->enbFlg = CLI_TRUE;

    cmdCb->lnkNode.data = cmdCb;

	if(cmdCfg->usrVal != NULL){
		cmdCb->usrVal = cmdCfg->usrVal;
	}
	else {
		cmdCb->usrVal = NULL;
	}

    ret = lnkLst_dblInit(&cmdCb->argCbLL, ~0);
    if(ret != LNKLST_OK){
        CLI_LOG(CLI_ERR,"argument linked list init failed(ret=%d)\n",ret);
        free(cmdCb);
        return CLI_ERR_LNKLST_INIT_FAILED;
    }

    cmdCb->cmdCode = cmdCfg->cmdCode;
    strncpy(cmdCb->cmdName, cmdCfg->cmdName, cmdCfg->cmdNameLen);
    cmdCb->cmdNameLen = cmdCfg->cmdNameLen;

    if(cmdCfg->descLen != 0){
        strncpy(cmdCb->desc, cmdCfg->desc, cmdCfg->descLen);
        cmdCb->descLen = cmdCfg->descLen;
    }

    if(cmdCfg->helpLen != 0){
        strncpy(cmdCb->help, cmdCfg->help, cmdCfg->helpLen);
        cmdCb->helpLen = cmdCfg->helpLen;
    }

    ret = lnkLst_dblInsertTail(&pkgCb->cmdCbLL, &cmdCb->lnkNode);
    if(ret != LNKLST_OK){
        CLI_LOG(CLI_ERR,"Package linked list init failed(ret=%d)\n",ret);
        return CLI_ERR_LNKLST_INIT_FAILED;
    }

	if(rt_cmdCb != NULL){
		*rt_cmdCb = cmdCb;
	}

    return CLI_OK;
}

int cli_cmdEnable(CliCmdCb *cmdCb)
{
	cmdCb->enbFlg = CLI_TRUE;

	return CLI_OK;
}

int cli_cmdDisable(CliCmdCb *cmdCb)
{
	cmdCb->enbFlg = CLI_FALSE;

	return CLI_OK;
}

int cli_cmdDstry(CliCmdCb *cmdCb, unsigned int usrValFreeFlg)
{
	int ret = 0;
	LnkLstDblNode *lnkNode = NULL;
	CliArgCb *argCb = NULL;

	cmdCb->cmdCode = 0;
	cmdCb->cmdNameLen = 0;
	cmdCb->descLen = 0;
	cmdCb->helpLen = 0;

	if(usrValFreeFlg == CLI_TRUE){
		if(cmdCb->usrVal != NULL){
			free(cmdCb->usrVal);
		}
	}

	while(1){
		if(cmdCb->argCbLL.nodeCnt == 0){
			break;
		}

		lnkNode = lnkLst_dblGetFirst(&cmdCb->argCbLL);
		if(lnkNode == NULL){
			break;
		}

		argCb = (CliArgCb*)lnkNode->data;

		ret = cli_argDstry(argCb, usrValFreeFlg);
		if(ret != CLI_OK){
			CLI_LOG(CLI_ERR,"Argument destroy failed(ret=%d)\n",ret);
		}

		free(argCb);
	}/* end of while(1) */

	return CLI_OK;
}

int cli_cmdDispCmdDesc(CliCmdCb *cmdCb)
{
	int ret = 0;
	unsigned int argLineLen = 0;
	unsigned int i = 0;
	unsigned int nodeCnt = 0;
	unsigned int remLen = 0;
	unsigned int len = 0;
	CliArgCb *argCb = NULL;
	LnkLstDblNode *lnkNode = NULL;
	char argLine[ARG_DISP_LINE_LEN];

	remLen = ARG_DISP_LINE_LEN;

	nodeCnt = cmdCb->argCbLL.nodeCnt;
	if(nodeCnt == 0){
		goto cmd_disp;
	}

	LNKLST_DBL_GET_FIRST(&cmdCb->argCbLL, lnkNode);
	for(i=0;i<nodeCnt;i++){

		argCb = (CliArgCb*)lnkNode->data;

		ret = cli_argGetDispStr(argCb, &argLine[argLineLen], remLen, &len);
		if(ret != CLI_OK){
			CLI_LOG(CLI_ERR,"Arguemnt display failed(ret=%d)\n",ret);
		}

		remLen -= len;
		argLineLen += len;

		LNKLST_DBL_GET_NEXT_NODE(lnkNode);
	}/* end of for(i=0;i<nodeCnt;i++) */

cmd_disp:
	CLI_DISP("%-5d  %-15.*s  %-40.*s  %-50.*s\n",
			cmdCb->cmdCode, cmdCb->cmdNameLen, cmdCb->cmdName, argLineLen, argLine, cmdCb->descLen, cmdCb->desc);

	return CLI_OK;
}
