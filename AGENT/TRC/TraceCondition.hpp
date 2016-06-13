
#ifndef __TRACE_CONDITION_HPP__
#define __TRACE_CONDITION_HPP__

class TraceCondition
{
public:
    explicit TraceCondition();
    ~TraceCondition();

    bool Check(const char * _key, const char * _value, size_t _len);
    bool IsDetail();

};

#endif // __TRACE_CONDITION_HPP__
