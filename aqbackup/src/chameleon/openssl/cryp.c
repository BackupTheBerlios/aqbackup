/***************************************************************************
 $RCSfile: cryp.c,v $
                             -------------------
    cvs         : $Id: cryp.c,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef __declspec
# if BUILDING_CHIPCARD_DLL
#  define CHIPCARD_API __declspec (dllexport)
# else /* Not BUILDING_CHIPCARD_DLL */
#  define CHIPCARD_API __declspec (dllimport)
# endif /* Not BUILDING_CHIPCARD_DLL */
#else
# define CHIPCARD_API
#endif


#include <chameleon/debug.h>
#include <chameleon/cryp.h>
#include <chameleon/chameleon.h>

#include <string.h>
#include <assert.h>

#include <openssl/bn.h>
#include <openssl/ripemd.h>
#include <openssl/rand.h>

#include <stdio.h> /* for debugging purposes */

const char *Cryp_ErrorString(int c);


int cryp_is_initialized=0;
ERRORTYPEREGISTRATIONFORM cryp_error_descr= {
    Cryp_ErrorString,
    0,
    "Crypt"};



ERRORCODE Cryp_ModuleInit(){
    if (!cryp_is_initialized) {
	if (!Error_RegisterType(&cryp_error_descr))
	    return Error_New(0,
			     ERROR_SEVERITY_ERR,
			     ERROR_TYPE_ERROR,
			     ERROR_COULD_NOT_REGISTER);
	cryp_is_initialized=1;
    }
    return 0;
}


ERRORCODE Cryp_ModuleFini(){
    if (cryp_is_initialized) {
	cryp_is_initialized=0;
	if (!Error_UnregisterType(&cryp_error_descr))
	    return Error_New(0,
			     ERROR_SEVERITY_ERR,
			     ERROR_TYPE_ERROR,
			     ERROR_COULD_NOT_UNREGISTER);
    }
    return 0;
}


CRYP_RSAKEYPTR Cryp_RsaKey_new(){
  CRYP_RSAKEYPTR k;

  k=(CRYP_RSAKEY*)malloc(sizeof(CRYP_RSAKEY));
  assert(k);
  k->key=0;
  return k;
}


void Cryp_RsaKey_free(CRYP_RSAKEYPTR k){
  assert(k);
  if (k->key)
    RSA_free(k->key);
  free(k);
}


ERRORCODE Cryp_RsaKey_Generate(CRYP_RSAKEYPTR k, int keylength, int expo){
  assert(k);
  if (keylength==0)
    keylength=CRYP_RSA_DEFAULT_KEY_LENGTH;
  if (expo==0)
    expo=CRYP_RSA_DEFAULT_EXPONENT;

  if (k->key)
    RSA_free(k->key);

  k->key=RSA_generate_key(keylength, expo, NULL, NULL);
  assert(k->key);
  if (RSA_size(k->key)==3) {
    BIGNUM *bn;
    /* UGLY UGLY HACK !
     * I have seen on one system that "n" and "e" are swapped,
     * so we need to swap them again
     */
    DBG_WARN("Modulus and exponent are reversed, swapping.");
    bn=k->key->e;
    k->key->e=k->key->n;
    k->key->n=bn;
  }

  return 0;
}


ERRORCODE Cryp_RsaKey_ToMessage(CRYP_RSAKEYPTR k, IPCMESSAGE *m, int pub){
    int l;
    char buffer[CRYP_RSA_MAX_KEY_LENGTH/8];
    ERRORCODE err;

    assert(k);
    assert(k->key);
    assert(m);

    // key type (public or private)
    err=IPCMessage_AddIntParameter(m,pub);
    if (!Error_IsOk(err))
	return err;

    // modulus
    l=BN_bn2bin(k->key->n, (unsigned char*) &buffer);
    err=IPCMessage_AddParameter(m,buffer,l);
    if (!Error_IsOk(err))
	return err;

    if (pub!=0) {
	// exponent
	l=BN_bn2bin(k->key->e, (unsigned char*) &buffer);
	err=IPCMessage_AddParameter(m,buffer,l);
	if (!Error_IsOk(err))
	    return err;
    }
    else {
	// p
	l=BN_bn2bin(k->key->p, (unsigned char*) &buffer);
	err=IPCMessage_AddParameter(m,buffer,l);
	if (!Error_IsOk(err))
	    return err;

	// q
	l=BN_bn2bin(k->key->q, (unsigned char*) &buffer);
	err=IPCMessage_AddParameter(m,buffer,l);
	if (!Error_IsOk(err))
	    return err;

	// dmp1
	l=BN_bn2bin(k->key->dmp1, (unsigned char*) &buffer);
	err=IPCMessage_AddParameter(m,buffer,l);
	if (!Error_IsOk(err))
	    return err;

	// dmq1
	l=BN_bn2bin(k->key->dmq1, (unsigned char*) &buffer);
	err=IPCMessage_AddParameter(m,buffer,l);
	if (!Error_IsOk(err))
	    return err;

	// iqmp
	l=BN_bn2bin(k->key->iqmp, (unsigned char*) &buffer);
	err=IPCMessage_AddParameter(m,buffer,l);
	if (!Error_IsOk(err))
	    return err;

	// d
	l=BN_bn2bin(k->key->d, (unsigned char*) &buffer);
	err=IPCMessage_AddParameter(m,buffer,l);
	if (!Error_IsOk(err))
	    return err;
    } // if pub==0

    err=IPCMessage_BuildMessage(m);
    return err;
}


ERRORCODE Cryp_RsaKey_FromMessage(CRYP_RSAKEYPTR k, IPCMESSAGE *m){
    ERRORCODE err;
    int pub;
    char *ptr;
    int size;
    BIGNUM *bn;

    assert(k);
    assert(m);

    // key type (public or private)
    err=IPCMessage_FirstIntParameter(m,&pub);
    if (!Error_IsOk(err))
	return err;

    if (k->key)
	RSA_free(k->key);

    // create key
    k->key=RSA_new();
    if (k->key==0)
	return Error_New(0,
			 ERROR_SEVERITY_ERR,
			 cryp_error_descr.typ,
			 CRYP_ERROR_MEMORY_FULL);
    // n
    err=IPCMessage_NextParameter(m,&ptr, &size);
    DBG_INFO("Modulus size is: %i",size);
    if (!Error_IsOk(err)) {
      BN_free(bn);
      return err;
    }
    bn=BN_new();
    k->key->n=BN_bin2bn((unsigned char*) ptr, size, bn);

    if (pub!=0) {
	// e
	err=IPCMessage_NextParameter(m,&ptr, &size);
	if (!Error_IsOk(err)) {
	    return err;
	}
	bn=BN_new();
	k->key->e=BN_bin2bn((unsigned char*) ptr, size, bn);
    }
    else {
      /* e, this is needed for newer OpenSSL versions */
      bn=BN_new();
      if (BN_dec2bn(&bn, CRYP_RSA_DEFAULT_EXPONENT_STR)==0)
	return Error_New(0,
			 ERROR_SEVERITY_ERR,
			 cryp_error_descr.typ,
			 CRYP_ERROR_BAD_EXPONENT);
      k->key->e=bn;

      // p
      err=IPCMessage_NextParameter(m,&ptr, &size);
      if (!Error_IsOk(err)) {
	return err;
      }
      bn=BN_new();
      k->key->p=BN_bin2bn((unsigned char*) ptr, size, bn);

      // q
      err=IPCMessage_NextParameter(m,&ptr, &size);
      if (!Error_IsOk(err)) {
	return err;
      }
      bn=BN_new();
      k->key->q=BN_bin2bn((unsigned char*) ptr, size, bn);

      // dmp1
      err=IPCMessage_NextParameter(m,&ptr, &size);
      if (!Error_IsOk(err)) {
	return err;
      }
      bn=BN_new();
      k->key->dmp1=BN_bin2bn((unsigned char*) ptr, size, bn);

      // dmq1
      err=IPCMessage_NextParameter(m,&ptr, &size);
      if (!Error_IsOk(err)) {
	return err;
      }
      bn=BN_new();
      k->key->dmq1=BN_bin2bn((unsigned char*) ptr, size, bn);

      // iqmp
      err=IPCMessage_NextParameter(m,&ptr, &size);
      if (!Error_IsOk(err)) {
	return err;
      }
      bn=BN_new();
      k->key->iqmp=BN_bin2bn((unsigned char*) ptr, size, bn);

      // d
      err=IPCMessage_NextParameter(m,&ptr, &size);
      if (!Error_IsOk(err)) {
	return err;
      }
      bn=BN_new();
      k->key->d=BN_bin2bn((unsigned char*) ptr, size, bn);
    } // if pub==0

    return 0;
}


ERRORCODE Cryp__Padd(int ps,
		     unsigned char *source,
		     unsigned int *size,
		     unsigned int bsize){
  unsigned int i;
  unsigned char *ptr;

  assert(source);
  assert(size);
  assert(bsize);

  i=ps-((*size)%ps); /* i is number of bytes to add */
  if (*size+i>bsize)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BUFFER_TOO_SMALL);
  //ptr=&source[*size];

  /* move behind */
  ptr=source;
  memmove(ptr+i, ptr, *size);
  *size+=i;

  // prepend 0x00 bytes
  i--; /* decrement, since the last byte is a 0x80 */
  while(i--)
    *(ptr++)=0x00;
  *ptr=0x80;

  return 0;
}


ERRORCODE Cryp_PaddForRSAKey(CRYP_RSAKEYPTR k,
			     unsigned char *source,
			     unsigned int *size,
			     unsigned int bsize){
  unsigned ps;

  assert(k);
  assert(k->key);

  ps=RSA_size(k->key);
  return Cryp__Padd(ps, source, size, bsize);
}


ERRORCODE Cryp_PaddForBFKey(CRYP_BFKEY *k,
			    unsigned char *source,
			    unsigned int *size,
			    unsigned int bsize){
  assert(k);
  return Cryp__Padd(8, source, size, bsize);
}


ERRORCODE Cryp_Unpadd(const unsigned char *source,
		      unsigned int *size){
  unsigned int i;
  unsigned int pos;
  const unsigned char *ptr;

  assert(source);
  assert(size);

  /* set maximum */
  i=(*size<CRYP_RSA_MAX_KEY_LENGTH/8)?*size:CRYP_RSA_MAX_KEY_LENGTH/8;
  ptr=source;

  pos=0;
  /* skip all 0x00 bytes */
  while (pos<i) {
    if (ptr[pos]!=0)
      break;
    pos++;
  }
  if (pos>=i) {
    DBG_ERROR("Too much padding");
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BAD_PADDING);
  }

  if (ptr[pos]!=0x80) {
    DBG_ERROR("Bad padding");
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BAD_PADDING);
  }
  pos++;
  *size=*size-pos;
  memmove((char*)ptr, (char*)(ptr+pos), *size);

  return 0;
}


int Cryp_Rsa_GetChunkSize(CRYP_RSAKEYPTR k){
  assert(k);
  assert(k->key);

  return RSA_size(k->key);
}


ERRORCODE Cryp_Rsa_CryptPublic(CRYP_RSAKEYPTR k,
			       const unsigned char *source,
			       unsigned int size,
			       unsigned char *target,
			       unsigned int bsize){
  unsigned int ksize;

  assert(k);
  assert(k->key);
  assert(k->key->n);
  assert(k->key->e);
  assert(source);
  assert(size);
  assert(target);
  assert(bsize);

  ksize=RSA_size(k->key);
  if (size!=ksize)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BAD_SIZE);

  if (RSA_public_encrypt(ksize, (unsigned char*)source,
			 target, k->key,RSA_NO_PADDING)!=ksize)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_ENCRYPTION);
  return 0;
}


ERRORCODE Cryp_Rsa_CryptPrivate(CRYP_RSAKEYPTR k,
				const unsigned char *source,
				unsigned int size,
				unsigned char *target,
				unsigned int bsize){
    unsigned int ksize;

    assert(k);
    assert(k->key);
    assert(source);
    assert(size);
    assert(target);
    assert(bsize);


    ksize=RSA_size(k->key);
    if (size!=ksize)
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       cryp_error_descr.typ,
		       CRYP_ERROR_BAD_SIZE);

    if (RSA_private_encrypt(ksize, (unsigned char*)source,
			    target, k->key,RSA_NO_PADDING)!=ksize)
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       cryp_error_descr.typ,
		       CRYP_ERROR_ENCRYPTION);

    return 0;
}


ERRORCODE Cryp_Rsa_DecryptPublic(CRYP_RSAKEYPTR k,
				 const unsigned char *source,
				 unsigned int size,
				 unsigned char *target,
				 unsigned int bsize){
  unsigned int ksize;

  assert(k);
  assert(k->key);
  assert(source);
  assert(size);
  assert(target);
  assert(bsize);


  ksize=RSA_size(k->key);
  if (size!=ksize)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BAD_SIZE);

  if (RSA_public_decrypt(ksize, (unsigned char*)source,
			 target, k->key,RSA_NO_PADDING)!=ksize)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_DECRYPTION);

    return 0;
}


ERRORCODE Cryp_Rsa_DecryptPrivate(CRYP_RSAKEYPTR k,
				  const unsigned char *source,
				  unsigned int size,
				  unsigned char *target,
				  unsigned int bsize){
  unsigned int ksize;

  assert(k);
  assert(k->key);
  assert(k->key->d);
  assert(k->key->p);
  assert(k->key->q);
  assert(source);
  assert(size);
  assert(target);
  assert(bsize);

  ksize=RSA_size(k->key);
  if (size!=ksize)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BAD_SIZE);

  if (RSA_private_decrypt(ksize, (unsigned char*)source,
			  target, k->key,RSA_NO_PADDING)!=ksize)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_DECRYPTION);

  return 0;
}


ERRORCODE Cryp_RipeMD160(const unsigned char *source,
			 unsigned int size,
			 unsigned char *buffer,
			 unsigned int bsize){
    if (bsize<RIPEMD160_DIGEST_LENGTH)
	return Error_New(0,
			 ERROR_SEVERITY_ERR,
			 cryp_error_descr.typ,
			 CRYP_ERROR_BUFFER_TOO_SMALL);
    RIPEMD160(source, size,buffer);

    return 0;
}




/* The code for signing and veryfing a signature was inspired by OpenHBCI
 * (src/openhbci/core/rsakey.{h,cpp}). The class HBCI::RSAKey was written
 * by Fabian Kaiser.
 */
ERRORCODE Cryp_Rsa_Sign(CRYP_RSAKEYPTR k,
			const unsigned char *text,
			unsigned int size,
			unsigned char *buffer,
			unsigned int *bsize){
    BN_CTX *ctx;
    BIGNUM *result1;
    BIGNUM *hashbn;
    int l;
    unsigned char hash[RIPEMD160_DIGEST_LENGTH];
    ERRORCODE err;

    ctx=BN_CTX_new();
    result1=BN_new();
    hashbn=BN_new();

    err=Cryp_RipeMD160((unsigned char*)text,size,hash,sizeof(hash));
    if (!Error_IsOk(err))
	return err;
    hashbn=BN_bin2bn(hash,sizeof(hash), hashbn);

    BN_CTX_start(ctx);
    // result=hashbn^EXPONENT mod MODULUS
    l=BN_mod_exp(result1, hashbn, k->key->d, k->key->n, ctx);

    // store resulting signature in buffer
    l=BN_bn2bin(result1,buffer);
    assert(l<=*bsize);
    *bsize=l;

    BN_free(result1);
    BN_free(hashbn);
    BN_CTX_free(ctx);

    return 0;
}


ERRORCODE Cryp_Rsa_Verify(CRYP_RSAKEYPTR k,
			  const unsigned char *text,
			  unsigned int size,
			  const unsigned char *signature,
			  unsigned int ssize) {
    BN_CTX *ctx;
    BIGNUM *result1;
    BIGNUM *hashbn;
    BIGNUM *signbn;
    int l;
    unsigned char hash[RIPEMD160_DIGEST_LENGTH];
    ERRORCODE err;
    int match;

    ctx=BN_CTX_new();
    result1=BN_new();
    hashbn=BN_new();
    signbn=BN_new();
    match=0;

    // decrypt signature
    signbn=BN_bin2bn(signature,ssize, signbn);
    BN_CTX_start(ctx);
    // result=hashbn^EXPONENT mod MODULUS
    l=BN_mod_exp(result1, signbn, k->key->e, k->key->n, ctx);

    // create rmd of the given data for comparison
    err=Cryp_RipeMD160(text,size,hash,sizeof(hash));
    if (!Error_IsOk(err))
	return err;
    hashbn=BN_bin2bn(hash,sizeof(hash), hashbn);

    if (BN_cmp(result1,hashbn)==0)
	match=1;

    BN_free(result1);
    BN_free(hashbn);
    BN_free(signbn);
    BN_CTX_free(ctx);
    if (match==0)
	return Error_New(0,
			 ERROR_SEVERITY_ERR,
			 cryp_error_descr.typ,
			 CRYP_ERROR_BAD_SIGNATURE);
    return 0;
}


ERRORCODE Cryp_RsaKey_GetChunkSize(CRYP_RSAKEYPTR k, int *size){
    assert(k);
    assert(k->key);
    *size=RSA_size(k->key);
    return 0;
}


CRYP_BFKEY *Cryp_BlowfishKey_new(){
  CRYP_BFKEY *k;

  k=(CRYP_BFKEY *)malloc(sizeof(CRYP_BFKEY));
  assert(k);
  memset(k,0,sizeof(CRYP_BFKEY));

  return k;
}


void Cryp_BlowfishKey_free(CRYP_BFKEY *k){
  if (k) {
    memset(k,0,sizeof(CRYP_BFKEY));
    free(k);
  }
}


ERRORCODE Cryp_BlowfishKey_SetKey(CRYP_BFKEY *key,
				  const char *data,
				  int len){
  assert(key);
  if (data==0) {
    memset(key,0,sizeof(CRYP_BFKEY));
    key->keylen=len;
    return 0;
  }
  if (len>sizeof(key->keydata))
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BAD_SIZE);

  memmove(key->keydata, data, len);
  key->keylen=len;
  BF_set_key(&key->key,len, (const unsigned char*)data);

  return 0;
}


ERRORCODE Cryp_BlowfishKey_GetKey(CRYP_BFKEY *key,
				  char **data,
				  int *len){
  assert(key);
  *data=key->keydata;
  *len=key->keylen;
  return 0;
}


ERRORCODE Cryp_Blowfish_Encrypt(CRYP_BFKEY *key,
				const char *indata,
				int size,
				char *outdata){
  const char *dbg_outdata;
  int dbg_outsize;

  dbg_outdata=outdata;
  dbg_outsize=size;

  assert(key);
  if (size%8)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BAD_SIZE);
  while(size) {
    BF_ecb_encrypt(indata,
		   outdata,
		   &key->key,
		   BF_ENCRYPT);
    indata+=8;
    outdata+=8;
    size-=8;
  }
  return 0;
}


ERRORCODE Cryp_Blowfish_Decrypt(CRYP_BFKEY *key,
				const char *indata,
				int size,
				char *outdata){
  const char *dbg_outdata;
  int dbg_outsize;

  dbg_outdata=outdata;
  dbg_outsize=size;

  assert(key);
  if (size%8)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BAD_SIZE);
  while(size) {
    BF_ecb_encrypt(indata,
		   outdata,
		   &key->key,
		   BF_DECRYPT);
    indata+=8;
    outdata+=8;
    size-=8;
  }
  return 0;
}


ERRORCODE Cryp_BlowfishKey_GenerateKey(CRYP_BFKEY *key){
  if (RAND_bytes(key->keydata,sizeof(key->keydata))!=1)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_KEY_GENERATION);
  BF_set_key(&(key->key),sizeof(key->keydata),
	     (const unsigned char*)(&key->keydata));
  key->keylen=sizeof(key->keydata);
  return 0;
}









const char *Cryp_ErrorString(int c){
    const char *s;

    switch(c) {
    case 0:
	s="Success";
	break;
    case CRYP_ERROR_MEMORY_FULL:
	s="Memory full";
        break;
    case CRYP_ERROR_KEY_GENERATION:
	s="Could not generate RSA key";
	break;
    case CRYP_ERROR_BUFFER_TOO_SMALL:
	s="Buffer too small";
	break;
    case CRYP_ERROR_BAD_PADDING:
	s="Bad padding";
	break;
    case CRYP_ERROR_ENCRYPTION:
	s="Error while encrypting";
	break;
    case CRYP_ERROR_DECRYPTION:
	s="Error while decrypting";
	break;
    case CRYP_ERROR_BAD_SIZE:
	s="Bad size of data, needs padding";
	break;
    case CRYP_ERROR_BAD_SIGNATURE:
	s="Bad signature";
        break;
    default:
	s=(const char*)0;
    } // switch
    return s;
}


ERRORCODE Cryp_Encrypt(void *key,
		       CryptAlgo algo,
		       const unsigned char *source,
		       int insize,
		       unsigned char **outbuffer,
		       int *outsize){
  int paddsize;
  char *buffer;
  char *destbuffer;
  ERRORCODE err;
  int i;
  int offset;

  /* get padding size */
  switch(algo) {
  case CryptAlgoBlowfish:
    paddsize=insize+(8-((insize)%8));
    break;

  case CryptAlgoRSA:
    paddsize=RSA_size(((CRYP_RSAKEY*)key)->key);
    if (insize>paddsize) {
      DBG_ERROR("Bad size of modulus: %d (should be >=%i)",
                paddsize, insize);
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       cryp_error_descr.typ,
		       CRYP_ERROR_BAD_SIZE);
    }
    break;

  default:
    DBG_ERROR("unknown algothithm");
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BAD_ALGO);
  } /* switch */

  /* allocate buffers */
  buffer=(char*)malloc(paddsize);
  assert(buffer);
  destbuffer=(char*)malloc(paddsize);
  assert(destbuffer);

  offset=paddsize-(insize%paddsize);
  /* copy message into buffer (at appropriate offset) */
  memmove(buffer+offset, source, insize);

  /* padd */
  for (i=0; i<offset-1; i++)
    buffer[i]=0;
  buffer[i]=0x80;

  /* now encode */
  switch(algo) {
  case CryptAlgoBlowfish:
    err=Cryp_Blowfish_Encrypt((CRYP_BFKEY*)key,
			      buffer,
			      paddsize,
			      destbuffer);
    break;
  case CryptAlgoRSA:
    err=Cryp_Rsa_CryptPublic((CRYP_RSAKEY*)key,
			     buffer,
			     paddsize,
			     destbuffer,
			     paddsize);
    break;
  default:
    DBG_ERROR("unknown algothithm");
    err=Error_New(0,
		  ERROR_SEVERITY_ERR,
		  cryp_error_descr.typ,
		  CRYP_ERROR_BAD_ALGO);
    break;
  } /* switch */
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    free(buffer);
    free(destbuffer);
    return err;
  }

  *outbuffer=destbuffer;
  *outsize=paddsize;
  memset(buffer, 0, paddsize);
  free(buffer);

  return 0;
}


ERRORCODE Cryp_Decrypt(void *key,
		       CryptAlgo algo,
		       const unsigned char *source,
		       int insize,
		       unsigned char **outbuffer,
		       int *outsize){
  char *buffer;
  char *destbuffer;
  ERRORCODE err;
  int i;
  char *ptr;

  buffer=(char*)malloc(insize);
  assert(buffer);

  /* decrypt */
  switch(algo) {
  case CryptAlgoBlowfish:
    err=Cryp_Blowfish_Decrypt((CRYP_BFKEY*)key,
			      source,insize,
			      buffer);
    break;

  case CryptAlgoRSA:
    err=Cryp_Rsa_DecryptPrivate((CRYP_RSAKEY*)key,
                                source, insize,
				buffer, insize);
    break;

  default:
    DBG_ERROR("unknown algothithm");
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BAD_ALGO);
  } /* switch */
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    free(buffer);
    return err;
  }

  /* unpadd */
  i=0;
  ptr=buffer;
  while (i<insize) {
    if (ptr[i]!=0)
      break;
    i++;
  } /* while */
  if (i>=insize) {
    DBG_ERROR("Bad padding (missing 0x80)");
    free(buffer);
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BAD_PADDING);
  }
  if ((unsigned char)(ptr[i++])!=0x80) {
    DBG_ERROR("Bad padding (char is not 0x80)");
    free(buffer);
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BAD_PADDING);
  }
  destbuffer=(char*)malloc(insize-i);
  assert(destbuffer);

  /* copy into destination buffer */
  memmove(destbuffer, ptr+i,insize-i);
  memset(buffer, 0, insize);
  free(buffer);
  *outbuffer=destbuffer;
  *outsize=insize-i;

  return 0;
}


CRYP_RMD160 *Cryp_RMD160_new(){
  CRYP_RMD160 *r;

  r=(CRYP_RMD160*)malloc(sizeof(CRYP_RMD160));
  assert(r);
  memset(r,0,sizeof(CRYP_RMD160));
  return r;
}


void Cryp_RMD160_free(CRYP_RMD160 *r){
  free(r);
}


ERRORCODE Cryp_RMD160_Init(CRYP_RMD160 *r){
  assert(r);
  RIPEMD160_Init(&r->ctx);
  return 0;
}


ERRORCODE Cryp_RMD160_Update(CRYP_RMD160 *r,
			     const unsigned char *data,
			     int bsize){
  assert(r);
  RIPEMD160_Update(&r->ctx,
		   data,
		   bsize);
  return 0;
}


ERRORCODE Cryp_RMD160_Final(CRYP_RMD160 *r,
			    unsigned char *buffer,
			    int *bsize){
  assert(r);
  if (*bsize<20)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     cryp_error_descr.typ,
		     CRYP_ERROR_BUFFER_TOO_SMALL);
  RIPEMD160_Final(buffer, &r->ctx);
  *bsize=20;
  return 0;
}





