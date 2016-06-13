#include <cstdio>
#include <iostream>

#include "CFileLog.hpp"
#include "CCmdRegist.hpp"

using std::string;

// global valiable
extern CFileLog* g_pclsLog;

/* CMD_REGIST
 * - Request
 *
 	{"BODY": {
		"pkgname": "ATOM",
		"nodetype": "EMS",
		"procname": "ATOM_SWM",
 		"procid": 1
		}
 	}
 *
 * - Response
 *
	{"BODY": {
		"code": 0,
		"text": "ok"
		}
	}
 */

bool CCmdRegist::RequestParse(const char* a_szJson)
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
		m_nProcNo		= docRoot["BODY"]["procno"].as_int();
	
	} catch(rabbit::type_mismatch& e) {
		g_pclsLog->ERROR("CMD 'REGIST', %s", e.what());
		return false;
	} catch(rabbit::parse_error& e) {
		g_pclsLog->ERROR("CMD 'REGIST', %s", e.what());
		return false;
	} catch(...) {
		g_pclsLog->ERROR("CMD 'REGIST', unknown error");
		return false;
	}
	
	return true;
}
