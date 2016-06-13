#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include <string>
#include <vector>

// function prototype 
int StringSplit(const std::string& text, const char* separators, 
												std::vector<std::string>& words);
bool UrlExtratIp(const char* a_szUrl, std::string& a_strIp);
bool CreatePath(const char* a_szPath, mode_t a_nMode);
bool DeletePath(const char* a_szPath);

#endif // UTILITY_HPP_
