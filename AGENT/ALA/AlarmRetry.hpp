
#ifndef __ALARM_RETRY_HPP__
#define __ALARM_RETRY_HPP__

#include <string>

#include "AlaInterface.hpp"
#include "AuditData.hpp"

class AlarmRetry
{
public:
    explicit AlarmRetry();
    ~AlarmRetry();

    bool Init(int           _node_no,
              std::string & _node_name,
              int           _proc_no);

    bool IsContinues();
    bool Request(std::string & _body, AuditData & _audit);
    bool Response(AlaInterface & _interface, AuditData & _audit);

private:
    void retryOn(unsigned int _id);
    void retryOff();

private:
    int             my_node_no_;
    std::string     my_node_name_;
    int             my_proc_no_;
    int             max_cnt_;

    bool            is_on_;
    unsigned int    req_seq_id_;
    unsigned int    now_seq_id_;

    std::vector<std::pair<unsigned int, std::string> >  vec_msg_;
};

#endif // __ALARM_RETRY_HPP__



