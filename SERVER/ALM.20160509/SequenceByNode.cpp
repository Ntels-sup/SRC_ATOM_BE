
#include <cstring>

#include "SequenceByNode.hpp"
#include "AlarmCFG.hpp"

#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;
extern AlarmCFG     gCFG;

SequenceByNode::
SequenceByNode()
{
    // Empty
}

SequenceByNode::
~SequenceByNode()
{
    // Empty
}

bool SequenceByNode::
Init(DB * _db)
{
    char sql[256];
    sprintf(sql, "SELECT NODE_NO FROM TAT_NODE WHERE USE_YN='Y'");

    if(_db->Query(sql, strlen(sql)) < 0)
    {
        gAlmLog->ERROR("%-24s| getNodeList - query fail [%d:%s] [%s]",
            "getNodeList",
            _db->GetError(),
            _db->GetErrorMsg(),
            sql);
        return false;
    }

    FetchMaria  f;
    f.Clear();

    char        node_no_buf[32];
    f.Set(node_no_buf, sizeof(node_no_buf));

    while(true)
    {
        if(f.Fetch(_db) != true)
            break;

        if(addMessageId(atoi(node_no_buf)) != true)
            return false;
    }

    removeFileForMessageId(_db);

    gAlmLog->INFO("%-24s| Init - SUCCESS",
        "SequenceByNode");

    return true;
}

bool SequenceByNode::
addMessageId(int _node_no)
{
    MessageId   mid;

    if(mid.Init(gCFG.ENV.message_id_path_.c_str(),
                std::to_string((long long int)_node_no).c_str()) != true)
    {
        gAlmLog->ERROR("%-24s| addMessageId - node[%d]'s messageId init fail [%s]",
            "SequenceByNode",
            _node_no,
            gCFG.ENV.message_id_path_.c_str());
        return false;
    }

    map_[_node_no] = mid;

    gAlmLog->INFO("%-24s| addMessageId - node[%d]'s messageId [%u]",
            "SequenceByNode",
            _node_no,
            mid.Get());

    return true;
}

void SequenceByNode::
removeFileForMessageId(DB * _db)
{
    char sql[256];
    sprintf(sql, "SELECT NODE_NO FROM TAT_NODE WHERE USE_YN='N'");

    if(_db->Query(sql, strlen(sql)) < 0)
        return ;

    FetchMaria  f;
    f.Clear();

    char        node_no_buf[32];
    f.Set(node_no_buf, sizeof(node_no_buf));

    while(true)
    {
        if(f.Fetch(_db) != true)
            break;

        MessageId   mid;
        if(mid.Init(gCFG.ENV.message_id_path_.c_str(), node_no_buf) == true)
            mid.RemoveFile();
    }
}

bool SequenceByNode::
IsItContinuously(int _node_no, unsigned int _seq_id)
{
    auto iter = map_.find(_node_no);

    if(iter == map_.end())
    {
        return addMessageId(_node_no);
    }

    if((iter->second).Get() + 1 == _seq_id)
    {
        (iter->second).Set(_seq_id);
        return true;
    }

    gAlmLog->WARNING("%-24s| IsItContinuously - node [%d] recved [%u] but expected [%u]",
        "SequenceByNode",
        _node_no,
        _seq_id,
        (iter->second).Get());

    return false;
}

unsigned int SequenceByNode::
GetId(int _node_no)
{
    auto iter = map_.find(_node_no);
    if(iter == map_.end())
    {
        addMessageId(_node_no);
        return 0;
    }

    return (iter->second).Get();
}
