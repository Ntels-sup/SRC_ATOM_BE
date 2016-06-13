#ifndef  _CAES_CRYPT_HPP
#define  _CAES_CRYPT_HPP

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <openssl/aes.h>    // ase en/decryption
#include <alloca.h>			//alloca, stack based memory allocation

const int 			LEN_ONE_CRYPTION 	= 16;		// En/Decryption per 16 bytes

class CAESCrypt {
	private :

		bool    Encrypt(unsigned char* ,unsigned char* ,unsigned char*);
		bool    Decrypt(unsigned char* ,unsigned char* ,unsigned char*);
	public:
		CAESCrypt() {};
		~CAESCrypt() {};

		int		EncryptAll(unsigned char* ,unsigned char*,unsigned char*     );
		int		DecryptAll(unsigned char* ,unsigned char*,int ,unsigned char* );

};

#endif