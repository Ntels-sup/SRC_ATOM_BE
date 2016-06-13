#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "lnkLst.h"
#include "cli.h"
#include "cliInt.h"

#define RL_PRMT_NAME_LEN 16+1

typedef struct RlBaseCmd RlBaseCmd;
typedef struct RlCfg     RlCfg;

struct RlBaseCmd{
	char            *cmdName;
	char            *shortcut;
	int             cmdId;
};

struct RlCfg{
	CliPreCmdRslt   preCmdRslt;
	unsigned int    prmtLen;
	char            prmt[RL_PRMT_NAME_LEN];
};

int initFlg = CLI_FALSE;

static RlCfg rlCfg;

static RlBaseCmd baseCmd[] = {
	{"history",  "his",        CLI_BASE_CMD_HISTORY},
	{"help",     "h",          CLI_BASE_CMD_HELP},
	{"list",     "l",          CLI_BASE_CMD_LIST},
	{"quit",     "q",          CLI_BASE_CMD_QUIT},
	{(char*)NULL, (char*)NULL, 0}
};

int    rl_help           ();
char   *rl_tapGen        (const char *line, int listCnt);
char   *rl_tapWhite      (const char *line, int lineCnt);
char   **rl_tapWord      (const char *line, unsigned int st, unsigned int end);
int    rl_prntCmdHelp    (CliCb *cliCb);

int cli_rlGetInitFlg()
{
	return initFlg;
}

int cli_rlSetPrmt(const char *prmt, unsigned int prmtLen)
{
    if(prmtLen >= RL_PRMT_NAME_LEN){
        CLI_LOG(CLI_ERR,"Invalid prompt len(%d)\n",prmtLen);
        return CLI_ERR_RL_INVALID_PRMT_LEN;
    }

    strncpy(rlCfg.prmt, prmt, prmtLen);

    rlCfg.prmtLen = prmtLen;

    rlCfg.prmt[prmtLen] = '\0';

	return CLI_OK;
}

int cli_rlInit()
{
    rl_readline_name = "CLI";

    rl_attempted_completion_function = (CPPFunction *)rl_tapWord;

	rl_catch_signals = 0;
    rl_basic_word_break_characters = ".,=: ";

    initFlg = CLI_TRUE;

    return CLI_OK;
}

char *rl_tapWhite(const char *line, int lineCnt)
{
	char *data = NULL;

	if(lineCnt == 0){
		data = malloc(sizeof(char));

		data[0] = '\0';
	}

	return data;
}

char *rl_tapGen(const char *line, int listCnt)
{
	static LnkLstDblNode *lnkNode = NULL;
	unsigned int lineLen = 0;
	CliCb *cliCb = NULL;
	CliPreCmdRslt *preCmdRslt = NULL;
	CliArgEnumCb *argEnumCb = NULL;
	CliPkgCb *pkgCb = NULL;
	CliCmdCb *cmdCb = NULL;
	CliArgCb *argCb = NULL;
	char *data = NULL;

	preCmdRslt = &rlCfg.preCmdRslt;

	if(preCmdRslt->argCb != NULL){
		argCb = preCmdRslt->argCb;

		if(listCnt == 0){
			LNKLST_DBL_GET_FIRST(&argCb->argEnumCbLL, lnkNode);
		}

		while(1){
			if(lnkNode == NULL){
				return NULL;
			}

			argEnumCb = lnkNode->data;

			if(line == NULL){
				lineLen = 0;
			}
			else {
				lineLen = strlen(line);
			}

			if((lineLen > argEnumCb->enumNameLen) || (argEnumCb->enbFlg == CLI_FALSE)){
				LNKLST_DBL_GET_NEXT_NODE(lnkNode);
				continue;
			}

			if(strncasecmp(argEnumCb->enumName, line, lineLen) == 0){
				data = malloc(argEnumCb->enumNameLen + 1);
				strncpy(data, argEnumCb->enumName, argEnumCb->enumNameLen);
				data[argEnumCb->enumNameLen] = '\0';
				LNKLST_DBL_GET_NEXT_NODE(lnkNode);

				return data;
			}

			LNKLST_DBL_GET_NEXT_NODE(lnkNode);
		}/* end of while(1) */
	}/* if(preCmdRslt->argCb != NULL) */
	else if(preCmdRslt->pkgCb != NULL){
		pkgCb = preCmdRslt->pkgCb;

		if(listCnt == 0){
			LNKLST_DBL_GET_FIRST(&pkgCb->cmdCbLL, lnkNode);
		}

		while(1){
			if(lnkNode == NULL){
				return NULL;
			}
			cmdCb = lnkNode->data;

			if(line == NULL){
				lineLen = 0;
			}
			else {
				lineLen = strlen(line);
			}

			if((lineLen > cmdCb->cmdNameLen) || (cmdCb->enbFlg == CLI_FALSE)){
				LNKLST_DBL_GET_NEXT_NODE(lnkNode);
				continue;
			}

			if(strncasecmp(cmdCb->cmdName, line, lineLen) == 0){
				data = malloc(cmdCb->cmdNameLen + 1);
				strncpy(data,cmdCb->cmdName, cmdCb->cmdNameLen);
				data[cmdCb->cmdNameLen] = '\0';
				LNKLST_DBL_GET_NEXT_NODE(lnkNode);

				if(cmdCb->argCbLL.nodeCnt == 0){
					rl_completion_append_character = ';';
				}

				return data;
			}

			LNKLST_DBL_GET_NEXT_NODE(lnkNode);
		}/* end of while(1) */
	}/* else if(preCmdRslt->pkgCb != NULL) */
	else if(preCmdRslt->pkgCb == NULL){
		cliCb = rlCfg.preCmdRslt.cliCb;

		if(listCnt == 0){
			LNKLST_DBL_GET_FIRST(&cliCb->pkgCbLL, lnkNode);
		}

		while(1){
			if(lnkNode == NULL){
				return NULL;
			}
			pkgCb = lnkNode->data;

			if(line == NULL){
				lineLen = 0;
			}
			else {
				lineLen = strlen(line);
			}

			if((lineLen > pkgCb->pkgNameLen) || (pkgCb->enbFlg == CLI_FALSE)){
				LNKLST_DBL_GET_NEXT_NODE(lnkNode);
				continue;
			}

			if(strncasecmp(pkgCb->pkgName, line, lineLen) == 0){
				data = malloc(pkgCb->pkgNameLen + 1);
				strncpy(data,pkgCb->pkgName, pkgCb->pkgNameLen);
				data[pkgCb->pkgNameLen] = '\0';
				LNKLST_DBL_GET_NEXT_NODE(lnkNode);

				if(cliCb->pkgCbLL.nodeCnt != 0){
					rl_completion_append_character = '.';
				}

				return data;
			}

			LNKLST_DBL_GET_NEXT_NODE(lnkNode);
		}/* end of while(1) */
	}
	else {
		return NULL;
	}

	return data;
}

char **rl_tapWord(const char *line, unsigned int st, unsigned int end)
{
	int ret = 0;
	char *stTkn = NULL;
	unsigned int bufLen = 0;
	CliCb *cliCb = NULL;
	CliCmdCb *cmdCb = NULL;
	char **matches = NULL;

	/* init word */
	cliCb = rlCfg.preCmdRslt.cliCb;

	bufLen = strlen(rl_line_buffer);
	ret = cli_parseGetPreCmd(cliCb, rl_line_buffer, bufLen, &rlCfg.preCmdRslt);
	if(ret != CLI_OK){
		goto goto_RetWhite;
	}

	cmdCb = rlCfg.preCmdRslt.cmdCb;

	if((cmdCb != NULL) && 
			(cmdCb->argCbLL.nodeCnt == rlCfg.preCmdRslt.argCnt)){
		rl_completion_append_character = ';';
	}
	else {
		rl_completion_append_character = ':';
	}

	stTkn = rlCfg.preCmdRslt.tkn;

	/* fine arg count */
	matches = rl_completion_matches(stTkn, rl_tapGen);
	if(matches == NULL){
		goto goto_RetWhite;
	}

	return matches;

goto_RetWhite:
	matches = rl_completion_matches(NULL, rl_tapWhite);
	rl_completion_append_character = '\0';

	return matches;
}

int cli_rlChkCmd(char *cmd, unsigned int cmdLen)
{
	unsigned int i = 0;
	unsigned int cmdNameLen = 0;
	unsigned int shortcutLen = 0;

	for(i=0; baseCmd[i].cmdName ; i++){
		cmdNameLen = strlen(baseCmd[i].cmdName);
		shortcutLen = strlen(baseCmd[i].shortcut);

		if(((cmdLen == cmdNameLen)  &&
					(strncasecmp(cmd,baseCmd[i].cmdName, cmdLen) == 0)) ||
				((cmdLen == shortcutLen) &&
				 (strncasecmp(cmd, baseCmd[i].shortcut, cmdLen) == 0))){
			return baseCmd[i].cmdId;
		}
	}/* end of for(i=0; baseCmd[i].cmdName ; i++) */

	return CLI_BASE_CMD_NONE;
}

int cli_rlHistDisp()
{
    unsigned int i = 0;
    HIST_ENTRY **histLst;

    histLst  = history_list();
    if(histLst != NULL){
        for(i=0;histLst[i] != NULL;i++){
            CLI_DISP("%-5d: %s\n",i + history_base, histLst[i]->line);
        }
    }

    return CLI_OK;
}

int cli_rlListDisp(CliCb *cliCb)
{
    int ret = 0;

    ret = cli_mainDispCmdLst(cliCb);
    if(ret != CLI_OK){
        CLI_LOG(CLI_ERR,"command Display failed\n");
        return CLI_NOK;
    }
    return CLI_OK;
}

int rl_help()
{
	char cmd[256];

	CLI_DISP("BASE COMMAND LIST\n");
	CLI_DISP("COMMAND           DESC\n");
	CLI_DISP("---------------------------------------------\n");
	snprintf(cmd,256,"%s(%s)","list", "l");
	CLI_DISP("%-15s : %s\n",cmd,  "Display command list");
	snprintf(cmd,256,"%s(%s)","history", "his");
	CLI_DISP("%-15s : %s\n",cmd, "Display command history");
	snprintf(cmd,256,"%s(%s)","help", "h");
	CLI_DISP("%-15s : %s\n",cmd, "Display basic command list");
	snprintf(cmd,256,"%s(%s)","quit", "q");
	CLI_DISP("%-15s : %s\n",cmd,  "Quit CLI");

	return CLI_OK;
}

int cli_rlBaseCmd(CliCb *cliCb, char *line, unsigned int lineLen)
{
    int ret = 0;
    unsigned int cmdCode = 0;
    char *stTkn = NULL;
    unsigned int tknLen = 0;

    tknLen = cli_parseGetToken(line, lineLen, &stTkn);
    if(tknLen == 0){
        return CLI_RSLT_CONT;
    }

    cmdCode = cli_rlChkCmd(stTkn, tknLen);
    switch(cmdCode){
        case CLI_BASE_CMD_HISTORY:
            {
                ret = cli_rlHistDisp();
            }
            break;
        case CLI_BASE_CMD_HELP:
            {
                ret = rl_help();
            }
            break;
        case CLI_BASE_CMD_LIST:
            {
                ret = cli_rlListDisp(cliCb);
                if(ret != CLI_OK){
                }
            }
            break;
		case CLI_BASE_CMD_QUIT:
		{
			return CLI_RSLT_QUIT;
		}
		break;

        default:
            return CLI_RSLT_CONT;
    }/* end of switch(cmdCode) */

    return CLI_OK;
}

int rl_prntCmdHelp(CliCb *cliCb)
{
	int ret = 0;
	unsigned int lineLen = 0;
	CliCmdCb *cmdCb = NULL;
	CliPreCmdRslt preCmdRslt;

	unsigned int i = 0;
	unsigned int nodeCnt = 0;
	CliArgCb *argCb = NULL;
	LnkLstDblNode *lnkNode = NULL;

	lineLen = strlen(rl_line_buffer);

	ret = cli_parseGetPreCmd(cliCb, rl_line_buffer, lineLen, &preCmdRslt);
	if(ret != CLI_OK){
		return ret;
	}

	cmdCb = preCmdRslt.cmdCb;
	if(cmdCb != NULL){
		CLI_DISP("Command : %d(%.*s)\n", cmdCb->cmdCode, cmdCb->cmdNameLen, cmdCb->cmdName); 
		CLI_DISP("Decription : %.*s\n", cmdCb->descLen, cmdCb->desc);
		CLI_DISP("Argument :\n");

		nodeCnt = cmdCb->argCbLL.nodeCnt;
		if(nodeCnt == 0){
			return CLI_OK;
		}

		LNKLST_DBL_GET_FIRST(&cmdCb->argCbLL, lnkNode);
		for(i=0;i<nodeCnt;i++){

			argCb = (CliArgCb*)lnkNode->data;
			cli_argPrntHelpDisp(argCb);

			LNKLST_DBL_GET_NEXT_NODE(lnkNode);
		}/* end of for(i=0;i<nodeCnt;i++) */
	}

	return CLI_OK;
}

int cli_rlRead(CliCb *cliCb, CliDoc **rt_doc)
{
	int ret = 0;
	unsigned int lineLen = 0;
	unsigned int stLineLen = 0;
	char *line = NULL;
	char *stLine = NULL;
	char *exp = NULL;

	if(initFlg == CLI_FALSE){
		CLI_LOG(CLI_ERR,"Init first\n");
		return CLI_ERR_RL_INIT_FIRST;
	}

	rlCfg.preCmdRslt.cliCb = cliCb;

	while(1){
		line = readline(rlCfg.prmt);
		if(line == NULL){
			return CLI_ERR_NOT_EXIST;
		}

		lineLen = strlen(line);
		if(lineLen == 0){
			return CLI_ERR_NOT_EXIST;
		}

		stLine  = cli_parseSkipSpace(line, lineLen);

		if(stLine == (line + lineLen)){
			return CLI_ERR_NOT_EXIST;
		}

		/* check expend */
		ret = history_expand(stLine, &exp);
		if(ret != 0){
			stLine = exp;
		}
		else if((ret < 0) || (ret == 2)){
			CLI_LOG(CLI_ERR,"expand error(ret=%d)\n",ret);
			return CLI_ERR_NOT_EXIST;
		}

		stLineLen = strlen(stLine);
		add_history(stLine);

		ret = cli_rlBaseCmd(cliCb, stLine , stLineLen);
		if(ret == CLI_OK){
			return CLI_ERR_NOT_EXIST;
		}
		else if(ret == CLI_RSLT_QUIT){
			return CLI_RSLT_QUIT;
		}

		/* check last key */
		if(stLine[stLineLen-1] == '?'){
			ret = rl_prntCmdHelp(cliCb);
			if(ret != CLI_OK){
				CLI_LOG(CLI_ERR,"Command help print failed(ret=%d)\n",ret);
			}
			continue;
		}
		break;
	}/* end of while(1) */

	ret = cli_parseCmd(cliCb, stLine, stLineLen , rt_doc);
	if(ret != CLI_OK){
		CLI_LOG(CLI_ERR,"Command parsing failed(ret=%d)\n",ret);
		free(line);
		return ret;
	}

	free(line);

	return CLI_OK;
}

