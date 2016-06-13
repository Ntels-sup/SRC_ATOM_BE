/* vim:ts=4:sw=4
 */
/**
 * \file	CRawMessage.hpp
 * \brief	실시간 message를 목적지 주소에 따른 경로 배분, 전달
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#ifndef CPROTOCOL_HPP_
#define CPROTOCOL_HPP_

#include <stdint.h>
#include <cstring>
#include <string>
#include <vector>
#include <map>

#include "CFileLog.hpp"


class CProtocol
{
public:
	// HEAD 구조 ----------------------------------------------------
	struct ST_HEAD {
		char	m_cVer;
		char	m_szCmd[10];
		char	m_cFlag;
		int		m_nSrcNode;
		int		m_nSrcProc;
		int		m_nDstNode;
		int		m_nDstProc;
		int		m_nSeq;
		int		m_nLength;
	} __attribute__ ((packed));

	// HEAD.flag 필드 값 정의
	static const char FLAG_REQUEST	= 0x10;
	static const char FLAG_RESPONSE	= 0x20;
	static const char FLAG_NOTIFY	= 0x40;
	static const char FLAG_RETRNS	= 0x01;
	static const char FLAG_BROAD	= 0x02;
	static const char FLAG_ERROR	= 0x04;
    
    static const int MAX_LENGTH		= (1024 * 500); // 500KB

	std::string m_strErrorMsg;

public:
	CProtocol();
	CProtocol(const CProtocol& a_clsProto);
	~CProtocol() {}

	// Message 값 설정
	void		SetCommand(const char* a_szCmd);
	void		SetCommand(int a_nCmd);
	
	void		SetFlagRequest();
	void		SetFlagResponse(void);
	void		SetFlagNotify(void);
	void		SetFlagRetransmit(void);
	void		SetFlagBroadcast(void);
	void		SetFlagError(void);
	
	void		SetSource(int a_nNodeNo, int a_nProcNo);
	void		SetSource(std::pair<int, int> a_parSrc);
	
	void		SetDestination(int a_nNodeNo, int a_nProcNo);
	void		SetDestination(std::pair<int, int> a_parDst);
	
	int			SetSequence(int a_nSequence = -1);
	
	void		SetPayload(const std::vector<char>& a_vecPay);
	void		SetPayload(const std::string& a_strPay);
	void		SetPayload(const char* a_szPay, int a_nLen);
    
	void		SetResponse(const CProtocol& a_clsProto);
	bool        SetAll(const char* a_pMsg, int a_nMsgSize);

	
	// Message 값 획득
	char		GetVersion(void);

	void		GetCommand(std::string& a_strCmd);
	std::string	GetCommand(void);
	
	char		GetFlag(void);
	
	void		GetSource(int& a_nNodeNo, int& a_nProcNo);
	std::pair<int, int> GetSource(void);
	
	void		GetDestination(int& a_nNodeNo, int& n_nProcNo);
	std::pair<int, int> GetDestination(void);
	
	int			GetSequence(void) { return m_stHead.m_nSeq; }

	int			GetLength(void) { return m_stHead.m_nLength; }
	
	int			GetPayload(std::vector<char>& a_strPay);
	std::string	GetPayload(void);
	
	bool		IsFlagRequest(void) {
					return (m_stHead.m_cFlag & FLAG_REQUEST) ? true : false;
				}
	bool		IsFlagResponse(void) {
					return (m_stHead.m_cFlag & FLAG_RESPONSE) ? true : false;
				}
	bool		IsFlagNotify(void) {
					return (m_stHead.m_cFlag & FLAG_NOTIFY) ? true : false;
				}
	bool		IsFlagRetransmit(void) {
					return (m_stHead.m_cFlag & FLAG_RETRNS) ? true : false;
				}
	bool		IsFlagBroadcast(void) {
					return (m_stHead.m_cFlag & FLAG_BROAD) ? true : false;
				}
	bool		IsFlagError(void) {
					return (m_stHead.m_cFlag & FLAG_ERROR) ? true : false;
				}

	void		MyAddress(int a_nNodeNo, int a_nProcNo);
	void		GenStream(std::vector<char>& a_vecMesg);
	inline void	Clear(void) { 
					memset(&m_stHead, 0x00, sizeof(m_stHead));
					m_stHead.m_cVer = 0x01;
					m_vecBody.clear();
				}

	void		Print(CFileLog* a_pclsLog, int a_nLevel, bool a_bPayPrint = false);

protected:
	ST_HEAD		m_stHead;
	std::vector<char> m_vecBody;
	
private:
	int			m_nSequence;
};

#endif // CMESSAGE_HPP_
