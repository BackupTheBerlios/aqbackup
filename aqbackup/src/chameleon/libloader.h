/***************************************************************************
  $RCSfile: libloader.h,v $
                             -------------------
    cvs         : $Id: libloader.h,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
    begin       : Fri Nov 22 2002
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


#ifndef CHAMELEON_LIBLOADER_H
#define CHAMELEON_LIBLOADER_H "$Id"

#define LIBLOADER_TYPE "LIB"
#define LIBLOADER_ERROR_COULD_NOT_LOAD  1
#define LIBLOADER_ERROR_NOT_OPEN        2
#define LIBLOADER_ERROR_COULD_NOT_CLOSE 3
#define LIBLOADER_ERROR_COULD_RESOLVE   4

#include <chameleon/error.h>

#ifdef __cplusplus
extern "C" {
#endif


CHIPCARD_API struct CHLIBLOADERSTRUCT {
  void *handle;
};


CHIPCARD_API typedef struct CHLIBLOADERSTRUCT CHLIBLOADERTABLE;
CHIPCARD_API typedef CHLIBLOADERTABLE* CHLIBLOADERHANDLE;


CHIPCARD_API ERRORCODE LibLoader_ModuleInit();
CHIPCARD_API ERRORCODE LibLoader_ModuleFini();


CHIPCARD_API CHLIBLOADERHANDLE LibLoader_new();
CHIPCARD_API void LibLoader_free(CHLIBLOADERHANDLE h);

CHIPCARD_API ERRORCODE LibLoader_OpenLibrary(CHLIBLOADERHANDLE h, const char *name);
CHIPCARD_API ERRORCODE LibLoader_CloseLibrary(CHLIBLOADERHANDLE h);
CHIPCARD_API ERRORCODE LibLoader_Resolve(CHLIBLOADERHANDLE h, const char *name, void **p);

#ifdef __cplusplus
}
#endif


#endif /* CHAMELEON_LIBLOADER_H */


