/***************************************************************************
 $RCSfile: converter_md5.c,v $
                             -------------------
    cvs         : $Id: converter_md5.c,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
    begin       : Sat May 31 2003
    copyright   : (C) 2003 by Martin Preuss
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


#include "converter_md5_p.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>


CONVERTER_MD5_DATA *ConverterMd5Data_new(){
  CONVERTER_MD5_DATA *dzd;

  dzd=(CONVERTER_MD5_DATA *)malloc(sizeof(CONVERTER_MD5_DATA));
  assert(dzd);
  memset(dzd, 0, sizeof(CONVERTER_MD5_DATA));
  return dzd;
}


void ConverterMd5Data_free(CONVERTER_MD5_DATA *dzd){
  if (dzd) {
    free(dzd->ctx);
    free(dzd);
  }
}



int ConverterMd5_Begin(CONVERTER *dm){
  CONVERTER_MD5_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_MD5_DATA *)(dm->privateData);

  dzd->nextPosForGetData=0;

  /* create context */
  dzd->ctx=(MD5_CTX*)malloc(sizeof(MD5_CTX));
  assert(dzd->ctx);
  memset(dzd->ctx, 0, sizeof(MD5_CTX));

  MD5_Init(dzd->ctx);

  return CONVERTER_RESULT_OK;
}



unsigned int ConverterMd5_HasData(CONVERTER *dm){
  CONVERTER_MD5_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_MD5_DATA *)(dm->privateData);
  assert(dzd->ctx);

  if (dzd->done)
    return sizeof(dzd->digest)-dzd->nextPosForGetData;
  return 0;
}



unsigned int ConverterMd5_NeedsData(CONVERTER *dm){
  CONVERTER_MD5_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_MD5_DATA *)(dm->privateData);
  assert(dzd->ctx);

  if (dzd->eof)
    return 0;
  return 1024;
}



int ConverterMd5_SetData(CONVERTER *dm,
			    const char *data,
			    unsigned int size){
  CONVERTER_MD5_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_MD5_DATA *)(dm->privateData);
  assert(dzd->ctx);

  /* check if eof was set */
  if (dzd->eof) {
    DBG_INFO("Eof already set, will not accept data");
    return 0;
  }

  /* check whether EOF is to be set (indicated by size=0) */
  if (size==0) {
    dzd->eof=1;
    dzd->done=1;
    DBG_DEBUG("EOF set");
    MD5_Final(dzd->digest, dzd->ctx);
    return 0;
  }
  else {
    MD5_Update(dzd->ctx,
	       data,
	       size);
    return size;
  }
}



int ConverterMd5_GetData(CONVERTER *dm,
			 char *data,
			 unsigned int size){
  CONVERTER_MD5_DATA *dzd;
  unsigned int i;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_MD5_DATA *)(dm->privateData);
  assert(dzd->ctx);

  if (!(dzd->done)) {
    DBG_ERROR("No data");
    return CONVERTER_RESULT_ERROR;
  }

  i=sizeof(dzd->digest)-dzd->nextPosForGetData;
  if (size>i)
    size=i;
  if (size) {
    assert(data);
    memmove(data, dzd->digest+dzd->nextPosForGetData, size);
  }
  dzd->nextPosForGetData+=size;
  return size;
}



int ConverterMd5_Work(CONVERTER *dm){
  CONVERTER_MD5_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_MD5_DATA *)(dm->privateData);
  assert(dzd->ctx);

  if (dzd->done)
    return CONVERTER_RESULT_EOF;
  return CONVERTER_RESULT_OK;
}



int ConverterMd5_End(CONVERTER *dm){
  CONVERTER_MD5_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_MD5_DATA *)(dm->privateData);
  assert(dzd->ctx);

  free(dzd->ctx);
  dzd->ctx=0;
  return CONVERTER_RESULT_OK;
}



void ConverterMd5_FreePrivate(CONVERTER *dm){
  assert(dm);
  if (dm->privateData)
    ConverterMd5Data_free((CONVERTER_MD5_DATA*)(dm->privateData));
}


CONVERTER *ConverterMd5_new(int level){
  CONVERTER *dm;
  CONVERTER_MD5_DATA *dzd;

  dm=Converter_new();
  dzd=ConverterMd5Data_new();
  dm->privateData=dzd;

  /* set function pointers */
  dm->beginPtr=ConverterMd5_Begin;
  dm->endPtr=ConverterMd5_End;
  dm->hasDataPtr=ConverterMd5_HasData;
  dm->needsDataPtr=ConverterMd5_NeedsData;
  dm->setDataPtr=ConverterMd5_SetData;
  dm->getDataPtr=ConverterMd5_GetData;
  dm->workPtr=ConverterMd5_Work;
  dm->freePrivatePtr=ConverterMd5_FreePrivate;

  dm->typename=strdup("Md5");
  return dm;
}






