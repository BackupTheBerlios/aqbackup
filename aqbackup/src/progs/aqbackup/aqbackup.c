/***************************************************************************
 $RCSfile: aqbackup.c,v $
                             -------------------
    cvs         : $Id: aqbackup.c,v 1.1 2003/06/07 21:07:53 aquamaniac Exp $
    begin       : Wed Jun 04 2003
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

/* Internationalization */
#ifdef HAVE_GETTEXT_ENVIRONMENT
# include <libintl.h>
# include <locale.h>
# define I18N(m) gettext(m)
#else
# define I18N(m) m
#endif
#define I18NT(m) m

#include <backup/aqbackup.h>
#include <backup/misc.h>

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>
#include <chameleon/logger.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>


#define k_PRG "aqbackup"
#define k_PRG_VERSION_INFO \
  "aqbackup v0.1\n"\
  "(c) 2003 Martin Preuss<martin@libchipcard.de>\n"\
  "This program is free software licensed under GPL.\n"\
  "See COPYING for details.\n"


typedef struct _S_PARAM FREEPARAM;
typedef struct _S_ARGS ARGUMENTS;

struct _S_PARAM {
  FREEPARAM  *next;
  const char *param;
};



FREEPARAM *FreeParam_new(const char *s) {
  FREEPARAM *fr;

  fr=(FREEPARAM*)malloc(sizeof(FREEPARAM));
  assert(fr);
  memset(fr, 0, sizeof(FREEPARAM));
  fr->param=s;
  return fr;
}


void FreeParam_free(FREEPARAM *fr) {
  if (fr)
    free(fr);
}


struct _S_ARGS {
  FREEPARAM *params;
  unsigned int flags;     /* -v -r -i -n */
  int allowSaveCfg; /* can only be set by the program itself ! */
  char *configFile;       /* -C */
  char *logFile;          /* --logfile  */
  LOGGER_LOGTYPE logType; /* --logtype  */
  LOGGER_LEVEL logLevel;  /* --loglevel */

  int verbosity;          /* -v */
  char *typ;              /* --type */
  char *basedir;          /* --basedir */
  char *user;             /* --user */
  char *passwd;           /* --passwd */
  char *address;          /* --address */
  int port;               /* --port */
  int ziplevel;           /* --ziplevel */
  int createLocalOnly;    /* --localonly */
  char *serverConfigFile; /* --servercfg (only important for type "direct")*/
  char *destdir;          /* --destdir  */
  time_t timeAndDate;     /* --date --time */
};


void setTimeAndDate(struct tm *tm, time_t t, int gmt) {
  struct tm *tt;

  if (gmt)
    tt=gmtime(&t);
  else
    tt=localtime(&t);
  memmove(tm, tt, sizeof(*tm));
}


char *getHomeDir(char *buffer, unsigned int size) {
  struct passwd *pw;

  pw=getpwuid(getuid());
  if (!pw) {
    DBG_ERROR("No pwent for this user");
    return 0;
  }
  if (size<strlen(pw->pw_dir)) {
    DBG_ERROR("Buffer too small");
    return 0;
  }
  strcpy(buffer, pw->pw_dir);
  return buffer;
}



ARGUMENTS *Arguments_new() {
  ARGUMENTS *ar;

  ar=(ARGUMENTS*)malloc(sizeof(ARGUMENTS));
  assert(ar);
  memset(ar, 0, sizeof(ARGUMENTS));
  ar->logFile="aqbackup.log";
  ar->flags=0;
  ar->logType=LoggerTypeConsole;
  ar->logLevel=LoggerLevelWarning;
  ar->configFile=0;
  ar->allowSaveCfg=0;

  ar->typ="direct";
  ar->basedir="";
  ar->user="";
  ar->passwd="";
  ar->address=AQBACKUP_REPOSITORY_DIR;
  ar->port=-1;
  ar->ziplevel=6;
  ar->createLocalOnly=0;
  ar->serverConfigFile=AQBACKUPD_CFGFILE;
  return ar;
}


void Arguments_free(ARGUMENTS *ar) {
  if (ar) {
    FREEPARAM *fr;
    FREEPARAM *next;

    fr=ar->params;
    while(fr) {
      next=fr->next;
      FreeParam_free(fr);
      fr=next;
    } /* while */
    free(ar);
  }
}



void usage(const char *name) {
  fprintf(stdout,"%s%s%s\n",
	  I18N("AqBackup - Manage your backups\n"
	       "(c) 2003 Martin Preuss<martin@libchipcard.de>\n"
	       "This library is free software; you can redistribute it and/or\n"
	       "modify it under the terms of the GNU Lesser General Public\n"
	       "License as published by the Free Software Foundation; either\n"
	       "version 2.1 of the License, or (at your option) any later version.\n"
	       "\n"
	       "Usage:\n"
	       k_PRG" [OPTIONS] \n"
	       "\nOptions:\n"
	       " -h            - show this help\n"
	       " -V            - show version information\n"
	       " -v            - be more verbous\n"
	       " --logfile F  - use given F as log file\n"
	       " --logtype T  - use given T as log type\n"
	       "                These are the valid types:\n"
	       "                  stderr (log to standard error channel)\n"
	       "                  file   (log to the file given by --logfile)\n"),
#ifdef HAVE_SYSLOG_H
	  I18N("                  syslog (log via syslog)\n"),
#else
	  "",
#endif
	  I18N("                Default is stderr\n"
	       " --loglevel L - set the loglevel\n"
	       "                Valid levels are:\n"
	       "                  emergency, alert, critical, error,\n"
	       "                  warning, notice, info and debug\n"
	       "                Default is \"warning\".\n"
	      ));
}


void Arguments_AddParam(ARGUMENTS *ar, const char *pr) {
  FREEPARAM *curr;
  FREEPARAM *nfp;

  DBG_ENTER;
  assert(ar);
  assert(pr);

  nfp=FreeParam_new(pr);

  curr=ar->params;
  if (!curr) {
    ar->params=nfp;
  }
  else {
    /* find last */
    while(curr->next) {
      curr=curr->next;
    } /* while */
    curr->next=nfp;
  }
  DBG_LEAVE;
}




int checkArgs(ARGUMENTS *args, int argc, char **argv) {
  int i;
  struct tm tm;
  int timeIsSet=0;
  int dateIsSet=0;

  setTimeAndDate(&tm, time(0), 0); /* store as local time */

  i=1;
  while (i<argc){
    if (strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0) {
      usage(argv[0]);
      return -1;
    }
    else if (strcmp(argv[i],"-V")==0 || strcmp(argv[i],"--version")==0) {
      fprintf(stdout,k_PRG_VERSION_INFO);
      return -1;
    }
    else if (strcmp(argv[i],"-v")==0) {
      args->verbosity++;
      if (args->flags & AQBACKUP_FLAGS_VERBOUS)
	args->flags|=AQBACKUP_FLAGS_VERY_VERBOUS;
      else
	args->flags|=AQBACKUP_FLAGS_VERBOUS;
    }
    else if (strcmp(argv[i],"-r")==0) {
      args->flags|=AQBACKUP_FLAGS_RECURSIVE;
    }
    else if (strcmp(argv[i],"--overwrite")==0) {
      args->flags|=AQBACKUP_FLAGS_OVERWRITE;
    }
    else if (strcmp(argv[i],"--ignore-errors")==0) {
      args->flags|=AQBACKUP_FLAGS_IGNORE_ERRORS;
    }
    else if (strcmp(argv[i],"--local-delete")==0) {
      args->flags|=AQBACKUP_FLAGS_LOCAL_DELETE;
    }
    else if (strcmp(argv[i],"--no-change-rights")==0) {
      args->flags|=AQBACKUP_FLAGS_DONT_CHANGE_RIGHTS;
    }
    else if (strcmp(argv[i],"--no-change-owner")==0) {
      args->flags|=AQBACKUP_FLAGS_DONT_CHANGE_OWNER;
    }
    else if (strcmp(argv[i],"--no-change-times")==0) {
      args->flags|=AQBACKUP_FLAGS_DONT_CHANGE_TIMES;
    }
    else if (strcmp(argv[i],"--deep-first")==0) {
      args->flags|=AQBACKUP_FLAGS_DEEP_FIRST;
    }
    else if (strcmp(argv[i],"--force")==0) {
      args->flags|=AQBACKUP_FLAGS_FORCE;
    }
    else if (strcmp(argv[i],"-C")==0) {
      i++;
      if (i>=argc)
	return 1;
      args->configFile=argv[i];
    }
    else if (strcmp(argv[i],"--date")==0) {
      int year, month, day;

      i++;
      if (i>=argc)
	return 1;
      if (sscanf(argv[i], "%d/%d/%d", &year, &month, &day)!=3) {
	fprintf(stderr,I18N("Bad date string \"%s\"\n"),
		argv[i]);
	return 1;
      }
      tm.tm_year=year;
      tm.tm_mon=month;
      tm.tm_mday=day;
      dateIsSet=1;
      if (!timeIsSet) {
	tm.tm_sec=0;
	tm.tm_min=0;
	tm.tm_hour=0;
      }
    }
    else if (strcmp(argv[i],"--time")==0) {
      int hour, min, sec;

      hour=min=sec=0;
      i++;
      if (i>=argc)
	return 1;
      if (sscanf(argv[i], "%d:%d:%d", &hour, &min, &sec)<2) {
	fprintf(stderr,I18N("Bad time string \"%s\"\n"),
		argv[i]);
	return 1;
      }
      tm.tm_sec=sec;
      tm.tm_min=min;
      tm.tm_hour=hour;
      timeIsSet=1;
    }
    else if (strcmp(argv[i],"--destdir")==0) {
      i++;
      if (i>=argc)
	return 1;
      args->destdir=argv[i];
    }
    else if (strcmp(argv[i],"--type")==0) {
      i++;
      if (i>=argc)
	return 1;
      args->typ=argv[i];
    }
    else if (strcmp(argv[i],"--basedir")==0) {
      i++;
      if (i>=argc)
	return 1;
      args->basedir=argv[i];
    }
    else if (strcmp(argv[i],"--user")==0) {
      i++;
      if (i>=argc)
	return 1;
      args->user=argv[i];
    }
    else if (strcmp(argv[i],"--passwd")==0) {
      i++;
      if (i>=argc)
	return 1;
      args->passwd=argv[i];
    }
    else if (strcmp(argv[i],"--address")==0) {
      i++;
      if (i>=argc)
	return 1;
      args->address=argv[i];
    }
    else if (strcmp(argv[i],"--port")==0) {
      i++;
      if (i>=argc)
	return 1;
      args->port=atoi(argv[i]);
    }
    else if (strcmp(argv[i],"--ziplevel")==0) {
      i++;
      if (i>=argc)
	return 1;
      args->ziplevel=atoi(argv[i]);
    }
    else if (strcmp(argv[i],"--localonly")==0) {
      args->createLocalOnly=1;
    }
    else if (strcmp(argv[i],"--servercfg")==0) {
      i++;
      if (i>=argc)
	return 1;
      args->serverConfigFile=argv[i];
    }
    else if (strcmp(argv[i],"--logfile")==0) {
      i++;
      if (i>=argc)
	return 1;
      args->logFile=argv[i];
    }
    else if (strcmp(argv[i], "--logtype")==0) {
      i++;
      if (i>=argc)
	return 1;
      if (strcmp(argv[i],"stderr")==0)
	args->logType=LoggerTypeConsole;
      else if (strcmp(argv[i],"file")==0)
	args->logType=LoggerTypeFile;
#ifdef HAVE_SYSLOG_H
      else if (strcmp(argv[i],"syslog")==0)
	args->logType=LoggerTypeSyslog;
#endif
      else {
	fprintf(stderr,I18N("Unknown log type \"%s\"\n"),
		argv[i]);
	return 1;
      }
    }
    else if (strcmp(argv[i], "--loglevel")==0) {
      i++;
      if (i>=argc)
	return 1;
      if (strcmp(argv[i], "emergency")==0)
	args->logLevel=LoggerLevelEmergency;
      else if (strcmp(argv[i], "alert")==0)
	args->logLevel=LoggerLevelAlert;
      else if (strcmp(argv[i], "critical")==0)
	args->logLevel=LoggerLevelCritical;
      else if (strcmp(argv[i], "error")==0)
	args->logLevel=LoggerLevelError;
      else if (strcmp(argv[i], "warning")==0)
	args->logLevel=LoggerLevelWarning;
      else if (strcmp(argv[i], "notice")==0)
	args->logLevel=LoggerLevelNotice;
      else if (strcmp(argv[i], "info")==0)
	args->logLevel=LoggerLevelInfo;
      else if (strcmp(argv[i], "debug")==0)
	args->logLevel=LoggerLevelDebug;
      else {
	fprintf(stderr,
		I18N("Unknown log level \"%s\"\n"),
		argv[i]);
	return 1;
      }
    }
    else if (strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--help")==0) {
      usage(argv[0]);
      return -1;
    }
    else if (strcmp(argv[i], "-V")==0 || strcmp(argv[i], "--version")==0) {
      fprintf(stdout,k_PRG_VERSION_INFO);
      return -1;
    }
    else {
      if (*argv[i]=='-') {
	fprintf(stderr,
		I18N("Unknown option \"%s\"\n"), argv[i]);
	return 1;
      }
      // otherwise add param
      Arguments_AddParam(args, argv[i]);
    }
    i++;
  } // while

  /* set time */
  if (timeIsSet || dateIsSet) {
    tm.tm_isdst=-1;
    args->timeAndDate=mktime(&tm); /* now this is UTC time */
  }
  else
    args->timeAndDate=0;

  // that's it
  return 0;
}


char *makeAbsolutePath(const char *path, char *buffer, int size) {
  /* try to chdir to that directory */
  if (chdir(path)) {
    DBG_ERROR("Error on chdir(%s): %s",
	      path,
	      strerror(errno));
    return 0;
  }

  /* get the absolute path */
  if (getcwd(buffer, size)==0) {
    DBG_ERROR("Error on getcwd(): %s", strerror(errno));
    return 0;
  }

  return buffer;
}


int store(AQBACKUP *b,  ARGUMENTS *args, CONFIGGROUP *cfg) {
  char cwdbuff[256];
  char absbuff[256];
  const char *dir;

  /* save current working directory */
  if (getcwd(cwdbuff, sizeof(cwdbuff)-1)==0) {
    DBG_ERROR("Error on getcwd(): %s", strerror(errno));
    return 1;
  }

  /* get directory to work on */
  dir=0;
  if (args->params)
    dir=args->params->param;
  if (dir==0) {
    dir=cwdbuff;
  }

  /* make the path absolute to allow looking it up in the repository list */
  if (makeAbsolutePath(dir, absbuff, sizeof(absbuff)-1)==0) {
    return 2;
  }

  /* restore the working directory */
  if (chdir(cwdbuff)) {
    DBG_ERROR("Error on chdir(%s): %s",
	      cwdbuff,
	      strerror(errno));
    return 2;
  }

  DBG_INFO("Storing directory \"%s\"", absbuff);

  /* now open the appropriate repository */
  if (AQBackup_Open(b, absbuff, 1)) {
    DBG_ERROR("Could not open directory \"%s\"", absbuff);
    return 2;
  }

  /* now store the given dir */
  if (AQBackup_HandleDir(b,
			 absbuff,
			 AQBackupJobStore,
			 0,
			 "",
			 args->flags)) {
    DBG_ERROR("Error handling dir \"%s\"", absbuff);
    return 3;
  }

  if (AQBackup_Close(b)) {
    DBG_ERROR("Error closing repository");
    return 4;
  }

  return 0;
}


int restore(AQBACKUP *b,  ARGUMENTS *args, CONFIGGROUP *cfg) {
  char cwdbuff[256];
  char absbuff[256];
  const char *dir;
  const char *destdir;

  /* save current working directory */
  if (getcwd(cwdbuff, sizeof(cwdbuff)-1)==0) {
    DBG_ERROR("Error on getcwd(): %s", strerror(errno));
    return 1;
  }

  /* get directory to work on */
  dir=0;
  if (args->params)
    dir=args->params->param;
  if (dir==0) {
    dir=cwdbuff;
  }

  /* make the path absolute to allow looking it up in the repository list */
  if (makeAbsolutePath(dir, absbuff, sizeof(absbuff)-1)==0) {
    return 2;
  }

  /* restore the working directory */
  if (chdir(cwdbuff)) {
    DBG_ERROR("Error on chdir(%s): %s",
	      cwdbuff,
	      strerror(errno));
    return 2;
  }

  /* get dest dir */
  destdir=0;
  if (args->destdir)
    destdir=args->destdir;
  if (destdir==0) {
    destdir=absbuff;
  }

  /* restore the working directory */
  if (chdir(cwdbuff)) {
    DBG_ERROR("Error on chdir(%s): %s",
	      cwdbuff,
	      strerror(errno));
    return 2;
  }

  DBG_INFO("Directory \"%s\" to \"%s\"", absbuff, destdir);

  /* now open the appropriate repository */
  if (AQBackup_Open(b, absbuff, 1)) {
    DBG_ERROR("Could not open directory \"%s\"", absbuff);
    return 2;
  }

  /* now store the given dir */
  if (AQBackup_HandleDir(b,
			 absbuff,
			 AQBackupJobRestore,
			 args->timeAndDate,
			 destdir,
			 args->flags)) {
    DBG_INFO("Error handling dir \"%s\"", absbuff);
    return 3;
  }

  if (AQBackup_Close(b)) {
    DBG_ERROR("Error closing repository");
    return 4;
  }

  return 0;
}



int showdiffs(AQBACKUP *b,  ARGUMENTS *args, CONFIGGROUP *cfg) {
  char cwdbuff[256];
  char absbuff[256];
  const char *dir;

  /* save current working directory */
  if (getcwd(cwdbuff, sizeof(cwdbuff)-1)==0) {
    DBG_ERROR("Error on getcwd(): %s", strerror(errno));
    return 1;
  }

  /* get directory to work on */
  dir=0;
  if (args->params)
    dir=args->params->param;
  if (dir==0) {
    dir=cwdbuff;
  }

  /* make the path absolute to allow looking it up in the repository list */
  if (makeAbsolutePath(dir, absbuff, sizeof(absbuff)-1)==0) {
    return 2;
  }

  /* restore the working directory */
  if (chdir(cwdbuff)) {
    DBG_ERROR("Error on chdir(%s): %s",
	      cwdbuff,
	      strerror(errno));
    return 2;
  }

  if (args->flags & AQBACKUP_FLAGS_VERY_VERBOUS) {
    fprintf(stderr,
	    I18N("Checking directory \"%s\"\n"),
	    absbuff);
  }
  DBG_INFO("Checking directory \"%s\"", absbuff);

  /* now open the appropriate repository */
  if (AQBackup_Open(b, absbuff, 1)) {
    DBG_ERROR("Could not open directory \"%s\"", absbuff);
    return 2;
  }

  /* now diff the given dir */
  if (AQBackup_HandleDir(b,
			 absbuff,
			 AQBackupJobShowDiffs,
			 args->timeAndDate,
			 "",
			 args->flags)) {
    DBG_ERROR("Error handling dir \"%s\"", absbuff);
    return 3;
  }

  if (AQBackup_Close(b)) {
    DBG_ERROR("Error closing repository");
    return 4;
  }

  return 0;
}



int createbak(AQBACKUP *b,  ARGUMENTS *args, CONFIGGROUP *cfg) {
  CONFIGGROUP *rg;
  char namebuff[256];

  if (strlen(args->basedir)==0 ||
      strlen(args->user)==0 ||
      strlen(args->typ)==0 ||
      strlen(args->address)==0) {
    fprintf(stderr,
	    I18N("Please provide at least the following options:\n"
		 " --basedir\n"
		 " --user\n"
		 " --typ\n"
		 " --address\n"));
    return 1;
  }

  if (Text_Escape(args->basedir, namebuff, sizeof(namebuff))==0) {
    DBG_ERROR("Could not escape base dir");
    return 1;
  }

  rg=Config_new();
  if (Config_SetValue(rg,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "basedir",
		      args->basedir)) {
    DBG_ERROR("Could not set value");
    Config_free(rg);
    return 1;
  }

  if (Config_SetValue(rg,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "name",
		      namebuff)) {
    DBG_ERROR("Could not set value");
    Config_free(rg);
    return 1;
  }

  if (Config_SetValue(rg,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "user",
		      args->user)) {
    DBG_ERROR("Could not set value");
    Config_free(rg);
    return 1;
  }

  if (Config_SetValue(rg,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "passwd",
		      args->passwd)) {
    DBG_ERROR("Could not set value");
    Config_free(rg);
    return 1;
  }

  if (Config_SetIntValue(rg,
			 CONFIGMODE_VARIABLE |
			 CONFIGMODE_NAMECREATE_VARIABLE |
			 CONFIGMODE_OVERWRITE_VARS,
			 "ziplevel",
			 args->ziplevel)) {
    DBG_ERROR("Could not set value");
    Config_free(rg);
    return 1;
  }

  if (Config_SetValue(rg,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "type",
		      args->typ)) {
    DBG_ERROR("Could not set value");
    Config_free(rg);
    return 1;
  }

  /* setup client arguments */
  if (Config_SetValue(rg,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "client/servercfg",
		      args->serverConfigFile)) {
    DBG_ERROR("Could not set value");
    Config_free(rg);
    return 1;
  }


  /* create repository */
  if (AQBackup_Create(b, rg, !args->createLocalOnly)) {
    DBG_ERROR("Could not create repository");
    Config_free(rg);
    return 2;
  }

  /* close it */
  if (AQBackup_Close(b)==0) {
    /* all is ok, so we may allow to save the configuration file */
    args->allowSaveCfg=1;
  }

  Config_free(rg);
  if (args->flags & AQBACKUP_FLAGS_VERBOUS)
    fprintf(stdout,
	    I18N("Repository created.\n"));
  return 0;
}





int main(int argc, char **argv) {
  ARGUMENTS *args;
  int rv;
  FREEPARAM  *fp;
  AQBACKUP *b;
  AQB_INTERACTOR *ia;
  CONFIGGROUP *cfg;
  CONFIGGROUP *outcfg;
  char configfilebuffer[300];

#ifdef HAVE_GETTEXT_ENVIRONMENT
  setlocale(LC_ALL,"");
  if (bindtextdomain("aqbackup", I18N_PATH)==0) {
    fprintf(stderr," Error bindtextdomain()\n");
  }
  if (textdomain("aqbackup")==0) {
    fprintf(stderr," Error textdomain()\n");
  }
#endif

  args=Arguments_new();
  rv=checkArgs(args,argc,argv);
  if (rv==-1) {
    Arguments_free(args);
    return 0;
  }
  else if (rv) {
    Arguments_free(args);
    return rv;
  }

  if (Logger_Open("AQBackup",
		  args->logFile,
                  args->logType,
		  LoggerFacilityUser)) {
    fprintf(stderr,I18N("Could not setup logging, aborting.\n"));
    return 2;
  }
  Logger_SetLevel(args->logLevel);

  if (args->configFile==0) {
    char *p;

    p=getHomeDir(configfilebuffer, sizeof(configfilebuffer));
    if (!p)
      return 2;
    p="/.aqbackup";
    if (strlen(p)+strlen(configfilebuffer)>=sizeof(configfilebuffer)) {
      DBG_ERROR("Buffer too small");
      return 2;
    }
    strcat(configfilebuffer, p);
    /* check whether that file exists */
    if (access(configfilebuffer, F_OK))
      args->configFile=AQBACKUPC_CFGFILE;
    else
      args->configFile=configfilebuffer;
  }

  cfg=Config_new();
  rv=Config_ReadFile(cfg,
		     args->configFile,
		     CONFIGMODE_ALLOW_GROUPS |
		     CONFIGMODE_REMOVE_QUOTES |
		     CONFIGMODE_REMOVE_STARTING_BLANKS |
		     CONFIGMODE_REMOVE_TRAILING_BLANKS
		    );
  if (rv) {
    DBG_ERROR("Could not read configuration file.");
    Arguments_free(args);
    Config_free(cfg);
    return 1;
  }

  b=AQBackup_new();

  /* setup interactor */
  ia=AQBInteractor_new();
  switch(args->verbosity) {
  case 0:
    break;
  case 1:
    AQBInteractor_SetLevel(ia, InteractorLevelVerbous);
    break;
  case 2:
    AQBInteractor_SetLevel(ia, InteractorLevelVeryVerbous);
    break;
  case 3:
  default:
    AQBInteractor_SetLevel(ia, InteractorLevelExtremelyVerbous);
    break;
  } /* switch */

  AQBackup_SetInteractor(b, ia);
  rv=AQBackup_Init(b, cfg);
  if (rv) {
    DBG_ERROR("Could not init");
    AQBackup_free(b);
    Arguments_free(args);
    Config_free(cfg);
    AQBInteractor_free(ia);
    return 2;
  }

  fp=args->params;
  if (fp) {
    /* unlink */
    args->params=fp->next;
    if (strcasecmp(fp->param, "store")==0) {
      rv=store(b, args, cfg);
    }
    else if (strcasecmp(fp->param, "restore")==0) {
      rv=restore(b, args, cfg);
    }
    else if (strcasecmp(fp->param, "diff")==0) {
      rv=showdiffs(b, args, cfg);
    }
    else if (strcasecmp(fp->param, "create")==0) {
      rv=createbak(b, args, cfg);
    }
    else {
      fprintf(stderr,
	      I18N("Unknown command \"%s\"\n"),
	      fp->param);
      rv=1;
    }
    FreeParam_free(fp);
  }
  else {
    usage(argv[0]);
  }

  outcfg=Config_new();
  if (AQBackup_Fini(b, outcfg)) {
    DBG_ERROR("Error deinitializing AqBackup");
  }

  if (args->allowSaveCfg && rv==0) {
    int lrv;

    lrv=Config_WriteFile(outcfg,
			 args->configFile,
			 CONFIGMODE_ALLOW_GROUPS |
			 CONFIGMODE_OVERWRITE_VARS |
			 CONFIGMODE_REMOVE_QUOTES |
			 CONFIGMODE_REMOVE_STARTING_BLANKS |
			 CONFIGMODE_REMOVE_TRAILING_BLANKS
			);
    if (lrv) {
      DBG_ERROR("Could not write configuration file.");
      rv=9;
    }
  }
  Config_free(outcfg);

  AQBackup_free(b);
  AQBInteractor_free(ia);
  Arguments_free(args);
  Config_free(cfg);
  return rv;
}






