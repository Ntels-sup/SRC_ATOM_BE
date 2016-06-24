#include <cstdio>
#include <iostream>

#include "CFileLog.hpp"
#include "CCmdNodeStatus.hpp"

using std::string;

// global valiable
extern CFileLog* g_pclsLog;

/* CMD_NODE_STATUS
 * - Notify
	{
		"BODY": {
			"uuid": "xxx-xxxxx--xxx",
			"status": "RUNNING"
			"pkgname": "vOFCS",
			"nodetype": "LB",
			"nodeno": 1,
			"nodename": "vOFCS-LB-12",
			"ip": "127.0.0.1",
			"version": "1.0.0"
		}
	}
 */

string CCmdNodeStatus::NotifyGen(void)
{
	rabbit::object  docRoot;
	rabbit::object  docBody = docRoot["BODY"];
	docBody["uuid"]		= m_strUuid;
	docBody["status"]	= m_strStatus;
	docBody["pkgname"]	= m_strPkgName;
	docBody["nodetype"]	= m_strNodeType;
	docBody["nodeno"]	= m_nNodeNo;
	docBody["nodename"]	= m_strNodeName;
	docBody["ip"]		= m_strIp;
	docBody["version"]	= m_strVersion;

	return docRoot.str();
}
