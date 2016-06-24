/* vim:ts=4:sw=4
 */
/**
 * \file	CCPkgManage.hpp
 * \brief	패키지 download & install, upgrade
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <openssl/md5.h>

#include "Utility.hpp"

#include "CPkgManage.hpp"

using std::string;


CPkgManage::CPkgManage(const char* a_szPkgHomePath)
{
	if (a_szPkgHomePath != NULL) {
		m_strPkgPath = a_szPkgHomePath;
	} else {
		// default value
		m_strPkgPath = getenv("HOME");
		m_strPkgPath += "/PKG";
	}
	m_strInstallScript = "INSTALL";

	return;
}

bool CPkgManage::Fetch(string& a_strUrl, string& a_strSum, 
								const char* a_szUser, const char* a_szPasswd)
{
	if (a_strUrl.empty()) {
		m_strErrorMsg = "PKG, invalied URL";
		return false;
	}
	
	if (CreatePath(m_strPkgPath.c_str(), 0755) == false) {
		m_strErrorMsg = "PKG, create directory failed, path: ";
		m_strErrorMsg += m_strPkgPath;
		return false;
	}
	
	// URL에서 파일명 추출
	char* pszFile = rindex((char*)a_strUrl.c_str(), '/');
	if (pszFile == NULL) {
		m_strErrorMsg = "PKG, invalied filename in URL";
		return false;
	}
	string strPkgFile = m_strPkgPath;
	strPkgFile += pszFile;

	// wget 명령 생성
	string strCmd = "/usr/bin/wget";
	strCmd += " -O ";
	strCmd += strPkgFile;
	if (a_szUser) {
		strCmd += " --user=";
		strCmd += a_szUser;
	}
	if (a_szPasswd) {
		strCmd += " --password=";
		strCmd += a_szPasswd;
	}
	strCmd += " ";
	strCmd += a_strUrl;
	strCmd += " > /dev/null 2>&1";

	char szOutput[160];
	szOutput[sizeof(szOutput) - 1] = '\0';

	// download 실행
	int nStat = system(strCmd.c_str());
	if (WIFEXITED(nStat)) {
		int nExit = WEXITSTATUS(nStat);
		if (nExit != 0) {
			m_strErrorMsg = "PKG, download failed, exit code not 0";
			return false;
		}
	} else {
		m_strErrorMsg = "PKG, download failed, abnormal exited";
		return false;
	}

	string strDigest;
	if (MD5Digest(strPkgFile, strDigest) == false) {
		m_strErrorMsg = "PKG, md5 digest failed, ";
		m_strErrorMsg += strerror(errno) ;
		return false;
	}
	if (strDigest != a_strSum) {
		m_strErrorMsg = "PKG, MD5 digest mismatch, url: ";
		m_strErrorMsg += "vnfm: ";
		m_strErrorMsg += a_strSum;
		m_strErrorMsg += ", na: ";
		m_strErrorMsg += strDigest;
		return false;
	}
	
	m_strPkgFile = strPkgFile;

	return true;
}

bool CPkgManage::UnPack(const char* a_szUnPackPath)
{
	if (m_strPkgFile.empty()) {
		m_strErrorMsg = "PKG, unknown package file";
		return false;
	}

	// unpack 위해 디렉토리 생성, 기존 디렉토리가 존재하면 먼제 삭제 
	size_t pos = m_strPkgFile.find(".tar.gz");
	if (pos == string::npos) {
		m_strErrorMsg = "PKG, invalied filename format, file: ";
		m_strErrorMsg += m_strPkgFile;
		return false;
	}
	string strUnPackPath = m_strPkgFile.substr(0, pos);
	if (DeletePath(strUnPackPath.c_str()) == false) {
		m_strErrorMsg = "PKG, delete directory failed, path: ";
		m_strErrorMsg += strUnPackPath;
		return false;
	}
	if (CreatePath(strUnPackPath.c_str(), 0755) == false) {
		m_strErrorMsg = "PKG, create directory failed, path: ";
		m_strErrorMsg += strUnPackPath;
		return false;
	}

	// tar decompress unpack
	string strCmd = "/bin/tar";
	strCmd += " -zx --overwrite -C";
	strCmd += " ";
	strCmd += strUnPackPath;
	strCmd += " -f ";
	strCmd += m_strPkgFile;

	int nStat = system(strCmd.c_str());
	if (WIFEXITED(nStat)) {
		int nExit = WEXITSTATUS(nStat);
		if (nExit != 0) {
			m_strErrorMsg = "PKG, download failed, exit code not zero";
			return false;
		}
	} else {
		m_strErrorMsg = "PKG, download failed, abnormal exited";
		return false;
	}

	m_strUnPackPath = strUnPackPath;
	
	return true;
}

bool CPkgManage::Install(const char* a_szInstallScript)
{
	// execute install script
	string strCmd = m_strUnPackPath;
	strCmd += "/";
	strCmd += m_strInstallScript;
	strCmd += " ";
	strCmd += m_strUnPackPath;

	int nStat = system(strCmd.c_str());
	if (WIFEXITED(nStat)) {
		int nExit = WEXITSTATUS(nStat);
		if (nExit != 0) {
			m_strErrorMsg = "PKG, install failed, exit code not zero";
			return false;
		}
	} else {
		m_strErrorMsg = "PKG, install failed, abnormal exited";
		return false;
	}
	
	return true;
}

bool CPkgManage::MD5Digest(string& a_strFile, string& a_strDigest)
{
	MD5_CTX md5ctx;
	MD5_Init(&md5ctx);

	int fd = open(a_strFile.c_str(), O_RDONLY);
	if (fd < 0) {
		m_strErrorMsg = "PKG, file open failed";
		return false;
	}

	char buffer[1024];
	int readn = 0;

	while (true) {
		readn = read(fd, buffer, sizeof(buffer));
		if (readn <= 0) {
			break;
		}
		MD5_Update(&md5ctx, buffer, readn);
	}

	unsigned char digest[16];
	memset(digest, 0x00, sizeof(digest));

	MD5_Final(digest, &md5ctx);

	memset(buffer, 0x00, sizeof(buffer));
	for (size_t i=0, j=0; i < sizeof(digest); i++, j+=2) {
		snprintf(buffer + j, 3, "%02x", digest[i]);
	}

	a_strDigest = buffer;

	return true;
}
