/***************************************************************************
 $RCSfile: backupserver.c,v $
                             -------------------
    cvs         : $Id: backupserver.c,v 1.1 2003/06/07 21:07:53 aquamaniac Exp $
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <chameleon/chameleon.h>
#include "backupserver_p.h"
#include "backup/errors.h"

#include <ctserver.h>
#include <chameleon/conf.h>
#include <chameleon/debug.h>


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>



BACKUPSERVERDATA *BackupServerData_new(){
  BACKUPSERVERDATA *bd;

  bd=(BACKUPSERVERDATA *)malloc(sizeof(BACKUPSERVERDATA));
  assert(bd);
  memset(bd, 0, sizeof(BACKUPSERVERDATA));
  bd->bserver=AQBServer_new();
  return bd;
}



void BackupServerData_free(BACKUPSERVERDATA *bd){
  if (bd) {
    AQBServer_free(bd->bserver);

    free(bd);
  }
}



ERRORCODE BackupServer_RequestHandler(CTSERVERDATA *sd,
				      IPCMESSAGELAYER *ml,
				      IPCMESSAGE *msg) {
  return 0;
}



ERRORCODE BackupServer_ClientUp(CTSERVERDATA *sd,
				IPCMESSAGELAYER *ml){
  return 0;
}



ERRORCODE BackupServer_ClientDown(CTSERVERDATA *sd,
				  IPCMESSAGELAYER *ml){
  return 0;
}




int BackupServer_Init(CTSERVERDATA *sd, CONFIGGROUP *root) {
  BACKUPSERVERDATA *rsd;
  ERRORCODE err;
  int rv;

  DBG_INFO("Initializing reader server");
  assert(sd);

  /* initialize server */
  err=CTServer_Init(sd, root);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return err;
  }

  /* create new private data */
  rsd=BackupServerData_new();
  assert(rsd);

  /* ok, server is running, now setup Backupservice */
  rv=AQBServer_Init(rsd->bserver, root);
  if (rv) {
    DBG_INFO("Error initializing backup server");
    BackupServerData_free(rsd);
    return BACKUP_ERROR_SERVER_INIT;
  }

  /* set private data and callbacks */
  CTServer_SetPrivateData(sd, rsd);
  CTServer_SetRequestHandler(sd, BackupServer_RequestHandler);
  CTServer_SetClientUpHandler(sd, BackupServer_ClientUp);
  CTServer_SetClientDownHandler(sd, BackupServer_ClientDown);


  DBG_INFO("Backup server initialized");
  return 0;
}



ERRORCODE BackupServer_Fini(CTSERVERDATA *sd){
  ERRORCODE err1, err2;
  BACKUPSERVERDATA *rsd;

  DBG_ENTER;
  DBG_INFO("Deinitializing reader server");
  assert(sd);
  rsd=CTServer_GetPrivateData(sd);
  assert(rsd);

  err1=CTServer_Fini(sd);
  if (!Error_IsOk(err1)) {
    DBG_ERROR_ERR(err1);
  }

  CTServer_SetPrivateData(sd,0);
  BackupServerData_free(rsd);

  DBG_INFO("Reader server deinitialized.");
  DBG_LEAVE;
  if (!Error_IsOk(err1))
    return err1;
  if (!Error_IsOk(err2))
    return err2;

  return 0;
}



ERRORCODE BackupServer_Work(CTSERVERDATA *sd,
			    int timeout,
			    int maxmsg){
  ERRORCODE err;
  BACKUPSERVERDATA *rsd;

  DBG_ENTER;
  assert(sd);
  rsd=(BACKUPSERVERDATA*)CTServer_GetPrivateData(sd);
  assert(rsd);

  err=CTServer_Work(sd, timeout, maxmsg);
  if (!Error_IsOk(err)) {
    DBG_DEBUG_ERR(err);
  }


  /* do whatever is necessary */

  DBG_LEAVE;
  return err;
}
