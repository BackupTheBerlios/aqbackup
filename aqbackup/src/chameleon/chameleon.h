/***************************************************************************
 $RCSfile: chameleon.h,v $
                             -------------------
    cvs         : $Id: chameleon.h,v 1.1 2003/06/07 21:07:48 aquamaniac Exp $
    begin       : Wed Oct 03 2002
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


#ifndef CHAMELEON_H
#define CHAMELEON_H


/* setup DLL imports/exports for Windoze */
#ifdef __declspec
# if BUILDING_CHIPCARD_DLL
#  define CHIPCARD_API __declspec (dllexport)
# else /* Not BUILDING_CHIPCARD_DLL */
#  define CHIPCARD_API __declspec (dllimport)
# endif /* Not BUILDING_CHIPCARD_DLL */
#else
# define CHIPCARD_API
#endif

#include "chameleon/error.h"

#ifdef __cplusplus
extern "C" {
#endif


CHIPCARD_API ERRORCODE Chameleon_Init();
CHIPCARD_API ERRORCODE Chameleon_Fini();

CHIPCARD_API void Chameleon_DumpString(const char *s, int l);


#ifdef __cplusplus
}
#endif

#endif // CHAMELEON_H

