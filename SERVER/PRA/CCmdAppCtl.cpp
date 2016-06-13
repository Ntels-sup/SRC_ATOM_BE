/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdAppCtl.cpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#include <cstdio>
#include <iostream>

#include "CFileLog.hpp"
#include "CCmdAppCtl.hpp"

using std::string;

// global valiable
extern CFileLog* g_pclsLogPra;

/* CMD_APP_CTL
 * - Request
	{
	   "BODY": {
	     "action": "STOP",
	     "option": "3"
	    }
	}
 *
 * - Response 
	{
	  "BODY": {
	    "code": 0,
	    "text": "ok or error message"
	  }
	}
 */

string CCmdAppCtl::RequestGen(void)
{
	rabbit::object  docRoot;

	rabbit::object  docBody = docRoot["BODY"];
	docBody["action"]	= m_strAction;
	docBody["option"]	= m_nOption;

	return docRoot.str();
}