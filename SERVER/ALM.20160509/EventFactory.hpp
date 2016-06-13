#ifndef __EVENT_FACTORY_HPP__
#define __EVENT_FACTORY_HPP__

#include    "AlarmEventHandler.hpp"
#include    "NodeStatusEventHandler.hpp"
#include    "UserChangeEventHandler.hpp"
#include    "SubscribeEventHandler.hpp"
#include    "TcaAlarmEventHandler.hpp"
#include    "RetryEventHandler.hpp"
#include    "AlarmSyncEventHandler.hpp"
#include    "NodeStatusEventHandler.hpp"

#include    "UnknownEventHandler.hpp"

#include    "DB.hpp"

class EventFactory
{
public:
    EventFactory();
    ~EventFactory();

    bool    Init(DB * _db);
    EventHandler &  Find(int _cmd_id);

private:

    SubscribeEventHandler       subscribe_h_;
    AlarmEventHandler           alarm_h_;
    NodeStatusEventHandler      node_status_h_;
    UserChangeEventHandler      user_change_h_;
    TcaAlarmEventHandler        tca_h_;

    RetryEventHandler           retry_h_;
    AlarmSyncEventHandler       alarm_sync_h_;

    UnknownEventHandler         unknown_h_;
};

#endif // __EVENT_FACTORY_HPP__
