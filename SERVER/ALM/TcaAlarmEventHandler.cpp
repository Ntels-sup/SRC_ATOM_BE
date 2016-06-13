
#include "CommonCode.hpp"
#include "TcaAlarmEventHandler.hpp"
#include "AlarmCFG.hpp"

#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;
extern AlarmCFG     gCFG;

TcaAlarmEventHandler::
TcaAlarmEventHandler()
{
    alarm_h_    = NULL;
    buffer_.resize(gCFG.DEF.buffer_size_);
}

TcaAlarmEventHandler::
~TcaAlarmEventHandler()
{
    // Empty
}

bool TcaAlarmEventHandler::
Init(DB * _db, AlarmEventHandler * _alarm_h)
{
    alarm_h_ = _alarm_h;

    if(alarm_code_conf_.SetAlarmCode(_db,
                                       gCFG.SVC.tca_over_code_) != true)
    {
        gAlmLog->ERROR("%-24s| Init - set tca over alarm code read fail",
            "TcaAlarmEventHandler");
        return false;
    }

    if(alarm_code_conf_.IsExist(gCFG.SVC.tca_over_code_) != true)
    {
        gAlmLog->ERROR("%-24s| Init - set tca over alarm code is not exist",
            "TcaAlarmEventHandler");
        return false;
    }

    if(alarm_code_conf_.SetAlarmCode(_db,
                                       gCFG.SVC.tca_under_code_) != true)
    {
        gAlmLog->ERROR("%-24s| Init - set tca under alarm code read fail",
            "TcaAlarmEventHandler");
        return false;
    }

    if(alarm_code_conf_.IsExist(gCFG.SVC.tca_under_code_) != true)
    {
        gAlmLog->ERROR("%-24s| Init - set tca under alarm code is not exist",
            "TcaAlarmEventHandler");
        return false;
    }

    gAlmLog->INFO("%-24s| Init - SUCCESS",
        "TcaAlarmEventHandler");

    return true;
}

void TcaAlarmEventHandler::
Clear()
{
    buffer_.clear();
}

bool TcaAlarmEventHandler::
DecodingJSON()
{
    using namespace rapidjson;

    try {

        stAlarmEventMsg_.Clear();

        rabbit::document    doc;
        doc.parse(buffer_);

        rabbit::object      body = doc["BODY"];

        stAlarmEventMsg_.node_no_      =   body["node_no"].as_int();
        // stAlarmEventMsg_.node_name_
        stAlarmEventMsg_.severity_id_  =   body["severity_id"].as_int();

        stAlarmEventMsg_.pkg_name_        = body["pkg_name"].as_string();

        // stAlarmEventMsg_.code_            = body["code"].as_string();
        // stAlarmEventMsg_.event_type_id_
        // stAlarmEventMsg_.event_group_id_
        // stAlarmEventMsg_.alias_code_      = stAlarmEventMsg_.code_;
        // stAlarmEventMsg_.probable_cause_  = body["probable_cause"].as_string();

        stAlarmEventMsg_.additional_text_ = body["additional_text"].as_string();
        stAlarmEventMsg_.location_        = "/ATOM/RSA";
        stAlarmEventMsg_.target_          = body["target"].as_string();
        stAlarmEventMsg_.complement_      = body["complement"].as_string();
        stAlarmEventMsg_.value_           = body["value"].as_string();
        // stAlarmEventMsg_.node_version_    ;
        // stAlarmEventMsg_.node_type_       ;
        stAlarmEventMsg_.prc_date_        = body["prc_date"].as_string();
        stAlarmEventMsg_.dst_yn_          = body["dst_yn"].as_string();


    } catch(rabbit::type_mismatch &  e) {

        gAlmLog->WARNING("%-24s| DecodingJSON - type mismatch [%s] [%s]",
            "TcaAlarmEventHandler",
            e.what(),
            buffer_.c_str());
        return false;
    } catch(rabbit::parse_error & e) {

        gAlmLog->WARNING("%-24s| DecodingJSON - parse error [%s] [%s]",
            "TcaAlarmEventHandler",
            e.what(),
            buffer_.c_str());
        return false;
    } catch(...) {
        gAlmLog->WARNING("%-24s| DecodingJSON - can't decoding [%s]",
            "TcaAlarmEventHandler",
            buffer_.c_str());
        return false;
    }

    if(stAlarmEventMsg_.complement_.compare("OVER") == 0)
    {
        stAlarmEventMsg_.code_          = gCFG.SVC.tca_over_code_;

        ST_AlarmCodeDef * def =   alarm_code_conf_.GetAlarmCodeDef(gCFG.SVC.tca_over_code_);

        if(def == NULL)
        {
            gAlmLog->WARNING("%-24s| DecodingJSON - undefined alarm code [%s]",
                "TcaAlarmEventHandler",
                stAlarmEventMsg_.code_.c_str());

            return false;
        }

        stAlarmEventMsg_.event_type_id_ = def->event_type_id_;
        stAlarmEventMsg_.event_group_id_= def->event_group_id_;
        stAlarmEventMsg_.alias_code_    = def->alias_code_;
        stAlarmEventMsg_.probable_cause_= def->probable_cause_;
    }
    else
    {
        stAlarmEventMsg_.code_          =  gCFG.SVC.tca_under_code_;

        ST_AlarmCodeDef * def =   alarm_code_conf_.GetAlarmCodeDef(gCFG.SVC.tca_under_code_);

        if(def == NULL)
        {
            gAlmLog->WARNING("%-24s| DecodingJSON - undefined alarm code [%s]",
                "TcaAlarmEventHandler",
                stAlarmEventMsg_.code_.c_str());

            return false;
        }

        stAlarmEventMsg_.event_type_id_ = def->event_type_id_;
        stAlarmEventMsg_.event_group_id_= def->event_group_id_;
        stAlarmEventMsg_.alias_code_    = def->alias_code_;
        stAlarmEventMsg_.probable_cause_= def->probable_cause_;
    }

    // stAlarmEventMsg_.manual_yn_         = "Y";

    return true;
}

// IF 를 맞추기 위함.
bool TcaAlarmEventHandler::
IsValidSequenceId(SequenceByNode & _sequence_by_node)
{
    return true;
}

bool TcaAlarmEventHandler::
Do(NodeManagerIF & _nm_if, DB * _db)
{
    alarm_h_->SetAlarmEventMsg(stAlarmEventMsg_);
    if(alarm_h_->Do(_nm_if, _db) == false)
        return false;

    return true;
}
