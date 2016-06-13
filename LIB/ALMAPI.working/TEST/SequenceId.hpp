
#ifndef __SEQUENCE_ID_HPP__
#define __SEQUENCE_ID_HPP__

#include <string>

#define SEQUENCE_STRING_MAX_SIZE    16

class SequenceId
{
public:
    explicit    SequenceId();
    explicit    SequenceId(const SequenceId & _id);
    ~SequenceId();
    SequenceId & operator=(const SequenceId & _id);

    bool        Init(const char * _path,
                     const char * _file_name);

    unsigned int Generate();
    bool         Set(unsigned int _value);

    unsigned int Get()          { return value_; }

private:
    bool         save(unsigned int _value);

private:
    unsigned int    value_;
    std::string     abs_file_name_;
};

#endif // __SEQUENCE_ID_HPP__
