
#ifndef __AUDIT_DATA_HPP__
#define __AUDIT_DATA_HPP__

#include <string>
#include <vector>
#include <cstdio>

#include "MessageId.hpp"

class AuditData
{
public:
    AuditData();
    ~AuditData();

    bool Init();
    bool Save(unsigned int   _id, std::string  & _body);
    unsigned int GenerateSequenceId() { return seq_id_.Generate(); }
    unsigned int GetSequenceId() { return seq_id_.Get(); }
    int ReadData(std::vector<std::pair<unsigned int, std::string> > & _vec_msg,
                 unsigned int                                      _req_seq_id,
                 int                                               _max_cnt);

private:
    unsigned int   extractLastSequenceId(std::string & _fname);
    void           openFile(unsigned int   _id,
                            std::string  & _fname,
                            bool           _is_truncate);

    FILE *         openAuditFile(unsigned int _seq_id);
    FILE *         openOldestAuditFile();
    bool           findOldestAuditFile(std::string & _fname, std::string & _path);
    FILE *         openLastAuditFile();
    bool           findLastAuditFile(std::string & _fname, std::string & _path);
    unsigned int   extractSequenceId(char * _data);
    char *         extractBody(char * _data);


private:
    MessageId       seq_id_;

    std::string     path_;
    std::string     now_file_;
    FILE *          now_fp_;

    int             rows_;
    int             file_cnt_;

};


#endif // __AUDIT_DATA_HPP__
