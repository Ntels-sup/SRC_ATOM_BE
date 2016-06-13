#include <unistd.h>
#include <stdlib.h>

#include "lnkLst.h"
#include "cli.h"
#include "cliInt.h"

int cli_mainInit(CliCb *cliCb)
{
	int ret = 0;

	if(cliCb == NULL){
		return CLI_ERR_CLICB_IS_NULL;
	}

	if(cli_globGetInitFlg() == CLI_FALSE){
		ret = cli_globInit();
		if(ret != CLI_OK){
			return CLI_ERR_INIT_FAILED;
		}
	}

	cliCb->dfltPkgCb = NULL;

	ret = lnkLst_dblInit(&cliCb->pkgCbLL, ~0);
	if(ret != LNKLST_OK){
		CLI_LOG(CLI_ERR,"Linked list init failed(ret=%d)\n",ret);
		return CLI_ERR_LNKLST_INIT_FAILED;
	}

	ret = cli_rlGetInitFlg();
	if(ret == CLI_FALSE){
		cli_rlInit(">", 1);
	}

	return CLI_OK;
}

int cli_mainDstry(CliCb *cliCb, unsigned int usrValFreeFlg)
{
    int ret = 0;
    CliPkgCb *pkgCb = NULL;
    LnkLstDblNode *lnkNode = NULL;

    while(1){
        if(cliCb->pkgCbLL.nodeCnt == 0){
            break;
        }

        lnkNode = lnkLst_dblGetFirst(&cliCb->pkgCbLL);
        if(lnkNode == NULL) {
            break;
        }

        pkgCb = (CliPkgCb*)lnkNode->data;

        ret = cli_pkgDstry(pkgCb, usrValFreeFlg);
        if(ret != CLI_OK){
            CLI_LOG(CLI_ERR,"Package destroy faiiled(ret=%d)\n",ret);
        }

        free(pkgCb);
    }/* end of while(1) */

    return CLI_OK;
}

int cli_mainDispCmdLst(CliCb *cliCb)
{
    int ret = 0;
    int i = 0;
    int nodeCnt = 0;
    CliPkgCb *pkgCb = NULL;
    LnkLstDblNode *lnkNode = NULL;

    nodeCnt = cliCb->pkgCbLL.nodeCnt;
    if(nodeCnt == 0){
        return CLI_OK;
    }

    LNKLST_DBL_GET_FIRST(&cliCb->pkgCbLL, lnkNode);

    for(i=0;i < nodeCnt;i++){
        pkgCb = (CliPkgCb*)lnkNode->data;

        ret = cli_pkgDispCmdLst(pkgCb);
        if(ret != CLI_OK){
            CLI_LOG(CLI_ERR,"package Command list display failed(ret=%d)\n",ret);
        }

        LNKLST_DBL_GET_NEXT_NODE(lnkNode);
    }/* end of while(1) */

    return CLI_OK;
}
