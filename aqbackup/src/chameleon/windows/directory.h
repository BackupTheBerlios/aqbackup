/***************************************************************************
 $RCSfile: directory.h,v $
                             -------------------
    cvs         : $Id: directory.h,v 1.1 2003/06/07 21:07:51 aquamaniac Exp $
    begin       : Tue Jan 10 2003
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


#ifndef CH_DIRECTORY_H
#define CH_DIRECTORY_H


#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

CHIPCARD_API typedef struct DIRECTORYDATASTRUCT DIRECTORYDATA;


CHIPCARD_API struct DIRECTORYDATASTRUCT {
  char pattern[256];
  HANDLE handle;
  char lastName[256];
};


CHIPCARD_API DIRECTORYDATA *Directory_new();
CHIPCARD_API void Directory_free(DIRECTORYDATA *d);



CHIPCARD_API int Directory_Open(DIRECTORYDATA *d, const char *n);
CHIPCARD_API int Directory_Close(DIRECTORYDATA *d);

CHIPCARD_API int Directory_Read(DIRECTORYDATA *d,
				char *buffer,
				int len);
CHIPCARD_API int Directory_Rewind(DIRECTORYDATA *d);


#ifdef __cplusplus
}
#endif

#endif
