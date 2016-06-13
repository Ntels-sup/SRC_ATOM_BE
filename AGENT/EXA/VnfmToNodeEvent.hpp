#ifndef __VNFM_TO_NODE_EVENT_HPP__
#define __VNFM_TO_NODE_EVENT_HPP__

#include "CommonCode.hpp"
#include "NodeData.hpp"
#include "HttpData.hpp"

#include "DashBoard.hpp"

#include "ExaCommonDef.hpp"

class VnfmToNodeEvent
{
public:
    VnfmToNodeEvent();
    ~VnfmToNodeEvent();

    bool        Init();
    NodeData &  Receive(HttpData & a_rHttpData, DashBoard & a_board);
    void        ReceiveResponse(HttpData & a_rHttpData, DashBoard & a_board);
    NodeData &  Request(ST_Profile & a_profile, DashBoard & a_board);
    NodeData &  MakeStart(DashBoard & a_board);

    const vnfm::eStatusCode   GetStatusCode();
    const std::string &       GetStatusMsg();

private:
    vnfm::eStatusCode   start(HttpData & m_rHttpData,   DashBoard & a_board);
    vnfm::eStatusCode   stop(HttpData & m_rHttpData,    DashBoard & a_board);
    vnfm::eStatusCode   install(HttpData & m_rHttpData, DashBoard & a_board);
    vnfm::eStatusCode   lifecycle(HttpData & m_rHttpData, DashBoard & a_board);
    vnfm::eStatusCode   unknown(HttpData & m_rHttpData);
    vnfm::eStatusCode   keepAlive(HttpData & m_rHttpData);

    void                getPrcDateAndDstYn(std::string & a_prcDate,
                                           std::string & a_dstYn);

private:
    NodeData            m_sNodeData;
    vnfm::eStatusCode   m_eStatusCode;
};

#endif // __VNFM_TO_NODE_EVENT_HPP__