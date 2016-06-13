#include <cstdio>
#include <iostream>

#include "CFileLog.hpp"
#include "CCmdNodeList.hpp"

using std::string;

// global valiable
extern CFileLog* g_pclsLog;

/* CMD_REGIST
 * - Request
	{
		"BODY": {
			"pkgname": "vOFCS",
			"nodetype": "LB"
		}
	}
 *
 * - Response
 *
	{
		"BODY": [{
			"pkgname": "vOFCS",
			"nodetype": "LB01",
			"nodeno": 10,
			"nodename": "vOFCS-LB-10",
			"ip": "127.0.0.1",
			"version": "1.0.0"
		},
		{
			"pkgname": "vOFCS",
			"nodetype": "LB02",
			"nodeno": 11,
			"nodename": "vOFCS-LB-11",
			"ip": "127.0.0.1",
			"version": "1.0.0"
		}]
	}
 */

string CCmdNodeList::ResponseGen(void)
{
	rabbit::object docRoot;
	rabbit::array docNode = docRoot["BODY"];
	rabbit::object o[m_lstNode.size()];
	int i = 0;
	for (auto iter = m_lstNode.begin(); iter != m_lstNode.end(); ++iter, i++) {
	    o[i]["pkgname"]		= iter->m_strPkgName;
	    o[i]["nodetype"]	= iter->m_strNodeType;
	    o[i]["nodeno"]		= iter->m_nNodeNo;
	    o[i]["nodename"]	= iter->m_strNodeName;
	    o[i]["ip"]			= iter->m_strIp;
	    o[i]["version"]		= iter->m_strVersion;

		docNode.push_back(o[i]);
	}

	return docRoot.str();
}

bool CCmdNodeList::RequestParse(const char* a_szJson)
{
	if (a_szJson == NULL) {
		return false;
	}

    try {
		rabbit::document docRoot;
		docRoot.parse(a_szJson);

		m_strPkgName	= docRoot["BODY"]["pkgname"].as_string();
		m_strNodeType	= docRoot["BODY"]["nodetype"].as_string();
	
	} catch(rabbit::type_mismatch& e) {
		g_pclsLog->ERROR("CMD 'NODE_LIST', %s", e.what());
		return false;
	} catch(rabbit::parse_error& e) {
		g_pclsLog->ERROR("CMD 'NODE_LIST', %s", e.what());
		return false;
	} catch(...) {
		g_pclsLog->ERROR("CMD 'NODE_LIST', unknown error");
		return false;
	}
	
	return true;
}
