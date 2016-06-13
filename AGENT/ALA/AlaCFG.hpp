
#ifndef __ALA_CFG_HPP__
#define __ALA_CFG_HPP__

#include <string>

#include "DB.hpp"
#include "CommonCode.hpp"
#include "CModule.hpp"

class AlaCFG
{
public:
    AlaCFG();
    ~AlaCFG();

    bool Init(char * _cfg_fname = NULL);

    bool InitLog(std::string & _log_path,
                 std::string & _log_file,
                 bool          _is_debug);
    bool SetProfile(CModule::ST_MODULE_OPTIONS & _option);

private:
    void prnLog();

public:

    struct ST_ENV {

        std::string     log_path_;
        std::string     file_name_;

        std::string     message_id_path_;
    } ENV;

    struct ST_SVC {
        bool            b_avoide_duplicate_;
    } SVC;

    ST_Profile      profile_;

};


#endif // __ALA_CFG_HPP__
