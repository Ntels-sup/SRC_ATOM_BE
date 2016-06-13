
#include <string>

#include "MessageId.hpp"
#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;

MessageId::
MessageId()
{
    value_  = 0u;
    abs_file_name_.clear();

    now_fp_ = NULL;
}

MessageId::
MessageId(const MessageId & _id)
{
    if(this != &_id)
    {
        value_          = _id.value_;
        abs_file_name_  = _id.abs_file_name_;
        now_fp_         = NULL;
    }
}

MessageId::
~MessageId()
{
    if(now_fp_ != NULL)
    {
        fclose(now_fp_);
        now_fp_ = NULL;
    }
}

MessageId & MessageId::
operator=(const MessageId & _id)
{
    if(this != &_id)
    {
        value_          = _id.value_;
        abs_file_name_  = _id.abs_file_name_;
        now_fp_         = NULL;
    }

    return *this;
}

bool MessageId::
Init(const char * _path, const char * _file_name)
{
    abs_file_name_.assign(_path);
    abs_file_name_.append("/");
    abs_file_name_.append(_file_name);
    abs_file_name_.append(".msgid");

    FILE * rfp = NULL;
    if((rfp = fopen(abs_file_name_.c_str(), "r")) == NULL)
    {
        if(errno == ENOENT)
            return Set(value_);

        gAlmLog->ERROR("%-24s| Init - file open error [%d:%s] [%s]",
            "MessageId",
            errno,
            strerror(errno),
            abs_file_name_.c_str());

        return false;
    }

    if(fscanf(rfp, "%u", &value_) != 1)
    {
        fclose(rfp);

        gAlmLog->ERROR("%-24s| Init - fread error [%d:%s] [%s]",
            "MessageId",
            errno,
            strerror(errno),
            abs_file_name_.c_str());

        return false;
    }

    fclose(rfp);

    return true;
}

unsigned int MessageId::
Generate()
{
    ++value_;
    save(value_);
    return value_;
}

bool MessageId::
Set(unsigned int _value)
{
    if(save(_value) == false)
        return false;

    value_ = _value;
    return true;
}

bool MessageId::
save(unsigned int _value)
{
    if(now_fp_ == NULL)
    {
        if((now_fp_ = fopen(abs_file_name_.c_str(), "w")) == NULL)
        {
            gAlmLog->ERROR("%-16s| save - file open error [%d:%s] [%s]",
                "MessageId",
                errno,
                strerror(errno),
                abs_file_name_.c_str());

            return false;
        }
    }

    if(fseek(now_fp_,   0, SEEK_SET) < 0)
    {
        fclose(now_fp_);
        now_fp_ = NULL;
        return false;
    }

    if(fprintf(now_fp_, "%016u", _value) < 0)
    {
        fclose(now_fp_);
        now_fp_ = NULL;
        return false;
    }

    fflush(now_fp_);

    gAlmLog->DEBUG("%-24s| SAVE SUCCESS value[%u]",
        "MessageId",
        value_);

    return true;
}

void MessageId::
RemoveFile()
{
    if(now_fp_ != NULL)
    {
        fclose(now_fp_);
        now_fp_ = NULL;
    }

    remove(abs_file_name_.c_str());
}