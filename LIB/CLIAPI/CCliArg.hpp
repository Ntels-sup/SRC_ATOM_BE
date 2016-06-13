#ifndef __CCLIARG_HPP__
#define __CCLIARG_HPP__

#include <stdio.h>
#include <string>
#include <list>

#include "CCliApi.hpp"
#include "CCliArgPara.hpp"

class CCliArg : public CCliApi {
    private:
        std::string m_strName;
        std::list<CCliArgPara*> m_lstArgPara;

    public:
        CCliArg();
        CCliArg(std::string &a_strName);
        CCliArg(char *a_szName);
        ~CCliArg();
        CCliArgPara& operator[] (unsigned int a_nIndex);
        template <typename T>
            void operator= (T a_para);
		void SetName(std::string &a_strName) { m_strName = a_strName; };
        std::string& GetName() { return m_strName; };
        unsigned int GetCount() { return m_lstArgPara.size(); };
        template <typename T>
            void PushPara(T a_nPara);
        void PopFirst();
        void PopLast();
        void Pop(unsigned int a_nIndex);
        void PopAll();
        int GetEncodeMessage(rabbit::object &a_cRecord);
        int GetDecodeMessage(rabbit::object &a_cRecord);
};

template <typename T>
void CCliArg::operator= (T a_nPara)
{
    PushPara(a_nPara);
}

template <typename T>
void CCliArg::PushPara(T a_nPara)
{
    CCliArgPara *cArgPara = NULL;

    cArgPara = new CCliArgPara(a_nPara);

    m_lstArgPara.push_back(cArgPara);
}

#endif
