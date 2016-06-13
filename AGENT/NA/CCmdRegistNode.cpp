#include <cstdio>
#include <iostream>

#include "CFileLog.hpp"
#include "CCmdRegistNode.hpp"

using std::string;

// global valiable
extern CFileLog* g_pclsLog;

/* CMD_REGIST_NODE
 * - Request
 	{"BODY": {
		"pkgname": "vOFCS",
		"nodetype": "AP",
		"procname": "NA",
		"uuid": "a209062c-7ea4-4712-97b8-214fc13219eb",
		"ip": "127.0.0.1",
 		"procno": 1
		}
 	}
 *
 * - Response 
 	{"BODY": {
 		"nodeid": 2,
 		"nodename": "vOFCS-LB-12"
		}
 	}
 */

string CCmdRegistNode::RequestGen(void)
{
	// lookup hostname
	char szHostName[80];
	memset(szHostName, 0x00, sizeof(szHostName));
	//gethostname(szHostName, sizeof(szHostName));

	rabbit::object  docRoot;
	rabbit::object  docBody = docRoot["BODY"];
	docBody["pkgname"]	= m_strPkgName;
	docBody["nodetype"]	= m_strNodeType;
	docBody["procname"]	= m_strProcName;
	docBody["nodename"] = szHostName;
	docBody["uuid"]		= m_strUuid;
	docBody["ip"]		= m_strNodeIp;
	docBody["procno"]	= m_nProcNo;
	docBody["version"]	= m_strVersion;

	return docRoot.str();
}

bool CCmdRegistNode::ResponseParse(const char* a_szJson)
{
	if (a_szJson == NULL) {
		return false;
	}

    try {
		rabbit::document docRoot;
		docRoot.parse(a_szJson);

		m_nNodeNo = docRoot["BODY"]["nodeno"].as_int();
		m_strNodeName = docRoot["BODY"]["nodename"].as_string();
	
	} catch(rabbit::type_mismatch& e) {
		g_pclsLog->ERROR("CMDREGIST,, invalied type mismatch");
		return false;
	} catch(rabbit::parse_error& e) {
		g_pclsLog->ERROR("CMDREGIST, parsing failed");
		return false;
	} catch(...) {
		g_pclsLog->ERROR("CMDREGIST, parsing failed");
		return false;
	}
	
	return true;
}
