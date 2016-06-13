#ifndef __CCLAINTERFACE_HPP__
#define __CCLAINTERFACE_HPP__

#include "TimerEvent.hpp"
#include "CNMInterface.hpp"

class CClaInterface : public CNMInterface{
	private:
		string m_strUserId;
	public :
		CClaInterface() {};
		~CClaInterface() {};
		void SetUserId(string &a_strUserId) { m_strUserId = a_strUserId; };
		int RegReqFunc(CProtocol &cProto);
		int RegRspFunc(CProtocol &cProto);
		//int RegReqFunc();
		//int RegRspFunc();
};

#endif
