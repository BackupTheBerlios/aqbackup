/***************************************************************************
 $RCSfile: backupserver_p.h,v $
                             -------------------
    cvs         : $Id: backupserver_p.h,v 1.1 2003/06/07 21:07:53 aquamaniac Exp $
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

#ifndef BACKUPSERVER_P_H
#define BACKUPSERVER_P_H

#include <backupserver.h>
#include <backup/server.h>

#include <chameleon/chameleon.h>
#include <service/ctserver.h>



struct _BACKUPSERVERDATA {
  AQB_SERVER *bserver;
};


BACKUPSERVERDATA *BackupServerData_new();
void BackupServerData_free(BACKUPSERVERDATA *bd);


ERRORCODE BackupServer_RequestHandler(CTSERVERDATA *sd,
				      IPCMESSAGELAYER *ml,
				      IPCMESSAGE *msg);
ERRORCODE BackupServer_ClientUp(CTSERVERDATA *sd,
				IPCMESSAGELAYER *ml);
ERRORCODE BackupServer_ClientDown(CTSERVERDATA *sd,
				  IPCMESSAGELAYER *ml);








#endif





