/***************************************************************************
 $RCSfile: backupservice.h,v $
                             -------------------
    cvs         : $Id: backupservice.h,v 1.2 2003/06/11 13:18:35 aquamaniac Exp $
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

#ifndef BACKUPSERVICE_H
#define BACKUPSERVICE_H

#include <service/ctservice.h>



/* ping */
#define BACKUPSERVICE_MSGCODE_RQ_PING 0x0000
#define BACKUPSERVICE_MSGCODE_RQ_PING_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_PING 0x0001
#define BACKUPSERVICE_MSGCODE_RP_PING_VERSION 0x0100


/* client management */
#define BACKUPSERVICE_MSGCODE_RQ_REGISTER 0x0002
#define BACKUPSERVICE_MSGCODE_RQ_REGISTER_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_REGISTER 0x0003
#define BACKUPSERVICE_MSGCODE_RP_REGISTER_VERSION 0x0100

#define BACKUPSERVICE_MSGCODE_RQ_UNREGISTER 0x0004
#define BACKUPSERVICE_MSGCODE_RQ_UNREGISTER_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_UNREGISTER 0x0005
#define BACKUPSERVICE_MSGCODE_RP_UNREGISTER_VERSION 0x0100


/* backup management */
#define BACKUPSERVICE_MSGCODE_RQ_OPEN_BACKUP 0x0006
#define BACKUPSERVICE_MSGCODE_RQ_OPEN_BACKUP_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_OPEN_BACKUP 0x0007
#define BACKUPSERVICE_MSGCODE_RP_OPEN_BACKUP_VERSION 0x0100

#define BACKUPSERVICE_MSGCODE_RQ_CLOSE_BACKUP 0x0008
#define BACKUPSERVICE_MSGCODE_RQ_CLOSE_BACKUP_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_CLOSE_BACKUP 0x0009
#define BACKUPSERVICE_MSGCODE_RP_CLOSE_BACKUP_VERSION 0x0100

#define BACKUPSERVICE_MSGCODE_RQ_CREATE_BACKUP 0x000a
#define BACKUPSERVICE_MSGCODE_RQ_CREATE_BACKUP_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_CREATE_BACKUP 0x000b
#define BACKUPSERVICE_MSGCODE_RP_CREATE_BACKUP_VERSION 0x0100


/* directory management */
#define BACKUPSERVICE_MSGCODE_RQ_OPEN_DIR 0x000c
#define BACKUPSERVICE_MSGCODE_RQ_OPEN_DIR_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_OPEN_DIR 0x000d
#define BACKUPSERVICE_MSGCODE_RP_OPEN_DIR_VERSION 0x0100

#define BACKUPSERVICE_MSGCODE_RQ_CLOSE_DIR 0x000e
#define BACKUPSERVICE_MSGCODE_RQ_CLOSE_DIR_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_CLOSE_DIR 0x000f
#define BACKUPSERVICE_MSGCODE_RP_CLOSE_DIR_VERSION 0x0100


/* entry management */
#define BACKUPSERVICE_MSGCODE_RQ_GET_ENTRIES 0x0010
#define BACKUPSERVICE_MSGCODE_RQ_GET_ENTRIES_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_GET_ENTRIES 0x0011
#define BACKUPSERVICE_MSGCODE_RP_GET_ENTRIES_VERSION 0x0100

#define BACKUPSERVICE_MSGCODE_RQ_SET_ENTRY 0x0012
#define BACKUPSERVICE_MSGCODE_RQ_SET_ENTRY_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_SET_ENTRY 0x0013
#define BACKUPSERVICE_MSGCODE_RP_SET_ENTRY_VERSION 0x0100

#define BACKUPSERVICE_MSGCODE_RQ_DEL_ENTRY 0x0014
#define BACKUPSERVICE_MSGCODE_RQ_DEL_ENTRY_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_DEL_ENTRY 0x0015
#define BACKUPSERVICE_MSGCODE_RP_DEL_ENTRY_VERSION 0x0100


/* retrieving files */
#define BACKUPSERVICE_MSGCODE_RQ_OPENIN_FILE 0x0016
#define BACKUPSERVICE_MSGCODE_RQ_OPENIN_FILE_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_OPENIN_FILE 0x0017
#define BACKUPSERVICE_MSGCODE_RP_OPENIN_FILE_VERSION 0x0100

#define BACKUPSERVICE_MSGCODE_RQ_CLOSEIN_FILE 0x0018
#define BACKUPSERVICE_MSGCODE_RQ_CLOSEIN_FILE_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_CLOSEIN_FILE 0x0019
#define BACKUPSERVICE_MSGCODE_RP_CLOSEIN_FILE_VERSION 0x0100

#define BACKUPSERVICE_MSGCODE_RQ_READ_FILE 0x001a
#define BACKUPSERVICE_MSGCODE_RQ_READ_FILE_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_READ_FILE 0x001b
#define BACKUPSERVICE_MSGCODE_RP_READ_FILE_VERSION 0x0100


/* storing files */
#define BACKUPSERVICE_MSGCODE_RQ_OPENOUT_FILE 0x001c
#define BACKUPSERVICE_MSGCODE_RQ_OPENOUT_FILE_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_OPENOUT_FILE 0x001d
#define BACKUPSERVICE_MSGCODE_RP_OPENOUT_FILE_VERSION 0x0100

#define BACKUPSERVICE_MSGCODE_RQ_CLOSEOUT_FILE 0x001e
#define BACKUPSERVICE_MSGCODE_RQ_CLOSEOUT_FILE_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_CLOSEOUT_FILE 0x001f
#define BACKUPSERVICE_MSGCODE_RP_CLOSEOUT_FILE_VERSION 0x0100

#define BACKUPSERVICE_MSGCODE_RQ_WRITE_FILE 0x0020
#define BACKUPSERVICE_MSGCODE_RQ_WRITE_FILE_VERSION 0x0100
#define BACKUPSERVICE_MSGCODE_RP_WRITE_FILE 0x0021
#define BACKUPSERVICE_MSGCODE_RP_WRITE_FILE_VERSION 0x0100




#endif


