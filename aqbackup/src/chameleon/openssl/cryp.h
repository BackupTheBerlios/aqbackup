/***************************************************************************
 $RCSfile: cryp.h,v $
                             -------------------
    cvs         : $Id: cryp.h,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
    begin       : Sat Nov 02 2002
    copyright   : (C) 2002 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef CRYP_H
#define CRYP_H "$Id"

#include <openssl/rsa.h>
#include <openssl/blowfish.h>
#include <openssl/ripemd.h>
#include <chameleon/error.h>
#include <chameleon/ipcmessage.h>


#ifdef __cplusplus
extern "C" {
#endif


#define CRYP_RSA_DEFAULT_KEY_LENGTH   1024
#define CRYP_RSA_MAX_KEY_LENGTH       4096
#define CRYP_RSA_DEFAULT_EXPONENT     65537
#define CRYP_RSA_DEFAULT_EXPONENT_STR "65537"


#define CRYP_ERROR_MEMORY_FULL      1
#define CRYP_ERROR_KEY_GENERATION   2
#define CRYP_ERROR_BUFFER_TOO_SMALL 3
#define CRYP_ERROR_BAD_PADDING      4
#define CRYP_ERROR_ENCRYPTION       5
#define CRYP_ERROR_DECRYPTION       6
#define CRYP_ERROR_BAD_SIZE         7
#define CRYP_ERROR_BAD_SIGNATURE    8
#define CRYP_ERROR_BAD_ALGO         9
#define CRYP_ERROR_BAD_EXPONENT     10


CHIPCARD_API typedef enum {
  CryptAlgoNone=0,
  CryptAlgoBlowfish,
  CryptAlgoRSA
} CryptAlgo;


/**
 *
 */
CHIPCARD_API struct CRYP_RSAKEYSTRUCT {
    RSA *key;
};


CHIPCARD_API typedef struct CRYP_RSAKEYSTRUCT CRYP_RSAKEY;
CHIPCARD_API typedef CRYP_RSAKEY *CRYP_RSAKEYPTR;


CHIPCARD_API ERRORCODE Cryp_ModuleInit();
CHIPCARD_API ERRORCODE Cryp_ModuleFini();

CHIPCARD_API CRYP_RSAKEYPTR Cryp_RsaKey_new();
CHIPCARD_API void Cryp_RsaKey_free(CRYP_RSAKEYPTR k);

CHIPCARD_API ERRORCODE Cryp_RsaKey_Generate(CRYP_RSAKEYPTR k, int keylength, int expo);
CHIPCARD_API ERRORCODE Cryp_RsaKey_ToMessage(CRYP_RSAKEYPTR k, IPCMESSAGE *m, int pub);
CHIPCARD_API ERRORCODE Cryp_RsaKey_FromMessage(CRYP_RSAKEYPTR k, IPCMESSAGE *m);
CHIPCARD_API ERRORCODE Cryp_RsaKey_GetChunkSize(CRYP_RSAKEYPTR k, int *size);
CHIPCARD_API ERRORCODE Cryp_Rsa_CryptPublic(CRYP_RSAKEYPTR k,
					    const unsigned char *source,
					    unsigned int size,
					    unsigned char *target,
					    unsigned int bsize);
CHIPCARD_API ERRORCODE Cryp_Rsa_CryptPrivate(CRYP_RSAKEYPTR k,
					     const unsigned char *source,
					     unsigned int size,
					     unsigned char *target,
					     unsigned int bsize);
CHIPCARD_API ERRORCODE Cryp_Rsa_DecryptPublic(CRYP_RSAKEYPTR k,
					      const unsigned char *source,
					      unsigned int size,
					      unsigned char *target,
					      unsigned int bsize);
CHIPCARD_API ERRORCODE Cryp_Rsa_DecryptPrivate(CRYP_RSAKEYPTR k,
					       const unsigned char *source,
					       unsigned int size,
					       unsigned char *target,
					       unsigned int bsize);
CHIPCARD_API ERRORCODE Cryp_Rsa_Sign(CRYP_RSAKEYPTR k,
				     const unsigned char *text,
				     unsigned int size,
				     unsigned char *buffer,
				     unsigned int *bsize);

CHIPCARD_API ERRORCODE Cryp_Rsa_Verify(CRYP_RSAKEYPTR k,
				       const unsigned char *text,
				       unsigned int size,
				       const unsigned char *signature,
				       unsigned int ssize);


CHIPCARD_API ERRORCODE Cryp_PaddForRSAKey(CRYP_RSAKEYPTR k,
					  unsigned char *source,
					  unsigned int *size,
					  unsigned int bsize);

CHIPCARD_API int Cryp_Rsa_GetChunkSize(CRYP_RSAKEYPTR k);

CHIPCARD_API ERRORCODE Cryp_Unpadd(const unsigned char *source,
				   unsigned int *size);

CHIPCARD_API ERRORCODE Cryp_RipeMD160(const unsigned char *source,
				      unsigned int size,
				      unsigned char *buffer,
				      unsigned int bsize);




/**
 *
 */
CHIPCARD_API struct CRYP_BFKEYSTRUCT {
  BF_KEY key;
  int keylen;
  char keydata[16];
};


CHIPCARD_API typedef struct CRYP_BFKEYSTRUCT CRYP_BFKEY;


CHIPCARD_API CRYP_BFKEY *Cryp_BlowfishKey_new();
CHIPCARD_API void Cryp_BlowfishKey_free(CRYP_BFKEY *);

CHIPCARD_API ERRORCODE Cryp_BlowfishKey_SetKey(CRYP_BFKEY *key,
					       const char *data,
					       int len);

CHIPCARD_API ERRORCODE Cryp_BlowfishKey_GenerateKey(CRYP_BFKEY *key);

CHIPCARD_API ERRORCODE Cryp_BlowfishKey_GetKey(CRYP_BFKEY *key,
					       char **data,
					       int *len);

CHIPCARD_API ERRORCODE Cryp_Blowfish_Encrypt(CRYP_BFKEY *key,
					     const char *indata,
					     int size,
					     char *outdata);

CHIPCARD_API ERRORCODE Cryp_Blowfish_Decrypt(CRYP_BFKEY *key,
					     const char *indata,
					     int size,
					     char *outdata);

CHIPCARD_API ERRORCODE Cryp_PaddForBFKey(CRYP_BFKEY *k,
					 unsigned char *source,
					 unsigned int *size,
					 unsigned int bsize);


CHIPCARD_API ERRORCODE Cryp_Encrypt(void *key,
				    CryptAlgo algo,
				    const unsigned char *source,
				    int insize,
				    unsigned char **outbuffer,
				    int *outsize);

CHIPCARD_API ERRORCODE Cryp_Decrypt(void *key,
				    CryptAlgo algo,
				    const unsigned char *source,
				    int insize,
				    unsigned char **outbuffer,
				    int *outsize);

/**
 *
 */
CHIPCARD_API struct CRYP_RMD160STRUCT {
  RIPEMD160_CTX ctx;
};
CHIPCARD_API typedef struct CRYP_RMD160STRUCT CRYP_RMD160;


CHIPCARD_API CRYP_RMD160 *Cryp_RMD160_new();
CHIPCARD_API void Cryp_RMD160_free(CRYP_RMD160 *r);

CHIPCARD_API ERRORCODE Cryp_RMD160_Init(CRYP_RMD160 *r);
CHIPCARD_API ERRORCODE Cryp_RMD160_Update(CRYP_RMD160 *r,
					  const unsigned char *data,
					  int bsize);
CHIPCARD_API ERRORCODE Cryp_RMD160_Final(CRYP_RMD160 *r,
					 unsigned char *buffer,
					 int *bsize);

#ifdef __cplusplus
}
#endif

#endif


