
#ifndef __EVENT_HANDLER_HPP__
#define __EVENT_HANDLER_HPP__

#include <string>

#include "CommandFormat.hpp"
#include "SequenceByNode.hpp"
#include "NodeManagerIF.hpp"
#include "DB.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

class EventHandler
{
public:
    EventHandler() {}
    virtual ~EventHandler() {}

    virtual void          Clear()       = 0;

    virtual std::string & Buffer()      = 0;
    virtual bool          DecodingJSON()= 0;
    virtual int           GetNodeId()   = 0;
    virtual unsigned int  GetSequenceId() = 0;
    virtual bool          IsValidSequenceId(SequenceByNode & _sequence_by_node) = 0;
    virtual bool          Do(NodeManagerIF & _nm_if, DB * _db) = 0;

};


#endif // __EVENT_HANDLER_HPP__
