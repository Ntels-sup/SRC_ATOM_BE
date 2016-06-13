
#ifndef __FETCH_DATA_R_HPP__
#define __FETCH_DATA_R_HPP__

class FetchData
{
public:
    explicit FetchData() {}
    virtual ~FetchData() {}

    virtual void Clear() = 0;

    virtual void Set(char * _out, size_t _size) = 0;
    virtual const char * Get(unsigned int _index) = 0;
    virtual int GetCount() = 0;

    virtual void SetResult(int _cnt, void * _res) = 0;
    virtual bool Fetch() = 0;

};


#endif // __FETCH_DATA_R_HPP__