/***************************************************************************
 $RCSfile: init.c,v $
                             -------------------
    cvs         : $Id: init.c,v 1.1 2003/06/07 21:07:49 aquamaniac Exp $
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


#include "chameleon/error.h"
#include "chameleon/inetaddr.h"


ERRORCODE Chameleon_Init() {
    ERRORCODE err;

    Error_ModuleInit();
    err=InetAddr_ModuleInit();
    if (!Error_IsOk(err))
	return err;
    err=Socket_ModuleInit();
    if (!Error_IsOk(err))
	return err;
    //add here more modules

    return 0;
}


ERRORCODE Chameleon_Fini() {
    ERRORCODE err;

    err=0;
    //add here more modules
    if (!Error_IsOk(InetAddr_ModuleFini()))
	err=Error_New(0,
		      ERROR_SEVERITY_ERR,
		      0,
		      ERROR_COULD_NOT_UNREGISTER);
    Error_ModuleFini();
    return err;
}





