
#ifndef __DASH_BOARD_HPP__
#define __DASH_BOARD_HPP__

#include <string.h>

#include <string>
#include <vector>
#include <unordered_map>

// #include "DebugMacro.hpp"
#include "ExaCommonDef.hpp"

#define     NAME_BUF_SIZE       32



struct ST_NETWORK {
    std::string     rx_;
    std::string     tx_;
};

/*--
struct ST_Interface {

    void    Clear();

    std::string     m_ip;
    std::string     m_name;
};
--*/

struct ST_VNF_CFG {

    void    Clear();

    std::string    m_cVnfmServiceURL;
    std::string    m_cVnfmPerformanceURL;
    std::string    m_cMyURL;
    std::string    m_cMyIP;
    int            m_nMyPort;
    std::string    m_cVnfName;
    std::string    m_cVnfType;
    std::string    m_cMetaDataURL;

    std::string    m_cInternalIP;
    std::string    m_cManagementIP;
};

struct ST_VNF_ID {
    void    Clear();

    std::string    m_cUUID;
    std::string    m_cSubscriberId;
};

struct ST_Resource {

    void    Clear();

    std::string     m_timestamp;
    std::string     m_dst_yn;

    std::string     m_cpu;
    std::string     m_mem;

    std::unordered_map<std::string, std::string>    m_disk;
    std::unordered_map<std::string, ST_NETWORK>     m_network;
};


// Get, Set 은 만들고 싶지 않았으나, 향후 많이 변경될 여지가 있는 부분이여서
// 멤버 함수로 쳐발쳐발 하였습니다.

class DashBoard
{
public:
    DashBoard();
    ~DashBoard();

    bool    Init();

    const char *    GetUUID();
    const char *    GetSubscriberId();
    const char *    GetVnfmServiceURL();
    const char *    GetVnfmPerformanceURL();
    const char *    GetMyURL();
    const char *    GetMyIP();
    std::string &   GetInternalInterfaceIP();

    void    SetUUID(std::string & _uuid);
    void    SetTPS(std::string & a_prcTime, std::string & a_title, int a_nTps);
    void    SetCPU(std::string & _usage);
    void    SetMEM(std::string & _usage);
    void    SetDISK(std::string & _mount_on,  std::string & _usage);
    void    SetNET(std::string & _if_name, std::string & _rx, std::string & _tx);
    void    SetTIME(std::string & _timestamp, std::string & _dst_yn);

    std::string & GetResourceGatheringTime();
    void          GetTPSTitles(std::vector<std::string> & _vec_tps_titles);
    std::string   GetTPS(std::string & _tps_title);
    std::string & GetCPU(int _unit);
    std::string & GetMEM(int _unit);
    void          GetDiskNames(std::vector<std::string> & _vec_disk_names);
    void          GetDISK(std::string & _name, std::string & _value);

    void          GetNetInterfaceNames(std::vector<std::string> & _vec_interface_names);
    std::string & GetRX(std::string & _interface_name);
    std::string & GetTX(std::string & _interface_name);


    void    SetSubscriberId(std::string & a_id);

    bool    IsValidUUID(const char * a_pcUUID);
    bool    IsValidSubscriberId(const char * a_pcSubscriberId);

private:
    bool    readVnfProperties(std::string & a_path, std::string & a_file);
    bool    setMyURI(std::string & a_ip, int a_nPort);

    bool    extractRightValue(std::string & a_out,
                              char * a_pStrLine,
                              const char * a_pStrLeftValue);
    void    setIPByInterfaceName(std::string & a_out,
                                 std::string & a_interface_name);


private:

    ST_VNF_CFG  m_vnfConfig; // /etc/vnf_env.properties
    ST_VNF_ID   m_vnfId;

    std::string                                 m_tpsPrcTime;
    std::unordered_map<std::string, unsigned long long>  m_mapTps;

    ST_Resource                                 m_stResource;


};


#endif // __DASH_BOARD_HPP__
