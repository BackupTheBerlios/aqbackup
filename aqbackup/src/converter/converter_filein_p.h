/***************************************************************************
 $RCSfile: converter_filein_p.h,v $
                             -------------------
    cvs         : $Id: converter_filein_p.h,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#ifndef AQBACKUP_CONVERTER_FILEIN_P_H
#define AQBACKUP_CONVERTER_FILEIN_P_H

#include "converter_filein.h"

#include <stdio.h>


typedef struct _CONVERTER_FILEIN_DATA CONVERTER_FILEIN_DATA;
struct _CONVERTER_FILEIN_DATA {
  int eof;
  int nextPosForGetData;
  char *filename;
  char outbuffer[2048];
  FILE *file;
  unsigned int bytesinbuffer;
};


CONVERTER_FILEIN_DATA *ConverterFileInData_new();
void ConverterFileInData_free(CONVERTER_FILEIN_DATA *dzd);

void ConverterFileIn_FreePrivate(CONVERTER *dm);


int ConverterFileIn_Begin(CONVERTER *dm);
unsigned int ConverterFileIn_HasData(CONVERTER *dm);
unsigned int ConverterFileIn_NeedsData(CONVERTER *dm);
int ConverterFileIn_SetData(CONVERTER *dm,
			 const char *data,
			 unsigned int size);
int ConverterFileIn_GetData(CONVERTER *dm,
			 char *data,
			 unsigned int size);
int ConverterFileIn_Work(CONVERTER *dm);
int ConverterFileIn_End(CONVERTER *dm);


#endif


