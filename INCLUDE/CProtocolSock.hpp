/* vim:ts=4:sw=4
 */
/**
 * \file	CMesgExch.cpp
 * \brief	프로세스 내의 event message 전달
 *
 * $Author: junls@ntels.com $
 * $Date: 2015.04.28 $
 * $Id: $
 */

#ifndef CMESSAGESOCK_HPP_
#define CMESSAGESOCK_HPP_

#include <string>
#include <vector>

#include "CSocket.hpp"
#include "CProtocol.hpp"


class CProtocolSock
{
public:
    CProtocol::EN_ERROR m_enError;
    
public:
	CProtocolSock(const CSocket& a_cSock);
	CProtocolSock(const std::string& a_strIP);
	~CProtocolSock();

	bool	Send(CProtocol& a_cMesg);
    
	size_t	Recv(CProtocol& a_cMesg);

	//int		FindProcId(const char* szProcName);

private:
    CSocket*	m_pcSocket;
	std::string	m_strIP; 
    
    std::vector<char> m_vecBuff;
};

#endif // CMESSAGESOCK_HPP_
