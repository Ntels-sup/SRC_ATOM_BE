
#ifndef __FETCH_DATA_HPP__
#define __FETCH_DATA_HPP__

#include "DB.hpp"

class FetchData
{
public:
    explicit FetchData() {}
    virtual ~FetchData() {}

    virtual void Clear() = 0;

    virtual void Set(char * _out, size_t _size) = 0;
    virtual const char * Get(unsigned int _index) = 0;

    virtual bool Fetch(DB * _db) = 0;
    virtual void FreeResult() = 0;

};


#endif // __FETCH_DATA_HPP__