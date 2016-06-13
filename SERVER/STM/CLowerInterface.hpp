#ifndef __CLOWERINTERFACE_HPP__
#define __CLOWERINTERFACE_HPP__

#include "TimerEvent.hpp"
#include "CNMInterface.hpp"

class CLowerInterface : public CNMInterface{
    private:
        int m_nNmProcNo;
    public :
        CLowerInterface() {};
        ~CLowerInterface() {};
        void SetNmProcNo(int a_nNmProcNo) { m_nNmProcNo = a_nNmProcNo; };
        int RegReqFunc(CProtocol &cProto);
        int RegRspFunc(CProtocol &cProto);
};

#endif
