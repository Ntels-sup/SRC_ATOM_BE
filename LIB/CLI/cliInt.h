#ifndef __CLI_INT_H__
#define __CLI_INT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CLI_MAX_BUF_LEN                  4096

#define ARG_DISP_LINE_LEN                1024

/* base command */
#define CLI_BASE_CMD_NONE                0
#define CLI_BASE_CMD_HISTORY             1
#define CLI_BASE_CMD_LIST                2
#define CLI_BASE_CMD_QUIT                3
#define CLI_BASE_CMD_HELP                4

/* TOKEN TYPE */
#define CLI_TKN_TYPE_NUMBER_CHAR         1
#define CLI_TKN_TYPE_LETTER_CHAR         2
#define CLI_TKN_TYPE_SPECIAL_CHAR        3
#define CLI_TKN_TYPE_COLON_CHAR          4
#define CLI_TKN_TYPE_SEMICOLON_CHAR      5
#define CLI_TKN_TYPE_EQUAL_CHAR          6
#define CLI_TKN_TYPE_PERIOD_CHAR         7
#define CLI_TKN_TYPE_SPACE_CHAR          8
#define CLI_TKN_TYPE_COMMA_CHAR          9
#define CLI_TKN_TYPE_QUST_MARK_CHAR      10 /* Question mark */

#define CLI_LOG(_lvl,...){\
	unsigned int d_logLvl = 0;\
	d_logLvl = cli_globGetLogLvl();\
	if((d_logLvl != CLI_NONE) && (d_logLvl >= (_lvl))){\
		cli_globLogPrnt((_lvl),__FILE__, __LINE__, __VA_ARGS__);\
	}\
}

#define CLI_DISP(...){\
	if(cli_globGetDispFunc() != NULL){\
		cli_globDispPrnt(__VA_ARGS__);\
	}\
}

#define CLI_PARSE_LOG(...){\
	if(cli_globGetParseLogFunc() != NULL){\
		cli_globParseLogPrnt(__VA_ARGS__);\
	}\
}

typedef struct CliPreCmdRslt    CliPreCmdRslt;
typedef struct CliGlobCb        CliGlobCb;

struct CliPreCmdRslt{
	CliCb               *cliCb;
	CliPkgCb            *pkgCb;
	CliCmdCb            *cmdCb;
	CliArgCb            *argCb;
	unsigned int        fixFlg;
	unsigned int        argCnt;
	unsigned int        tknLen;
	char                *tkn;
};

struct CliGlobCb{
	unsigned int         logLvl;
	CliLogFunc           logFunc;
	CliDispFunc          dispFunc;
	CliParseLogFunc      parseLogFunc;
	unsigned int         bufLen;
	char                 buf[CLI_MAX_BUF_LEN];
};

/* cli_glob.c */
unsigned int    cli_globGetInitFlg          ();
int             cli_globInit                ();
int             cli_globLogPrnt             (unsigned int lvl, char *file, unsigned int line, const char *fmt,...);
int             cli_globDispPrnt            (const char *fmt,...);
int             cli_globParseLogPrnt        (const char *fmt,...);

/* cli_parse.c */
int             cli_parseIsNumber           (const char *tkn, unsigned int tknLen);
int             cli_parseGetToken           (char *cliLine, unsigned int cliLineLen, char **rt_stTkn);
char*           cli_parseSkipSpace          (char *cliLine, unsigned int cliLineLen);
int             cli_parseGetPreCmd          (CliCb *cliCb, const char *cliLine, unsigned int cliLineLen, 
		CliPreCmdRslt *rt_preCmdRslt);

/* cli_rl.c */
int             cli_rlGetInitFlg            ();
int             cli_rlInit                  ();

#ifdef __cplusplus
}
#endif

#endif
