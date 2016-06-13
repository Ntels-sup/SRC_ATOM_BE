#ifndef __ALARM_DATA_DEF_HPP__
#define __ALARM_DATA_DEF_HPP__

#include <string>

#include "DB.hpp"
#include "CommonCode.hpp"

struct ST_ProtocolHeader
{
    void            Clear();

    char            version_;
    std::string     command_;
    char            flag_;
    int             src_node_;
    int             src_proc_;
    int             dest_node_;
    int             dest_proc_;
    int             seq_;
    int             length_;
};

struct ST_AlarmStatus;

struct ST_AlarmEventMsg
{
    void            Clear();
    ST_AlarmEventMsg & operator=(const ST_AlarmEventMsg & _stAlarmEventMsg);
    void            SetFromAlarmStatus(ST_AlarmStatus & _stAlarmStatus);

    std::string     message_;
    unsigned int    sequence_id_;
    int             node_no_;
    std::string     node_name_;
    int             proc_no_;
    int             severity_id_;

    std::string     pkg_name_;
    int             event_type_id_;
    int             event_group_id_;
    std::string     code_;
    std::string     alias_code_;
    std::string     probable_cause_;
    std::string     additional_text_;
    std::string     location_;
    std::string     target_;
    std::string     complement_;
    std::string     value_;
    std::string     node_version_;
    std::string     node_type_;
    std::string     prc_date_;
    std::string     dst_yn_;
    std::string     manual_yn_;
    std::string     vnfm_yn_;

    int             oper_no_;
};

struct ST_AlarmStatus
{
    void            Clear();
    ST_AlarmStatus & operator=(const ST_AlarmStatus & _stAlarmStatus);
    void            SetFromAlarmEventMsg(ST_AlarmEventMsg & _stAlarmEventMsg);

    bool            Insert(DB * _db);
    bool            Delete(DB * _db);
    static bool     Delete(DB * _db, unsigned int _msg_id);
    bool            CompareSeverity(int _severity_id);
    bool            IsCleared();
    unsigned int    GetMessageId() { return msg_id_; }

    unsigned int    msg_id_;

    int             node_no_;
    std::string     node_name_;
    int             proc_no_;
    int             severity_id_;

    std::string     pkg_name_;
    int             event_type_id_;
    int             event_group_id_;
    std::string     code_;
    std::string     alias_code_;
    std::string     probable_cause_;
    std::string     additional_text_;
    std::string     location_;
    std::string     target_;
    std::string     complement_;
    std::string     value_;
    std::string     node_version_;
    std::string     node_type_;
    std::string     prc_date_;
    std::string     lst_date_;
    std::string     confirm_yn_;
    std::string     manual_yn_;

    int             oper_no_;
    std::string     dst_yn_;
    unsigned int    associated_msg_id_;
};

struct ST_AlarmHistory
{
    void            Clear();
    bool            Insert(DB * _db);

    ST_AlarmStatus  stAlarmStatus_;

    std::string     cleared_date_;
    std::string     elapsed_time_;

    unsigned int    snmp_seq_id_;
    unsigned int    node_seq_id_;
    unsigned int    cleared_msg_id_;

    std::string     cleared_yn_;
    std::string     snmp_send_yn_;
    std::string     sms_send_yn_;
    std::string     email_send_yn_;

};

struct ST_Subscriber
{
    bool            Compare(ST_Subscriber & _stSubscriber);

    std::string     action_;
    std::string     pkg_name_;
    std::string     code_;
    int             severity_id_;
    int             node_no_;
    int             proc_no_;
};

struct ST_UserChangeMsg
{
    void            Clear();
    bool            IsFill();

    unsigned int    msg_id_;
    int             oper_no_;
    std::string     user_id_;
    int             severity_id_;
    std::string     prc_date_;
    std::string     dst_yn_;
};

struct ST_ProcStatusMsg
{
    void            Clear();

    unsigned int    sequence_id_;
    int             node_no_;

    std::string     worst_status_;
    int             proc_no_;
    std::string     proc_name_;
    std::string     proc_status_;

    std::string     prc_date_;
    std::string     dst_yn_;
};

struct ST_ConnectStatusMsg
{
    void            Clear();

    unsigned int    sequence_id_;
    int             node_no_;

    std::string     my_ip_;
    std::string     peer_ip_;
    int             server_port_;

    std::string     status_;
    std::string     prc_date_;
    std::string     dst_yn_;
};

struct ST_PingStatusMsg
{
    void            Clear();

    unsigned int    sequence_id_;

    int             node_no_;
    int             peer_no_;

    std::string     my_ip_;
    std::string     peer_ip_;

    std::string     status_;
    std::string     prc_date_;
    std::string     dst_yn_;
};

struct ST_VnfmSendDef
{
    void            Clear();

    std::string     code_;
    int             severity_id_;
    std::string     pkg_name_;
    std::string     alias_code_;
    int             alias_severity_id_;
    std::string     alias_probable_cause_;
    std::string     reboot_yn_;
};



#endif // __ALARM_DATA_DEF_HPP__
