#ifndef __ALARM_MANAGER_HPP__
#define __ALARM_MANAGER_HPP__

#include "DebugMacro.hpp"

#include "DB.hpp"
#include "NodeManagerIF.hpp"
#include "EventHandler.hpp"
#include "EventFactory.hpp"

#include "SequenceByNode.hpp"

class AlarmManager
{
public:
    AlarmManager();
    ~AlarmManager();

    bool        Init();
    void        Do();
    void        Final();

private:

    void        requestRetry(int _node_no, unsigned int _seq_id);

private:

    DB *                db_;

    SequenceByNode      sequence_by_node_;
    NodeManagerIF       nm_if_;

    EventFactory        event_factory_;

};


#endif // __ALARM_MANAGER_HPP__
