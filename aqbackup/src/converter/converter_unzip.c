/***************************************************************************
 $RCSfile: converter_unzip.c,v $
                             -------------------
    cvs         : $Id: converter_unzip.c,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#include "converter_unzip_p.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>


CONVERTER_UNZIP_DATA *ConverterUnzipData_new(){
  CONVERTER_UNZIP_DATA *dzd;

  dzd=(CONVERTER_UNZIP_DATA *)malloc(sizeof(CONVERTER_UNZIP_DATA));
  assert(dzd);
  memset(dzd, 0, sizeof(CONVERTER_UNZIP_DATA));
  return dzd;
}


void ConverterUnzipData_free(CONVERTER_UNZIP_DATA *dzd){
  if (dzd) {
    free(dzd->zstream);
    free(dzd);
  }
}



int ConverterUnzip_Begin(CONVERTER *dm){
  CONVERTER_UNZIP_DATA *dzd;
  int rv;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_UNZIP_DATA *)(dm->privateData);

  /* create z_stream */
  dzd->zstream=(z_stream*)malloc(sizeof(z_stream));
  assert(dzd->zstream);
  memset(dzd->zstream, 0, sizeof(z_stream));

  /* setup buffers */
  dzd->zstream->next_in=dzd->inbuffer;
  dzd->zstream->avail_in=0;
  dzd->zstream->next_out=dzd->outbuffer;
  dzd->zstream->avail_out=sizeof(dzd->outbuffer);

  /* setup defaults for memory allocation functions */
  dzd->zstream->zalloc=Z_NULL;
  dzd->zstream->zfree=Z_NULL;

  dzd->nextPosForGetData=0;
  dzd->nextPosForSetData=0;

  rv=inflateInit(dzd->zstream);
  if (rv!=Z_OK) {
    DBG_ERROR("Error on inflateInit(): %s (%d)",
	      dzd->zstream->msg, rv);
    return CONVERTER_RESULT_ERROR;
  }

  return CONVERTER_RESULT_OK;
}



unsigned int ConverterUnzip_HasData(CONVERTER *dm){
  CONVERTER_UNZIP_DATA *dzd;
  unsigned int zpos;
  unsigned int size;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_UNZIP_DATA *)(dm->privateData);
  assert(dzd->zstream);

  zpos=((char*)(dzd->zstream->next_out))-dzd->outbuffer;

  if (dzd->nextPosForGetData>zpos)
    size=sizeof(dzd->outbuffer)-dzd->nextPosForGetData;
  else if (dzd->nextPosForGetData==zpos &&
	   dzd->zstream->avail_out==0)
    size=sizeof(dzd->outbuffer)-dzd->nextPosForGetData;
  else
    size=zpos-dzd->nextPosForGetData;

  if (dzd->nextPosForGetData>zpos)
    return sizeof(dzd->outbuffer)-dzd->nextPosForGetData;
  else
    return zpos-dzd->nextPosForGetData;
  return size;
}



unsigned int ConverterUnzip_NeedsData(CONVERTER *dm){
  CONVERTER_UNZIP_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_UNZIP_DATA *)(dm->privateData);
  assert(dzd->zstream);

  if (dzd->zstream->avail_in==0 && !dzd->eof)
    return sizeof(dzd->inbuffer);
  return 0;
}



int ConverterUnzip_SetData(CONVERTER *dm,
			    const char *data,
			    unsigned int size){
  CONVERTER_UNZIP_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_UNZIP_DATA *)(dm->privateData);
  assert(dzd->zstream);

  /* check if eof was set */
  if (dzd->eof) {
    DBG_ERROR("Eof already set, will not accept data");
    return 0;
  }

  /* check whether EOF is to be set (indicated by size=0) */
  if (size==0) {
    dzd->eof=1;
    DBG_DEBUG("EOF set");
    return 0;
  }
  else {
    if (size>sizeof(dzd->inbuffer))
      size=sizeof(dzd->inbuffer);
    if (size) {
      assert(data);
      memmove(dzd->inbuffer, data, size);
      dzd->zstream->next_in=dzd->inbuffer;
      dzd->zstream->avail_in=size;
    }
    DBG_DEBUG("Data taken: %d bytes", size);
    return size;
  }
}



int ConverterUnzip_GetData(CONVERTER *dm,
			    char *data,
			    unsigned int size){
  CONVERTER_UNZIP_DATA *dzd;
  unsigned int i;
  unsigned int zpos;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_UNZIP_DATA *)(dm->privateData);
  assert(dzd->zstream);

  i=ConverterUnzip_HasData(dm);
  if (size>i)
    size=i;
  if (size) {
    zpos=((char*)(dzd->zstream->next_out))-dzd->outbuffer;
    assert(data);
    memmove(data, dzd->outbuffer+dzd->nextPosForGetData, size);
    dzd->nextPosForGetData+=size;

    /* validate all pointers */
    if (dzd->nextPosForGetData>=sizeof(dzd->outbuffer))
      dzd->nextPosForGetData=0;

    if (zpos>=sizeof(dzd->outbuffer)) {
      zpos=0;
      dzd->zstream->next_out=dzd->outbuffer;
    }

    /* set avail_out */
    if (zpos>=dzd->nextPosForGetData) {
      dzd->zstream->avail_out=sizeof(dzd->outbuffer)-zpos;
    }
    else {
      dzd->zstream->avail_out=dzd->nextPosForGetData-zpos;
    }
  }
  return size;
}



int ConverterUnzip_Work(CONVERTER *dm){
  CONVERTER_UNZIP_DATA *dzd;
  int rv;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_UNZIP_DATA *)(dm->privateData);
  assert(dzd->zstream);

  if (dzd->zstream->avail_out==0) {
    DBG_INFO("Output buffer full, doing nothing");
    return CONVERTER_RESULT_OK;
  }

  if (dzd->eof)
    rv=inflate(dzd->zstream, Z_FINISH);
  else
    rv=inflate(dzd->zstream, Z_NO_FLUSH);
  if (rv!=Z_OK) {
    if (rv==Z_STREAM_END) {
      DBG_INFO("Stream end reached");
      return CONVERTER_RESULT_EOF;
    }
    else {
      DBG_ERROR("Error on inflate(): %s (%d)",
		dzd->zstream->msg, rv);
      return CONVERTER_RESULT_ERROR;
    }
  }

  return CONVERTER_RESULT_OK;
}



int ConverterUnzip_End(CONVERTER *dm){
  CONVERTER_UNZIP_DATA *dzd;
  int rv;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_UNZIP_DATA *)(dm->privateData);
  assert(dzd->zstream);

  rv=inflateEnd(dzd->zstream);
  if (rv!=Z_OK) {
    DBG_ERROR("Error on inflateEnd(): %s (%d)",
	      dzd->zstream->msg, rv);
    return CONVERTER_RESULT_ERROR;
  }

  free(dzd->zstream);
  dzd->zstream=0;

  return CONVERTER_RESULT_OK;
}



void ConverterUnzip_FreePrivate(CONVERTER *dm){
  assert(dm);
  if (dm->privateData)
    ConverterUnzipData_free((CONVERTER_UNZIP_DATA*)(dm->privateData));
}


CONVERTER *ConverterUnzip_new(){
  CONVERTER *dm;
  CONVERTER_UNZIP_DATA *dzd;

  dm=Converter_new();
  dzd=ConverterUnzipData_new();
  dm->privateData=dzd;

  /* set function pointers */
  dm->beginPtr=ConverterUnzip_Begin;
  dm->endPtr=ConverterUnzip_End;
  dm->hasDataPtr=ConverterUnzip_HasData;
  dm->needsDataPtr=ConverterUnzip_NeedsData;
  dm->setDataPtr=ConverterUnzip_SetData;
  dm->getDataPtr=ConverterUnzip_GetData;
  dm->workPtr=ConverterUnzip_Work;
  dm->freePrivatePtr=ConverterUnzip_FreePrivate;

  dm->typename=strdup("Unzip");
  return dm;
}






