#include <unistd.h>
#include "CGlobal.hpp"
#include "CLC.h"
#include "CCliDoc.hpp"

int CCliDoc::ArgEncode(CliArgDoc *a_stArgDoc, CCliReq &a_cReq)
{
	int i = 0;
	int nodeCnt = 0;
	CliArgParaDoc *argParaDoc = NULL;
	LnkLstDblNode *lnkNode = NULL;
	std::string strArgName;
	string strPara;

	strArgName.append(a_stArgDoc->argName, a_stArgDoc->argNameLen);

	LNKLST_DBL_GET_FIRST(&a_stArgDoc->argParaLL, lnkNode);
	nodeCnt = a_stArgDoc->argParaLL.nodeCnt;
	for(i=0;i<nodeCnt;i++){
		argParaDoc = (CliArgParaDoc*)lnkNode->data;

		if(argParaDoc->valType == CLI_ARG_TYPE_NUMBER){
			a_cReq[strArgName] = argParaDoc->u.num.val;
		}
		else {
			strPara.append(argParaDoc->u.str.val, argParaDoc->u.str.valLen);
			a_cReq[strArgName] = strPara;

			strPara.clear();
		}

		LNKLST_DBL_GET_NEXT_NODE(lnkNode);
	}/* end of for(i=0;i<nodeCnt;i++) */

	return CLC_OK;
}

int CCliDoc::Encode(CliDoc *a_stDoc, CCliReq &a_cReq)
{
	int nRet = 0;
	int i = 0;
	int nodeCnt = 0;
	CliArgDoc *argDoc = NULL;
	LnkLstDblNode *lnkNode = NULL;
	string strCmdLine;
	string strPkgName;
	string strCmdName; 

	if(a_stDoc->cmdLine != NULL){
		strCmdLine.append(a_stDoc->cmdLine, a_stDoc->cmdLineLen);
		a_cReq.SetCmdLine(strCmdLine);
	}

	if(a_stDoc->pkgNameLen != 0){
		if(strncasecmp(a_stDoc->pkgName, "DEFAULT",7) != 0){
			strPkgName.append(a_stDoc->pkgName, a_stDoc->pkgNameLen);
			a_cReq.SetPkgName(strPkgName);
		}
	}

	strCmdName.append(a_stDoc->cmdName, a_stDoc->cmdNameLen);
	a_cReq.SetCmdName(strCmdName);

	a_cReq.SetCmdCode(a_stDoc->cmdCode);

	nodeCnt = a_stDoc->argDocLL.nodeCnt;

	LNKLST_DBL_GET_FIRST(&a_stDoc->argDocLL, lnkNode);
	for(i=0;i<nodeCnt;i++){
		argDoc = (CliArgDoc*)lnkNode->data;

		nRet = ArgEncode(argDoc, a_cReq);
		if(nRet != CLC_OK){
			CLC_LOG(CLC_ERR,"Argument setting failed(ret=%d)\n",nRet);
			return CLC_NOK;
		}

		LNKLST_DBL_GET_NEXT_NODE(lnkNode);
	}/* end of for(i=0;i<nodeCnt;i++) */

	return CLC_OK;
}

