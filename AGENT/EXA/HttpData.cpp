
#include <stdarg.h>

// #include <algorithm>
#include <vector>

#include "ExaCommonDef.hpp"
#include "HttpData.hpp"

extern CFileLog *   gLog;

HttpData::
HttpData()
{
    // DEBUG_PRINT("Construct\n");
}

HttpData::
HttpData(const HttpData & _data)
{
    if(this == &_data)
        return ;

    raw_    = _data.raw_;

    line_   = _data.line_;
    header_ = _data.header_;
    body_   = _data.body_;

    header_map_.clear();

    // DEBUG_PRINT("Copy Construct\n");
}

HttpData::
~HttpData()
{
    header_map_.clear();
    // DEBUG_PRINT("Destroy Success\n");
}

void HttpData::
Clear()
{
    raw_.clear();

    line_.clear();
    header_.clear();
    body_.clear();

    for(auto iter = header_map_.begin(); iter != header_map_.end(); ++iter)
    {
        (iter->second).clear();
    }

    // DEBUG_PRINT("Clear Success\n");
}

bool HttpData::
IsFill()
{
    return line_.size() > 0;
}

int HttpData::
RecvLine(CSocket *     _sock, int _sec)
{
    line_.clear();

    int                 ret     = 0;
    char                c;

    std::vector<char>   vBuf('\0', vnfm::nDEF_BUF_SIZE);
    std::string         sCRLF(MACRO_CRLF);

    // gLog->DEBUG("---------- RecvLine Start");

    while(true)
    {
        if((ret = _sock->Recv(&c, 1, _sec)) < 0)
        {
            // Notice
            gLog->WARNING("%-24s| Recv [%d] [%s]",
                "HttpData",
                ret,
                _sock->m_strErrorMsg.c_str());
            return ret;
        }

        vBuf.push_back(c);
        if(std::string(vBuf.end()-2, vBuf.end()).compare(sCRLF) == 0)
            break;
    }

    line_ = std::string(vBuf.begin(), vBuf.end());

    // gLog->DEBUG("---------- RecvLine End [%s]", line_.c_str());

    return (int)line_.size();
}

int HttpData::
RecvHeader(CSocket * _sock, int _sec)
{
    header_.clear();

    int                 ret     = 0;
    char                c;


    std::vector<char>   vBuf('\0', vnfm::nDEF_BUF_SIZE);
    std::string         sCRLF(MACRO_CRLF);

    sCRLF.append(MACRO_CRLF);

    // gLog->DEBUG("---------- Recv Header Start");

    while(true)
    {
        if((ret = _sock->Recv(&c, 1, _sec)) <= 0)
        {
            // Notice
            // DEBUG_PRINT("Recv Header [%d]\n", ret);
            return ret;
        }

        vBuf.push_back(c);

        if(std::string(vBuf.end()-4, vBuf.end()).compare(sCRLF) == 0)
            break;
    }

    // gLog->DEBUG("---------- Recv Header End");

    header_ = std::string(vBuf.begin(), vBuf.end());

    // map 정리
    char field[vnfm::nDEF_BUF_SIZE];
    char value[vnfm::nDEF_BUF_SIZE];

    char * pos = const_cast<char *>(header_.c_str());

    while(pos != NULL && *pos != '\0')
    {
        if(sscanf(pos, "%[^:]: %[^\x01-\x1f]", field, value) != 2)
            break;

        AddHeader(field, value);

        if((pos = strstr(pos, MACRO_CRLF)) == NULL)
            break;

        pos += strlen(MACRO_CRLF);
    }

    return (int)header_.size();
}

int HttpData::
RecvBody(CSocket * _sock, int _sec)
{
    body_.clear();

    int size = 0;

    if(GetHeader("Content-Length") != NULL)
    {
       size = atoi(GetHeader("Content-Length"));
    }

    if(size == 0)
        return 0;

    // gLog->DEBUG("---------- Recv Body Start");

    char * buf = new char[size+1];
    memset(buf, 0, size+1);

    int ret = _sock->Recv(buf, size, _sec);

    body_ = buf;

    delete [] buf;

    return ret;
}


bool HttpData::
AddLine(const char * _arg0, const char * _arg1, const char * _arg2)
{
    if( strlen(_arg0) == 0 ||
        strlen(_arg1) == 0 ||
        strlen(_arg2) == 0)
    {
        // Error
        return false;
    }

    line_.clear();

    line_ = _arg0;
    line_.append(" ");
    line_.append(_arg1);
    line_.append(" ");
    line_.append(_arg2);

    return true;
}

void HttpData::
AddHeader(const char * _field, const char * _value)
{
    auto iter = header_map_.find(_field);

    if(iter == header_map_.end())
        header_map_[_field] = _value;
    else
    {
        iter->second = _value;
    }
}

void HttpData::
AddHeader(const char * _field, int _value)
{
    char temp[16];
    sprintf(temp, "%d", _value);

    AddHeader(_field, temp);
}

void HttpData::
DelHeader(const char * _field)
{
    header_map_.erase(_field);
}

void HttpData::
AddBody(const char * _s, size_t _size)
{
    body_.clear();
    body_ = _s;

    AddHeader("Content-Length", (int)body_.size());
}

const char * HttpData::
GetLine()
{
    return line_.c_str();
}

const char * HttpData::
GetMethod(char * _out, size_t _size)
{
    // 첫번째 값을 얻습니다.
    const char * in = line_.c_str();
    if(sscanf(in, "%s %*s %*[^\x01-\x1f]", _out) != 1)
        return NULL;

    return _out;
}

const char * HttpData::
GetUri(char * _out, size_t _size)
{
    // 두번째 값을 얻습니다.
    const char * in = line_.c_str();
    if(sscanf(in, "%*s %s %*[^\x01-\x1f]", _out) != 1)
        return NULL;

    return _out;
}

const char * HttpData::
GetProtocol(char * _out, size_t _size)
{
    char * in = const_cast<char *>(line_.c_str());
    char * p  = NULL;

    if((p = strstr(in, "HTTP")) == NULL)
        return NULL;

    if(sscanf(p, "%s", _out) != 1)
        return NULL;

    return _out;
}

const char * HttpData::
GetStatus(char * _out, size_t _size)
{
    return GetMethod(_out, _size);
}

const char * HttpData::
GetStatusMessage(char * _out, size_t _size)
{
    return GetUri(_out, _size);
}

const char * HttpData::
GetHeader()
{
    makeHeader();

    return header_.c_str();
}

const char * HttpData::
GetHeader(const char * _field)
{
    auto iter = header_map_.find(_field);

    if(iter == header_map_.end())
    {
        return NULL;
    }

    return (iter->second).c_str();
}

const char * HttpData::
GetBody()
{
    return body_.c_str();
}

const char * HttpData::
GetPtr()
{
    return raw_.c_str();
}

size_t  HttpData::
GetSize()
{
    return raw_.size();
}

void HttpData::
MakeData()
{
    raw_.clear();

    raw_.append(line_);
    raw_.append(MACRO_CRLF);
    makeHeader();
    raw_.append(header_);
    raw_.append(MACRO_CRLF);
    raw_.append(body_);
}

size_t HttpData::
makeHeader()
{
    header_.clear();

    for(auto iter= header_map_.begin(); iter != header_map_.end(); ++iter)
    {
        if((iter->second).size() == 0)
            continue;

        header_.append(iter->first);
        header_.append(MACRO_COLON_SPACE);
        header_.append(iter->second);
        header_.append(MACRO_CRLF);
    }

    return header_.size();
}
