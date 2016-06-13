/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdBatchStart.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDBATCHSTART_HPP_
#define CCMDBATCHSTART_HPP_

#include <string>

#include "CCmdBase.hpp"


class CCmdBatchStart : public CCmdBase
{
public:
	// Request
	std::string	m_strGroupName;
	std::string	m_strJobName;
	std::string	m_strPkgName;
	int			m_nProcNo;
	std::string	m_strPrcDate;
	std::string	m_strExecBin;
	std::string	m_strExecArg;
	std::string	m_strExecEnv;
	
	// Response
	int			m_nExitCd;
	std::string	m_strStatus;

public:
	CCmdBatchStart() {
		m_nProcNo = 0;
		m_nExitCd = 0;
		return;
	}
	~CCmdBatchStart() {}

	std::string	ResponseGen(void);
	bool		RequestParse(const char* a_szJson);
	
private:

};

#endif // CCMDPKGINSTALL_HPP_
