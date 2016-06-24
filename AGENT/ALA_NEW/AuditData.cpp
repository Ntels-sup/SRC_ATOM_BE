
#include <sys/types.h>
#include <dirent.h>

#include "AuditData.hpp"
#include "CFileLog.hpp"

#include "AlaCFG.hpp"

extern CFileLog *   gAlmLog;
extern AlaCFG       gAlaCFG;

AuditData::
AuditData()
{
    path_       = "";
    now_file_   = "";
    now_fp_     = NULL;

    rows_       = 10000;    // 10 단위로 하세요.. 왠만하면요..
    file_cnt_   = 10;
}

AuditData::
~AuditData()
{
    if(now_fp_ != NULL)
    {
        fclose(now_fp_);
        now_fp_ = NULL;
    }

}

bool AuditData::
Init()
{
    path_ = gAlaCFG.ENV.message_id_path_;

    if(seq_id_.Init(path_.c_str(), "seq.last") == false)
    {
        if(findLastAuditFile(now_file_, path_) == false)
        {
            gAlmLog->WARNING("%-24s| can't find last audit files dir[%s]",
                "AuditData",
                path_.c_str());
            return false;
        }

        unsigned int last_id = 0;
        if(now_file_.size() > 0)
            last_id = extractLastSequenceId(now_file_);

        gAlmLog->WARNING("%-24s| Init - try to get sequenceid from last audit file [%s]",
            "AuditData",
            now_file_.c_str());

        seq_id_.Set(last_id);
    }

    openFile(seq_id_.Get(), now_file_, false);

    return true;
}

bool AuditData::
findLastAuditFile(std::string & _fname, std::string & _path)
{
    DIR           * d      = NULL;

    _fname = "";
    if((d = opendir(_path.c_str())) == NULL)
        return false;

    struct stat     s;
    time_t          last = 0;
    struct dirent   entry;
    struct dirent * result;

    std::string     ftemp = path_ + "/";

    while(readdir_r(d, &entry, &result) != 0)
    {
        if(result == NULL)
            break;

        ftemp.erase(ftemp.rfind("/")+1);
        ftemp.append(result->d_name);

        if(stat(ftemp.c_str(), &s) < 0)
            continue;

        if(S_ISREG(s.st_mode) != true)
            continue;

        if(s.st_mtime > last)
        {
            last  = s.st_mtime;
            _fname = ftemp;
        }
    }

    closedir(d);

    return true;
}


// TO DO : 빈 파일에 접근하게 될때, 기대와 다르게 0 이 갈수도 있어요...
unsigned int AuditData::
extractLastSequenceId(std::string & _fname)
{
    FILE *  fp = NULL;

    if((fp = fopen(_fname.c_str(), "r")) == NULL)
    {
        gAlmLog->WARNING("%-24s| can't open file [%s]",
            "AuditData",
            _fname.c_str());
        return 0;
    }

    char    ch;
    char    sequence_id[MESSAGEID_STRING_SIZE+1];
    memset(sequence_id, 0, sizeof(sequence_id));

    while(true)
    {
        if(fread(&ch, 1, 1, fp) != 1)
            break;

        if(ch == '\n')
        {
            if(fgets(sequence_id, MESSAGEID_STRING_SIZE, fp) == NULL)
                break;
        }
    }

    fclose(fp);

    return atoi(sequence_id);
}

bool AuditData::
Save(unsigned int   _id,  std::string  & _body)
{
    if(_id % rows_ == 0)
        openFile(_id, now_file_, true);

    if(now_fp_ == NULL)
    {
        gAlmLog->WARNING("%-24s| can't save data [%u]",
            "AuditData", _id);
        return false;
    }

    fprintf(now_fp_, "%016u;%s\n", _id, _body.c_str());
    fflush(now_fp_);

    return true;
}

void AuditData::
openFile(unsigned int _id, std::string & _fname, bool _is_truncate)
{
    if(now_fp_ != NULL)
    {
        fclose(now_fp_);
        now_fp_ = NULL;
    }

    int     index = (_id / rows_) % file_cnt_;

    _fname = path_ + "/" + "audit.";
    _fname.append(std::to_string((long long)index));

    if(_is_truncate == true)
        truncate(_fname.c_str(), 0);

    now_fp_ = fopen(_fname.c_str(), "a");

    if(now_fp_ == NULL)
    {
        gAlmLog->WARNING("%-24s| openFile - can't open file [%s]",
            "AuditData",
            _fname.c_str());
        return ;
    }

    gAlmLog->DEBUG("%-24s| openFile - SUCCESS [%s]",
        "AuditData",
        _fname.c_str());
}

int AuditData::
ReadData(std::vector<std::pair<unsigned int, std::string> > & _vec_msg,
         unsigned int                                      _req_seq_id,
         int                                               _max_cnt)
{
    _vec_msg.clear();

    // 파일명을 찾아야 합니다.
    // 그 offset 부터 읽기를 해야 합니다.
    // 만약에 파일을 넘겨서 있다면, 파일을 넘겨서 읽어야 합니다.

    FILE * fp = NULL;

    if((fp = openAuditFile(_req_seq_id)) == NULL)
    {
        if((fp = openLastAuditFile()) == NULL)
            return 0;
    }

    char            buf[1024];

    int             cnt = 0;
    unsigned int    id  = 0;
    char *          body;

    while(true)
    {
        if(fgets(buf, sizeof(buf), fp) == NULL)
            break;

        id   = extractSequenceId(buf);
        body = extractBody(buf);

        // 새로운걸 보내야 하니까요..
        if(id <= _req_seq_id)
            continue;

        if(cnt >= _max_cnt)
            break;

        _vec_msg.push_back({id, body});
        cnt++;
    }

    fclose(fp);
    return  cnt;
}

// _seq_id 의 다음 데이터를 얻게 됩니다.

FILE * AuditData::
openAuditFile(unsigned int _seq_id)
{
    int index = (_seq_id / rows_) % file_cnt_;

    std::string     fname = path_ + "audit.";
    fname.append(std::to_string((long long)index));

    FILE * fp = fopen(fname.c_str(), "r");

    if(fp == NULL)
    {
        gAlmLog->WARNING("%-24s| openAuditFile - can't open file [%s]",
            "AuditData",
            fname.c_str());
        return NULL;
    }

    unsigned int    id = 0;
    char            buf[1024];

    while(true)
    {
        if(fgets(buf, sizeof(buf), fp) == NULL)
            break;

        id = extractSequenceId(buf);

        if(id == _seq_id)
            return fp;
    }

    fclose(fp);
    return NULL;
}

FILE * AuditData::
openLastAuditFile()
{
    std::string     fname;

    if(findLastAuditFile(fname, path_) == false)
        return NULL;

    FILE * fp = fopen(fname.c_str(), "r");

    if(fp == NULL)
    {
        gAlmLog->WARNING("%-24s| openLastestAuditFile - can't open file [%s]",
            "AuditData",
            fname.c_str());
        return NULL;
    }

    return fp;
}

FILE * AuditData::
openOldestAuditFile()
{
    std::string     fname;

    if(findOldestAuditFile(fname, path_) == false)
        return NULL;

    FILE * fp = fopen(fname.c_str(), "r");

    if(fp == NULL)
    {
        gAlmLog->WARNING("%-24s| openOldestAuditFile - can't open file [%s]",
            "AuditData",
            fname.c_str());
        return NULL;
    }

    return fp;
}

bool AuditData::
findOldestAuditFile(std::string & _fname, std::string & _path)
{
    DIR           * d      = NULL;

    _fname = "";
    if((d = opendir(_path.c_str())) == NULL)
        return false;

    struct stat     s;
    time_t          old = time(NULL);
    struct dirent   entry;
    struct dirent * result;

    std::string     ftemp = path_ + "/";

    while(readdir_r(d, &entry, &result) != 0)
    {
        if(result == NULL)
            break;

        ftemp.erase(ftemp.rfind("/")+1);
        ftemp.append(result->d_name);

        if(stat(ftemp.c_str(), &s) < 0)
            continue;

        if(S_ISREG(s.st_mode) != true)
            continue;

        if(s.st_mtime < old)
        {
            old = s.st_mtime;
            _fname = ftemp;
        }
    }

    closedir(d);

    return true;
}

unsigned int AuditData::
extractSequenceId(char * _data)
{
    char str_id[32];

    // Save(), fprintf() 함수 참조.
    strncpy(str_id, _data, MESSAGEID_STRING_SIZE);
    str_id[MESSAGEID_STRING_SIZE] = '\0';

    return atoi(str_id);
}

char * AuditData::
extractBody(char * _data)
{
    // Save(), fprintf() 함수 참조.
    char * p = strchr(_data, ';');

    return (p == NULL)?NULL:++p;
}