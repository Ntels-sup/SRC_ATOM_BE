
##ifndef __TRACE_API_HPP__
#define __TRACE_API_HPP__

class TraceAPI
{
public:
    explicit TraceAPI();
    ~TraceAPI();

    bool             IsOn();
    TraceCondition * Check(const char * _key, const char * _value, size_t _value_len);
    bool             Send(const char * _value, size_t _size);

private:

    // TO DO : 채워야 지요..
};


#endif // __TRACE_API_HPP__
