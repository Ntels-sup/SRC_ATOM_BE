#ifndef __VNFM_IF_HPP__
#define __VNFM_IF_HPP__

#include <string>

#include "TimeOutCheck.hpp"
#include "DashBoard.hpp"
#include "NodeToVnfmEvent.hpp"
#include "VnfmToNodeEvent.hpp"
#include "NodeInterface.hpp"

#include "HttpClient.hpp"
#include "HttpServer.hpp"

#include "CommonCode.hpp"
#include "CModule.hpp"

class VnfmIF
{
public:
    VnfmIF();
    ~VnfmIF();

    bool Init(void * a_pArg);
    bool Do();
    void Final();

private:
    void setProfile(ST_Profile        & a_stProfile,
           CModule::ST_MODULE_OPTIONS & a_stOption);


private:

    bool                m_bInitialized;
    ST_Profile          m_profile;

    TimeOutCheck        m_timerTryInit;
    TimeOutCheck        m_timerForReportPerf;

    DashBoard           m_board;

    NodeToVnfmEvent     m_nodeToVnfmEvent;
    VnfmToNodeEvent     m_vnfmToNodeEvent;

    NodeInterface       m_nodeInterface;
    HttpClient *        m_pHttpClient;
    HttpServer *        m_pHttpServer;

    ContentsProvider    m_resourceAndTpsProvider;
};

#endif // __VNFM_IF_HPP__

