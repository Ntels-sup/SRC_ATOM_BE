
#ifndef __FETCH_MARIA_R_HPP__
#define __FETCH_MARIA_R_HPP__

#include <string>

#include <mysql.h>
#include <errmsg.h>

#include "DB.hpp"
#include "FetchData.hpp"

class FetchMaria_r : public FetchData_r
{
public:
    explicit FetchMaria_r();
    virtual ~FetchMaria();

    void Clear();

    void         Set(char * _out, size_t _size);
    const char * Get(unsigned int _index);
    int          GetCount() { return Res_cnt_; }

    void         SetResult(int _cnt, void * _res);
    bool         Fetch();

private:
    void         freeResult();

private:
    MYSQL_RES *          Res_;
    int                  Res_cnt_;

    struct ST_AssignPointer
    {
        char *  p_;
        size_t  size_;
    };

    ST_AssignPointer *  ret_;

    unsigned int        assigned_column_cnt_;
    unsigned int        column_cnt_;
};


#endif // __FETCH_MARIA_R_HPP__
