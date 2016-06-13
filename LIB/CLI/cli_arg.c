#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lnkLst.h"
#include "cli.h"
#include "cliInt.h"

const char *cli_argGetStrType(unsigned int type)
{
	switch(type){
		case  CLI_ARG_TYPE_STRING:
			return "STRING";
			break;
		case  CLI_ARG_TYPE_NUMBER:
			return "NUMBER";
			break;
		case  CLI_ARG_TYPE_ENUM:
			return "ENUM";
			break;
		default :
			return "UNKONWON";
	}
}

int cli_argFindBySeq(CliCmdCb *cmdCb, unsigned int seq, CliArgCb **rt_argCb)
{
    unsigned int i = 0;
    unsigned int nodeCnt = 0;
    LnkLstDblNode *lnkNode = NULL;

    if(cmdCb == NULL){
        CLI_LOG(CLI_ERR,"Command control block not exist\n");
        return CLI_ERR_CMDCB_IS_NULL;
    }

    nodeCnt = cmdCb->argCbLL.nodeCnt;
    if((nodeCnt == 0) || (seq > nodeCnt)){
        CLI_LOG(CLI_ERR,"arguemnt not exist(cnt=%d, seq=%d)\n", nodeCnt, seq);
        return CLI_ERR_NOT_EXIST;
    }

    LNKLST_DBL_GET_FIRST(&cmdCb->argCbLL, lnkNode);
    for(i=1;i<seq;i++){
        LNKLST_DBL_GET_NEXT_NODE(lnkNode);
    }/* end of for(i=0;i<seq;i++) */


    *rt_argCb = (CliArgCb*)lnkNode->data;

    return CLI_OK;
}

int cli_argFind(CliCmdCb *cmdCb, const char *argName, unsigned int argNameLen, CliArgCb **rt_argCb)
{
    unsigned int i = 0;
    unsigned int nodeCnt = 0;
    CliArgCb *argCb = NULL;
    LnkLstDblNode *lnkNode = NULL;

    if(cmdCb == NULL){
        CLI_LOG(CLI_ERR,"Command control block not exist\n");
        return CLI_ERR_CMDCB_IS_NULL;
    }

    nodeCnt = cmdCb->argCbLL.nodeCnt;
    if(nodeCnt == 0){
        return CLI_ERR_NOT_EXIST;
    }

    LNKLST_DBL_GET_FIRST(&cmdCb->argCbLL, lnkNode);
    for(i=0;i<nodeCnt;i++){
        argCb = (CliArgCb*)lnkNode->data;

        if(argCb->argNameLen != argNameLen){
            LNKLST_DBL_GET_NEXT_NODE(lnkNode);
            continue;
        }
        if(strncasecmp(argCb->argName, argName, argNameLen) == 0){
            *rt_argCb = argCb;
            return CLI_OK;
        }

        LNKLST_DBL_GET_NEXT_NODE(lnkNode);
    }/* end of for(i=0;i<nodeCnt;i++) */

    return CLI_ERR_NOT_EXIST;
}

int cli_argChkRng(CliArgCb *argCb, unsigned int tknLen)
{
    if((tknLen >= argCb->rngMin) && (tknLen <= argCb->rngMax)){
        return CLI_OK;
    }
    return CLI_NOK;
}

int cli_argAdd(CliCmdCb *cmdCb, CliArgCfg *argCfg, CliArgCb **rt_argCb)
{
	int ret = 0;
	CliArgCb *argCb = NULL;

	if(cmdCb == NULL){
		CLI_LOG(CLI_ERR,"Command control block not exist\n");
		return CLI_ERR_CMDCB_IS_NULL;
	}

	if(argCfg == NULL){
		CLI_LOG(CLI_ERR,"argument config not exist\n");
		return CLI_ERR_ARG_CONFIG_IS_NULL;
	}

	if(argCfg->argNameLen == 0){
		CLI_LOG(CLI_ERR,"Arguemnt name not exist\n");
		return CLI_ERR_ARG_NAME_NOT_EXIST;
	}

	if(argCfg->argNameLen > CLI_ARG_NAME_LEN){
		CLI_LOG(CLI_ERR,"Invalid arguemnt name length(len=%d)\n",argCfg->argNameLen);
		return CLI_ERR_ARG_INVALID_ARG_NAME_LEN;
	}

	if(argCfg->dfltArgValFlg == CLI_TRUE){
		if((argCfg->argType != CLI_ARG_TYPE_STRING) &&
				(argCfg->argType != CLI_ARG_TYPE_NUMBER)){
			CLI_LOG(CLI_ERR,"Default value not support (type=%d)\n", argCfg->argType);
			return CLI_ERR_ARG_DFLT_VAL_NOT_SUPP;
		}

		if(argCfg->argType == CLI_ARG_TYPE_STRING){
			if((argCfg->u.str.dfltArgValLen != 0) && (argCfg->u.str.dfltArgValLen > CLI_ARG_VAL_LEN)){
				CLI_LOG(CLI_ERR,"Invalid default arguemnt value length(len=%d)\n",argCfg->argNameLen);
				return CLI_ERR_ARG_VAL_INVALID_LEN;
			}
		}

		if(argCfg->optFlg == CLI_TRUE){
			CLI_LOG(CLI_ERR,"Invalid arguemnt flag(optFlg=TRUE, default ArgVal is set))\n");
			return CLI_ERR_ARG_INVALID_OPT_FLAG;
		}
	}/* end of if(dfltArgValType != CLI_ARG_TYPE_NONE) */

	ret = cli_argFind(cmdCb, argCfg->argName, argCfg->argNameLen, &argCb);
	if(ret == CLI_OK){
		CLI_LOG(CLI_ERR,"argument already exist(ret=%d)\n",ret);
		return CLI_ERR_ARG_ALREADY_EXIST;
	}

	argCb = (CliArgCb*)malloc(sizeof(CliArgCb));

	strncpy(argCb->argName, argCfg->argName, argCfg->argNameLen);
	argCb->argNameLen = argCfg->argNameLen;

	if(argCfg->dfltArgValFlg == CLI_TRUE){
		if(argCfg->argType == CLI_ARG_TYPE_STRING){
			argCb->u.str.dfltArgValLen = argCfg->u.str.dfltArgValLen;
			strncpy(argCb->u.str.dfltArgVal,argCfg->u.str.dfltArgVal, argCfg->u.str.dfltArgValLen);
		}
		else {
			argCb->u.num.dfltArgVal= argCfg->u.num.dfltArgVal;
		}
		argCb->dfltArgValFlg = CLI_TRUE;
	}
	else {
		argCb->dfltArgValFlg = CLI_FALSE;
	}

	if(argCfg->usrVal != NULL){
		argCb->usrVal = argCfg->usrVal;
	}
	else {
		argCb->usrVal = NULL;
	}

	if((argCfg->argType != CLI_ARG_TYPE_STRING)  &&
			(argCfg->argType != CLI_ARG_TYPE_NUMBER)  &&
			(argCfg->argType != CLI_ARG_TYPE_ENUM)){
		CLI_LOG(CLI_ERR,"Invalid argument type(%d)\n",argCfg->argType);
		free(argCb);
		return CLI_ERR_ARG_INVALID_TYPE;
	}
	argCb->argType = argCfg->argType;

	argCb->optFlg = argCfg->optFlg;

	if(argCb->optFlg != CLI_TRUE){
		cmdCb->argMandCnt++;
	}
	argCb->multiFlg = argCfg->multiFlg;
	if(argCb->multiFlg == CLI_TRUE){
		argCb->multiMaxCnt = argCfg->multiMaxCnt;
	}
	else {
		argCb->multiMaxCnt = 0;
	}

	if((argCfg->rngMin > argCfg->rngMax) || 
			(argCfg->rngMax == 0)){
		CLI_LOG(CLI_ERR,"Invalid range(min=%d, max=%d)\n",argCfg->rngMin, argCfg->rngMax);
		free(argCb);
		return CLI_ERR_ARG_INVLAID_RANGE;
	}

	argCb->rngMin = argCfg->rngMin;
	argCb->rngMax = argCfg->rngMax;

	ret = lnkLst_dblInit(&argCb->argEnumCbLL, ~0);
	if(ret != LNKLST_OK){
		CLI_LOG(CLI_ERR,"Linked list init failed(ret=%d)\n",ret);
		free(argCb);
		return CLI_ERR_LNKLST_INIT_FAILED;
	}

	argCb->lnkNode.data = argCb;

	ret = lnkLst_dblInsertTail(&cmdCb->argCbLL, &argCb->lnkNode);
	if(ret != LNKLST_OK){
		CLI_LOG(CLI_ERR,"Linked list insert failed(ret=%d)\n",ret);
		free(argCb);
		return CLI_ERR_LNKLST_INSERT_FAILED;
	}

	if(rt_argCb != NULL){
		*rt_argCb = argCb;
	}

	return CLI_OK;
}

int cli_argDstry(CliArgCb *argCb, unsigned int usrValFreeFlg)
{
	int ret = 0;
	CliArgEnumCb *argEnumCb = NULL;
	LnkLstDblNode *lnkNode = NULL;

	argCb->argNameLen = 0;

	argCb->argType = 0;
	argCb->optFlg = 0;
	argCb->multiFlg= 0;
	argCb->multiMaxCnt = 0;
	argCb->rngMin = 0;
	argCb->rngMax = 0;

	if(usrValFreeFlg == CLI_TRUE){
		if(argCb->usrVal != NULL){
			free(argCb->usrVal);
		}
	}

	while(1){
		if(argCb->argEnumCbLL.nodeCnt == 0){
			break;
		}
		lnkNode = lnkLst_dblGetFirst(&argCb->argEnumCbLL);
		if(lnkNode == NULL){
			break;
		}

		argEnumCb = (CliArgEnumCb*)lnkNode->data;

		ret = cli_argEnumDstry(argEnumCb, usrValFreeFlg);
		if(ret != CLI_OK){
			CLI_LOG(CLI_ERR,"Enum para delete failed (ret=%d)\n",ret);
		}

		free(argEnumCb);
	}/* end of while(1) */

	return CLI_OK;
}

int cli_argEnumFind(CliArgCb *argCb, const char *enumName, unsigned int enumNameLen, CliArgEnumCb **rt_argEnumCb)
{
    unsigned int i = 0;
    unsigned int nodeCnt = 0;
    CliArgEnumCb *argEnumCb = NULL;
    LnkLstDblNode *lnkNode = NULL;

    if(argCb == NULL){
        CLI_LOG(CLI_ERR,"Arguemnt enum control block not exist\n");
        return CLI_ERR_ARGENUMCB_IS_NULL;
    }

    nodeCnt = argCb->argEnumCbLL.nodeCnt;
    if(nodeCnt == 0){
        return CLI_ERR_NOT_EXIST;
    }

    LNKLST_DBL_GET_FIRST(&argCb->argEnumCbLL, lnkNode);
    for(i=0;i<nodeCnt;i++){
        argEnumCb = (CliArgEnumCb*)lnkNode->data;

        if(argEnumCb->enumNameLen != enumNameLen){
            LNKLST_DBL_GET_NEXT_NODE(lnkNode);
            continue;
        }

        if(strncasecmp(argEnumCb->enumName, enumName, enumNameLen) == 0){
            *rt_argEnumCb = argEnumCb;
            return CLI_OK;
        }

        LNKLST_DBL_GET_NEXT_NODE(lnkNode);
    }/* end of for(i=0;i<nodeCnt;i++) */

    return CLI_ERR_NOT_EXIST;
}

int cli_argEnumEnable(CliArgEnumCb *argEnumCb)
{
	argEnumCb->enbFlg = CLI_TRUE;

	return CLI_OK;
}

int cli_argEnumDisable(CliArgEnumCb *argEnumCb)
{
	argEnumCb->enbFlg = CLI_FALSE;

	return CLI_OK;
}

int cli_argEnumAdd(CliArgCb *argCb, CliArgEnumCfg *enumCfg, CliArgEnumCb **rt_argEnumCb)
{
	int ret = 0;
	CliArgEnumCb *argEnumCb = NULL;

	if(argCb == NULL){
		CLI_LOG(CLI_ERR,"Argument control block not exist\n");
		return CLI_ERR_ARGCB_IS_NULL;
	}

	if(argCb->argType != CLI_ARG_TYPE_ENUM){
		CLI_LOG(CLI_ERR,"Invalid argument type(type=%d)\n",argCb->argType);
		return CLI_ERR_ARG_INVALID_TYPE;
	}

	if(enumCfg == NULL){
		CLI_LOG(CLI_ERR,"enum config not exist\n");
		return CLI_ERR_ENUM_CONFIG_IS_NULL;
	}

	if(enumCfg->enumNameLen == 0){
		CLI_LOG(CLI_ERR,"Enum name not exist\n");
		return CLI_ERR_ENUM_NAME_NOT_EXIST;
	}

	if(enumCfg->enumValFlg == CLI_TRUE){
		if((enumCfg->enumValType != CLI_ARG_TYPE_NUMBER) && 
				(enumCfg->enumValType != CLI_ARG_TYPE_STRING)){
			CLI_LOG(CLI_ERR,"Invalid enum value type(%d)\n",enumCfg->enumValType);
			return CLI_ERR_ARG_INVALID_TYPE;
		}
		if(enumCfg->enumValType == CLI_ARG_TYPE_STRING){
			if(enumCfg->u.str.enumValLen == 0){
				CLI_LOG(CLI_ERR,"Enum value not exist\n");
				return CLI_ERR_ENUM_VAL_NOT_EXIST;
			}
			if(enumCfg->u.str.enumValLen > CLI_ARG_VAL_LEN){
				CLI_LOG(CLI_ERR,"Invalid enum value length(%d)\n", enumCfg->u.str.enumValLen);
				return CLI_ERR_ENUM_INVALID_VAL_LEN;
			}
		}
	}/* end of if(enumCfg->enumValFlg == CLI_TRUE) */

	if(enumCfg->enumNameLen > CLI_ENUM_NAME_LEN){
		CLI_LOG(CLI_ERR,"Invalid enum name length(%d)\n", enumCfg->enumNameLen);
		return CLI_ERR_ENUM_INVALID_NAME_LEN;
	}

	ret = cli_argEnumFind(argCb, enumCfg->enumName, enumCfg->enumNameLen, &argEnumCb);
	if(ret == CLI_OK){
		CLI_LOG(CLI_ERR,"enum value already exist(%.*s)\n", enumCfg->enumNameLen, enumCfg->enumName);
		return CLI_ERR_ENUM_ALREADY_EXIST;
	}

	argEnumCb = (CliArgEnumCb*)malloc(sizeof(CliArgEnumCb));

	strncpy(argEnumCb->enumName, enumCfg->enumName, enumCfg->enumNameLen);
	argEnumCb->enumNameLen = enumCfg->enumNameLen;

	argEnumCb->enumValFlg = enumCfg->enumValFlg;

	if(argEnumCb->enumValFlg == CLI_TRUE){
		if(enumCfg->enumValType == CLI_ARG_TYPE_NUMBER){
			argEnumCb->u.num.enumVal = enumCfg->u.num.enumVal;

			if(argCb->rngMax < 11){
				argCb->rngMax = 11;
			}
		}
		else {
			strncpy(argEnumCb->u.str.enumVal, enumCfg->u.str.enumVal, enumCfg->u.str.enumValLen);
			argEnumCb->u.str.enumValLen = enumCfg->u.str.enumValLen;

			if(argCb->rngMin > enumCfg->u.str.enumValLen){
				argCb->rngMin = enumCfg->u.str.enumValLen;
			}

			if(argCb->rngMax < enumCfg->u.str.enumValLen){
				argCb->rngMax = enumCfg->u.str.enumValLen;
			}
		}

		argEnumCb->enumValType = enumCfg->enumValType;
	}/* end of if(argEnumCb->enumValFlg == CLI_TRUE) */

	if(enumCfg->enbFlg == CLI_FALSE){
		argEnumCb->enbFlg = CLI_FALSE;
	}
	else {
		argEnumCb->enbFlg = CLI_TRUE;
	}

	if(enumCfg->usrVal != NULL){
		argEnumCb->usrVal = enumCfg->usrVal;
	}
	else {
		argEnumCb->usrVal = NULL;
	}

    argEnumCb->lnkNode.data = argEnumCb;

    ret = lnkLst_dblInsertTail(&argCb->argEnumCbLL, &argEnumCb->lnkNode);
    if(ret != CLI_OK){
        CLI_LOG(CLI_ERR,"Enum insert failed(ret=%d)\n",ret);
        free(argEnumCb);
        return CLI_ERR_LNKLST_INSERT_FAILED;
    }

	if(rt_argEnumCb != NULL){
		*rt_argEnumCb = argEnumCb;
	}

    return CLI_OK;
}

int cli_argEnumDstry(CliArgEnumCb *argEnumCb, unsigned int usrValFreeFlg)
{
	argEnumCb->enumNameLen = 0;	
	argEnumCb->enumValFlg = 0;	
	argEnumCb->enumValType = 0;	

	if(usrValFreeFlg == CLI_TRUE){
		if(argEnumCb->usrVal != NULL){
			free(argEnumCb->usrVal);
		}
	}

	return CLI_OK;
}

int cli_argPrntEnumHalpDisp(CliArgEnumCb *argEnumCb)
{

	if(argEnumCb->enbFlg == CLI_FALSE){
		return CLI_OK;
	}

	if(argEnumCb->enumValFlg == CLI_TRUE){
		if(argEnumCb->enumValType == CLI_ARG_TYPE_STRING){
			CLI_DISP("%27s%-20.*s =  \"%.*s\"\n","", argEnumCb->enumNameLen, argEnumCb->enumName,
					argEnumCb->u.str.enumValLen,
					argEnumCb->u.str.enumVal);
		}
		else {
			CLI_DISP("%27s%-20.*s = %d\n","", argEnumCb->enumNameLen, argEnumCb->enumName,
					argEnumCb->u.num.enumVal);
		}
	}
	else {
		    CLI_DISP("%27s%-20.*s = \"%.*s\"\n","", argEnumCb->enumNameLen, argEnumCb->enumName,
				argEnumCb->enumNameLen, argEnumCb->enumName);
	}

	return CLI_OK;
}

int cli_argPrntHelpDisp(CliArgCb *argCb)
{
	unsigned int nodeCnt = 0;
	CliArgEnumCb *argEnumCb = NULL;
	LnkLstDblNode *lnkNode = NULL;
	char *argName = NULL;
	unsigned int argNameLen = 0;

	if(argCb->optFlg == CLI_TRUE){
		argNameLen = argCb->argNameLen + 2/* [] */ + 1/* * */ + 1;
		argName = malloc(argNameLen);

		if(argCb->multiFlg == CLI_TRUE){
			snprintf(argName,argNameLen+1,"[*%.*s]",argCb->argNameLen,argCb->argName);
		}
		else {
			snprintf(argName,argNameLen,"[%.*s]",argCb->argNameLen,argCb->argName);
		}
	}
	else {
		if(argCb->multiFlg == CLI_TRUE){
			snprintf(argName,argNameLen,"*%.*s",argCb->argNameLen,argCb->argName);
		}
		else {
			argName = argCb->argName;
			argNameLen = argCb->argNameLen;
		}
	}

	if(argCb->multiFlg == CLI_TRUE){
		CLI_DISP("%24.*s : %s(%d, %d) (MAX PARA COUNT=%d)\n",argNameLen, argName,
				cli_argGetStrType(argCb->argType),
				argCb->rngMin, argCb->rngMax,
				argCb->multiMaxCnt
				);
	}
	else {
		CLI_DISP("%24.*s : %s(%d, %d)\n",argNameLen, argName,
				cli_argGetStrType(argCb->argType),
				argCb->rngMin, argCb->rngMax);
	}

	if(argCb->argType == CLI_ARG_TYPE_ENUM){
		nodeCnt = argCb->argEnumCbLL.nodeCnt;
		if(nodeCnt == 0){
			return CLI_OK;
		}

		LNKLST_DBL_GET_FIRST(&argCb->argEnumCbLL, lnkNode);
		while(1){
			if(lnkNode == NULL){
				break;
			}

			argEnumCb = (CliArgEnumCb*)lnkNode->data;

			cli_argPrntEnumHalpDisp(argEnumCb);

			LNKLST_DBL_GET_NEXT_NODE(lnkNode);
		}/* end of while(1) */
	}/* end of if(argCb->argType == CLI_ARG_TYPE_ENUM) */

	return CLI_OK;
}

int cli_argGetDispStr(CliArgCb *argCb, char *rt_line, unsigned int maxLineLen, unsigned int *rt_lineLen)
{

	if(argCb->optFlg == CLI_TRUE){
		if(argCb->multiFlg == CLI_TRUE){
			*rt_lineLen = snprintf(rt_line, maxLineLen, ":[*%.*s]",argCb->argNameLen, argCb->argName);
		}
		else {
			*rt_lineLen = snprintf(rt_line, maxLineLen, ":[%.*s]",argCb->argNameLen, argCb->argName);
		}
	}
	else {
		if(argCb->multiFlg == CLI_TRUE){
			*rt_lineLen = snprintf(rt_line, maxLineLen, ":*%.*s",argCb->argNameLen, argCb->argName);
		}
		else {
			*rt_lineLen = snprintf(rt_line, maxLineLen, ":%.*s",argCb->argNameLen, argCb->argName);
		}
	}

	return CLI_OK;
}
