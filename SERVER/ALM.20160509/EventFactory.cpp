
#include "EventFactory.hpp"
#include "AlarmCFG.hpp"

#include "CommandFormat.hpp"

#include "CFileLog.hpp"

extern CFileLog *  gAlmLog;

EventFactory::
EventFactory()
{
    // Empty
}

EventFactory::
~EventFactory()
{
    // Empty
}

bool EventFactory::
Init(DB * _db)
{
    if(subscribe_h_.Init(_db) == false)
        return false;

    if(alarm_h_.Init(_db, &subscribe_h_) == false)
        return false;

    if(node_status_h_.Init(_db, &alarm_h_) == false)
        return false;

    if(user_change_h_.Init(&alarm_h_) == false)
        return false;

    if(tca_h_.Init(_db, &alarm_h_) == false)
        return false;

    if(retry_h_.Init(&alarm_h_,
                    &subscribe_h_) == false)
        return false;

    if(alarm_sync_h_.Init() == false)
        return false;

    return true;
}

EventHandler & EventFactory::
Find(int _cmd_id)
{
    switch(_cmd_id)
    {
    case CMD_ALM_EVENT:             return alarm_h_;
    case CMD_ALM_TCA_EVENT:         return tca_h_;
    case CMD_STATUS_NODE_EVENT:     return node_status_h_;
    case CMD_ALM_SUBSCRIBE_EVENT:   return subscribe_h_;
    case CMD_ALM_USER_CHANGE:       return user_change_h_;
    case CMD_ALM_RETRY_RESP:        return retry_h_;
    case CMD_ALM_SYNC_REQ:          return alarm_sync_h_;
    default:                        return unknown_h_;
    }
}

