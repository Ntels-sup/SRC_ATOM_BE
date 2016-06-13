#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "lnkLst.h"
#include "cli.h"
#include "cliInt.h"

int cli_docInit(CliDoc *doc)
{
    int ret = 0;

	doc->cmdLineLen = 0;
	doc->cmdLine = NULL;
    doc->pkgNameLen = 0;
    doc->cmdNameLen = 0;
	doc->cmdCode = 0;
    doc->nxtSeq = 0;
    doc->fixArgFlg = CLI_FALSE;
    doc->argMandCnt = 0;

    ret = lnkLst_dblInit(&doc->argDocLL, ~0);
    if(ret != LNKLST_OK){
        CLI_LOG(CLI_ERR,"Linked list init failed(ret=%d)\n",ret);
        return CLI_ERR_LNKLST_INIT_FAILED;
    }

    return CLI_OK;
}

int cli_docArgParaDstry(CliArgParaDoc *argParaDoc)
{
    argParaDoc->valType = 0;

    return CLI_OK;
}

int cli_docArgDstry(CliArgDoc *argDoc)
{
    int ret = 0;
    CliArgParaDoc *argParaDoc = NULL;
    LnkLstDblNode *lnkNode = NULL;
    argDoc->argNameLen = 0;

    while(1){
        if(argDoc->argParaLL.nodeCnt == 0){
            break;
        }

        lnkNode = lnkLst_dblGetFirst(&argDoc->argParaLL);
        if(lnkNode == NULL){
            break;
        }

        argParaDoc = (CliArgParaDoc*)lnkNode->data;

        ret = cli_docArgParaDstry(argParaDoc);
        if(ret != CLI_OK){
            CLI_LOG(CLI_ERR," Arguemnt para destory failed(ret=%d)\n",ret);
        }

        free(argParaDoc);
    }/* end of while(1) */


    return CLI_OK;
}

int cli_docDstry(CliDoc *doc)
{
    int ret = 0;
    unsigned int nodeCnt = 0;
    CliArgDoc *argDoc = NULL;
    LnkLstDblNode *lnkNode = NULL;

	if(doc->cmdLine != NULL){
		free(doc->cmdLine);
	}
	doc->cmdLineLen = 0;

    doc->pkgNameLen = 0;
    doc->cmdNameLen = 0;
    doc->nxtSeq = 0;
    doc->fixArgFlg = 0;

    nodeCnt = doc->argDocLL.nodeCnt;

    while(1){
        if(doc->argDocLL.nodeCnt == 0){
            break;
        }

        lnkNode = lnkLst_dblGetFirst(&doc->argDocLL);
        if(lnkNode == NULL){
            break;
        }

        argDoc = (CliArgDoc*)lnkNode->data;

        ret = cli_docArgDstry(argDoc);
        if(ret != CLI_OK){
            CLI_LOG(CLI_ERR," Arguemnt destory failed(ret=%d)\n",ret);
        }

        free(argDoc);
    }/* end of while(1) */

    free(doc);

    return CLI_OK;
}


int cli_docSetPkg(CliDoc *doc, char *pkgName, unsigned int pkgNameLen)
{
	if(pkgNameLen > CLI_PKG_NAME_LEN){
		CLI_LOG(CLI_ERR,"Invalid package length(%d)\n",pkgNameLen);
		return CLI_ERR_INVALID_PKG_NAME_LEN;
	}

    strncpy(doc->pkgName, pkgName, pkgNameLen);
    doc->pkgNameLen = pkgNameLen;

    return CLI_OK;
}

int cli_docSetCmd(CliDoc *doc, unsigned int cmdCode, char *cmdName, unsigned int cmdNameLen)
{
	if(cmdNameLen > CLI_CMD_NAME_LEN){
		CLI_LOG(CLI_ERR,"Invalid command name length(%d)\n",cmdNameLen);
		return CLI_ERR_INVALID_CMD_NAME_LEN;
	}

	doc->cmdCode = cmdCode;
	strncpy(doc->cmdName, cmdName, cmdNameLen);
	doc->cmdNameLen = cmdNameLen;

	return CLI_OK;
}

int cli_docSetCmdDesc(CliDoc *doc, char *desc, unsigned int descLen)
{
	if(descLen > CLI_CMD_DESC_LEN){
		CLI_LOG(CLI_ERR,"Invalid command description length(%d)\n",descLen);
		return CLI_ERR_INVALID_DESC_LEN;
	}

    strncpy(doc->cmdDesc, desc, descLen);
    doc->cmdDescLen = descLen;

	return CLI_OK;
}

int cli_docFindArg(CliDoc *doc, const char *argName, unsigned int argNameLen, CliArgDoc **rt_argDoc)
{
    int i = 0;
    int nodeCnt = 0;
    LnkLstDblNode *lnkNode = NULL;
    CliArgDoc *argDoc = NULL;

    nodeCnt = doc->argDocLL.nodeCnt;
    if(nodeCnt == 0){
        return CLI_ERR_NOT_EXIST;
    }

    LNKLST_DBL_GET_FIRST(&doc->argDocLL, lnkNode);
    for(i=0;i<nodeCnt;i++){
        argDoc = (CliArgDoc*)lnkNode->data;

        if(argDoc->argNameLen != argNameLen){
            LNKLST_DBL_GET_NEXT_NODE(lnkNode);
            continue;
        }

        if(strncasecmp(argDoc->argName, argName, argNameLen) == 0){
            *rt_argDoc = argDoc;

            return CLI_OK;
        }

        LNKLST_DBL_GET_NEXT_NODE(lnkNode);
    }/* end of for(i=0;i<nodeCnt;i++) */

    return CLI_ERR_NOT_EXIST;
}

int cli_docArgAdd(CliDoc *doc, char *argName, unsigned int argNameLen, CliArgDoc **rt_argDoc)
{
    int ret = 0;
    CliArgDoc *argDoc = NULL;

    if(argNameLen == 0){
        CLI_LOG(CLI_ERR,"Arguemnt name not exist\n");
        return CLI_ERR_ARG_NAME_NOT_EXIST;
    }

    if(argNameLen > CLI_ARG_NAME_LEN){
        CLI_LOG(CLI_ERR,"Invalid arguemnt name length(%d)\n", argNameLen);
        return CLI_ERR_ARG_NAME_INVALID_LEN;
    }

    ret = cli_docFindArg(doc, argName, argNameLen, &argDoc);
    if(ret == CLI_OK){
        CLI_LOG(CLI_ERR,"Arguemnt already exist(ret=%d)\n",ret);
        return CLI_ERR_ARG_ALREADY_EXIST;
    }

    argDoc = (CliArgDoc*)malloc(sizeof(CliArgDoc));

    strncpy(argDoc->argName, argName, argNameLen);

    argDoc->argNameLen = argNameLen;

    ret = lnkLst_dblInit(&argDoc->argParaLL, ~0);
    if(ret  != CLI_OK){
        CLI_LOG(CLI_ERR,"Linked list init failed(ret=%d)\n",ret);
        return CLI_ERR_LNKLST_INIT_FAILED;
    }

    argDoc->lnkNode.data = argDoc;

    ret = lnkLst_dblInsertTail(&doc->argDocLL, &argDoc->lnkNode);
    if(ret != CLI_OK){
        CLI_LOG(CLI_ERR,"argument insert failed(ret=%d)\n",ret);
        return CLI_ERR_LNKLST_INSERT_FAILED;
    }

    *rt_argDoc = argDoc;

    return  CLI_OK;
}

int cli_docArgParaIsStr(CliArgParaDoc *argParaDoc)
{
	if(argParaDoc->valType == CLI_ARG_TYPE_STRING){
		return CLI_TRUE;
	}
	return CLI_FALSE;
}

int cli_docArgParaIsNum(CliArgParaDoc *argParaDoc)
{
	if(argParaDoc->valType == CLI_ARG_TYPE_NUMBER){
		return CLI_TRUE;
	}
	return CLI_FALSE;
}

int cli_docArgParaGetNumVal(CliArgParaDoc *argParaDoc, unsigned int *rt_val)
{
	if(argParaDoc->valType != CLI_ARG_TYPE_NUMBER){
		CLI_LOG(CLI_ERR,"Invalid Argument type(%d)\n",argParaDoc->valType);
		return CLI_ERR_ARG_INVALID_TYPE;
	}

	*rt_val = argParaDoc->u.num.val;

	return CLI_OK;
}

int cli_docArgParaGetStrVal(CliArgParaDoc *argParaDoc, char **rt_val, unsigned int *rt_valLen)
{
	if(argParaDoc->valType != CLI_ARG_TYPE_STRING){
		CLI_LOG(CLI_ERR,"Invalid Argument type(%d)\n",argParaDoc->valType);
		return CLI_ERR_ARG_INVALID_TYPE;
	}

	*rt_val = argParaDoc->u.str.val;
	*rt_valLen = argParaDoc->u.str.valLen;

	return CLI_OK;
}

int cli_docArgParaNumCmp(CliArgParaDoc *argParaDoc, unsigned int val)
{
	if(argParaDoc->valType != CLI_ARG_TYPE_NUMBER){
		return CLI_FALSE;
	}
	
	if(argParaDoc->u.num.val == val){
		return CLI_TRUE;
	}

	return CLI_FALSE;
}

int cli_docArgParaStrCmp(CliArgParaDoc *argParaDoc, const char *val, unsigned int valLen)
{
	if(argParaDoc->valType != CLI_ARG_TYPE_STRING){
		return CLI_FALSE;
	}

	if((argParaDoc->u.str.valLen == valLen) &&
			(strncasecmp(argParaDoc->u.str.val, val, valLen) == 0)){
		return CLI_TRUE;
	}

	return CLI_FALSE;
}

int cli_docArgParaAdd(CliArgCb *argCb, CliArgDoc *argDoc, const char *val, unsigned int valLen)
{
	int ret = 0;
	CliArgParaDoc *argParaDoc = NULL;
	CliArgEnumCb *argEnumCb = NULL;
	int insrtType = 0;
	int insrtValNum = 0;
	const char *insrtVal = NULL;
	int insrtValLen = 0;

	if(argCb->argType == CLI_ARG_TYPE_ENUM){
		if(valLen == 0){
			CLI_LOG(CLI_ERR,"Value not exist(len=%d)\n",valLen);
			return CLI_ERR_ARG_VAL_NOT_EXIST;
		}

		if(valLen > CLI_ARG_VAL_LEN){
			CLI_LOG(CLI_ERR,"Invalid value length(len=%d)\n",valLen);
			return CLI_ERR_ARG_VAL_INVALID_LEN;
		}

		ret = cli_argEnumFind(argCb, val, valLen, &argEnumCb);
		if(ret != CLI_OK){
			CLI_LOG(CLI_ERR,"Enum not exist(%.*s)\n", valLen, val);
			return CLI_ERR_ENUM_NAME_NOT_EXIST;
		}

		if(argEnumCb->enbFlg == CLI_FALSE){
			CLI_LOG(CLI_ERR,"Enum not exist(%.*s)\n", valLen, val);
			return CLI_ERR_ENUM_NAME_NOT_EXIST;
		}

		if(argEnumCb->enumValFlg == CLI_TRUE){
			if(argEnumCb->enumValType == CLI_ARG_TYPE_STRING){
				insrtVal = argEnumCb->u.str.enumVal;
				insrtValLen = argEnumCb->u.str.enumValLen;
				insrtType = CLI_ARG_TYPE_STRING;
			}
			else {
				insrtValNum = argEnumCb->u.num.enumVal;
				insrtType = CLI_ARG_TYPE_NUMBER;
			}
		}
		else {
			insrtVal = argEnumCb->enumName;
			insrtValLen = argEnumCb->enumNameLen;
			insrtType = CLI_ARG_TYPE_STRING;
		}
	}/* if(argCb->type == CLI_ARG_TYPE_ENUM) */
	else {
		if(valLen == 0){ /* check default value */
			if(argCb->dfltArgValFlg == CLI_FALSE){
				CLI_LOG(CLI_ERR," Default value not exist\n");
				return CLI_ERR_ARG_VAL_NOT_EXIST;
			}

			insrtType = argCb->argType;
			if(insrtType == CLI_ARG_TYPE_NUMBER){
				insrtValNum = argCb->u.num.dfltArgVal;
			}
			else {
				insrtVal = argCb->u.str.dfltArgVal;
				insrtValLen = argCb->u.str.dfltArgValLen;
			}
		}
		else {
			insrtType = argCb->argType;
			if(insrtType == CLI_ARG_TYPE_NUMBER){
				ret = cli_parseIsNumber(val, valLen);
				if(ret != CLI_OK){
					CLI_LOG(CLI_ERR,"Value is not number(%.*s)\n",valLen, val);
					return CLI_ERR_INVALID_NUM_VALUE;
				}
				insrtValNum = atoi(val);
			}
			else {
				insrtVal = val;
				insrtValLen = valLen;
			}
		}
	}/* end of else */

	argParaDoc = (CliArgParaDoc*)malloc(sizeof(CliArgParaDoc));

	if(insrtType == CLI_ARG_TYPE_NUMBER){
		argParaDoc->u.num.val = insrtValNum;
	}
	else {
		strncpy(argParaDoc->u.str.val, insrtVal, insrtValLen);
		argParaDoc->u.str.valLen = insrtValLen;
	}

	argParaDoc->valType = insrtType;

	argParaDoc->lnkNode.data = argParaDoc;

	ret = lnkLst_dblInsertTail(&argDoc->argParaLL, &argParaDoc->lnkNode);
	if(ret != LNKLST_OK){
		CLI_LOG(CLI_ERR,"argument parameter linked list insert failed(ret=%d)\n",ret);
		return CLI_ERR_LNKLST_INSERT_FAILED;
	}

	return CLI_OK;
}

int cli_docDebugArgParaPrint(CliArgParaDoc *argParaDoc)
{
    if(argParaDoc->valType == CLI_ARG_TYPE_NUMBER){
        CLI_DISP("    + PARAMETER=(%d)\n",argParaDoc->u.num.val);
    }
    else {
        CLI_DISP("    + PARAMETER=(\"%.*s\")\n",argParaDoc->u.str.valLen, argParaDoc->u.str.val);
    }

    return CLI_OK;
}

int cli_docDebugArgPrint(CliArgDoc *argDoc)
{
    int ret = 0;
    int i = 0;
    int nodeCnt = 0;
    CliArgParaDoc *argParaDoc = NULL;
    LnkLstDblNode *lnkNode = NULL;

    CLI_DISP("  + ARGUMENT=(%.*s)\n",argDoc->argNameLen, argDoc->argName);

    nodeCnt = argDoc->argParaLL.nodeCnt;

    LNKLST_DBL_GET_FIRST(&argDoc->argParaLL, lnkNode);
    for(i=0;i<nodeCnt;i++){
        argParaDoc = (CliArgParaDoc*)lnkNode->data;

        ret = cli_docDebugArgParaPrint(argParaDoc);
        if(ret != CLI_OK){
            CLI_LOG(CLI_ERR,"Argument para debug print failed(ret=%d)\n",ret);
            return CLI_NOK;
        }

        LNKLST_DBL_GET_NEXT_NODE(lnkNode);
    }/* end of for(i=0;i<nodeCnt;i++) */

    return CLI_OK;
}

int cli_docDebugPrint(CliDoc *doc)
{
    int ret = 0;
    int i = 0;
    int nodeCnt = 0;
    CliArgDoc *argDoc = NULL;
    LnkLstDblNode *lnkNode = NULL;

    if(doc->pkgNameLen != 0){
        CLI_DISP("PACKAGE=(%.*s)\n",doc->pkgNameLen, doc->pkgName);
    }

    CLI_DISP("COMMAND=(%.*s)\n",doc->cmdNameLen, doc->cmdName);
    CLI_DISP("------------------------------------\n");

    nodeCnt = doc->argDocLL.nodeCnt;

    LNKLST_DBL_GET_FIRST(&doc->argDocLL, lnkNode);
    for(i=0;i<nodeCnt;i++){
        argDoc = (CliArgDoc*)lnkNode->data;

        ret = cli_docDebugArgPrint(argDoc);
        if(ret != CLI_OK){
            CLI_LOG(CLI_ERR,"Argument debug print failed(ret=%d)\n",ret);
            return CLI_NOK;
        }
        CLI_DISP("------------------------------------\n");

        LNKLST_DBL_GET_NEXT_NODE(lnkNode);
    }/* end of for(i=0;i<nodeCnt;i++) */

    return CLI_OK;
}

