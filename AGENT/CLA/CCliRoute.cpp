#include <strings.h>
#include "CLA.h"
#include "CGlobal.hpp"
#include "CCliRoute.hpp"

CCliRoute::CCliRoute()
{
}

CCliRoute::~CCliRoute()
{
	while(m_lstRoute.size()){
		delete m_lstRoute.front();
		m_lstRoute.pop_front();
	}
}


int CCliRoute::Insert(unsigned int a_unCmdCode, string &a_strCmdName, string &a_strPkgName, string &a_strDest)
{
	STRouteInfo *stRouteInfo = NULL;

	stRouteInfo = new STRouteInfo;

	stRouteInfo->unCmdCode = a_unCmdCode;
	stRouteInfo->strCmdName = a_strCmdName;
	stRouteInfo->strPkgName = a_strPkgName;
	stRouteInfo->strDest = a_strDest;

	m_lstRoute.push_back(stRouteInfo);

	return CLA_OK;
}

int CCliRoute::Insert(unsigned int a_unCmdCode, const char *a_szCmdName, const char *a_szPkgName, unsigned int a_szDest)
{
	STRouteInfo *stRouteInfo = NULL;
	CGlobal *cGlob = NULL;
	CModuleIPC *cIpc = NULL;
	CAddress *cAddr = NULL;

	stRouteInfo = new STRouteInfo;

	cGlob = CGlobal::GetInstance();
	cIpc = cGlob->GetModuleIPC();
	cAddr = cGlob->GetAddress();

	stRouteInfo->unCmdCode = a_unCmdCode;
	stRouteInfo->strCmdName = a_szCmdName;
	stRouteInfo->strPkgName = a_szPkgName;

	stRouteInfo->unNodeNo = cGlob->GetLocalNodeNo();
	stRouteInfo->unDstNo = a_szDest;

	m_lstRoute.push_back(stRouteInfo);

	return CLA_OK;
}

int CCliRoute::Delete(string &a_strCmdName, string &a_strPkgName)
{
	STRouteInfo *stRouteInfo = NULL;
	list<STRouteInfo*>::iterator iter;

	for(iter = m_lstRoute.begin();iter != m_lstRoute.end();iter++){
		stRouteInfo = *iter;
		if(a_strCmdName.size() != stRouteInfo->strCmdName.size()){
			continue;
		}

		if(a_strPkgName.size() != stRouteInfo->strPkgName.size()){
			continue;
		}

		if(strncasecmp(a_strCmdName.c_str(), stRouteInfo->strCmdName.c_str(), a_strCmdName.size()) == 0){
			if(strncasecmp(a_strPkgName.c_str(), stRouteInfo->strPkgName.c_str(), a_strPkgName.size()) == 0){
				/* delete */
				delete stRouteInfo;

				m_lstRoute.erase(iter);
				return CLA_OK;
			}
		}
	}

	return CLA_OK;
}

int CCliRoute::FindDestinationNo(string &a_strCmdName, string &a_strPkgName, int *unDstNo, int *unDstNodeNo)
{
	STRouteInfo *stRouteInfo = NULL;
	list<STRouteInfo*>::iterator iter;

	for(iter = m_lstRoute.begin();iter != m_lstRoute.end();iter++){
		stRouteInfo = *iter;
		if(a_strCmdName.size() != stRouteInfo->strCmdName.size()){
			continue;
		}

		if(a_strPkgName.size() != stRouteInfo->strPkgName.size()){
			continue;
		}

		if(strncasecmp(a_strCmdName.c_str(), stRouteInfo->strCmdName.c_str(), a_strCmdName.size()) == 0){
			if(strncasecmp(a_strPkgName.c_str(), stRouteInfo->strPkgName.c_str(), a_strPkgName.size()) == 0){
				*unDstNo = stRouteInfo->unDstNo;
				*unDstNodeNo = stRouteInfo->unNodeNo;
				return CLA_OK;
			}
		}
	}

	return CLA_NOK;
}

string *CCliRoute::FindDestination(string &a_strCmdName, string &a_strPkgName)
{
	STRouteInfo *stRouteInfo = NULL;
	list<STRouteInfo*>::iterator iter;

	for(iter = m_lstRoute.begin();iter != m_lstRoute.end();iter++){
		stRouteInfo = *iter;
		if(a_strCmdName.size() != stRouteInfo->strCmdName.size()){
			continue;
		}

		if(a_strPkgName.size() != stRouteInfo->strPkgName.size()){
			continue;
		}

		if(strncasecmp(a_strCmdName.c_str(), stRouteInfo->strCmdName.c_str(), a_strCmdName.size()) == 0){
			if(strncasecmp(a_strPkgName.c_str(), stRouteInfo->strPkgName.c_str(), a_strPkgName.size()) == 0){
				return &stRouteInfo->strDest;
			}
		}
	}

	return NULL;
}

