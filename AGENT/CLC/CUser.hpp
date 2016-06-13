#ifndef __CUSER_HPP__
#define __CUSER_HPP__

#include "CLC.h"

class CUser{
	private:
		char m_szUserId[CLC_USER_ID_LEN];
		char m_szPasswd[CLC_PASSWD_LEN];

	public:
		CUser(const char *a_szUserId, const char *a_szPasswd);
		~CUser();
		char *GetUserId();
		char *GetPasswd();
		int CheckUserId(const char *a_szUserId);
		int CheckPasswd(const char *a_szPasswd);
};

#endif
