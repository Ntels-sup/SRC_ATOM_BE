/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdAppInit.cpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#include <cstdio>
#include <iostream>

#include "CFileLog.hpp"

#include "CCmdAppInit.hpp"

using std::string;

// global valiable
extern CFileLog* g_pclsLogPra;


/* CMD_APP_INIT
 * - Response 
	{
	  "BODY": {
	      "process_status": [
	          {"procno": 1, "procname": "APP01", "success": true},
	          {"procno": 2, "procname": "APP02", "success": true},
	          {"procno": 3, "procname": "APP03", "success": true}
	      ]
	   }
    }
 */

string CCmdAppInit::ResponseGen(void)
{
	rabbit::object docRoot;
	rabbit::object docBody = docRoot["BODY"];
	
	rabbit::array docProcess = docBody["process_status"];
	rabbit::object o[m_lstResponse.size()];
	int i = 0;
	for (auto iter = m_lstResponse.begin(); iter != m_lstResponse.end(); ++iter, ++i) {
	    o[i]["procno"]		= iter->m_nProcNo;
	    o[i]["procname"]	= iter->m_strProcName;

		docProcess.push_back(o[i]);
	}

	return docRoot.str();
}
