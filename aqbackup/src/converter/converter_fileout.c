/***************************************************************************
 $RCSfile: converter_fileout.c,v $
                             -------------------
    cvs         : $Id: converter_fileout.c,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#include "converter_fileout_p.h"

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>


CONVERTER_FILEOUT_DATA *ConverterFileOutData_new(const char *fname,
						 const char *mode){
  CONVERTER_FILEOUT_DATA *dzd;

  dzd=(CONVERTER_FILEOUT_DATA *)malloc(sizeof(CONVERTER_FILEOUT_DATA));
  assert(dzd);
  memset(dzd, 0, sizeof(CONVERTER_FILEOUT_DATA));
  if (fname)
    dzd->filename=strdup(fname);
  if (mode)
    dzd->mode=strdup(mode);
  return dzd;
}


void ConverterFileOutData_free(CONVERTER_FILEOUT_DATA *dzd){
  if (dzd) {
    free(dzd->filename);
    free(dzd->mode);
    free(dzd);
  }
}



int ConverterFileOut_Begin(CONVERTER *dm){
  CONVERTER_FILEOUT_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEOUT_DATA *)(dm->privateData);

  if (dzd->filename) {
    dzd->file=fopen(dzd->filename, dzd->mode);
    if (!dzd->file) {
      DBG_ERROR("Error on fopen(%s): %s",
		dzd->filename, strerror(errno));
      return CONVERTER_RESULT_ERROR;
    }
  }
  else {
    DBG_INFO("Using stdout");
    dzd->file=stdout;
  }

  dzd->eof=0;

  return CONVERTER_RESULT_OK;
}



unsigned int ConverterFileOut_HasData(CONVERTER *dm){
  CONVERTER_FILEOUT_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEOUT_DATA *)(dm->privateData);
  assert(dzd->file);

  return 0;
}



unsigned int ConverterFileOut_NeedsData(CONVERTER *dm){
  CONVERTER_FILEOUT_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEOUT_DATA *)(dm->privateData);
  assert(dzd->file);

  if (dzd->eof)
    return 0;
  return CONVERTER_FILEOUT_CHUNKSIZE;
}



int ConverterFileOut_SetData(CONVERTER *dm,
			    const char *data,
			    unsigned int size){
  CONVERTER_FILEOUT_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEOUT_DATA *)(dm->privateData);
  assert(dzd->file);

  if (dzd->eof) {
    DBG_ERROR("EOF already set, will not accept more data");
    return 0;
  }
  if (size==0) {
    dzd->eof=1;
    return 0;
  }
  else {
    if (fwrite(data, size, 1, dzd->file)!=1) {
      DBG_ERROR("Error on fwrite(%s): %s",
		dzd->filename, strerror(errno));
      return CONVERTER_RESULT_ERROR;
    }
    dm->bytesReceived+=size;
  }
  return size;
}



int ConverterFileOut_GetData(CONVERTER *dm,
			 char *data,
			 unsigned int size){
  CONVERTER_FILEOUT_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEOUT_DATA *)(dm->privateData);
  assert(dzd->file);

  return 0;
}



int ConverterFileOut_Work(CONVERTER *dm){
  CONVERTER_FILEOUT_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEOUT_DATA *)(dm->privateData);
  assert(dzd->file);

  if (dzd->eof)
    return CONVERTER_RESULT_EOF;
  return CONVERTER_RESULT_OK;
}



int ConverterFileOut_End(CONVERTER *dm){
  CONVERTER_FILEOUT_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEOUT_DATA *)(dm->privateData);
  assert(dzd->file);

  if (dzd->filename) {
    int rv;

    rv=fclose(dzd->file);
    dzd->file=0;
    if (rv) {
      DBG_ERROR("Error on fclose(%s): %s",
		dzd->filename, strerror(errno));
      return CONVERTER_RESULT_ERROR;
    }
  }
  dzd->file=0;

  return CONVERTER_RESULT_OK;
}



void ConverterFileOut_FreePrivate(CONVERTER *dm){
  assert(dm);
  if (dm->privateData)
    ConverterFileOutData_free((CONVERTER_FILEOUT_DATA*)(dm->privateData));
}


CONVERTER *ConverterFileOut_new(const char *fname, const char *mode){
  CONVERTER *dm;
  CONVERTER_FILEOUT_DATA *dzd;

  dm=Converter_new();
  dzd=ConverterFileOutData_new(fname, mode);
  dm->privateData=dzd;

  /* set function pointers */
  dm->beginPtr=ConverterFileOut_Begin;
  dm->endPtr=ConverterFileOut_End;
  dm->hasDataPtr=ConverterFileOut_HasData;
  dm->needsDataPtr=ConverterFileOut_NeedsData;
  dm->setDataPtr=ConverterFileOut_SetData;
  dm->getDataPtr=ConverterFileOut_GetData;
  dm->workPtr=ConverterFileOut_Work;
  dm->freePrivatePtr=ConverterFileOut_FreePrivate;

  dm->typename=strdup("FileOut");
  return dm;
}






