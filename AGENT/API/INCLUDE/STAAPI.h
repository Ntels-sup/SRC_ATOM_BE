#ifndef __STAAPI_H__
#define __STAAPI_H__

#define STAAPI_GET_4BYTE(_dat, _mBuf){\
		(_dat) = (((unsigned char)(_mBuf)[0] << 24) |\
						((unsigned char)(_mBuf)[1] << 16) |\
						((unsigned char)(_mBuf)[2] << 8) |\
						((unsigned char)(_mBuf)[3]));\
}

#define STAAPI_GET_8BYTE(_dat, _mBuf){\
        (_dat) = (      ((unsigned long)((unsigned char)(_mBuf)[0]) << 56) |\
                        ((unsigned long)((unsigned char)(_mBuf)[1]) << 48) |\
                        ((unsigned long)((unsigned char)(_mBuf)[2]) << 40) |\
                        ((unsigned long)((unsigned char)(_mBuf)[3]) << 32) |\
                        ((unsigned long)((unsigned char)(_mBuf)[4]) << 24) |\
                        ((unsigned long)((unsigned char)(_mBuf)[5]) << 16) |\
                        ((unsigned long)((unsigned char)(_mBuf)[6]) << 8) |\
                        ((unsigned long)((unsigned char)(_mBuf)[7])));\
}

#define STAAPI_PUT_4BYTE(_dat, _mBuf){\
    (_mBuf)[0] = ((_dat) & 0xff000000) >> 24;\
    (_mBuf)[1] = ((_dat) & 0x00ff0000) >> 16;\
    (_mBuf)[2] = ((_dat) & 0x0000ff00) >> 8;\
    (_mBuf)[3] = (_dat) & 0x000000ff;\
}

#define STAAPI_PUT_8BYTE(_dat, _mBuf){\
    (_mBuf)[0] = ((_dat) & 0xff00000000000000) >> 56;\
    (_mBuf)[1] = ((_dat) & 0x00ff000000000000) >> 48;\
    (_mBuf)[2] = ((_dat) & 0x0000ff0000000000) >> 40;\
    (_mBuf)[3] = ((_dat) & 0x000000ff00000000) >> 32;\
    (_mBuf)[4] = ((_dat) & 0x00000000ff000000) >> 24;\
    (_mBuf)[5] = ((_dat) & 0x0000000000ff0000) >> 16;\
    (_mBuf)[6] = ((_dat) & 0x000000000000ff00) >> 8;\
    (_mBuf)[7] = (_dat) &  0x00000000000000ff;\
}

#define STAAPI_TABLE_TYPE_RCD 1
#define STAAPI_TABLE_TYPE_STS 2
#define STAAPI_TABLE_TYPE_RES 3
#define STAAPI_TABLE_TYPE_ALM 4

#define STAAPI_BUFFER_TOO_SMALL 100
#define STAAPI_INVALID_MESSGAE_LEN  101
#define STAAPI_INVALID_TABLE_TYPE 102
#define STAAPI_VALUE_NOT_EXIST 103
#define STAAPI_PARSING_ERROR 104
#define STAAPI_TABLE_PARSING_ERROR 105
#define STAAPI_TABLE_NAME_NOT_EXIST 106
#define STAAPI_PRIMARY_KEY_INSERT_FAILED 107
#define STAAPI_VALUE_INSERT_FAILED 108
#define STAAPI_TABLE_SETTING_FAEILD 109

#define STAAPI_ERROR_MESSAGE_LEN 1024

#define STAAPI_ERR 1

#define STAAPI_LOG(_LOG_LVL,...) {\
		m_nErrorLen = snprintf(m_chError, STAAPI_ERROR_MESSAGE_LEN,__VA_ARGS__);\
}

#define STAAPI_OK 1
#define STAAPI_NOK 0

#endif
