#include <cstdio>
#include <iostream>

#include "CFileLog.hpp"
#include "CCmdRegistNode.hpp"

using std::string;

// global valiable
extern CFileLog* g_pclsLog;

/* CMD_REGIST
 * - Request
  {
    "BODY": {
      "pkgname": "vOFCS",
      "nodetype": "LB",
      "procname": "NA",
      "version": "1.0.0",
      "uuid": "a209062c-7ea4-4712-97b8-214fc13219eb",
      "ip": "127.0.0.1",
      "procno": 12
    }
  }
 *
 * - Response
 *
	{
	  "BODY": {
	    "nodeno": 1,
	    "nodename": "vOFCS-LB-12"
	  }
	}
 */

string CCmdRegistNode::ResponseGen(void)
{
	rabbit::object docRoot;

	rabbit::object docBody = docRoot["BODY"];
	docBody["nodeno"] = m_nNodeNo;
	docBody["nodename"] = m_strNodeName;

	return docRoot.str();
}

bool CCmdRegistNode::RequestParse(const char* a_szJson)
{
	if (a_szJson == NULL) {
		return false;
	}

    try {
		rabbit::document docRoot;
		docRoot.parse(a_szJson);

		m_strPkgName	= docRoot["BODY"]["pkgname"].as_string();
		m_strNodeType	= docRoot["BODY"]["nodetype"].as_string();
		m_strProcName	= docRoot["BODY"]["procname"].as_string();
		m_strUuid		= docRoot["BODY"]["uuid"].as_string();
		m_strIp			= docRoot["BODY"]["ip"].as_string();
		m_nProcNo		= docRoot["BODY"]["procno"].as_int();
		m_strVersion	= docRoot["BODY"]["version"].as_string();
	
	} catch(rabbit::type_mismatch& e) {
		g_pclsLog->ERROR("CMD 'REGIST_NODE', %s", e.what());
		return false;
	} catch(rabbit::parse_error& e) {
		g_pclsLog->ERROR("CMD 'REGIST_NODE', %s", e.what());
		return false;
	} catch(...) {
		g_pclsLog->ERROR("CMD 'REGIST_NODE', unknown error");
		return false;
	}
	
	return true;
}
