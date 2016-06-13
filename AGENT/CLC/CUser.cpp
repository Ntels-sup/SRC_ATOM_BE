#include <stdio.h>
#include <string.h>
#include "CUser.hpp"

CUser::CUser(const char *a_szUserId, const char *a_szPasswd)
{
	strcpy(m_szUserId, a_szUserId);
	strcpy(m_szPasswd, a_szPasswd);
}

CUser::~CUser()
{
}

char *CUser::GetUserId()
{
	return m_szUserId;
}

char *CUser::GetPasswd()
{
	return m_szPasswd;
}

int CUser::CheckUserId(const char *a_szUserId)
{
	if(strcmp(m_szUserId, a_szUserId) == 0){
		return CLC_OK;
	}

	return CLC_NOK;
}

int CUser::CheckPasswd(const char *a_szPasswd)
{
	if(strcmp(m_szPasswd, a_szPasswd) == 0){
		return CLC_OK;
	}

	return CLC_NOK;
}

