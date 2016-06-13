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
	DB* pclsDB = CConfigNm::Instance().m_pclsDB;
	if (pclsDB == false) {
		g_pclsLog->ERROR("DBIO, db connection failed");
		return -1;
	}

	// NodeName�� �����ϱ� ���� ������ NodeNo�� ���´�.
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
		m_nNodeNo = atoi(szNodeNo);

	} else {
		g_pclsLog->ERROR("DBIO, select failed, %s", szSql);
		return -1;
	}

	char szNodeName[DB_NODE_NAME_SIZE];
	snprintf(szNodeName, sizeof(szNodeName), "%s-%s-%d", 
			a_strPkgName.c_str(), a_strNodeType.c_str(), m_nNodeNo+1);


	// �ű� NODE ��
	nSql = snprintf(szSql, sizeof(szSql),
				"INSERT INTO TAT_NODE "
					"(node_type, uuid, node_name, ip, internal_yn, use_yn, rddc_yn, "
					"node_status_ccd, proc_status_ccd, create_date, update_date, "
					"node_grp_id, image_no, pkg_name, description) "
				"VALUES "
					"('%s', '%s', '%s', '%s' , 'Y', 'Y', 'N', "
					"'UNKNOWN', 'UNKNOWN', sysdate(), sysdate(), "
					"0, 0, '%s' , '')",
				a_strNodeType.c_str(), a_strUuid.c_str(), szNodeName, a_strIp.c_str(), 
				a_strPkgName.c_str());

	nRowCnt = pclsDB->Execute(szSql, nSql);
	if (nRowCnt != 1) {
		g_pclsLog->ERROR("DBIO, node create failed, dbcd: %d, dbmsg: %s",
					nRowCnt, pclsDB->GetErrorMsg(nRowCnt));
		g_pclsLog->ERROR("      pkgnm: %s, ntype: %s, uuid: %s, ip: %s, nname: %s",
					a_strPkgName.c_str(),
					a_strNodeType.c_str(), 
					a_strUuid.c_str(),
					a_strIp.c_str(),
					szNodeName);
		return -1;
	}

	// �ű� �߰��� NODE_NO �˻�
	if (FindNodeNo(a_strUuid) <= 0) {
		return -1;
	}
	g_pclsLog->INFO("DBIO, Node created, pkgnm: %s, ntype: %s, uuid: %s, ip: %s, nname: %s",
					a_strPkgName.c_str(),
					a_strNodeType.c_str(), 
					a_strUuid.c_str(),
					a_strIp.c_str(),
					szNodeName);
				

    return m_nNodeNo;
}

bool CDBInOut::NodeStatus(string& a_strUuid, const char* a_szStatus)
{
	DB* pclsDB = CConfigNm::Instance().m_pclsDB;
	if (pclsDB == false) {
		g_pclsLog->ERROR("DBIO, db connection failed");
		return false;
	}

	char szSql[360];
	snprintf(szSql, sizeof(szSql), "SELECT max(node_no) FROM TAT_NODE");
	
	// NODE ���� ����
	int nSql = snprintf(szSql, sizeof(szSql),
				"UPDATE TAT_NODE SET node_status_ccd = '%s' WHERE uuid = '%s' ",
				a_szStatus, a_strUuid.c_str());

	int nRowCnt = pclsDB->Execute(szSql, nSql);
	if (nRowCnt != 1) {
		g_pclsLog->ERROR("DBIO, node status update failed, dbcd: %d, dbmsg: %s",
					nRowCnt, pclsDB->GetErrorMsg(nRowCnt));
		g_pclsLog->ERROR("      uuid: %s, status: %s", a_strUuid.c_str(), a_szStatus);
		return false;
	}

	g_pclsLog->INFO("DBIO, node status changeed, uuid: %s, status: %s",
					a_strUuid.c_str(), a_szStatus);

	return true;
}
