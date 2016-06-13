#ifndef __CLI_H__
#define __CLI_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CLI_ARG_NAME_LEN                 32
#define CLI_PKG_NAME_LEN                 64
#define CLI_CMD_NAME_LEN                 64
#define CLI_CMD_DESC_LEN                 128
#define CLI_CMD_HELP_LEN                 2096
#define CLI_ARG_VAL_LEN                  128
#define CLI_ARG_DESC_LEN                 128
#define CLI_ENUM_NAME_LEN                32
#define CLI_ENUM_DESC_LEN                128
#define CLI_ENUM_VALUE_LEN               32
#define CLI_ARG_NAME_LEN                 32

#define CLI_ARG_TYPE_NONE                0
#define CLI_ARG_TYPE_STRING              1
#define CLI_ARG_TYPE_NUMBER              2
#define CLI_ARG_TYPE_ENUM                3

/* result code */
#define CLI_NOK                          0
#define CLI_OK                           1

#define CLI_FALSE                        0
#define CLI_TRUE                         1

#define CLI_RSLT_TERM                    100
#define CLI_RSLT_CONT                    101 /* continue */
#define CLI_RSLT_QUIT                    102

#define CLI_ERR_PARSING_FAILED           200
#define CLI_ERR_NOT_EXIST                201
#define CLI_ERR_END_OF_LINE              202
#define CLI_ERR_PACKAGE_ALREADY_EXIST    203
#define CLI_ERR_INVALID_SEPERATOR        204
#define CLI_ERR_SEPERATOR_NOT_EXIST      205
#define CLI_ERR_TOKEN_NOT_EXIST          206
#define CLI_ERR_CLICB_IS_NULL            207
#define CLI_ERR_LNKLST_INIT_FAILED       208
#define CLI_ERR_INVALID_PKG_NAME_LEN     209
#define CLI_ERR_PKG_ALREADY_EXIST        210
#define CLI_ERR_PKG_MAKE_FAILED          211
#define CLI_ERR_PKGCB_IS_NULL            212
#define CLI_ERR_CMD_CONFIG_IS_NULL       213
#define CLI_ERR_CMD_ALREADY_EXIST        214
#define CLI_ERR_CMD_NAME_NOT_EXIST       215
#define CLI_ERR_PKG_NOT_EXIST            216
#define CLI_ERR_CMD_NOT_EXIST            217
#define CLI_ERR_DOC_INIT_FAILED          218
#define CLI_ERR_DOC_ALREADY_EXIST        219
#define CLI_ERR_DOC_PKG_SET_FAIELD       220
#define CLI_ERR_DOC_CMD_SET_FAIELD       221
#define CLI_ERR_CMDCB_IS_NULL            222
#define CLI_ERR_ARG_CONFIG_IS_NULL       223
#define CLI_ERR_ARG_NAME_NOT_EXIST       224
#define CLI_ERR_ARG_INVALID_TYPE         225
#define CLI_ERR_ARG_INVLAID_RANGE        226
#define CLI_ERR_LNKLST_INSERT_FAILED     227
#define CLI_ERR_ARG_ALREADY_EXIST        228
#define CLI_ERR_FIX_ARG_FLAG_SET         229
#define CLI_ERR_DOC_ARG_ADD_FAIELD       230
#define CLI_ERR_ARG_VAL_NOT_EXIST        231
#define CLI_ERR_ARG_VAL_INVALID_LEN      232
#define CLI_ERR_ARG_NAME_INVALID_LEN     233
#define CLI_ERR_MULTI_FLAG_NOT_SET       234
#define CLI_ERR_TOO_MANY_ARG_PARA        235
#define CLI_ERR_DOC_ARG_PARA_ADD_FAILED  236
#define CLI_ERR_ENUM_CONFIG_IS_NULL      237
#define CLI_ERR_ENUM_NAME_NOT_EXIST      238
#define CLI_ERR_ENUM_VAL_NOT_EXIST       239
#define CLI_ERR_ARGCB_IS_NULL            240
#define CLI_ERR_ENUM_INVALID_NAME_LEN    241
#define CLI_ERR_ENUM_INVALID_VAL_LEN     242
#define CLI_ERR_ARGENUMCB_IS_NULL        243
#define CLI_ERR_ENUM_ALREADY_EXIST       244
#define CLI_ERR_ARG_MAND_TYPE            245
#define CLI_ERR_ARG_INVALID_ARG_NAME_LEN 246
#define CLI_ERR_ARG_INVALID_OPT_FLAG     247
#define CLI_ERR_RL_INIT_FIRST            248
#define CLI_ERR_RL_INVALID_PRMT_LEN      249
#define CLI_ERR_TOO_FEW_ARG              250
#define CLI_ERR_INVALID_CMD_NAME_LEN     251
#define CLI_ERR_INVALID_DESC_LEN         252
#define CLI_ERR_DOC_DESC_SET_FAIELD      253
#define CLI_ERR_TOKEN_END                254
#define CLI_ERR_INIT_FAILED              255
#define CLI_ERR_INVALID_NUM_VALUE        256
#define CLI_ERR_ARG_DFLT_VAL_NOT_SUPP    227

#define CLI_NONE 0
#define CLI_ERR  1


#define CLI_CMD_CFG_INIT(_cmdCfg){\
    (_cmdCfg)->cmdCode = 0;\
    (_cmdCfg)->cmdNameLen = 0;\
    (_cmdCfg)->cmdName = NULL;\
    (_cmdCfg)->descLen = 0;\
    (_cmdCfg)->desc = NULL;\
    (_cmdCfg)->helpLen = 0;\
    (_cmdCfg)->help = NULL;\
	(_cmdCfg)->usrVal = NULL;\
	(_cmdCfg)->enbFlg = CLI_TRUE;\
}

#define CLI_ARG_CFG_INIT(_argCfg){\
	(_argCfg)->argNameLen = 0;\
    (_argCfg)->argName = NULL;\
	(_argCfg)->dfltArgValFlg = CLI_FALSE;\
	(_argCfg)->usrVal = NULL;\
    (_argCfg)->descLen = 0;\
    (_argCfg)->desc = NULL;\
    (_argCfg)->argType = CLI_ARG_TYPE_NONE;\
    (_argCfg)->optFlg = CLI_FALSE;\
    (_argCfg)->multiFlg = CLI_FALSE;\
    (_argCfg)->multiMaxCnt = 0;\
    (_argCfg)->rngMin = 0;\
    (_argCfg)->rngMax = 0;\
}

#define CLI_ARG_ENUM_CFG_INIT(_argEnumCfg){\
    (_argEnumCfg)->enumNameLen = 0;\
    (_argEnumCfg)->enumName = NULL;\
	(_argEnumCfg)->enbFlg = CLI_FALSE;\
    (_argEnumCfg)->descLen = 0;\
    (_argEnumCfg)->desc = NULL;\
	(_argEnumCfg)->usrVal = NULL;\
    (_argEnumCfg)->enumValFlg = CLI_FALSE;\
    (_argEnumCfg)->enumValType = CLI_ARG_TYPE_NONE;\
}

#define CLI_ARG_GET_FIRST_ARG_PARA(_argDoc, _rt_argParaDoc){\
	LnkLstDblNode *d_lnkNode = NULL;\
	LNKLST_DBL_GET_FIRST(&(_argDoc)->argParaLL, d_lnkNode);\
	if(d_lnkNode == NULL){\
		(_rt_argParaDoc) = NULL;\
	}\
	else {\
		(_rt_argParaDoc) = (CliArgParaDoc*)d_lnkNode->data;\
	}\
}

#define CLI_ARG_GET_NEXT_ARG_PARA(_argParaDoc){\
	LnkLstDblNode *d_lnkNode = NULL;\
	d_lnkNode = &(_argParaDoc)->lnkNode;\
	LNKLST_DBL_GET_NEXT_NODE(d_lnkNode);\
	(_argParaDoc) = (CliArgParaDoc*)d_lnkNode->data;\
}

typedef void (*CliLogFunc) (unsigned int, char *, unsigned int, char*);
typedef void (*CliDispFunc) (char*);
typedef void (*CliParseLogFunc) (char*);

typedef struct CliArgParaDoc    CliArgParaDoc;
typedef struct CliArgDoc        CliArgDoc;
typedef struct CliDoc           CliDoc;
typedef struct CliCmdCfg        CliCmdCfg;
typedef struct CliArgCfg        CliArgCfg;
typedef struct CliArgEnumCfg    CliArgEnumCfg;
typedef struct CliArgEnumCb     CliArgEnumCb;
typedef struct CliArgCb         CliArgCb;
typedef struct CliCmdCb         CliCmdCb;
typedef struct CliPkgCb         CliPkgCb;
typedef struct CliCb            CliCb;

#define GET_ARG_PARA_NUM(_argParaDoc, _rt_val, _rt_ret){\
    if((_argParaDoc)->valType == CLI_ARG_TYPE_NUMBER){\
        (_rt_val) = (_argParaDoc)->u.num.val;\
        (_rt_ret) = CLI_OK;\
    }\
    else {\
        (_rt_ret) = CLI_NOK;\
    }\
}

#define GET_ARG_PARA_STR(_argParaDoc, _rt_val, _rt_valLen, _rt_ret){\
    if((_argParaDoc)->valType == CLI_ARG_TYPE_STRING){\
        (_rt_val) = (_argParaDoc)->u.str.val;\
        (_rt_valLen) = (_argParaDoc)->u.str.valLen;\
        (_rt_ret) = CLI_OK;\
    }\
    else {\
        (_rt_ret) = CLI_NOK;\
    }\
}


struct CliArgParaDoc{
	unsigned int         valType;
	union {
		struct {
			unsigned int val;
		} num;
		struct {
			unsigned int valLen;
			char         val[CLI_ARG_VAL_LEN];
		} str;
	} u;
	LnkLstDblNode        lnkNode;
};

struct CliArgDoc{
    unsigned int         argNameLen;
    char                 argName[CLI_ARG_NAME_LEN];
    LnkLstDbl            argParaLL;
    LnkLstDblNode        lnkNode;
};

struct CliDoc{
	unsigned int         cmdLineLen;
	char                 *cmdLine;
    unsigned int         pkgNameLen;
    char                 pkgName[CLI_PKG_NAME_LEN];
	unsigned int         cmdCode;
    unsigned int         cmdNameLen;
    char                 cmdName[CLI_CMD_NAME_LEN];
    unsigned int         cmdDescLen;
    char                 cmdDesc[CLI_CMD_DESC_LEN];
    unsigned int         fixArgFlg;
    unsigned int         argMandCnt;
    unsigned int         nxtSeq;
    LnkLstDbl            argDocLL;
};

struct CliCmdCfg{
    unsigned int         cmdCode;
    unsigned int         cmdNameLen;
    const char           *cmdName;
    unsigned int         descLen;
    const char           *desc;
    unsigned int         helpLen;
    const char           *help;
	void                 *usrVal;
	unsigned int         enbFlg; /* enable flag */
};

struct CliArgCfg{
    unsigned int         argNameLen;
    const char           *argName;
	unsigned int         dfltArgValFlg;
	union {
		struct {
			unsigned int dfltArgVal;
		}num;
		struct {
			unsigned int dfltArgValLen;
			const char   *dfltArgVal;
		}str;
	} u;
	void                 *usrVal;
    unsigned int         descLen;
    const char           *desc;
    unsigned int         argType;
    unsigned int         optFlg;
    unsigned int         multiFlg;
    unsigned int         multiMaxCnt;
    unsigned int         rngMin;	
    unsigned int         rngMax;	
};

struct CliArgEnumCfg{
    unsigned int         enumNameLen;
    const char           *enumName;
	unsigned int         enbFlg; /* enable flag */
    unsigned int         descLen;
    char                 *desc;
	void                 *usrVal;
    unsigned int         enumValFlg;
    unsigned int         enumValType;
    union {
        struct {
            unsigned int enumVal;
        } num;
        struct {
            unsigned int enumValLen;
            char         *enumVal;
        } str;
    } u;
};

struct CliArgEnumCb{
    unsigned int         enumNameLen;
    char                 enumName[CLI_ENUM_NAME_LEN];
    unsigned int         descLen;
    char                 *desc;
	unsigned int         enbFlg; /* enable flag */
    unsigned int         enumValFlg;
    unsigned int         enumValType;
	void                 *usrVal;
    union {
        struct {
            unsigned int enumVal;
        } num;
        struct {
            unsigned int enumValLen;
            char         enumVal[CLI_ENUM_VALUE_LEN];
        } str;
    } u;
    LnkLstDblNode        lnkNode;
};

struct CliArgCb{
    unsigned int         argNameLen;
    char                 argName[CLI_ARG_NAME_LEN];
    unsigned int         descLen;
    char                 *desc;
	unsigned int         dfltArgValFlg;
	union {
		struct {
			unsigned int dfltArgValLen;
			char         dfltArgVal[CLI_ARG_VAL_LEN];
		} str;
		struct {
			unsigned int dfltArgVal;
		} num;
	} u;
	void                 *usrVal;
    int                  argType;
    int                  optFlg;
    int                  multiFlg;
    unsigned int         multiMaxCnt;
    unsigned int         rngMin;	
    unsigned int         rngMax;	
    LnkLstDbl            argEnumCbLL;
    LnkLstDblNode        lnkNode;
};

struct CliCmdCb{
    unsigned int         cmdCode;
    LnkLstDblNode        lnkNode;
	unsigned int         enbFlg; /* enable flag */
	void                 *usrVal;
    unsigned int         argMandCnt;
    LnkLstDbl            argCbLL;
    unsigned int         cmdNameLen;
    char                 cmdName[CLI_CMD_NAME_LEN];
    unsigned int         descLen;
    char                 desc[CLI_CMD_DESC_LEN];
    unsigned int         helpLen;
    char                 help[CLI_CMD_HELP_LEN];
};

struct CliPkgCb{
    unsigned int         pkgNameLen;
    char                 pkgName[CLI_PKG_NAME_LEN];
	void                 *usrVal;
	unsigned int         enbFlg; /* enable flag */
    LnkLstDbl            cmdCbLL;
    LnkLstDblNode        lnkNode;
};

struct CliCb{
    CliPkgCb             *dfltPkgCb;
    LnkLstDbl            pkgCbLL;
};

/* cli_main.c */
int             cli_mainInit                (CliCb *cliCb);
int             cli_mainDstry               (CliCb *cliCb, unsigned int usrValFreeFlg);
int             cli_mainDispCmdLst          (CliCb *cliCb);

/* cli_glob.c */
CliDispFunc     cli_globGetDispFunc         ();
CliParseLogFunc cli_globGetParseLogFunc     ();
int             cli_globSetLogFunc          (unsigned int lvl, CliLogFunc logFunc);
int             cli_globSetDispFunc         (CliDispFunc logFunc);
int             cli_globSetParseLogFunc     (CliParseLogFunc logFunc);
int             cli_globGetLogLvl           ();

/* cld_doc.c */
int             cli_docInit                 (CliDoc *doc);
int             cli_docArgParaDstry         (CliArgParaDoc *argParaDoc);
int             cli_docArgDstry             (CliArgDoc *argDoc);
int             cli_docDstry                (CliDoc *doc);
int             cli_docSetPkg               (CliDoc *doc, char *pkgName, unsigned int pkgNameLen);
int             cli_docSetCmd               (CliDoc *doc, unsigned int cmdCode, char *cmdName, unsigned int cmdNameLen);
int             cli_docSetCmdDesc           (CliDoc *doc, char *desc, unsigned int descLen);
int             cli_docFindArg              (CliDoc *doc, const char *argName, unsigned int argNameLen, CliArgDoc **rt_argDoc);
int             cli_docArgParaIsStr         (CliArgParaDoc *argParaDoc);
int             cli_docArgParaIsNum         (CliArgParaDoc *argParaDoc);
int             cli_docArgParaGetNumVal     (CliArgParaDoc *argParaDoc, unsigned int *rt_val);
int             cli_docArgParaGetStrVal     (CliArgParaDoc *argParaDoc, char **rt_val, unsigned int *rt_valLen);
int             cli_docArgParaNumCmp        (CliArgParaDoc *argParaDoc, unsigned int val);
int             cli_docArgParaStrCmp        (CliArgParaDoc *argParaDoc, const char *val, unsigned int valLen);
int             cli_docArgParaAdd           (CliArgCb *argCb, CliArgDoc *argDoc, const char *val, unsigned int vallen);
int             cli_docArgAdd               (CliDoc *doc, char *argName, unsigned int argNameLen, CliArgDoc **rt_argDoc);
int             cli_docDebugArgParaPrint    (CliArgParaDoc *argParaDoc);
int             cli_docDebugArgPrint        (CliArgDoc *argDoc);
int             cli_docDebugPrint           (CliDoc *doc);

/* cli_arg.c */
int             cli_argEnumFind             (CliArgCb *argCb, const char *enumName, unsigned int enumNameLen, 
		CliArgEnumCb **rt_argEnumCb);
int             cli_argEnumEnable           (CliArgEnumCb *argEnumCb);
int             cli_argEnumDisable          (CliArgEnumCb *argEnumCb);
int             cli_argEnumAdd              (CliArgCb *argCb, CliArgEnumCfg *enumCfg, CliArgEnumCb **rt_argEnumCb);
int             cli_argEnumDstry            (CliArgEnumCb *argEnumCb, unsigned int usrValFreeFlg);
int             cli_argFindBySeq            (CliCmdCb *cmdCb, unsigned int seq, CliArgCb **rt_argCb);
int             cli_argFind                 (CliCmdCb *cmdCb, const char *argName, unsigned int argNameLen, 
		CliArgCb **rt_argCb);
int             cli_argAdd                  (CliCmdCb *cmdCb, CliArgCfg *argCfg, CliArgCb **rt_argCb);
int             cli_argDstry                (CliArgCb *argCb, unsigned int usrValFreeFlg);
int             cli_argChkRng               (CliArgCb *argCb, unsigned int tknLen);
int             cli_argGetDispStr           (CliArgCb *argCb, char *rt_line, unsigned int maxLineLen, unsigned int *rt_lineLen);
int             cli_argPrntHelpDisp         (CliArgCb *argCb);

/* cli_cmd.c */
int             cli_cmdFind                 (CliPkgCb *pkgCb, const char *cmdName, unsigned int cmdNameLen, 
		CliCmdCb **rt_cmdCb);
int             cli_cmdAdd                  (CliPkgCb *pkgCb, CliCmdCfg *cmdCfg, CliCmdCb **rt_cmdCb);
int             cli_cmdEnable               (CliCmdCb *cmdCb);
int             cli_cmdDisable              (CliCmdCb *cmdCb);
int             cli_cmdDstry                (CliCmdCb *cmdCb, unsigned int usrValFreeFlg);
int             cli_cmdDispCmdDesc          (CliCmdCb *cmdCb);

/* cli_pkg.c */
int             cli_pkgAdd                  (CliCb *cliCb, unsigned int dfltFlg, const char *pkgName, unsigned int pkgNameLen, 
		void *usrVal, CliPkgCb **rt_pkgCb);
int             cli_pkgDstry                (CliPkgCb *pkgCb, unsigned int usrValFreeFlg);
CliPkgCb*       cli_pkgGetDfltPkg           (CliCb *cliCb);
int             cli_pkgFind                 (CliCb *cliCb, const char *pkgName, unsigned int pkgNameLen, CliPkgCb **rt_pkgCb);
int             cli_pkgSetEnable            (CliPkgCb *pkgCb);
int             cli_pkgSetDisable           (CliPkgCb *pkgCb);
int             cli_pkgSetDflt              (CliCb *cliCb, CliPkgCb *pkgCb);
int             cli_pkgSetDfltByName        (CliCb *cliCb, const char *pkgName, unsigned int pkgNameLen);
int             cli_pkgDispCmdLst           (CliPkgCb *pkgCb);

/* cli_parse.c */
int             cli_parseCmd                (CliCb *cliCb, char *cliLine, unsigned int cliLineLen, CliDoc **rt_doc);

/* cli_rl.c */
int             cli_rlSetPrmt               (const char *prmt, unsigned int prmtLen);
int             cli_rlRead                  (CliCb *cliCb, CliDoc **rt_doc);

#ifdef __cplusplus
}
#endif

#endif
