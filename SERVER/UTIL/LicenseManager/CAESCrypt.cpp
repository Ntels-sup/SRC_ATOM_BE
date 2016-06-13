#include "CAESCrypt.hpp"
		
bool CAESCrypt::
Encrypt( unsigned char *a_szPlainText, unsigned char* a_szCipherText,unsigned char* a_szKey)
{
	AES_KEY stEncKey;
	
	if (AES_set_encrypt_key(a_szKey, 128, &stEncKey) < 0)
		return false;
	
	AES_encrypt(a_szPlainText, a_szCipherText, &stEncKey);
	
	return true;
}
 
 
bool CAESCrypt::
Decrypt( unsigned char *a_szPlainText, unsigned char* a_szCipherText,unsigned char* a_szKey)
{
    AES_KEY stDecKey;

    if (AES_set_decrypt_key(a_szKey, 128, &stDecKey) < 0)
        return false;
    
    AES_decrypt(a_szCipherText, a_szPlainText, &stDecKey);
 
	return true;
}

int CAESCrypt::
EncryptAll(unsigned char* a_szPlainText,unsigned char* a_szCipherText,unsigned char* a_szKey)
{
	int nLine = 0;
	int nLen  = 0;
	int i     = 0;
	
	unsigned char* szPlainText = NULL;
	
	nLen = strlen((char *)a_szPlainText);
	
	nLine = nLen/LEN_ONE_CRYPTION;
	if(nLen%LEN_ONE_CRYPTION > 0 ) nLine++; 
	
	szPlainText =  (unsigned char *)alloca(nLine*LEN_ONE_CRYPTION +1);
	
	if(NULL == szPlainText)
		return -1;
		
	memset(szPlainText, 0x00, nLine*LEN_ONE_CRYPTION+1);
	memcpy(szPlainText,a_szPlainText,nLen);
	
	for(i=0; i< nLine; i++)
	{
		if(false == Encrypt(&szPlainText[i*16],&a_szCipherText[i*16],a_szKey))
			return -1;
	}
	
	return nLine*LEN_ONE_CRYPTION;
}

int CAESCrypt::
DecryptAll(unsigned char* a_szPlainText,unsigned char* a_szCipherText, int a_nCipherLen,unsigned char* a_szKey)
{
	int nLine = 0;
	int i     = 0;
	
	unsigned char* szCipherText	= NULL;
	
	nLine = a_nCipherLen/LEN_ONE_CRYPTION;
	if(a_nCipherLen%LEN_ONE_CRYPTION > 0 ) nLine++; 
	
	szCipherText = (unsigned char *)alloca(nLine*LEN_ONE_CRYPTION+1);
	if( NULL == szCipherText)
		return -1;
	
	memset(szCipherText, 0x00, nLine*LEN_ONE_CRYPTION+1);
	memcpy(szCipherText,a_szCipherText,a_nCipherLen);
	
	for(i=0; i< nLine; i++)
	{
		if(false == Decrypt(&a_szPlainText[i*16],&szCipherText[i*16],a_szKey))
			return -1;
	}	
	
	return nLine*LEN_ONE_CRYPTION;
}
