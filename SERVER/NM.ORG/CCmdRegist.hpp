/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdRegist.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDREGIST_HPP_
#define CCMDREGIST_HPP_

#include <string>

#include "CCmdBase.hpp"


class CCmdRegist : public CCmdBase
{
public:
	std::string m_strPkgName;
	std::string m_strNodeType;
	std::string m_strProcName;
	int			m_nProcNo;

public:
	CCmdRegist() {
		m_nProcNo = -1;
		return;
	}
	~CCmdRegist() {}
	
	bool		RequestParse(const char* a_szJson);

private:

};

#endif // CCMDREGIST_HPP_
