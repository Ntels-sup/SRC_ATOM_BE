#include <stdio.h>
#include <string.h>

#include "lnkLst.h"
#include "cli.h"

int MakeCLI(CliCb *cliCb)
{
		CliPkgCb *pkgCb = NULL;
		CliCmdCb *cmdCb = NULL;
		int ret = 0;

		cli_mainInit(cliCb);

#if 0
		ret = cli_pkgAdd(cliCb, "OFCS2", strlen("OFCS2")); 
		if(ret != CLI_OK){
				fprintf(stderr,"Package add failed(ret=%d)\n",ret);
				return 0;
		}

		CliCmdCfg cmdCfg = { 1,
				strlen("DIS-DIAM-PEER"),
				"DIS-DIAM-PEER",
				strlen("DESCRIPTION"),
				"DESCRIPTION",
				strlen("HELP"),
				"HELP"
		};

		ret = cli_pkgFind(cliCb, "OFCS2", strlen("OFCS2"), &pkgCb);
		if(ret != CLI_OK){
				fprintf(stderr,"Can not find package(ret=%d)\n", ret);
				return 0;
		}

		ret = cli_cmdAdd(pkgCb, &cmdCfg);
		if(ret != CLI_OK){
				fprintf(stderr,"Command add faield(ret=%d)\n",ret);
				return 0;
		}

		ret = cli_cmdFind(pkgCb, "DIS-DIAM-PEER", strlen("DIS-DIAM-PEER"), &cmdCb);
		if(ret != CLI_OK){
				fprintf(stderr,"Can not find cmd\n");
				return 0;
		}

		pkgCb = cli_pkgGetDfltPkg(cliCb);

		ret = cli_cmdAdd(pkgCb, &cmdCfg);
		if(ret != CLI_OK){
				fprintf(stderr,"Command add faield(ret=%d)\n",ret);
				return 0;
		}

		CliArgCfg argCfg = {
				strlen("ABC"),
				"ABC",
				strlen("ABC_VAL"),
				"ABC_VAL",
				CLI_ARG_TYPE_STRING,
				CLI_FALSE,
				CLI_TRUE,
				2,
				0,
				10
		};

		ret = cli_argAdd(cmdCb, &argCfg, NULL);
		if(ret != CLI_OK){
				fprintf(stderr,"ARG add failed(ret=%d)\n",ret);
				return 0;
		}

		CliArgCfg argCfg2 = {
				strlen("DEF"),
				"DEF",
				0,
				NULL,
				CLI_ARG_TYPE_STRING,
				CLI_FALSE,
				CLI_FALSE,
				0,
				0,
				10
		};

		ret = cli_argAdd(cmdCb, &argCfg2, NULL);
		if(ret != CLI_OK){
				fprintf(stderr,"ARG add failed(ret=%d)\n",ret);
				return 0;
		}

		CliArgCfg argCfg3 = {
				strlen("ENUM_TEST"),
				"ENUM_TEST",
				0,
				NULL,
				CLI_ARG_TYPE_ENUM,
				CLI_TRUE,
				CLI_FALSE,
				0,
				0,
				10
		};
	
		CliArgCb *enumArg = NULL;

		ret = cli_argAdd(cmdCb, &argCfg3, &enumArg);
		if(ret != CLI_OK){
				fprintf(stderr,"ARG add failed(ret=%d)\n",ret);
				return 0;
		}

		CliArgEnumCfg enumCfg = {
				strlen("TEST_ENUM"),
				"TEST_ENUM",
				CLI_TRUE,
				CLI_ARG_TYPE_NUMBER,
				.u.num.enumVal = 12,
		};

		ret = cli_argEnumAdd(enumArg, &enumCfg);
		if(ret != CLI_OK){
				fprintf(stderr,"enum add failed(ret=%d)\n",ret);
				return 0;
		}

		CliArgEnumCfg enumCfg2 = {
				strlen("TEST_ENUM2"),
				"TEST_ENUM2",
				CLI_TRUE,
				CLI_ARG_TYPE_STRING,
				.u.str.enumValLen = strlen("CHANGE"),
				.u.str.enumVal = "CHANGE"
		};

		ret = cli_argEnumAdd(enumArg, &enumCfg2);
		if(ret != CLI_OK){
				fprintf(stderr,"enum add failed(ret=%d)\n",ret);
				return 0;
		}
#endif

		return 1;
}

int Parse(CliCb *cliCb)
{
		int ret = 0;
		FILE *fp = NULL;
		char cmdLine[128];
		int cmdLineLen = 0;
		CliDoc *doc = NULL;

		fp = fopen("testCmd", "r");


		while(1){
				if(fgets(cmdLine, 128, fp) == NULL){
						break;
				}

				cmdLineLen = strlen(cmdLine);

				if(cmdLine[cmdLineLen-1] == '\n') { 
						cmdLine[cmdLineLen-1] = '\0';
						cmdLineLen--;
				}

				fprintf(stderr,"====INPUT CMD=%-35s====\n",cmdLine);
				ret = cli_parseCmd(cliCb, cmdLine , cmdLineLen, &doc);
				if(ret != CLI_OK){
						fprintf(stderr,"parsing failed(ret=%d)\n",ret);
						continue;
				}
				fprintf(stderr,"=====================================================\n");
				fprintf(stderr,"=====================================================\n");
				cli_docDebugPrint(doc);

				cli_docDstry(doc);


				fprintf(stderr,"\n");
		}

		while(1){
				ret = cli_lnRead(cliCb, &doc);
                if(ret != CLI_OK){
                    if(ret == CLI_ERR_NOT_EXIST){
                        continue;
                    }
                    fprintf(stderr,"Parsing error(ret=%d)\n", ret);
                    continue;
                }

				cli_docDebugPrint(doc);

				cli_docDstry(doc);
		}


		fclose(fp);

		return 1;
}

int main()
{
		CliCb cliCb;


		MakeCLI(&cliCb);

		cli_lnInit("TEST>", 5);

		Parse(&cliCb);

		cli_mainDstry(&cliCb);

		return 0;
}
