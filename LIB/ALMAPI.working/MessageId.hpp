
#ifndef __MESSAGE_ID_HPP__
#define __MESSAGE_ID_HPP__

#include <string>

#define     MESSAGEID_STRING_SIZE   16

class MessageId
{
public:
    explicit    MessageId();
    explicit    MessageId(const MessageId & _id);
    ~MessageId();
    MessageId & operator=(const MessageId & _id);

    bool        Init(const char * _path,
                     const char * _file_name);

    unsigned int Generate();
    bool         Set(unsigned int _value);

    unsigned int Get()          { return value_; }

    void        RemoveFile();

private:
    bool        save(unsigned int _value);

private:
    unsigned int    value_;
    std::string     abs_file_name_;

    FILE *          now_fp_;
};

#endif // __MESSAGE_ID_HPP__
