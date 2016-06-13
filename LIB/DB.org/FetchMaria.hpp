
#ifndef __FETCH_MARIA_HPP__
#define __FETCH_MARIA_HPP__

#include <string>

#include <mysql.h>
#include <errmsg.h>

#include "DB.hpp"
#include "FetchData.hpp"

class FetchMaria : public FetchData
{
public:
    explicit FetchMaria();
    virtual ~FetchMaria();

    void Clear();

    void         Set(char * _out, size_t _size);
    const char * Get(unsigned int _index);

    void         SetRes(MYSQL_RES * _res);
    bool         Fetch(DB * _db);
    bool         Fetch();
    void         FreeResult();

private:

    MYSQL_RES *          Res_;

    // char **         ret_;

    struct ST_AssignPointer
    {
        char *  p_;
        size_t  size_;
    };

    ST_AssignPointer *  ret_;

    bool                bfirst_;
    unsigned int        assigned_column_cnt_;

    unsigned int        column_cnt_;
};


#endif // __FETCH_MARIA_HPP__
