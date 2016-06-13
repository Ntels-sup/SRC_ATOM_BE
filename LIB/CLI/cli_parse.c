#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "lnkLst.h"
#include "cli.h"
#include "cliInt.h"

#define SKIP_SPACE(_cur, _remLen){\
    char *d_tmp;\
    d_tmp = cli_parseSkipSpace(_cur , _remLen);\
    _remLen -= d_tmp - _cur;\
    _cur = d_tmp;\
}

#define GET_TOKEN(_cur, _remLen, _stTkn, _tknLen, _ret){\
	int d_len = 0;\
    (_tknLen) = cli_parseGetToken((_cur), (_remLen), &(_stTkn));\
    if((_tknLen) == 0){\
        (_ret) = CLI_ERR_TOKEN_NOT_EXIST;\
    }\
	d_len = ((_stTkn)+(_tknLen)) - (_cur);\
	if(d_len >= _remLen){\
        (_ret) = CLI_ERR_TOKEN_END;\
	}\
	else {\
		(_remLen) -= d_len;\
		(_cur) = (_stTkn)+(_tknLen);\
		SKIP_SPACE((_cur), (_remLen));\
		(_ret) = CLI_OK;\
	}\
}

int    parse_ChkChar     (char data);
char*  parse_GetString   (char *cliLine, unsigned int cliLineLen);
int    cli_parseGetToken (char *cliLine, unsigned int cliLineLen, char **rt_stTkn);
int    parse_GetArgPara  (CliArgCb *argCb, CliArgDoc *argDoc, char *cliLine, unsigned int cliLineLen, char **rt_nxtStTkn);
int    parse_GetArg      (CliDoc *doc, CliCmdCb *cmdCb, char *cliLine, unsigned int cliLineLen, char **rt_nxtStTkn);
int    parse_GetCmd      (CliCb *cliCb, char *cliLine, unsigned int cliLineLen, char **rt_nxtStTkn, 
		CliCmdCb **rt_cmdCb, CliDoc **rt_doc);

int parse_ChkChar(char data)
{
    if(data >= 48 &&  data <= 57){ /* 0 ~ 9 */
        return CLI_TKN_TYPE_NUMBER_CHAR;
    }
    else if((data >=  65 &&  data <= 90) || /* A ~ Z */
            (data >= 97 && data<= 122)){ /* a ~ z */
        return CLI_TKN_TYPE_LETTER_CHAR;
    }
    else if((data == '-') || (data == '_')){
        return CLI_TKN_TYPE_SPECIAL_CHAR;
    }
    else if(data == ','){
        return CLI_TKN_TYPE_COMMA_CHAR;
    }
    else if(data == ':'){
        return CLI_TKN_TYPE_COLON_CHAR;
    }
    else if(data == ';'){
        return CLI_TKN_TYPE_SEMICOLON_CHAR;
    }
    else if(data == '='){
        return CLI_TKN_TYPE_EQUAL_CHAR;
    }
    else if(data == '.'){
        return CLI_TKN_TYPE_PERIOD_CHAR;
    }
	else if(data == '?'){
		return CLI_TKN_TYPE_QUST_MARK_CHAR;
	}
    else if(data == 32){
        return CLI_TKN_TYPE_SPACE_CHAR;
    }

    return CLI_NOK;
}

char *cli_parseSkipSpace(char *cliLine, unsigned int cliLineLen)
{
    int ret = 0;
    unsigned int i = 0;
    char *cur = NULL;

    cur = cliLine;

    for(i=0;i<cliLineLen;i++){
        ret = parse_ChkChar(*cur);
        if(ret == CLI_TKN_TYPE_SPACE_CHAR){
            cur++;
        }
    }/* end of for(i=0;i<cliLineLen;i++) */

    return cur;
}

int cli_parseIsNumber(const char *tkn, unsigned int tknLen)
{
	int ret = 0;
	unsigned int i = 0;
	char *cur = NULL;

	cur = (char*)tkn;
	for(i=0;i<tknLen;i++){
        ret = parse_ChkChar(*cur);
        if((ret != CLI_TKN_TYPE_NUMBER_CHAR)){
			return CLI_NOK;
        }
        cur++;
	}

	return CLI_OK;
}

char *parse_GetString(char *cliLine, unsigned int cliLineLen)
{
    int ret = 0;
    unsigned int i = 0;
    char *cur = NULL;

    cur = cliLine;

    for(i=0;i<cliLineLen;i++){
        ret = parse_ChkChar(*cur);
        if((ret != CLI_TKN_TYPE_NUMBER_CHAR) && 
                (ret != CLI_TKN_TYPE_LETTER_CHAR) &&
                (ret != CLI_TKN_TYPE_SPECIAL_CHAR)){
            return cur; 
        }
        cur++;
    }

    return cur;
}

int cli_parseGetToken(char *cliLine, unsigned int cliLineLen, char **rt_stTkn)
{
    char *cur = NULL;
    char *stTkn = NULL;
    int remLen = 0;

    remLen = cliLineLen;

    cur = cliLine;

    stTkn = cli_parseSkipSpace(cur, remLen);

    remLen -= stTkn - cur;

    cur = parse_GetString(stTkn, remLen);

    *rt_stTkn = stTkn;

    return (cur - stTkn);
}

int cli_parseGetPreCmd(CliCb *cliCb, const char *cliLine, unsigned int cliLineLen, CliPreCmdRslt *rt_preCmdRslt)
{
	unsigned int argCnt = 0;
	unsigned int fixFlg = CLI_FALSE;
	int ret = 0;
	int remLen = 0;
	int tknType = 0;
	unsigned int qustMaskFlg = CLI_FALSE;
	CliPkgCb *pkgCb = NULL;
	CliCmdCb *cmdCb = NULL;
	CliArgCb *argCb = NULL;
	char *cur = NULL;
	char *stTkn = NULL;
	int stTknLen = 0;
	unsigned int argTknLen = 0;
	char *argTkn =NULL;

	cur = (char*)cliLine;
	remLen = cliLineLen;

	if((cliLineLen != 0) && (cliLine[cliLineLen-1] == '?')){
		qustMaskFlg = CLI_TRUE;
	}

	while(1){
		GET_TOKEN(cur, remLen, stTkn, stTknLen, ret);
		if(ret != CLI_OK){
			if(ret == CLI_ERR_TOKEN_END){
				goto goto_RetRsltOk;
			}
			else {
				CLI_LOG(CLI_ERR,"Command not exist(ret=%d)\n",ret);
				return ret;
			}
		}

		tknType = parse_ChkChar(*cur);
		switch(tknType){
			case CLI_TKN_TYPE_PERIOD_CHAR:
				{
					if(pkgCb != NULL){
						CLI_LOG(CLI_ERR,"Package already exist\n");
						return CLI_ERR_NOT_EXIST;
					}

					ret = cli_pkgFind(cliCb, stTkn, stTknLen, &pkgCb);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"Package not exist(%.*s)\n",stTknLen, stTkn);
						return CLI_ERR_NOT_EXIST;
					}

					cur++;
					remLen--;
				}
				break;
			case CLI_TKN_TYPE_EQUAL_CHAR:
				{
					fixFlg = CLI_TRUE;
					argTknLen = stTknLen;
					argTkn = stTkn;

					cur++;
					remLen--;
				}
				break;
			case CLI_TKN_TYPE_COMMA_CHAR:
				{
					cur++;
					remLen--;
				}
				break;
			case CLI_TKN_TYPE_COLON_CHAR:
			case CLI_TKN_TYPE_QUST_MARK_CHAR:
				{
					if(cmdCb == NULL){
						if(pkgCb == NULL){
							pkgCb = cli_pkgGetDfltPkg(cliCb);
							if(pkgCb == NULL){
								CLI_LOG(CLI_ERR,"Default Package not exist(%.*s)\n",stTknLen, stTkn);
								return CLI_ERR_NOT_EXIST;
							}
						}

						ret = cli_cmdFind(pkgCb, stTkn, stTknLen, &cmdCb);
						if(ret != CLI_OK){
							CLI_LOG(CLI_ERR,"command not exist(%.*s)\n",stTknLen, stTkn);
							return CLI_ERR_NOT_EXIST;
						}

						if(qustMaskFlg == CLI_TRUE){
							goto goto_RetRsltOk;
						}
					}
					else {
						if(qustMaskFlg == CLI_TRUE){
							goto goto_RetRsltOk;
						}

						if(argTknLen != 0){
							argTknLen = 0;
							argTkn = NULL;
						}

						argCnt++;
					}

					cur++;
					remLen--;
				}
				break;
			case CLI_TKN_TYPE_SEMICOLON_CHAR:
				{
					return CLI_RSLT_TERM;
				}
				break;
			default:
				{
					return CLI_ERR_NOT_EXIST;
				}
				break;
		};/* end of switch(tknType) */
	}/* end of while(1) */

goto_RetRsltOk:
	if(pkgCb == NULL){
		rt_preCmdRslt->pkgCb = cli_pkgGetDfltPkg(cliCb);
	}
	else {
		rt_preCmdRslt->pkgCb = pkgCb;
	}

	rt_preCmdRslt->cmdCb = cmdCb;

	if(cmdCb != NULL){
		if(fixFlg == CLI_TRUE){
			ret = cli_argFind(cmdCb, argTkn, argTknLen, &argCb);
			if(ret != CLI_OK){
				return CLI_ERR_NOT_EXIST;
			}
			argCnt = 0;
		}
		else {
			/* get next argument */
			ret = cli_argFindBySeq(cmdCb, ++argCnt, &argCb);
			if(ret != CLI_OK){
				return CLI_ERR_NOT_EXIST;
			}
		}
		rt_preCmdRslt->argCb = argCb;
		rt_preCmdRslt->fixFlg = fixFlg;
		rt_preCmdRslt->argCnt = argCnt;
	}
	else {
		rt_preCmdRslt->argCb = NULL;
		rt_preCmdRslt->fixFlg = CLI_FALSE;
		rt_preCmdRslt->argCnt = 0;
	}

	rt_preCmdRslt->tkn = stTkn;
	rt_preCmdRslt->tknLen = stTknLen;

	return CLI_OK;
}

int parse_GetCmd(CliCb *cliCb, char *cliLine, unsigned int cliLineLen, char **rt_nxtStTkn, CliCmdCb **rt_cmdCb, CliDoc **rt_doc)
{
	int ret = 0;
	int tknLen = 0;
	int remLen = 0;
	int tknType = 0;
	int pkgFlag = CLI_FALSE;
	char *cur = NULL;
	char *stTkn = NULL;
	CliPkgCb *pkgCb = NULL;
	CliCmdCb *cmdCb = NULL;
	CliDoc *doc = NULL;

	cur = cliLine;
	remLen = cliLineLen;

	while(1){
		GET_TOKEN(cur, remLen, stTkn, tknLen, ret);
		if(ret != CLI_OK){
			CLI_LOG(CLI_ERR,"Command not exist(ret=%d)\n",ret);
			if(ret == CLI_ERR_TOKEN_NOT_EXIST){
				CLI_PARSE_LOG("TOKEN NOT EXIST\n");
			}
			else if(ret == CLI_ERR_TOKEN_END){
				CLI_PARSE_LOG("SEPERATOR NOT EXIST(%.*s)\n", remLen, &cliLine[cliLineLen-remLen]);
			}
			return ret;
		}

		tknType = parse_ChkChar(*cur);
		switch(tknType){
			case CLI_TKN_TYPE_PERIOD_CHAR:
				if(pkgFlag == CLI_TRUE){
					CLI_LOG(CLI_ERR,"Package alreay exist\n");
					CLI_PARSE_LOG("PACKAGE ALREADY EXIST\n");
					return CLI_ERR_PACKAGE_ALREADY_EXIST;
				}
				//fprintf(stderr,"PKG = %.*s\n",tknLen , stTkn);
				cur++;
				remLen--;

				pkgFlag = CLI_TRUE;

				/* find package */
				ret = cli_pkgFind(cliCb, stTkn, tknLen, &pkgCb);
				if(ret != CLI_OK){
					CLI_LOG(CLI_ERR,"Package not exist(%.*s)\n",tknLen, stTkn);
					CLI_PARSE_LOG("PACKAGE NOT EXIST\n");
					return CLI_ERR_PKG_NOT_EXIST; 
				}

				break;
			case CLI_TKN_TYPE_COLON_CHAR:
			case CLI_TKN_TYPE_SEMICOLON_CHAR:
				/* find all commmand */
				if(pkgFlag != CLI_TRUE){
					pkgCb = cli_pkgGetDfltPkg(cliCb);
					if(pkgCb == NULL){
						CLI_LOG(CLI_ERR,"Default Package not exist(%.*s)\n",tknLen, stTkn);
						CLI_PARSE_LOG("DEFAULT PACKAGE NOT EXIST\n");
						return CLI_ERR_PKG_NOT_EXIST; 
					}
				}

				ret = cli_cmdFind(pkgCb, stTkn, tknLen, &cmdCb);
				if(ret != CLI_OK){
					CLI_LOG(CLI_ERR,"Command not exist(%.*s)\n",tknLen, stTkn);
					CLI_PARSE_LOG("COMMAND NOT EXIST\n");
					return CLI_ERR_CMD_NOT_EXIST; 

				}

				if(cmdCb->enbFlg == CLI_FALSE){
					CLI_LOG(CLI_ERR,"Command not exist(%.*s)\n",tknLen, stTkn);
					CLI_PARSE_LOG("COMMAND NOT EXIST\n");
					return CLI_ERR_CMD_NOT_EXIST; 
				}

				/* make doc */
				if(doc != NULL){
					CLI_LOG(CLI_ERR,"Document already exist\n");
					CLI_PARSE_LOG("INTERNAL ERROR\n");
					return CLI_ERR_DOC_ALREADY_EXIST;
				}

				doc = (CliDoc*)malloc(sizeof(CliDoc));

				ret = cli_docInit(doc);
				if(ret != CLI_OK){
					CLI_LOG(CLI_ERR,"Document init failed(ret=%d)\n",ret);
					CLI_PARSE_LOG("INTERNAL ERROR\n");
					free(doc);
					return CLI_ERR_DOC_INIT_FAILED;
				}

				ret = cli_docSetPkg(doc, pkgCb->pkgName, pkgCb->pkgNameLen);
				if(ret != CLI_OK){
					CLI_LOG(CLI_ERR,"Package setting failed(ret=%d)\n",ret);
					CLI_PARSE_LOG("INTERNAL ERROR\n");
					free(doc);
					return CLI_ERR_DOC_PKG_SET_FAIELD;
				}

				ret = cli_docSetCmd(doc, cmdCb->cmdCode, cmdCb->cmdName, cmdCb->cmdNameLen);
				if(ret != CLI_OK){
					CLI_LOG(CLI_ERR,"Package setting failed(ret=%d)\n",ret);
					CLI_PARSE_LOG("INTERNAL ERROR\n");
					free(doc);
					return CLI_ERR_DOC_PKG_SET_FAIELD;
				}

				ret = cli_docSetCmdDesc(doc, cmdCb->desc, cmdCb->descLen);
				if(ret != CLI_OK){
					CLI_LOG(CLI_ERR,"command description setting failed(ret=%d)\n",ret);
					CLI_PARSE_LOG("INTERNAL ERROR\n");
					free(doc);
					return CLI_ERR_DOC_DESC_SET_FAIELD;
				}

				if(rt_doc != NULL){
					*rt_doc = doc;
				}
				*rt_cmdCb = cmdCb;
				*rt_nxtStTkn = ++cur;

				if(tknType == CLI_TKN_TYPE_SEMICOLON_CHAR){
					if(cmdCb->argMandCnt != 0){
						CLI_LOG(CLI_ERR,"Too few arguemnt(cnt=%d)\n",cmdCb->argMandCnt);
						CLI_PARSE_LOG("TOO FEW ARGUMENT\n");
						return CLI_ERR_TOO_FEW_ARG;
					}
					else {
						return CLI_RSLT_TERM;
					}
				}
				return CLI_OK;
			default:
				CLI_LOG(CLI_ERR,"Invalid sperator(%c)\n",*cur);
				CLI_PARSE_LOG("INVALID SEPERATOR(%c)\n",*cur);
				return CLI_ERR_INVALID_SEPERATOR;
		};

		SKIP_SPACE(cur, remLen);
	}/* end of while(1) */

	CLI_LOG(CLI_ERR,"Seperator not exist\n");
	CLI_PARSE_LOG("SEPERATOR NOT EXIST\n");

	return CLI_ERR_SEPERATOR_NOT_EXIST;
}

int parse_GetArgPara(CliArgCb *argCb, CliArgDoc *argDoc, char *cliLine, unsigned int cliLineLen, char **rt_nxtStTkn)
{
	unsigned int ret = 0;
	unsigned int remLen = 0;
	unsigned int paraCnt = 0;
	unsigned int tknType = 0;
	char *cur = NULL;
	unsigned int tknLen = 0;
	char *stTkn = NULL;

	cur = cliLine;
	remLen = cliLineLen;

	while(1){
		GET_TOKEN(cur, remLen, stTkn, tknLen, ret);
		if(ret != CLI_OK){
			if(ret == CLI_ERR_TOKEN_END){
				CLI_LOG(CLI_ERR,"seperator not exist\n");
				CLI_PARSE_LOG("SEPERATOR NOT EXIST\n");
			}
			else {
				CLI_LOG(CLI_ERR,"arg Parameter not exist(ret=%d)\n",ret);
				CLI_PARSE_LOG("ARGUMENT TOKEN NOT EXIST\n");
			}
			return ret;
		}

		tknType  = parse_ChkChar(*cur);
		switch(tknType){
			case CLI_TKN_TYPE_COMMA_CHAR:
			case CLI_TKN_TYPE_COLON_CHAR:
			case CLI_TKN_TYPE_SEMICOLON_CHAR:
				{
					//fprintf(stderr, "PARA=%.*s\n", tknLen, stTkn);

					/* insert doc */
					if((argCb->multiFlg == CLI_FALSE) && 
							(paraCnt != 0)){
						CLI_LOG(CLI_ERR,"Mulit flag not set(argName=%.*s, paraCnt=%d)\n",
								argCb->argNameLen, argCb->argName, paraCnt);
						CLI_PARSE_LOG("MULTI PARAMETER NOT ALLOWD\n");
						return CLI_ERR_MULTI_FLAG_NOT_SET;
					}

					if((argCb->multiFlg == CLI_TRUE) &&
							(argCb->multiMaxCnt <= argDoc->argParaLL.nodeCnt)){
						CLI_LOG(CLI_ERR,"too many arg para(argName=%.*s, max=%d)\n",
								argCb->argNameLen, argCb->argName, argCb->multiMaxCnt);
						CLI_PARSE_LOG("TOO MANY ARGUMENT PARAMETER\n");
						return CLI_ERR_TOO_MANY_ARG_PARA;
					}

					ret = cli_argChkRng(argCb, tknLen);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"Invalid argument range(min=%d, max=%d, tknLen=%d)\n", 
								argCb->rngMin, argCb->rngMax, tknLen);
						CLI_PARSE_LOG("INVALID %.*s ARGUMENT RANGE(MIN=%d, MAX=%d, LEN=%d)\n", 
								argCb->argNameLen, argCb->argName, argCb->rngMin, argCb->rngMax, tknLen);
						return CLI_ERR_ARG_INVLAID_RANGE; 
					}

					ret = cli_docArgParaAdd(argCb, argDoc, stTkn, tknLen);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"arguemnt parameter add failed(ret=%d, tkn=%.*s)\n",
								ret, tknLen, stTkn);
						if(ret == CLI_ERR_ENUM_NAME_NOT_EXIST){
							CLI_PARSE_LOG("\"%.*s\" ENUM VALUE NOT EXIST\n",argCb->argNameLen, argCb->argName);
						}
						else if(ret == CLI_ERR_INVALID_NUM_VALUE){
							CLI_PARSE_LOG("\"%.*s\" IS NOT NUMBER\n",tknLen, stTkn);
						}
						else {
							CLI_PARSE_LOG("INTERNAL ERROR\n");
						}
						return CLI_ERR_DOC_ARG_PARA_ADD_FAILED;
					}

					paraCnt++;

					if(tknType == CLI_TKN_TYPE_COMMA_CHAR){
						cur++;
						remLen--;
					}
					else if(tknType == CLI_TKN_TYPE_SEMICOLON_CHAR){
						return CLI_RSLT_TERM;
					}
					else if(tknType == CLI_TKN_TYPE_COLON_CHAR){
						*rt_nxtStTkn = ++cur;

						return CLI_OK;
					}
				}
				break;
			default:
				CLI_LOG(CLI_ERR,"Invalid sperator(%c)\n",*cur);
				CLI_PARSE_LOG("INVALID SEPERATOR(%c)\n", *cur);
				return CLI_ERR_INVALID_SEPERATOR;
		};/* end of switch(ret) */
		SKIP_SPACE(cur, remLen);
	}/* end of while(1) */

	CLI_LOG(CLI_ERR,"Unkonwon error\n");
	CLI_PARSE_LOG("UNKNOWN ERROR\n");
	return CLI_ERR_PARSING_FAILED;
}

int parse_GetArg(CliDoc *doc, CliCmdCb *cmdCb, char *cliLine, unsigned int cliLineLen, char **rt_nxtStTkn)
{
	int ret = 0;
	int seq = 0;
	int tknType = 0;
	int remLen = 0;
	int tknLen = 0;
	char *cur = NULL;
	char *stTkn = NULL;
	CliArgCb *argCb = NULL;
	CliArgDoc *argDoc = NULL;

	cur = cliLine;
	remLen = cliLineLen;

	GET_TOKEN(cur, remLen, stTkn, tknLen, ret);
	if(ret != CLI_OK){
		if(ret == CLI_ERR_TOKEN_END){
			CLI_LOG(CLI_ERR,"seperator not exist\n");
			CLI_PARSE_LOG("SEPERATOR NOT EXIST\n");
		}
		else {
			CLI_LOG(CLI_ERR,"arg Parameter not exist(ret=%d)\n",ret);
			CLI_PARSE_LOG("ARGUMENT PARAMETER NOT EXIST\n");
		}
		return ret;
	}

	tknType = parse_ChkChar(*cur);
	switch(tknType){
		case CLI_TKN_TYPE_EQUAL_CHAR:
		case CLI_TKN_TYPE_COMMA_CHAR:
			{
				seq = ++doc->nxtSeq;

				/* find */
				if(tknType == CLI_TKN_TYPE_COMMA_CHAR){
					ret = cli_argFindBySeq(cmdCb, seq, &argCb);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"argumnet paramter not exist(seq=%d, ret=%d)\n",seq, ret);
						CLI_PARSE_LOG("ARGUMENT NOT EXIST\n");
						return CLI_ERR_ARG_NAME_NOT_EXIST;
					}

					ret = cli_argChkRng(argCb, tknLen);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"Invalid argument range(min=%d, max=%d, tknLen=%d)\n", 
								argCb->rngMin, argCb->rngMax, tknLen);
						CLI_PARSE_LOG("INVALID %.*s ARGUMENT RANGE(MIN=%d, MAX=%d, LEN=%d)\n", 
								argCb->argNameLen, argCb->argName, argCb->rngMin, argCb->rngMax, tknLen);
						return CLI_ERR_ARG_INVLAID_RANGE; 
					}

					ret = cli_docArgAdd(doc, argCb->argName, argCb->argNameLen, &argDoc);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"arguemnt add failed(ret=%d)\n",ret);
						CLI_PARSE_LOG("INTERNAL ERROR\n");
						return CLI_ERR_DOC_ARG_ADD_FAIELD;
					}

					ret = cli_docArgParaAdd(argCb, argDoc, stTkn, tknLen);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"Argument parameter add failed(ret=%d)\n",ret);
						if(ret == CLI_ERR_ENUM_NAME_NOT_EXIST){
							CLI_PARSE_LOG("\"%.*s\" ENUM VALUE NOT EXIST\n",argCb->argNameLen, argCb->argName);
						}
						else if(ret == CLI_ERR_INVALID_NUM_VALUE){
							CLI_PARSE_LOG("\"%.*s\" IS NOT NUMBER\n",tknLen, stTkn);
						}
						else {
							CLI_PARSE_LOG("INTERNAL ERROR\n");
						}
						return CLI_ERR_DOC_ARG_PARA_ADD_FAILED;
					}

				}
				else {
					doc->fixArgFlg = CLI_TRUE;
					ret = cli_argFind(cmdCb, stTkn, tknLen, &argCb);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"argumnet paramter not exist(arg=%.*s, ret=%d)\n",
								tknLen, stTkn, ret);
						CLI_PARSE_LOG("ARGUMENT NOT EXIST\n");
						return CLI_ERR_ARG_NAME_NOT_EXIST;
					}

					ret = cli_argChkRng(argCb, tknLen);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"Invalid argument range(min=%d, max=%d, tknLen=%d)\n", 
								argCb->rngMin, argCb->rngMax, tknLen);
						CLI_PARSE_LOG("INVALID %.*s ARGUMENT RANGE(MIN=%d, MAX=%d, LEN=%d)\n", 
								argCb->argNameLen, argCb->argName, argCb->rngMin, argCb->rngMax, tknLen);
						return CLI_ERR_ARG_INVLAID_RANGE; 
					}

					ret = cli_docFindArg(doc, argCb->argName, argCb->argNameLen, &argDoc);
					if(ret == CLI_OK){
						CLI_LOG(CLI_ERR,"arguemnt already exist(ret=%d)\n",ret);
						CLI_PARSE_LOG("ARGUMENT ALREADY EXIST\n");
						return CLI_ERR_ARG_ALREADY_EXIST;
					}

					ret = cli_docArgAdd(doc, argCb->argName, argCb->argNameLen, &argDoc);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"arguemnt add failed(ret=%d)\n",ret);
						CLI_PARSE_LOG("INTERNAL ERROR\n");
						return CLI_ERR_DOC_ARG_ADD_FAIELD;
					}
				}

				if(argCb->optFlg != CLI_TRUE){
					doc->argMandCnt++;
				}

				/* get para */
				cur++;
				remLen--;

				ret = parse_GetArgPara(argCb, argDoc, cur, remLen, &stTkn);
				if((ret != CLI_OK) && (ret != CLI_RSLT_TERM)){
					CLI_LOG(CLI_ERR,"argumnet paramter parsing failed(ret=%d)\n",ret);
					return ret;
				}

				*rt_nxtStTkn = stTkn;

				return ret;
			}
			break;
		case CLI_TKN_TYPE_COLON_CHAR:
		case CLI_TKN_TYPE_SEMICOLON_CHAR:
			{
				/* find */
				if(doc->fixArgFlg == CLI_TRUE){
					CLI_LOG(CLI_ERR,"fixed argumnet paramter parsing failed(ret=%d)\n",ret);
					CLI_PARSE_LOG("PARSING ERROR(USE FIX ARGUMENT)\n");
					return CLI_ERR_FIX_ARG_FLAG_SET;
				}

				seq = ++doc->nxtSeq;

				ret = cli_argFindBySeq(cmdCb, seq, &argCb);
				if(ret != CLI_OK){
					CLI_LOG(CLI_ERR,"argumnet paramter not exist(seq=%d, ret=%d)\n",seq, ret);
					CLI_PARSE_LOG("ARGUMENT NOT EXIST\n");
					return CLI_ERR_ARG_NAME_NOT_EXIST;
				}

				if(argCb->optFlg != CLI_TRUE){
					doc->argMandCnt++;
				}

				if(tknLen != 0){
					ret = cli_docArgAdd(doc, argCb->argName, argCb->argNameLen, &argDoc);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"arguemnt add failed(ret=%d)\n",ret);
						CLI_PARSE_LOG("INTERNAL ERROR\n");
						return CLI_ERR_DOC_ARG_ADD_FAIELD;
					}

					ret = cli_docArgParaAdd(argCb, argDoc, stTkn, tknLen);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"Argument parameter add failed(ret=%d)\n",ret);
						if(ret == CLI_ERR_ENUM_NAME_NOT_EXIST){
							CLI_PARSE_LOG("\"%.*s\" ENUM VALUE NOT EXIST\n",argCb->argNameLen, argCb->argName);
						}
						else if(ret == CLI_ERR_INVALID_NUM_VALUE){
							CLI_PARSE_LOG("\"%.*s\" IS NOT NUMBER\n",tknLen, stTkn);
						}
						else {
							CLI_PARSE_LOG("INTERNAL ERROR\n");
						}
						return CLI_ERR_DOC_ARG_PARA_ADD_FAILED;
					}
				}
				else if(argCb->optFlg != CLI_TRUE){ /* mandantory arguemnt */
					if(argCb->dfltArgValFlg == CLI_FALSE){
						CLI_LOG(CLI_ERR,"token not exist(mandatory argument is set)(argName=%.*s)\n",
								argCb->argNameLen, argCb->argName);
						CLI_PARSE_LOG("MANDARORY ARGUMENT NOT EXIST\n");
						return CLI_ERR_ARG_MAND_TYPE;
					}

					ret = cli_docArgAdd(doc, argCb->argName, argCb->argNameLen, &argDoc);
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"arguemnt add failed(ret=%d)\n",ret);
						CLI_PARSE_LOG("INTERNAL ERROR\n");
						return CLI_ERR_DOC_ARG_ADD_FAIELD;
					}

					ret = cli_docArgParaAdd(argCb, argDoc, NULL, 0); /* set default */
					if(ret != CLI_OK){
						CLI_LOG(CLI_ERR,"Argument parameter add failed(ret=%d)\n",ret);
						if(ret == CLI_ERR_ENUM_NAME_NOT_EXIST){
							CLI_PARSE_LOG("\"%.*s\" ENUM VALUE NOT EXIST\n",argCb->argNameLen, argCb->argName);
						}
						else if(ret == CLI_ERR_INVALID_NUM_VALUE){
							CLI_PARSE_LOG("\"%.*s\" IS NOT NUMBER\n",tknLen, stTkn);
						}
						else {
							CLI_PARSE_LOG("INTERNAL ERROR\n");
						}
						return CLI_ERR_DOC_ARG_PARA_ADD_FAILED;
					}
				}

				/* COLON */
				if(tknType == CLI_TKN_TYPE_SEMICOLON_CHAR){
					return CLI_RSLT_TERM;
				}

				*rt_nxtStTkn = ++cur;

				return CLI_OK;
			}
			break;
	}; /* end ofswitch(ret) */

	CLI_LOG(CLI_ERR,"Invalid sperator(%c)\n",*cur);
	CLI_PARSE_LOG("INVALID SPERATOR(%c)\n", *cur);

	return CLI_ERR_INVALID_SEPERATOR;
}

int cli_parseCmd(CliCb *cliCb, char *cliLine, unsigned int cliLineLen, CliDoc **rt_doc)
{
	int ret = 0;
	int remLen = 0;
	char *cur = NULL; /* cursor */
	char *stTkn = NULL; /* start token */
	char *cmdLine = NULL;
	CliCmdCb *cmdCb = NULL;
	CliDoc *doc = NULL;

	remLen = cliLineLen;
	cur = cliLine;

	if(cliLineLen == 0){
		return CLI_ERR_NOT_EXIST;
	}

	stTkn = cliLine;
	cur = cli_parseSkipSpace(stTkn, remLen);

	remLen -= cur - stTkn; 

	stTkn = cur;

	/* get command */
	ret = parse_GetCmd(cliCb, stTkn, remLen, &cur, &cmdCb, &doc);
	if(ret != CLI_OK){
		if(ret == CLI_RSLT_TERM){
			goto goto_ParseCmdRetOk;
		}

		CLI_LOG(CLI_ERR,"command parinsg failed(ret=%d)\n",ret);
		return ret;
	}

	remLen = remLen - (cur-stTkn);

	stTkn = cur;

	while(1){
		ret = parse_GetArg(doc, cmdCb, stTkn, remLen, &cur);
		if(ret == CLI_RSLT_TERM){
			goto goto_ParseCmdRetOk;
		}
		else if(ret != CLI_OK){
			cli_docDstry(doc);
			return ret;
		}

		remLen = remLen - (cur-stTkn);

		stTkn = cur;

		if(remLen == 0){
			cli_docDstry(doc);
			return CLI_ERR_END_OF_LINE;
		}
	}/* end of while(1) */

goto_ParseCmdRetOk:
	/* set command line */
	cmdLine = malloc(cliLineLen+1);

	strncpy(cmdLine, cliLine, cliLineLen);

	cmdLine[cliLineLen] = '\0';
	doc->cmdLine = cmdLine;
	doc->cmdLineLen = cliLineLen;

	/* return document */
	*rt_doc = doc;

	return CLI_OK;

}

