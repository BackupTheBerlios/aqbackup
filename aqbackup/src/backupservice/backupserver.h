/***************************************************************************
 $RCSfile: backupserver.h,v $
                             -------------------
    cvs         : $Id: backupserver.h,v 1.2 2003/06/11 13:18:35 aquamaniac Exp $
    begin       : Sat Jun 07 2003
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


#ifndef BACKUPSERVER_H
#define BACKUPSERVER_H

#include <service/ctserver.h>
#include <backupservice/backupservice.h>


typedef struct _BACKUPSERVERDATA BACKUPSERVERDATA;



int BackupServer_Init(CTSERVERDATA *sd, CONFIGGROUP *root);
ERRORCODE BackupServer_Fini(CTSERVERDATA *sd);
ERRORCODE BackupServer_Work(CTSERVERDATA *sd,
			    int timeout,
			    int maxmsg);







#endif







