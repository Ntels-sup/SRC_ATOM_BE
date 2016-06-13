/* vim:ts=4:sw=4
 */
/**
 * \file	CModuleManager.hpp
 * \brief	동적 모듈 관리
 *			
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#ifndef CMODULEMANAGER_HPP_
#define CMODULEMANAGER_HPP_

#include <map>

#include "CModule.hpp"


class CModuleManager
{
public:
	static CModuleManager& Instance(void);
	
	bool	AddModule(std::string& a_strModPath);
	bool	ActiveModule(const char* a_szModName = NULL);
	bool	StopModule(const char* a_szModName = NULL);
	bool	FindModule(const char* a_szModName);

	bool	SetOptPkgName(std::string& a_strPkgName, const char* a_szModName = NULL);
	bool	SetOptNodeType(std::string& a_strNodeType, const char* a_szModName = NULL);
	bool	SetOptNodeName(std::string& a_strNodeName, const char* a_szModName = NULL);
	bool	SetOptNodeVersion(std::string& a_strNodeVer, const char* a_szModName = NULL);
	bool	SetOptNodeNo(int a_nNodeNo, const char* a_szModName = NULL);
	bool	SetOptProcNo(int a_nProcNo, const char* a_szModName);

private:
	std::map<std::string, CModule*> m_mapModule;

private:
	static CModuleManager*  m_pclsInstance;          // singleton instance
	CModuleManager();
	CModuleManager(CModuleManager&) {}
	~CModuleManager();
};

#endif // CMODULEMANAGER_HPP_
