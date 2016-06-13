#ifndef __HTTP_DATA_HPP__
#define __HTTP_DATA_HPP__

#include <stdio.h>
#include <string.h>
#include <string>
#include <unordered_map>

#include "CSocket.hpp"

// #include "DebugMacro.hpp"
#include "ExaCommonDef.hpp"

#define     MACRO_CRLF          "\r\n"
#define     MACRO_COLON_SPACE   ": "
#define     MACRO_SPACE         " "

class HttpData
{
public:
    HttpData();
    HttpData(const HttpData & _http_data);

    ~HttpData();

    void            Clear();
    bool            IsFill();

    const char *    GetPtr();
    size_t          GetSize();

    void            MakeData();

    int             RecvLine(CSocket   * _sock, int _sec =0);
    int             RecvHeader(CSocket * _sock, int _sec =0);
    int             RecvBody(CSocket   * _sock, int _sec =0);

    bool            AddLine(const char * _arg0,
                            const char * _arg1,
                            const char * _arg2);
    void            AddHeader(const char * _field, const char * _value);
    void            AddHeader(const char * _field, int _value);
    void            DelHeader(const char * _field);

    void            AddBody(const char * _str, size_t _size);

    const char *    GetLine    ();
    const char *    GetMethod  (char *  _out,  size_t _size);
    const char *    GetUri     (char *  _out,  size_t _size);
    const char *    GetProtocol(char *  _out,  size_t _size);
    const char *    GetStatus  (char *  _out,  size_t _size);
    const char *    GetStatusMessage(char * _out, size_t _size);

    const char *    GetHeader();
    const char *    GetHeader(const char * _field);

    const char *    GetBody();

private:
    size_t          makeHeader();

private:

    std::string      raw_;

    std::string      line_;
    std::string      header_;
    std::string      body_;

    std::unordered_map<std::string, std::string>     header_map_;

};

#endif