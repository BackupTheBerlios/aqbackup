/***************************************************************************
 $RCSfile: server_p.h,v $
                             -------------------
    cvs         : $Id: server_p.h,v 1.1 2003/06/07 21:07:47 aquamaniac Exp $
    begin       : Thue May 21 2003
    copyright   : (C) 2003 by Martin Preuss
    email       : openhbci@aquamaniac.de

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


#ifndef AQBACKUP_SERVER_P_H
#define AQBACKUP_SERVER_P_H


#define AQB_SERVER_MAX_TRY_NAME 10

#include "server.h"


char *AQBServer__EscapePath(const char *path,
			    char *buffer,
			    int maxsize);

AQB_ENTRY *AQBServer__GetEntry(AQB_SERVER *s,
			       AQB_DIRECTORY *d,
			       const char *name);

AQB_ENTRY *AQBServer__ReadEntry(AQB_SERVER *s,
				AQB_DIRECTORY *d,
				const char *name);

int AQBServer__WriteEntry(AQB_SERVER *s,
			  AQB_DIRECTORY *d,
			  AQB_ENTRY *e);

int AQBServer__AddEntryToDir(AQB_SERVER *s,
			     AQB_DIRECTORY *d,
			     AQB_ENTRY *enew);
int AQBServer__ReadEntries(AQB_SERVER *s, AQB_DIRECTORY *d);

int AQBServer__SetEntry(AQB_SERVER *s,
			int cid,
			int did,
			AQB_ENTRY *entry,
			int deleteIt);

AQB_REPOSITORY *AQBServer__LoadRepository(AQB_SERVER *s,
					  const char *hostname,
					  const char *repository);
int AQBServer__SaveRepository(AQB_SERVER *s,
			      AQB_REPOSITORY *r);
int AQBServer__CheckRepositoryUser(AQB_REPOSITORY *r, AQB_SERVER_CLIENT *c);


/**
 * Creates a filepath for a given fileid and fileversion.
 * Such a path looks like this:
 *   AA/AA/AA/AA/BB/BB/BB/BB.data
 * where AA is the hexadecimal representation of the file id
 * and   BB is the hexadecimal representation of the file version
 * The suffix ".data" is appended.
 */
char *AQBServer__MakeStoragePath(unsigned int fileid,
				 unsigned int filever,
				 char *buffer,
				 unsigned int maxsize);

int AQBServer__FileClose(AQB_SERVER *s,
			 int cid,
			 int fid);


#endif

