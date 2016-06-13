
#include <string>

#include "SequenceId.hpp"
#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;

SequenceId::
SequenceId()
{
    value_  = 0u;
    abs_file_name_.clear();
}

SequenceId::
SequenceId(const SequenceId & _id)
{
    if(this != &_id)
    {
        value_          = _id.value_;
        abs_file_name_  = _id.abs_file_name_;
    }
}

SequenceId::
~SequenceId()
{
    // Empty
}

SequenceId & SequenceId::
operator=(const SequenceId & _id)
{
    if(this != &_id)
    {
        value_          = _id.value_;
        abs_file_name_  = _id.abs_file_name_;
    }

    return *this;
}

bool SequenceId::
Init(const char * _path, const char * _file_name)
{
    abs_file_name_.assign(_path);
    abs_file_name_.append("/");
    abs_file_name_.append(_file_name);

    FILE * fp = NULL;
    if((fp = fopen(abs_file_name_.c_str(), "r")) == NULL)
    {
        if(errno == ENOENT)
            return Set(value_);

        gAlmLog->ERROR("%-16s| Init - file open error [%d:%s] [%s]",
            "SequenceId",
            errno,
            strerror(errno),
            abs_file_name_.c_str());

        return false;
    }

    char    readbuf[SEQUENCE_STRING_MAX_SIZE+1];
    memset(readbuf, 0, sizeof(readbuf));

    if(SEQUENCE_STRING_MAX_SIZE !=
       fread(readbuf, 1, SEQUENCE_STRING_MAX_SIZE, fp))
    {
        fclose(fp);

        gAlmLog->ERROR("%-24s| Init - fread error [%d:%s] [%s]",
            "SequenceId",
            errno,
            strerror(errno),
            abs_file_name_.c_str());

        return false;
    }
    fclose(fp);

    value_ = atoi(readbuf);
    gAlmLog->DEBUG("%-24s| INIT SUCCESS value[%u]",
        "SequenceId",
        value_);

    return true;
}

unsigned int SequenceId::
Generate()
{
    ++value_;
    save(value_);
    return value_;
}

bool SequenceId::
Set(unsigned int _value)
{
    if(save(_value) == false)
        return false;

    value_ = _value;
    return true;
}

bool SequenceId::
save(unsigned int _value)
{
    std::string     val(std::to_string((unsigned long long)_value));

    std::string     str(SEQUENCE_STRING_MAX_SIZE, '0');
    str.replace(str.size() - val.size(), val.size(), val);

    FILE * fp = NULL;
    if((fp = fopen(abs_file_name_.c_str(), "w")) == NULL)
    {
        gAlmLog->ERROR("%-16s| save - file open error [%d:%s] [%s]",
            "SequenceId",
            errno,
            strerror(errno),
            abs_file_name_.c_str());

        return false;
    }

    if(SEQUENCE_STRING_MAX_SIZE !=
       fwrite(str.c_str(), 1, SEQUENCE_STRING_MAX_SIZE, fp))
    {
        fclose(fp);

        gAlmLog->ERROR("%-24s| save - fwrite error [%d:%s] [%s]",
            "SequenceId",
            errno,
            strerror(errno),
            abs_file_name_.c_str());

        return false;
    }
    fclose(fp);

    gAlmLog->DEBUG("%-24s| SAVE SUCCESS value[%u]",
        "SequenceId",
        value_);

    return true;
}
