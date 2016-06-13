
#ifndef __ALARM_CFG_HPP__
#define __ALARM_CFG_HPP__

#include <string>

#include "DebugMacro.hpp"
#include "CommonCode.hpp"

#include "DB.hpp"

// Adapter Pattern
class AlarmCFG
{
public:

    AlarmCFG();
    ~AlarmCFG();

    bool Init();

    bool InitLog(std::string & _log_path,
                 std::string & _log_file,
                 bool          _is_debug);
    bool SetProfile(DB * _db);

private:
    void prnLog();

public:
    struct ST_DB {
        std::string     ip_;
        int             port_;
        std::string     user_;
        std::string     pw_;
        std::string     name_;
    } DBInfo ;

    struct ST_NM {
        std::string     ip_;
        int             port_;
        int             retry_cnt_;

        int             wsm_node_no_;
        int             wsm_proc_no_;

        int             nm_node_no_;
        int             nm_proc_no_;
    } NM ;

    struct ST_DEF {
        int             buffer_size_;

        std::string     ccd_for_protocol_;
    } DEF;

    struct ST_ENV {

        std::string     log_path_;
        std::string     file_name_;

        std::string     message_id_path_;
    } ENV;

    struct ST_SVC {
        bool            b_avoide_duplicate_;
        std::string     tca_over_code_;
        std::string     tca_under_code_;

        int             trap_type_;


    } SVC;

    ST_Profile      profile_;

};


#endif // __ALARM_CFG_HPP__