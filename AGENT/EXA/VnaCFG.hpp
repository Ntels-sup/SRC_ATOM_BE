
#ifndef __VNA_CFG_HPP__
#define __VNA_CFG_HPP__

#include <string>

#include "DB.hpp"

class VnaCFG
{
public:

    VnaCFG();
    ~VnaCFG();

    bool    Init(char * _cfg_fname,
                 DB   * _db,
                 std::string & _pkg_name,
                 std::string & _node_type);

    bool    InitLog(std::string & _log_path,
                    std::string & _log_file,
                    bool _is_debug);

private:
    void    getDBConfig(int        & _out,
                        DB         * _db,
                        const char * _section,
                        const char * _tag);
    void    prnLog();

public:

    std::string         my_pkg_name_;
    std::string         my_node_type_;

    struct ST_ENV {

        std::string     log_path_;
        std::string     file_name_;

    } ENV;

    struct ST_SVC {

        int     gathering_period_;
        int     tps_period_;
        int     cpu_period_;
        int     mem_period_;
        int     disk_period_;
        int     net_period_;
    } SVC;

};

#endif // __VNA_CFG_HPP__
