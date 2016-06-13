#ifndef __UNKNOWN_EVENT_HANDLER_HPP__
#define __UNKNOWN_EVENT_HANDLER_HPP__

#include "EventHandler.hpp"

class UnknownEventHandler : public EventHandler
{
public:
    UnknownEventHandler();
    virtual ~UnknownEventHandler();

    bool            Init();
    void            Clear();

    std::string &   Buffer()        { return buffer_; }
    bool            DecodingJSON();
    int             GetNodeId();
    unsigned int    GetSequenceId();

    bool            IsValidSequenceId(SequenceByNode & _sequence_by_node);
    bool            Do(NodeManagerIF & _nm_if, DB * _db);

private:

    std::string             buffer_;

};

#endif // __UNKNOWN_EVENT_HANDLER_HPP__
