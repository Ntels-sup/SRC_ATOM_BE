
#ifndef __C_COMMON_CODE_HPP__
#define __C_COMMON_CODE_HPP__

#include <string>

// DB 를 읽는 것은 아니구요,
// TAT_COMMON_CODE 에 정의한 값을 동일하게 적어두고, 이 클래스를 통하여 읽으려구요..

class CCommonCode
{
public:
    static void SeverityIdToString(std::string & _out, int _severity_id);

private:
    explicit CCommonCode() { }
    ~CCommonCode() { }
};


#endif // __C_COMMON_CODE_HPP__
