
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>

#include "DashBoard.hpp"

extern CFileLog *   gLog;


void ST_VNF_CFG::
Clear()
{
    m_cVnfmServiceURL.clear();
    m_cVnfmPerformanceURL.clear();
    m_cMyURL.clear();
    m_cMyIP.clear();
    m_nMyPort = 9000;
    m_cVnfName.clear();
    m_cVnfType.clear();
    m_cMetaDataURL.clear();

    m_cInternalIP.clear();
    m_cManagementIP.clear();
}

void ST_VNF_ID::
Clear()
{
    m_cUUID.clear();
    m_cSubscriberId.clear();
}

void ST_Resource::
Clear()
{
    m_timestamp.clear();
    m_dst_yn.clear();

    m_cpu.clear();
    m_mem.clear();

    m_disk.clear();
    m_network.clear();
}

DashBoard::
DashBoard()
{
    m_vnfConfig.Clear();
    m_vnfId.Clear();

    m_tpsPrcTime.clear();
    m_mapTps.clear();
    m_stResource.Clear();
}

DashBoard::
~DashBoard()
{
    // Empty
}


bool DashBoard::
Init()
{
    // TO DO : 설정으로 변경 필요함.
    std::string     path = "/etc";
    std::string     name = "vnf_env.properties";

    if(readVnfProperties(path, name) == false)
    {
        gLog->WARNING("%-24s| Init - read Vnf Properties fail",
            "DashBoard");

        return false;
    }

    /*--
    if(readUUID(m_vnfConfig.m_cMetaDataURL.c_str()) == false)
    {
        gLog->WARNING("%-24s| Init - read UUID fail",
            "DashBoard");
        return false;
    }
    --*/

    if(setMyURI(m_vnfConfig.m_cManagementIP, VNF_LISTEN_PORT) == false)
    {
        gLog->ERROR("%-24s| Init - can't set my url",
            "DashBoard");

        return false;
    }

    return true;
}

/*---
bool DashBoard::
readUUID(const char * a_pcURI)
{
    // default
    m_vnfId.m_cUUID = "52fe8307-4223-4918-a933-072cb76700c0";

    char cmd[URI_MAX_SIZE];
    char line[2048];

    sprintf(cmd, "curl %s", a_pcURI);

    FILE * fp = popen(cmd, "r");

    if(fp == NULL)
    {
        gLog->WARNING("%-24s| readUUID - can't read UUID [cmd:%s]",
            "DashBoard",
            cmd);
        return true;
    }

    while(true)
    {
        if(fgets(line, sizeof(line), fp) == NULL)
            break;
    }

    pclose(fp);

    // JSON Parsing
    try {
        rabbit::document    doc;
        doc.parse(line);

        m_vnfId.m_cUUID = doc["uuid"].as_string();
    } catch(...) {

        gLog->WARNING("%-24s| readUUID - can't read uuid [%s]",
            "DashBoard",
            line);
    }

    gLog->INFO("%-24s| readUUID - uuid [%s]",
        "DashBoard",
        m_vnfId.m_cUUID.c_str());

    return true;
}
--*/

bool DashBoard::
readVnfProperties(std::string & a_path, std::string & a_file)
{

    std::string     full_name = a_path + "/" + a_file;

    FILE * fp = fopen(full_name.c_str(), "r");

    if(fp == NULL)
    {
        gLog->WARNING("%-24s| readVnfProperties - file open fail [%s/%s] [%d:%s]",
            "DashBoard",
            a_path.c_str(),
            a_file.c_str(),
            errno,
            strerror(errno));
        return false;
    }

    char            line[512];
    std::string     out;

    while(true)
    {
        memset(line, 0, sizeof(line));
        if(fgets(line, sizeof(line), fp) <= 0)
            break;

        line[strlen(line)-1] = '\0';
        out.clear();

        if(extractRightValue(out, line, "vnfm_service_endpoint") == true)
            m_vnfConfig.m_cVnfmServiceURL       = out;
        else if(extractRightValue(out, line, "vnfm_performance_endpoint") == true)
            m_vnfConfig.m_cVnfmPerformanceURL   = out;
        else if(extractRightValue(out, line, "vnf_name") == true)
            m_vnfConfig.m_cVnfName              = out;
        else if(extractRightValue(out, line, "vnfc_type") == true)
            m_vnfConfig.m_cVnfType              = out;
        else if(extractRightValue(out, line, "meta_data_url") == true)
            m_vnfConfig.m_cMetaDataURL          = out;
        else if(extractRightValue(out, line, "use_internal") == true)
            setIPByInterfaceName(m_vnfConfig.m_cInternalIP, out);
        else if(extractRightValue(out, line, "use_management") == true)
            setIPByInterfaceName(m_vnfConfig.m_cManagementIP, out);

        // else if(extractRightValue(out, line, "interface_list") == true)
        //    setInterfaceInfo(out, a_path);
    }

    fclose(fp);

    if(m_vnfConfig.m_cVnfmServiceURL.empty() ||
       m_vnfConfig.m_cVnfmPerformanceURL.empty() ||
       m_vnfConfig.m_cVnfName.empty() ||
       m_vnfConfig.m_cVnfType.empty())
    {
        gLog->WARNING("%-24s| readVnfProperties - file read fail [%s/%s]",
            "DashBoard",
             a_path.c_str(),
             a_file.c_str());
        return false;
    }

    if(m_vnfConfig.m_cMetaDataURL.empty())
    {
        m_vnfConfig.m_cMetaDataURL = "curl http://169.254.169.254/openstack/latest/meta_data.json";
    }

    gLog->INFO("%-24s| readVnfProperties - vnfm_service_endpoint [%s]",
        "DashBoard",
        m_vnfConfig.m_cVnfmServiceURL.c_str());
    gLog->INFO("%-24s| readVnfProperties - vnfm_performance_endpoint [%s]",
        "DashBoard",
        m_vnfConfig.m_cVnfmPerformanceURL.c_str());
    gLog->INFO("%-24s| readVnfProperties - vnfc_name [%s]",
        "DashBoard",
        m_vnfConfig.m_cVnfName.c_str());
    gLog->INFO("%-24s| readVnfProperties - vnf_type [%s]",
        "DashBoard",
        m_vnfConfig.m_cVnfType.c_str());
    gLog->INFO("%-24s| readVnfProperties - internal ip [%s]",
        "DashBoard",
        m_vnfConfig.m_cInternalIP.c_str());
    gLog->INFO("%-24s| readVnfProperties - management ip [%s]",
        "DashBoard",
        m_vnfConfig.m_cManagementIP.c_str());

    return true;
}

bool DashBoard::
extractRightValue(std::string & a_out, char * a_pStrLine, const char * a_pStrLeftValue)
{
    std::string line = a_pStrLine;
    std::string left = line.substr(0, line.find("="));

    if(left.find(a_pStrLeftValue) == std::string::npos)
        return false;

    left.erase(0, left.find_first_not_of(" \t\n\r\f\v"));
    left.erase(left.find_last_not_of(" \t\n\r\f\v")+1);

    if(left.size() != strlen(a_pStrLeftValue))
        return false;

    a_out = line.substr(line.find("=")+1, line.size());

    a_out.erase(0, a_out.find_first_not_of(" \t\n\r\f\v"));
    a_out.erase(a_out.find_last_not_of(" \t\n\r\f\v")+1);

    // gLog->DEBUG("----[%s] [%s] [%s]", a_pStrLine, a_pStrLeftValue, a_out.c_str());
    return true;
}

void DashBoard::
setIPByInterfaceName(std::string & a_out, std::string & a_interface_name)
{
    struct ifaddrs * ifAddr;
    struct ifaddrs * ifIter;

    int     ret;
    char    host[NI_MAXHOST];

    a_out.clear();

    if(a_interface_name.find_first_of(", \t\n\r\f\v") != std::string::npos)
        a_interface_name.erase(a_interface_name.find_first_of(", \t\n\r\f\v"));

    if (getifaddrs(&ifAddr) == -1)
    {
        gLog->WARNING("%-24s| setIPByInterfaceName - getifaddrs fail",
            "DashBoard");

        a_out = "127.0.0.1";
        return ;
    }

    for (ifIter = ifAddr; ifIter != NULL; ifIter = ifIter->ifa_next)
    {
        if (ifIter->ifa_addr == NULL)
            continue;

        memset(host, 0, sizeof(host));
        ret = getnameinfo(ifIter->ifa_addr,
                          sizeof(struct sockaddr_in),
                          host,
                          NI_MAXHOST,
                          NULL,
                          0,
                          NI_NUMERICHOST);

        if(ifIter->ifa_addr->sa_family == AF_INET)
        {
            if (ret != 0)
                continue;

            if(strcasecmp(ifIter->ifa_name, a_interface_name.c_str()) == 0 &&
               (strlen(ifIter->ifa_name) == strlen(a_interface_name.c_str())) )
            {
                a_out = host;
                break;
            }
        }
    }

    if(a_out.size() == 0)
    {
        gLog->WARNING("%-24s| setIPByInterfaceName - fail",
				"DashBoard");
        a_out = "127.0.0.1";
    }

    gLog->DEBUG("%-24s| setIPByInterfaceName - interface [%s] ip [%s]",
        "DashBoard",
        a_interface_name.c_str(),
        a_out.c_str());

}

bool DashBoard::
setMyURI(std::string & a_ip, int a_nPort)
{
    if(a_ip.size() <= 0)
    {
        gLog->ERROR("%-24s| setMyURI - ip is null",
            "DashBoard");
        return false;
    }

    char    tempUri[URI_MAX_SIZE];
    sprintf(tempUri, "http://%s:%d", a_ip.c_str(), a_nPort);

    m_vnfConfig.m_cMyURL = tempUri;
    m_vnfConfig.m_cMyIP  = a_ip;

    return true;
}


/*----
bool DashBoard::
setMyURI(const char * a_pcPrefixIP, int a_nPort)
{
    struct ifaddrs * ifAddr;
    struct ifaddrs * ifIter;

    int     ret;
    char    host[NI_MAXHOST];

    if (getifaddrs(&ifAddr) == -1)
    {
        gLog->WARNING("getifaddrs fail");
        return false;
    }

    for (ifIter = ifAddr; ifIter != NULL; ifIter = ifIter->ifa_next)
    {
        if (ifIter->ifa_addr == NULL)
            continue;

        memset(host, 0, sizeof(host));
        ret = getnameinfo(ifIter->ifa_addr,
                          sizeof(struct sockaddr_in),
                          host,
                          NI_MAXHOST,
                          NULL,
                          0,
                          NI_NUMERICHOST);

        if(ifIter->ifa_addr->sa_family == AF_INET)
        {
            if (ret != 0)
            {
                gLog->WARNING("getnameinfo failed");
                continue;
            }

            gLog->INFO("host [%s] prefix [%s]", host, a_pcPrefixIP);
            if(strncasecmp(host, a_pcPrefixIP, strlen(a_pcPrefixIP)) == 0)
                break;
        }
    }

    if(strlen(host) == 0)
    {
        gLog->WARNING("setMyURI - fail");
        strcpy(host, "127.0.0.1");
    }

    char    tempUri[URI_MAX_SIZE];
    sprintf(tempUri, "http://%s:%d", host, a_nPort);

    m_vnfConfig.m_cMyURL = tempUri;
    m_vnfConfig.m_cMyIP  = host;

    return true;
}
---*/

std::string & DashBoard::
GetResourceGatheringTime()
{
    return m_stResource.m_timestamp;
}

// 같은 이름끼리 모아서 Reporting 해야 합니다.

void DashBoard::
GetTPSTitles(std::vector<std::string> & _vec_tps_titles)
{
    _vec_tps_titles.clear();

    for(auto iter = m_mapTps.begin(); iter != m_mapTps.end(); ++iter)
        _vec_tps_titles.push_back(iter->first);
}


std::string DashBoard::
GetTPS(std::string & _title)
{
    unsigned long long ret = 0;

    auto iter = m_mapTps.find(_title);

    if(iter == m_mapTps.end())
        return "0";
    else
    {
        ret = iter->second;
        iter->second = 0;
    }

    return std::to_string(ret);
}

std::string & DashBoard::
GetCPU(int _unit)
{
    auto pos = m_stResource.m_cpu.find(".");

    if(pos == std::string::npos)
        return m_stResource.m_cpu;

    m_stResource.m_cpu = m_stResource.m_cpu.substr(0, pos+_unit);

    return m_stResource.m_cpu;
}

std::string & DashBoard::
GetMEM(int _unit)
{
    auto pos = m_stResource.m_mem.find(".");

    if(pos == std::string::npos)
        return m_stResource.m_mem;

    m_stResource.m_mem = m_stResource.m_mem.substr(0, pos+_unit);

    return m_stResource.m_mem;
}

void DashBoard::
GetDiskNames(std::vector<std::string> & _vec_disk_names)
{
    _vec_disk_names.clear();

    for(auto iter  = m_stResource.m_disk.begin();
             iter != m_stResource.m_disk.end();
             ++iter)
    {
        _vec_disk_names.push_back(iter->first);
    }
}

void DashBoard::
GetDISK(std::string & _name, std::string & _value)
{
    auto iter = m_stResource.m_disk.find(_name);

    if(iter != m_stResource.m_disk.end())
        _value = iter->second;
    else
        _value = "0";
}

void DashBoard::
GetNetInterfaceNames(std::vector<std::string> & _vec_interface_names)
{
    _vec_interface_names.clear();

    for(auto iter = m_stResource.m_network.begin();
             iter != m_stResource.m_network.end();
             ++iter)
    {
        _vec_interface_names.push_back(iter->first);
    }
}

std::string & DashBoard::
GetRX(std::string & _interface_name)
{
    return m_stResource.m_network[_interface_name].rx_;
}

std::string & DashBoard::
GetTX(std::string & _interface_name)
{
    return m_stResource.m_network[_interface_name].tx_;
}


const char * DashBoard::
GetUUID()
{
    return m_vnfId.m_cUUID.c_str();
}

const char * DashBoard::
GetSubscriberId()
{
    return m_vnfId.m_cSubscriberId.c_str();
}

const char * DashBoard::
GetVnfmServiceURL()
{
    return m_vnfConfig.m_cVnfmServiceURL.c_str();
}

const char * DashBoard::
GetVnfmPerformanceURL()
{
    return m_vnfConfig.m_cVnfmPerformanceURL.c_str();
}

const char * DashBoard::
GetMyURL()
{
    return m_vnfConfig.m_cMyURL.c_str();
}

const char * DashBoard::
GetMyIP()
{
    return m_vnfConfig.m_cMyIP.c_str();
}


std::string & DashBoard::
GetInternalInterfaceIP()
{
    // return m_vnfConfig.m_internalInterface.m_ip;

    return m_vnfConfig.m_cInternalIP;
}

void DashBoard::
SetUUID(std::string & _uuid)
{
    m_vnfId.m_cUUID     = _uuid;
}

void DashBoard::
SetTPS(std::string & a_prcTime, std::string & a_tpsTitle, int a_nTps)
{
    m_tpsPrcTime    = a_prcTime;

    auto iter       = m_mapTps.find(a_tpsTitle);

    if(iter == m_mapTps.end())
        m_mapTps[a_tpsTitle] = a_nTps;
    else
        iter->second += a_nTps;
}

void DashBoard::
SetCPU(std::string & _usage)
{
    m_stResource.m_cpu          = _usage;
}

void DashBoard::
SetMEM(std::string & _usage)
{
    m_stResource.m_mem          = _usage;
}

void DashBoard::
SetDISK(std::string & _mount_on,  std::string & _usage)
{
    m_stResource.m_disk[_mount_on] = _usage;
}

void DashBoard::
SetNET(std::string  & _if_name,
       std::string  & _rx,
       std::string  & _tx)
{
    m_stResource.m_network[_if_name].rx_ = _rx;
    m_stResource.m_network[_if_name].tx_ = _tx;
}

void DashBoard::
SetTIME(std::string & _timestamp,   std::string & _dst_yn)
{
    m_stResource.m_timestamp    = _timestamp;
    m_stResource.m_dst_yn       = _dst_yn;
}


void DashBoard::
SetSubscriberId(std::string & a_id)
{
    m_vnfId.m_cSubscriberId     = a_id;
}


bool DashBoard::
IsValidUUID(const char * a_pcUUID)
{
    if(m_vnfId.m_cUUID.size() <= 0)
        return true;

    return (m_vnfId.m_cUUID.compare(a_pcUUID) == 0);
}

bool DashBoard::
IsValidSubscriberId(const char * a_pcSubscriberId)
{
    return (m_vnfId.m_cSubscriberId.compare(a_pcSubscriberId) == 0);
}
