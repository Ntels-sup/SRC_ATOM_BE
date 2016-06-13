#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

#include "lnkLst.h"
#include "cli.h"
#include "cliInt.h"

static unsigned int intGlobInitFlg = CLI_FALSE;

static CliGlobCb intGlobCb;

unsigned int cli_globGetInitFlg()
{
	return intGlobInitFlg;
}

int cli_globInit()
{
	intGlobCb.logLvl = CLI_NONE;
	intGlobCb.logFunc = NULL;
	intGlobCb.dispFunc = NULL;
	intGlobCb.parseLogFunc = NULL;

	intGlobInitFlg = CLI_TRUE;

	return CLI_OK;
}

int cli_globSetLogFunc(unsigned int lvl, CliLogFunc logFunc)
{
	intGlobCb.logLvl = lvl;
	intGlobCb.logFunc = logFunc;

	return CLI_OK;
}

CliDispFunc cli_globGetDispFunc()
{
	return intGlobCb.dispFunc;
}

CliParseLogFunc cli_globGetParseLogFunc()
{
	return intGlobCb.parseLogFunc;
}

int cli_globSetDispFunc(CliDispFunc dispFunc)
{
	intGlobCb.dispFunc = dispFunc;

	return CLI_OK;
}

int cli_globSetParseLogFunc(CliParseLogFunc parseLogFunc)
{
	intGlobCb.parseLogFunc = parseLogFunc;

	return CLI_OK;
}

int cli_globGetLogLvl()
{
	return intGlobCb.logLvl;
}

int cli_globLogPrnt(unsigned int lvl, char *file, unsigned int line, const char *fmt,...)
{
	va_list ap;

	if(intGlobCb.logFunc == NULL){
		return CLI_OK;
	}

	va_start(ap, fmt);

	intGlobCb.bufLen = vsnprintf(intGlobCb.buf, CLI_MAX_BUF_LEN, fmt, ap);

	intGlobCb.logFunc(lvl, file, line, intGlobCb.buf);

	va_end(ap);

	return CLI_OK;
}

int cli_globDispPrnt(const char *fmt,...)
{
	va_list ap;

	if(intGlobCb.dispFunc == NULL){
		return CLI_OK;
	}

	va_start(ap, fmt);

	intGlobCb.bufLen = vsnprintf(intGlobCb.buf, CLI_MAX_BUF_LEN, fmt, ap);

	intGlobCb.dispFunc(intGlobCb.buf);

	va_end(ap);

	return CLI_OK;
}

int cli_globParseLogPrnt(const char *fmt,...)
{
	va_list ap;

	if(intGlobCb.dispFunc == NULL){
		return CLI_OK;
	}

	va_start(ap, fmt);

	intGlobCb.bufLen = vsnprintf(intGlobCb.buf, CLI_MAX_BUF_LEN, fmt, ap);

	intGlobCb.parseLogFunc(intGlobCb.buf);

	va_end(ap);

	return CLI_OK;
}
