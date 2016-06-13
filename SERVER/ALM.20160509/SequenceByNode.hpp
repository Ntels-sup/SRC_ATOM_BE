
#ifndef __SEQUENCE_BY_NODE_HPP__
#define __SEQUENCE_BY_NODE_HPP__

#include <unordered_map>

#include "DB.hpp"
#include "FetchMaria.hpp"

#include "MessageId.hpp"

class SequenceByNode
{
public:

    SequenceByNode();
    ~SequenceByNode();

    bool Init(DB * _db);
    bool IsItContinuously(int _node_no, unsigned int _seq_id);
    unsigned int GetId(int _node_no);

private:
    bool addMessageId(int _node_no);
    void removeFileForMessageId(DB * _db);

private:
    std::unordered_map<int, MessageId> map_;
};



#endif // __SEQ_ID_BY_NODE_HPP__

