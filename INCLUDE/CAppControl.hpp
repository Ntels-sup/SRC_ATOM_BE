/* vim:ts=4:sw=4
 */
/**
 * \file	CAppControl.hpp
 * \brief	
 *
 * $Author: junls $
 * $Date: $
 * $Id: $
 */

#ifndef CAPPCONTROL_HPP_
#define CAPPCONTROL_HPP_

#include <string>


class CAppControl
{
public:
	enum EN_APPCTL {
		ACTL_INIT,
		ACTL_STOP,
		ACTL_SUSPEND,
		ACTL_RESUME,
		ACTL_LOGLEVEL,
		ACTL_UNKNOWN
	};
	
	std::string	m_strErrorMsg;

public:
	CAppControl() {}
	~CAppControl() {}

	bool		Init(int a_nArguemnt, char* a_szArgument[]);

	const char*	GetPkgName(std::string* a_strPkgName = NULL);
	const char*	GetSvcName(std::string* a_strSvcName = NULL);
	const char*	GetNodeType(std::string* a_strNodeType = NULL);
	const char*	GetNodeName(std::string* a_strNodeName = NULL);
	const char*	GetProcName(std::string* a_strProcName = NULL);
	int			GetNodeNo(void);
	int			GetProcNo(void);

	EN_APPCTL	Request(std::string& a_strBody, int *a_pnOpt);
	std::string	Response(bool a_bIsSuccess, const char* a_szText);

private:
	std::string	m_strPkgName;
	std::string	m_strSvcName;
	std::string	m_strNodeType;
	std::string	m_strNodeName;
	std::string	m_strProcName;
};

#endif // CAPPCONTROL_HPP_
