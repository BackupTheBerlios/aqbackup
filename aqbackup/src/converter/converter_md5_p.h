/***************************************************************************
 $RCSfile: converter_md5_p.h,v $
                             -------------------
    cvs         : $Id: converter_md5_p.h,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#ifndef AQBACKUP_CONVERTER_MD5_P_H
#define AQBACKUP_CONVERTER_MD5_P_H

#include "converter_md5.h"

#include <openssl/md5.h>



typedef struct _CONVERTER_MD5_DATA CONVERTER_MD5_DATA;
struct _CONVERTER_MD5_DATA {
  MD5_CTX *ctx;
  int eof;
  int done;
  int nextPosForGetData;
  char digest[MD5_DIGEST_LENGTH];
};


CONVERTER_MD5_DATA *ConverterMd5Data_new();
void ConverterMd5Data_free(CONVERTER_MD5_DATA *dzd);

void ConverterMd5_FreePrivate(CONVERTER *dm);


int ConverterMd5_Begin(CONVERTER *dm);
unsigned int ConverterMd5_HasData(CONVERTER *dm);
unsigned int ConverterMd5_NeedsData(CONVERTER *dm);
int ConverterMd5_SetData(CONVERTER *dm,
			 const char *data,
			 unsigned int size);
int ConverterMd5_GetData(CONVERTER *dm,
			 char *data,
			 unsigned int size);
int ConverterMd5_Work(CONVERTER *dm);
int ConverterMd5_End(CONVERTER *dm);


#endif


