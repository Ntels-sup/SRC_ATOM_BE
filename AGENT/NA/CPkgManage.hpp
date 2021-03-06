/* vim:ts=4:sw=4
 */
/**
 * \file	CCPkgManage.hpp
 * \brief	패키지 download & install, upgrade
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CPKGMANAGE_HPP_
#define CPKGMANAGE_HPP_

#include <string>


class CPkgManage
{
public:
	std::string	m_strErrorMsg;
	
public:
	CPkgManage(const char* a_szPkgHomePath = NULL);
	~CPkgManage() {}

	bool	Fetch(const char* a_szUrl, const char* a_szUser, const char* a_szPasswd);
	bool	UnPack(const char* a_szUnPackPath = NULL);
	bool	Install(const char* a_szInstallScript = NULL);

private:
	std::string	m_strPkgPath;
	std::string m_strUnPackPath;
	std::string	m_strInstallScript;
	
	std::string m_strPkgFile;
	
	CPkgManage(const CPkgManage&) {};
};

#endif //CPKGMANAGE_HPP_
