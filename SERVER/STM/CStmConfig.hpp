#ifndef __CSTMCONFIG_HPP__
#define __CSTMCONFIG_HPP__

#include "STM.h"
#include "mysql.h"
#include <list>

#include "MariaDB.hpp"
#include "CResourceInfo.hpp"
#include "CAlarmInfo.hpp"
#include "CNodeInfo.hpp"
#include "CPackageInfo.hpp"
#include "CTCAInfo.hpp"

using namespace std;

class CStmConfig{
	private:
		DB *m_cDb;
	public:
		CStmConfig(DB *a_cDb, int &nRet);
		~CStmConfig();
		int DBLoadNode(list<CNodeInfo*> *a_lstNodeInfo, list<CPackageInfo*> *a_cPackage);
		int DBLoadAlarmMergeTime(CAlarmInfo *a_cAlarm);
		int DBLoadResourceMergeTime(CResourceInfo *a_cResource);
		int DBLoadColumn(string a_strPkgName, CTableInfo *a_cTable);
		int DBLoadTable(CPackageInfo *a_cPackage);
		int DBLoadPackage(list<CPackageInfo*> *a_lstPackageInfo);
		int DBLoadPackageStsConfig(CPackageInfo *cPackage);
		int DBLoadResourceInfo(CResourceInfo *a_cResInfo);
		int DBLoadPKGNode(list<CPackageInfo*> *a_cPackage, list<CNodeInfo*> *a_cNode);
		int DBLoadTCARule(int nNodeId, CTCAInfo *cTCAInfo);
		int DBLoadTCAInfo(CNodeInfo *cNode);
		int DBGetProcNo(int *a_nProcNo);
		int DBGetNodeId(char *a_szPkgName, char *a_szNodeType, char *a_szNodeName ,int *a_nNodeNo);
		int DBGetProcNo(char *a_szPkgName, char *a_szNodeType, char *a_szProcName ,int *a_nProcNo);
};

#endif
