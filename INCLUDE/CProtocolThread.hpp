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

#ifndef CMESSAGETHREAD_HPP_
#define CMESSAGETHREAD_HPP_

#include <vector>
#include <pthread.h>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>

#include "CProtocol.hpp"


class CProtocolThread
{
public:
	CProtocol::EN_ERROR m_enErrorCd;
    
public:
	CProtocolSock();
	~CProtocolSock();

	bool	Send(CProtocol& a_cMesg);
	int		Recv(CProtocol& a_cMesg);

private:
    typedef boost::lockfree::queue<CProtocol, boost::lockfree::fixed_size<true> > queue_t;
    std::map<std:string, queue_t> m_queChnnel;   // key: agent process name

    pthread_mutex_t	m_stLock;
};

#endif // CMESSAGETHREAD_HPP_
