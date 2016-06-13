
#include <cstring>
#include <cstdlib>
#include <string>

#include "FetchMaria_r.hpp"

FetchMaria_r::
FetchMaria()
{
    Res_                = NULL;

    assigned_column_cnt_   = 16;
    ret_    = new (std::nothrow) ST_AssignPointer [assigned_column_cnt_];

    column_cnt_         = 0;
}

FetchMaria_r::
~FetchMaria()
{
    if(ret_ != NULL)
    {
        delete [] ret_;
        ret_ = NULL;
    }

    Clear();
}

void FetchMaria_r::
Clear()
{
    freeResult();
}

void FetchMaria_r::
freeResult()
{
    if(Res_ != NULL)
    {
        mysql_free_result(Res_);
        Res_ = NULL;
    }

    column_cnt_  = 0;
}

void FetchMaria_r::
Set(char * _out, size_t    _size)
{
    memset(_out, 0, _size);

    if(assigned_column_cnt_ <= column_cnt_)
    {
        int next_column_cnt = assigned_column_cnt_ * 2;
        ST_AssignPointer * temp = new (std::nothrow) ST_AssignPointer [next_column_cnt];

        if(temp == NULL)
        {
            DEBUG_PRINT("new operator file [%d]\n", next_column_cnt);
            return ;
        }

        memcpy(temp, ret_, sizeof(ST_AssignPointer) * assigned_column_cnt_);
        delete [] ret_;

        ret_ = temp;
        assigned_column_cnt_ = next_column_cnt;

        DEBUG_PRINT("-------- [%p] resize [%d]\n", ret_, assigned_column_cnt_);
    }

    ret_[column_cnt_].p_    = _out;
    ret_[column_cnt_].size_ = _size;
    column_cnt_++;

}

const char * FetchMaria_r::
Get(unsigned int _index)
{
    if(_index >= column_cnt_)
        return "";

    return ret_[_index].p_;
}

void FetchMaria_r::
SetResult(int _cnt, void * _res)
{
    freeResult();

    Res_Cnt_= _cnt;
    Res_    = (MYSQL_RES *)_res;
}

bool FetchMaria_r::
Fetch()
{
    if(Res_ == NULL)
        return false;

    MYSQL_ROW   row;

    if((row = mysql_fetch_row(Res_)) == NULL)
        return false;

    unsigned int columns = mysql_num_fields(Res_);

    unsigned long *lengths = mysql_fetch_lengths(Res_);

     if(columns > column_cnt_)
    {
        DEBUG_PRINT("Fetch - Column Cnt [%u] is more than the assigned cnt [%u]\n",
            columns, column_cnt_);
        columns = column_cnt_;
    }

    for(unsigned int nLoop = 0; nLoop < columns; ++nLoop)
    {
        if(row[nLoop] == '\0' || lengths[nLoop] < 0)
        {
            ret_[nLoop].p_[0] = '\0';
            continue;
        }

        if(lengths[nLoop] >= ret_[nLoop].size_)
        {
            DEBUG_PRINT("Fetch - Column Size [%lu] is more than the assigned size [%zu]\n",
                lengths[nLoop], ret_[nLoop].size_);
            strncpy(ret_[nLoop].p_, row[nLoop], ret_[nLoop].size_-1);
            ret_[nLoop].p_[ret_[nLoop].size_-1] = '\0';
        }
        else
        {
            strncpy(ret_[nLoop].p_, row[nLoop], lengths[nLoop]);
            ret_[nLoop].p_[lengths[nLoop]] = '\0';
        }
    }

    return true;
}

