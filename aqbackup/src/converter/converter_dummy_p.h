/***************************************************************************
 $RCSfile: converter_dummy_p.h,v $
                             -------------------
    cvs         : $Id: converter_dummy_p.h,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#ifndef AQBACKUP_CONVERTER_DUMMY_P_H
#define AQBACKUP_CONVERTER_DUMMY_P_H

#include "converter_dummy.h"

#include <stdio.h>


#define CONVERTER_DUMMY_CHUNKSIZE 2048


typedef struct _CONVERTER_DUMMY_DATA CONVERTER_DUMMY_DATA;
struct _CONVERTER_DUMMY_DATA {
  int eof;
  int nextPosForGetData;
  char outbuffer[2048];
  unsigned int bytesinbuffer;
};


CONVERTER_DUMMY_DATA *ConverterDummyData_new();
void ConverterDummyData_free(CONVERTER_DUMMY_DATA *dzd);

void ConverterDummy_FreePrivate(CONVERTER *dm);


int ConverterDummy_Begin(CONVERTER *dm);
unsigned int ConverterDummy_HasData(CONVERTER *dm);
unsigned int ConverterDummy_NeedsData(CONVERTER *dm);
int ConverterDummy_SetData(CONVERTER *dm,
			 const char *data,
			 unsigned int size);
int ConverterDummy_GetData(CONVERTER *dm,
			 char *data,
			 unsigned int size);
int ConverterDummy_Work(CONVERTER *dm);
int ConverterDummy_End(CONVERTER *dm);


#endif


