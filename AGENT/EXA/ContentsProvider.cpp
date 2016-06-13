
#include "ContentsProvider.hpp"

#include "CProtocol.hpp"
#include "CModuleIPC.hpp"

#include "CommonCode.hpp"
#include "CFileLog.hpp"

extern CFileLog * gLog;

ContentsProvider::
ContentsProvider()
{
    timer_.Update();
    seq_        =  0;

    map_.clear();

    my_node_no_ = -1;
    my_proc_no_ = -1;
    vec_.resize(64);

    ipc_        = NULL;
    period_     = 0;
}

ContentsProvider::
~ContentsProvider()
{
    // Empty
}

bool ContentsProvider::
Init(int            _my_node_no,
     int            _my_proc_no,
     CModuleIPC *   _ipc,
     int            _period)
{
    my_node_no_  = _my_node_no;
    my_proc_no_  = _my_proc_no;

    ipc_         = _ipc;
    period_      = _period;

    if(ipc_ == NULL || my_node_no_ < 0 || my_proc_no_ < 0)
        return false;

	// TO DO :
	// 여기서, Register 를 호출했떤 놈들을 파일에서 읽어서 map 을 채워야 합니다.

    return true;
}


bool ContentsProvider::
Register(std::string & _apiMsg)
{
    int             node_no;
    int             proc_no;
    int             command_code;
    std::string     body;

    size_t  first   = 0;
    size_t  end     = _apiMsg.find('\0', first);

    try
    {
        node_no = atoi(_apiMsg.substr(first).c_str());

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        proc_no = atoi(_apiMsg.substr(first).c_str());

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        command_code = atoi(_apiMsg.substr(first).c_str());

        first   = end + 1;
        end     = _apiMsg.find('\0', first);
        body    = atoi(_apiMsg.substr(first).c_str());

    } catch(std::exception & e) {

        return false;
    }

    Register(command_code, proc_no, body);
    return true;
}


// node 내에서, 특정 command 에 대하여 나에게 요청해 주세요.. 하고 등록하는 겁니다.
void ContentsProvider::
Register(int            _command_code,
         int            _proc_no,
         std::string &  _body)
{
    auto iter = map_.find(_command_code);

    if(iter != map_.end())
    {
        std::list<std::pair<int, std::string> >  & list_proc_no = iter->second;

        for(auto iter_list  = list_proc_no.begin() ;
                 iter_list != list_proc_no.end() ;
                 ++iter_list)
        {
            // 이미 등록되어 있는지 확인
            if(iter_list->first == _proc_no)
                return ;
        }

        list_proc_no.push_back({_proc_no, _body});
        return ;
    }

    std::list<std::pair<int, std::string> >  list_proc_no;
    list_proc_no.push_back({_proc_no, _body});

    map_[_command_code] = list_proc_no;

	// TO DO 
	// 여기서, File 에다가 저장해야 할 듯 합니다.
	// 그래야, 죽었다가 살아나도, 이전에 요청했떤 Proc 들의 Lit 를 채우죠..
}


bool ContentsProvider::
RequestAll()
{
    if(timer_.TimeOut(period_) == false)
        return true;

    timer_.Update();
    for(auto iter = map_.begin(); iter != map_.end(); ++iter)
    {
        std::list<std::pair<int, std::string> >  & list_pair = iter->second;
        for(auto iter_list  = list_pair.begin();
                 iter_list != list_pair.end();
                 ++iter_list)
        {
            makeRequest(protocol_,
                        iter_list->first,
                        iter->first);

            makeBody(protocol_, iter_list->second);

            if(ipc_->SendMesg(protocol_) == false)
            {
                gLog->WARNING("%-24s| RequestAll - SendMesg fail [%s]",
                     "ContentsProvider",
                    ipc_->m_strErrorMsg.c_str());
                return false;
            }

            gLog->DEBUG("%-24s| RequestAll - SendMesg success proc [%d] cmd [%d]",
                "ContentsProvider",
                iter_list->first,
                iter->first);
        }
    }

    return true;
}

void ContentsProvider::
makeRequest(CProtocol &     _protocol,
            int             _proc_no,
            int             _command_code)
{
    _protocol.Clear();

    char    command_id[16];
    sprintf(command_id, "%010d", _command_code);

    _protocol.SetCommand(command_id);
    _protocol.SetFlagRequest();
    _protocol.SetSource(my_node_no_, my_proc_no_);
    _protocol.SetDestination(my_node_no_, _proc_no);
    _protocol.SetSequence(genSequence());

}

void ContentsProvider::
makeBody(CProtocol   &  _protocol,
         std::string &  _body)
{
    vec_.clear();

    if(_body.size() <= 0)
        return ;

    vec_.assign(_body.begin(), _body.end());

    _protocol.SetPayload(vec_);
}

int     ContentsProvider::
genSequence()
{
    return ++seq_;
}

