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

bool CCmdNodeStatus::NotifyParse(const char* a_szJson)
{
	if (a_szJson == NULL) {
		return false;
	}

    try {
		rabbit::document docRoot;
		docRoot.parse(a_szJson);

		m_strUuid		= docRoot["BODY"]["uuid"].as_string();
		m_strStatus		= docRoot["BODY"]["status"].as_string();
		m_strPkgName	= docRoot["BODY"]["pkgname"].as_string();
		m_strNodeType	= docRoot["BODY"]["nodetype"].as_string();
		m_nNodeNo		= docRoot["BODY"]["nodeno"].as_int();
		m_strNodeName	= docRoot["BODY"]["nodename"].as_string();
		m_strIp			= docRoot["BODY"]["ip"].as_string();
		m_strVersion	= docRoot["BODY"]["version"].as_string();
	
	} catch(rabbit::type_mismatch& e) {
		g_pclsLog->ERROR("CMD 'NODESTATUS', %s", e.what());
		return false;
	} catch(rabbit::parse_error& e) {
		g_pclsLog->ERROR("CMD 'NODESTATUS', %s", e.what());
		return false;
	} catch(...) {
		g_pclsLog->ERROR("CMD 'NODESTATUS', unknown error");
		return false;
	}
	
	return true;
}
