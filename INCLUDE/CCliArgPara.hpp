#ifndef __CCLIARGPARA_HPP__
#define __CCLIARGPARA_HPP__
#include <string>

#include "CCliApi.hpp"

class CCliArgPara : public CCliApi{
    private:
        int m_nParaType;
        std::string m_strPara;
        unsigned int m_nPara;

    public:
        static const int PARA_TYPE_NONE = 0;
        static const int PARA_TYPE_NUMBER  = 1;
        static const int PARA_TYPE_STRING = 2;

        CCliArgPara();
        CCliArgPara(std::string &a_strPara);
        CCliArgPara(const char *a_szPara);
        CCliArgPara(int &a_nPara);
        CCliArgPara(unsigned int &a_nPara);
        ~CCliArgPara();
        void SetString(std::string &a_strPara);
        void SetString(const char *a_szPara);
        void SetNumber(int a_nPara);
        void SetNumber(unsigned int a_nPara);
        std::string& GetString();
        int GetNumber();
        int GetType() { return m_nParaType; };
        bool IsNumber();
        bool IsString();
        int GetEncodeMessage(rabbit::object &a_cRecord);
};

#endif
