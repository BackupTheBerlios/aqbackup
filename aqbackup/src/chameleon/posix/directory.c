/***************************************************************************
 $RCSfile: directory.c,v $
                             -------------------
    cvs         : $Id: directory.c,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
    begin       : Tue Dec 03 2002
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

#include "directory.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <chameleon/debug.h>



DIRECTORYDATA *Directory_new(){
  DIRECTORYDATA *d;

  d=(DIRECTORYDATA *)malloc(sizeof(DIRECTORYDATA));
  assert(d);
  memset(d,0,sizeof(DIRECTORYDATA));
  return d;
}


void Directory_free(DIRECTORYDATA *d){
  if (d) {
    if (d->handle)
      closedir(d->handle);
    d->handle=0;
    free(d);
  }
}


int Directory_Open(DIRECTORYDATA *d, const char *n){
  assert(d);

  d->handle=opendir(n);
  if (d->handle==0)
    return 1;
  return 0;
}


int Directory_Close(DIRECTORYDATA *d){
  int rv;

  assert(d);
  rv=closedir(d->handle);
  d->handle=0;
  return rv;
}


int Directory_Read(DIRECTORYDATA *d,
		   char *buffer,
		   int len){
  struct dirent *de;

  assert(d);
  assert(buffer);
  assert(len);

  de=readdir(d->handle);
  if (de) {
    if (len<strlen(de->d_name)+1) {
      DBG_ERROR("Buffer too small");
      return 1;
    }
    strcpy(buffer,de->d_name);
    return 0;
  }
  return 1;
}


int Directory_Rewind(DIRECTORYDATA *d){
  assert(d);
  if (d->handle==0)
    return 1;
  rewinddir(d->handle);
  return 0;
}
















