/* vim:ts=4:sw=4
 */
/**
 * \file	CDBInOut.hpp
 * \brief	
 *
 * $Author: junls&ntels.com $
 * $Date: $
 * $Id: $
 */

#include "CommonCode.hpp"
#include "CFileLog.hpp"
#include "FetchMaria.hpp"

#include "CConfigNm.hpp"
#include "CDBInOut.hpp"

using std::string;
using std::vector;

//extern global symbol
extern CFileLog* g_pclsLog;


CDBInOut::CDBInOut()
{
	return;
}

CDBInOut::~CDBInOut()
{
	return;
}
	
int CDBInOut::FindNodeNo(string& a_strUuid)
{
	DB* pclsDB = CConfigNm::Instance().m_pclsDB;
	if (pclsDB == false) {
		g_pclsLog->ERROR("DBIO, db connection failed");
		return -1;
	}

	char szSql[360];
	int nSql = snprintf(szSql, sizeof(szSql),
				"SELECT node_no, node_name FROM TAT_NODE WHERE uuid = '%s'",
				a_strUuid.c_str());

	FetchMaria fdata;
	int nRowCnt = pclsDB->Query(&fdata, szSql, nSql);
	if (nRowCnt == 0) {
		g_pclsLog->INFO("DBIO, not found nodeno, uuid: %s", a_strUuid.c_str());
		return 0;
	} else if (nRowCnt < 0) {
		g_pclsLog->ERROR("DBIO, select failed nodeno, uuid: %s", a_strUuid.c_str());
		return -1;
	} else if (nRowCnt > 1) {
		g_pclsLog->ERROR("DBIO, too many rows nodeno, uuid: %s", a_strUuid.c_str());
		return -1;
	}

	char szNodeNo[11];
	char szNodeName[DB_NODE_NAME_SIZE];
	memset(szNodeNo, 0x00, sizeof(szNodeNo));
	memset(szNodeName, 0x000, sizeof(szNodeName));
	
	fdata.Set(szNodeNo, sizeof(szNodeNo));
	fdata.Set(szNodeName, sizeof(szNodeName));
	if (fdata.Fetch() == false) {
		g_pclsLog->ERROR("DBIO, %s, select fatch failed", __FUNCTION__);
		return -1;
	}
	
	m_nNodeNo = atoi(szNodeNo);
	m_strNodeName = szNodeName;
	
	return m_nNodeNo;
}

int CDBInOut::FindNodeNo(string& a_strPkgName, string& a_strNodeType)
{
	DB* pclsDB = CConfigNm::Instance().m_pclsDB;
	if (pclsDB == false) {
		g_pclsLog->ERROR("DBIO, db connection failed");
		return -1;
	}

	char szSql[360];
	int nSql = snprintf(szSql, sizeof(szSql), "SELECT node_no, node_name FROM TAT_NODE "
	                                    "WHERE node_name = '%s' AND node_type = '%s'",
										a_strPkgName.c_str(), a_strNodeType.c_str());

	FetchMaria fdata;
	int nRowCnt = pclsDB->Query(&fdata, szSql, nSql);
	if (nRowCnt == 0) {
		g_pclsLog->INFO("DBIO, not found nodeno, pkgnm: %s, ntype: %s",
									a_strPkgName.c_str(), a_strNodeType.c_str());
		return 0;
	} else if (nRowCnt < 0) {
		g_pclsLog->ERROR("DBIO, select failed nodeno, pkgnm: %s, ntype: %s",
									a_strPkgName.c_str(), a_strNodeType.c_str());
		return -1;
	} else if (nRowCnt > 1) {
		g_pclsLog->ERROR("DBIO, too many rows nodeno, pkgnm: %s, ntype: %s",
									a_strPkgName.c_str(), a_strNodeType.c_str());
		return -1;
	}


	char szNodeNo[11];
	char szNodeName[DB_NODE_NAME_SIZE];
	memset(szNodeNo, 0x00, sizeof(szNodeNo));
	memset(szNodeName, 0x00, sizeof(szNodeName));
	
	fdata.Set(szNodeNo, sizeof(szNodeNo));
	fdata.Set(szNodeName, sizeof(szNodeName));
	if (fdata.Fetch() == false) {
		g_pclsLog->ERROR("DBIO, %s, select fatch failed", __FUNCTION__);
		return -1;
	}
	
	m_nNodeNo = atoi(szNodeNo);
	m_strNodeName = szNodeName;
	
	
	return m_nNodeNo;
}

int CDBInOut::FindProcNo(string& a_strPkgName, string& a_strNodeType,
														string& a_strProcName)
{
	DB* pclsDB = CConfigNm::Instance().m_pclsDB;
	if (pclsDB == false) {
		g_pclsLog->ERROR("DBIO, db connection failed");
		return -1;
	}

	char szSql[360];
	int nSql = snprintf(szSql, sizeof(szSql),
				"SELECT proc_no FROM TAT_PROCESS "
				"WHERE pkg_name = '%s' AND node_type = '%s' AND proc_name = '%s'",
				a_strPkgName.c_str(),
				a_strNodeType.c_str(),
				a_strProcName.c_str());

	FetchMaria fdata;
	int nRowCnt = pclsDB->Query(&fdata, szSql, nSql);
	if (nRowCnt == 0) {
		g_pclsLog->INFO("DBIO, %s, not found PROC_NO, "
						"pkgnm: %s, ntype: %s, procnm: %s",
						__FUNCTION__,
						a_strPkgName.c_str(), a_strNodeType.c_str(), a_strProcName.c_str());
		return 0;
	} else if (nRowCnt < 0) {
		g_pclsLog->ERROR("DBIO, %s, select failed PROC_NO, "
						"pkgnm: %s, ntype: %s, procnm: %s",
						__FUNCTION__,
						a_strPkgName.c_str(), a_strNodeType.c_str(), a_strProcName.c_str());
		return -1;
	} else if (nRowCnt > 1) {
		g_pclsLog->ERROR("DBIO, %s, too many rows PROC_NO, "
						"pkgnm: %s, ntype: %s, procnm: %s",
						__FUNCTION__,
						a_strPkgName.c_str(), a_strNodeType.c_str(), a_strProcName.c_str());
		return -1;
	}

	char szProcNo[11];
	memset(szProcNo, 0x00, sizeof(szProcNo));

	fdata.Set(szProcNo, sizeof(szProcNo));
	if (fdata.Fetch() == false) {
		g_pclsLog->ERROR("DBIO, %s, select fatch failed", __FUNCTION__);
		return false;
	}

	m_nProcNo = atoi(szProcNo);

    return m_nProcNo;
}

int CDBInOut::NodeCreate(string& a_strPkgName, string& a_strNodeType,
											string& a_strUuid, string& a_strIp)
{
	vector<ST_GUINODE> vecNodeName;
	if (NodeName(a_strPkgName, a_strNodeType, vecNodeName) <= 0) {
		g_pclsLog->ERROR("DBIO, get node name and image no failed");
		return -1;
	}
	
	DB* pclsDB = CConfigNm::Instance().m_pclsDB;
	if (pclsDB == false) {
		g_pclsLog->ERROR("DBIO, db connection failed");
		return -1;
	}

	int nRowCnt = 0;
	int nSql = 0;
	char szSql[360];

	auto viter = vecNodeName.begin();
	for (; viter != vecNodeName.end(); ++viter) {
		nSql = snprintf(szSql, sizeof(szSql),
					"INSERT INTO TAT_NODE "
						"(node_type, uuid, node_name, ip, internal_yn, use_yn, rddc_yn, "
						"node_status_ccd, proc_status_ccd, create_date, update_date, "
						"node_grp_id, image_no, pkg_name, description) "
					"VALUES "
						"('%s', '%s', '%s', '%s' , 'Y', 'Y', 'N', "
						"'UNKNOWN', 'UNKNOWN', sysdate(), sysdate(), "
						"%d, %d, '%s' , '')",
					a_strNodeType.c_str(), a_strUuid.c_str(), 
					viter->m_strNodeName.c_str(), a_strIp.c_str(), 
					viter->m_nNodeGrpId, viter->m_nImageNo, 
					a_strPkgName.c_str());
	
		nRowCnt = pclsDB->Execute(szSql, nSql);
		if (nRowCnt == 1) {
			break;
		}

		g_pclsLog->WARNING("DBIO, node create failed, dbcd: %d, dbmsg: %s",
										nRowCnt, pclsDB->GetErrorMsg(nRowCnt));
	}

	if (viter == vecNodeName.end()) {
		// NODE 추가에 실패한 경우
		g_pclsLog->ERROR("DBIO, node create failed, dbcd: %d, dbmsg: %s",
										nRowCnt, pclsDB->GetErrorMsg(nRowCnt));
		g_pclsLog->ERROR("      pkgnm: %s, ntype: %s, uuid: %s, ip: %s",
						a_strPkgName.c_str(),
						a_strNodeType.c_str(), 
						a_strUuid.c_str(),
						a_strIp.c_str());
			return -1;
	}

	// 생성된 Node no를 얻는다.
	if (FindNodeNo(a_strUuid) <= 0) {
		return -1;
	}
	g_pclsLog->INFO("DBIO, Node created, pkgnm: %s, ntype: %s, uuid: %s, ip: %s, nname: %s",
					a_strPkgName.c_str(),
					a_strNodeType.c_str(), 
					a_strUuid.c_str(),
					a_strIp.c_str(),
					m_strNodeName.c_str());

    return m_nNodeNo;
}

bool CDBInOut::NodeUse(string& a_strUuid, char a_cUsed)
{
	DB* pclsDB = CConfigNm::Instance().m_pclsDB;
	if (pclsDB == false) {
		g_pclsLog->ERROR("DBIO, db connection failed");
		return false;
	}

	char szSql[360];
	
	// NODE ???? ????
	int nSql = snprintf(szSql, sizeof(szSql),
				"UPDATE TAT_NODE SET use_yn = '%c' WHERE uuid = '%s' ",
				a_cUsed, a_strUuid.c_str());

	int nRowCnt = pclsDB->Execute(szSql, nSql);
	if (nRowCnt != 1) {
		g_pclsLog->ERROR("DBIO, unused node update failed, dbcd: %d, dbmsg: %s",
					nRowCnt, pclsDB->GetErrorMsg(nRowCnt));
		g_pclsLog->ERROR("      uuid: %s", a_strUuid.c_str());
		return false;
	}

	g_pclsLog->INFO("DBIO, unused node, uuid: %s", a_strUuid.c_str());

	return true;
}

int CDBInOut::NodeName(string& a_strPkgName, string& a_strNodeType,
									vector<ST_GUINODE>& a_vecNodeName)
{
	DB* pclsDB = CConfigNm::Instance().m_pclsDB;
	if (pclsDB == false) {
		g_pclsLog->ERROR("DBIO, db connection failed");
		return -1;
	}

	// node type이 'AP'인 경우 node_name을 동적으로 생성하기 위해 현재 node_no을
	// 이용한다. 
	int nMaxNodeNo = 0;
	char szNodeNo[11];
	memset(szNodeNo, 0x00, sizeof(szNodeNo));

	char szSql[360];
	int nSql = snprintf(szSql, sizeof(szSql), "SELECT max(node_no) FROM TAT_NODE");

	FetchMaria fdata;
	int nRowCnt = pclsDB->Query(&fdata, szSql, nSql);

	if (nRowCnt == 1) {
		fdata.Set(szNodeNo, sizeof(szNodeNo));
		if (fdata.Fetch() == false) {
			g_pclsLog->ERROR("DBIO, select fatch failed, %s", szSql);
			return -1;
		}
		nMaxNodeNo = atoi(szNodeNo);
	
	} else {
		g_pclsLog->ERROR("DBIO, select failed, %s", szSql);
		return -1;
	}

	// NodeName과 Image No를 얻는다.
	char szAPNodeName[61];
	char szNodeName[61];
	char szNodeGrpId[11];
	char szImageNo[11];
	memset(szAPNodeName, 0x00, sizeof(szAPNodeName));
	memset(szNodeName, 0x00, sizeof(szNodeName));
	memset(szNodeGrpId, 0x00, sizeof(szNodeGrpId));
	memset(szImageNo, 0x00, sizeof(szImageNo));

	nSql = snprintf(szSql, sizeof(szSql), 
				"SELECT node_name, node_grp_id, image_no FROM TAT_NODE_GUI "
					"WHERE pkg_name = '%s' "
						"AND node_type = '%s' "
						"AND internal_yn = 'Y' "
						"AND use_yn = 'Y'",
				a_strPkgName.c_str(), a_strNodeType.c_str());

	fdata.Clear();
	nRowCnt = pclsDB->Query(&fdata, szSql, nSql);
	if (nRowCnt < 1) {
		g_pclsLog->ERROR("DBIO, select failed, %s", szSql);
		return -1;
	}
	
	ST_GUINODE stV;

	fdata.Set(szNodeName, sizeof(szNodeName));
	fdata.Set(szNodeGrpId, sizeof(szNodeGrpId));
	fdata.Set(szImageNo, sizeof(szImageNo));

	for (int i=0; i < nRowCnt; i++) {
		if (fdata.Fetch() == false) {
			g_pclsLog->ERROR("DBIO, select fatch failed, %s", szSql);
			return -1;
		}
		
		if (a_strNodeType.compare("AP") == 0) {
			snprintf(szAPNodeName, sizeof(szAPNodeName), "%s-%d", szNodeName, nMaxNodeNo);
			stV.m_strNodeName = szAPNodeName;
		} else {
			stV.m_strNodeName = szNodeName;
		}
		stV.m_nNodeGrpId = atoi(szNodeGrpId);
		stV.m_nImageNo = atoi(szImageNo);

		a_vecNodeName.push_back(stV);
	}

	return (int)a_vecNodeName.size();
}
