/***************************************************************************
 $RCSfile: aqbackupd.c,v $
                             -------------------
    cvs         : $Id: aqbackupd.c,v 1.1 2003/06/11 13:18:35 aquamaniac Exp $
    begin       : Sun Jun 08 2003
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

/*
#ifdef HAVE_FORK
# warning Debugging, please remove later
# undef HAVE_FORK
#endif
*/

/* Internationalization */
#ifdef HAVE_GETTEXT_ENVIRONMENT
# include <libintl.h>
# include <locale.h>
# define I18N(m) gettext(m)
#else
# define I18N(m) m
#endif
#define I18NT(m) m


#include <chameleon/chameleon.h>
#include <chameleon/conf.h>
#include <chameleon/debug.h>
#include <backupservice/backupserver.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#ifdef OS_WIN32
# include <process.h> /* for getpid */
#endif


#define RETURNVALUE_PARAM   1
#define RETURNVALUE_SETUP   2
#define RETURNVALUE_NOSTART 3
#define RETURNVALUE_DEINIT  4
#define RETURNVALUE_HANGUP  9


static int AQBackupdStop=0;
static int AQBackupdHangup=0;
static int AQBackupdNannyStop=0;
static int AQBackupdNannySuspend=0;
static int AQBackupdNannyResume=0;
static time_t LastFailedTime=0;
static int ShortFailCounter=0;


#define k_PRG "aqserver"
#define k_PRG_VERSION_INFO \
  "AQBackupd v0.1\n"\
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
  int verbous;
#ifdef HAVE_FORK
  int daemonMode;
#endif
  const char *configFile;
#ifdef HAVE_GETPID
  const char *pidFile;
#endif
  const char *logFile;
  LOGGER_LOGTYPE logType;
  LOGGER_LEVEL logLevel;
  int exitOnSetupError;
};


ARGUMENTS *Arguments_new() {
  ARGUMENTS *ar;

  ar=(ARGUMENTS*)malloc(sizeof(ARGUMENTS));
  assert(ar);
  memset(ar, 0, sizeof(ARGUMENTS));
  ar->verbous=0;
#ifdef HAVE_FORK
  ar->daemonMode=1;
#endif
  ar->configFile=AQBACKUPD_CFGFILE;
#ifdef HAVE_GETPID
  ar->pidFile=AQBACKUPD_PIDDIR "/" AQBACKUPD_PIDFILE;
#endif
  ar->logFile=AQBACKUPD_LOGDIR "/" AQBACKUPD_LOGFILE;
  ar->logLevel=LoggerLevelNotice;
#ifdef HAVE_SYSLOG_H
  ar->logType=LoggerTypeSyslog;
#else
  ar->logType=LoggerTypeFile;
#endif
  ar->exitOnSetupError=0;
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
  fprintf(stdout,"%s%s%s%s%s%s",
	  I18N("AQBackupd - A daemon for services\n"
	       "(c) 2002 Martin Preuss<martin@libchipcard.de>\n"
	       "This library is free software; you can redistribute it and/or\n"
	       "modify it under the terms of the GNU Lesser General Public\n"
	       "License as published by the Free Software Foundation; either\n"
	       "version 2.1 of the License, or (at your option) any later version.\n"
	       "\n"
	       "Usage:\n"
	       k_PRG" COMMAND [OPTIONS]\n"
	       "\nOptions:\n"
	       " -C CONFIGFILE    - path and name of the configuration file\n"
	       "                    defaults to \""AQBACKUPD_CFGFILE"\"\n"),
#ifdef HAVE_FORK
	  I18N(" -f               - stay in foreground (do not fork)\n"),
#else
	  "",
#endif
	  I18N(" -h               - show this help\n"
	       " -V               - show version information\n"
	       " -v               - be more verbous\n"
	       " --pidfile FILE   - use given FILE as PID file\n"
	       "                    defaults to \""AQBACKUPD_PIDDIR"/"AQBACKUPD_PIDFILE"\"\n"
	       " --logfile FILE   - use given FILE as log file\n"
	       "                    defaults to \""AQBACKUPD_LOGDIR"/"AQBACKUPD_LOGFILE"\"\n"
	       " --logtype TYPE   - use given TYPE as log type\n"
	       "                    These are the valid types:\n"
	       "                     stderr (log to standard error channel)\n"
	       "                     file   (log to the file given by --logfile)\n"),
#ifdef HAVE_SYSLOG_H
	  I18N("                     syslog (log via syslog)\n"),
#else
	  "",
#endif
#ifdef HAVE_SYSLOG_H
	  I18N("                    Default is syslog\n"),
#else
	  I18N("                    Default is stderr\n"),
#endif
	  I18N(" --loglevel LEVEL - set the loglevel\n"
	       "                    Valid levels are:\n"
	       "                     emergency, alert, critical, error,\n"
	       "                     warning, notice, info and debug\n"
	       "                    Default is \"notice\".\n"
	       " --exit-on-error  - makes chipcardd exit on setup errors\n")
	 );
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

  i=1;
  while (i<argc){
    if (strcmp(argv[i],"-C")==0) {
      i++;
      if (i>=argc)
	return RETURNVALUE_PARAM;
      args->configFile=argv[i];
    }
    else if (strcmp(argv[i],"--pidfile")==0) {
      i++;
      if (i>=argc)
	return RETURNVALUE_PARAM;
      args->pidFile=argv[i];
    }
    else if (strcmp(argv[i],"--logfile")==0) {
      i++;
      if (i>=argc)
	return RETURNVALUE_PARAM;
      args->logFile=argv[i];
    }
    else if (strcmp(argv[i],"--logtype")==0) {
      i++;
      if (i>=argc)
	return RETURNVALUE_PARAM;
      if (strcmp(argv[i], "stderr")==0)
	args->logType=LoggerTypeConsole;
      else if (strcmp(argv[i], "file")==0)
	args->logType=LoggerTypeFile;
#ifdef HAVE_SYSLOG_H
      else if (strcmp(argv[i], "syslog")==0)
	args->logType=LoggerTypeSyslog;
#endif
      else {
	fprintf(stderr,
		I18N("Unknown log type \"%s\"\n"),
		argv[i]);
	return RETURNVALUE_PARAM;
      }
    }
    else if (strcmp(argv[i],"--loglevel")==0) {
      i++;
      if (i>=argc)
	return RETURNVALUE_PARAM;
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
	return RETURNVALUE_PARAM;
      }
    }
    else if (strcmp(argv[i],"--exit-on-error")==0) {
      args->exitOnSetupError=1;
    }
    else if (strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0) {
      usage(argv[0]);
      return -1;
    }
    else if (strcmp(argv[i],"-V")==0 || strcmp(argv[i],"--version")==0) {
      fprintf(stdout,k_PRG_VERSION_INFO);
      return -1;
    }
    else if (strcmp(argv[i],"-v")==0) {
      args->verbous=1;
    }
#ifdef HAVE_FORK
    else if (strcmp(argv[i],"-f")==0) {
      args->daemonMode=0;
    }
#endif
    else {
      // otherwise add param
      if (argv[i][0]=='-') {
	fprintf(stderr,I18N("Unknown option \"%s\"\n"),argv[i]);
	return RETURNVALUE_PARAM;
      }
      else
	Arguments_AddParam(args, argv[i]);
    }
    i++;
  } /* while */

  /* that's it */
  return 0;
}


#ifdef HAVE_SIGACTION
/* Signal handler */

void familySignalHandler(int s, int child) {
  switch(s) {
  case SIGINT:
    if (child) {
      DBG_NOTICE("Daemon got an interrupt signal, will go down.");
      AQBackupdStop=1;
    }
    else {
      DBG_NOTICE("Nanny got an interrupt signal, will terminate child");
      AQBackupdNannyStop=1;
    }
    break;

  case SIGTERM:
    if (child) {
      DBG_NOTICE("Daemon got a termination signal");
      AQBackupdStop=1;
    }
    else {
      DBG_NOTICE("Nanny got a termination signal, will terminate child.");
      AQBackupdNannyStop=1;
    }
    break;

#ifdef SIGINFO
  case SIGINFO:
    if (child) {
      DBG_NOTICE("Daemon got an info signal");
    }
    else {
      DBG_NOTICE("Nanny got an info signal");
    }
    break;
#endif

#ifdef SIGCHLD
  case SIGCHLD:
    if (!child) {
      DBG_NOTICE("Nanny got a child signal");
    }
    break;
#endif

#ifdef SIGHUP
  case SIGHUP:
    if (child) {
      /* child exits on this signal and will be respawned by the nanny */
      DBG_NOTICE("Daemon got a hangup signal, going down.");
      AQBackupdStop=1;
      AQBackupdHangup=1;
    }
    else {
      if (AQBackupdNannySuspend) {
	DBG_NOTICE("Restarting daemon");
	AQBackupdNannyResume=1;
      }
    }
    break;
#endif

#ifdef SIGALRM
  case SIGALRM:
    DBG_NOTICE("Got an alarm signal");
    break;
#endif

#ifdef SIGUSR1
  case SIGUSR1:
    if (Logger_GetLevel()<LoggerLevelDebug) {
      DBG_NOTICE("Got signal USR1, will increase log level");
      Logger_SetLevel(Logger_GetLevel()+1);
    }
    else {
      DBG_NOTICE("Got signal USR1, but log level already is lowest");
    }
    break;
#endif

#ifdef SIGUSR2
  case SIGUSR2:
    if (Logger_GetLevel()>LoggerLevelEmergency) {
      DBG_NOTICE("Got signal USR2, will decrease log level");
      Logger_SetLevel(Logger_GetLevel()-1);
    }
    else {
      DBG_NOTICE("Got signal USR2, but log level already is highest");
    }
    break;
#endif

#ifdef SIGTSTP
  case SIGTSTP:
    if (!child) {
      DBG_NOTICE("Suspending daemon");
      AQBackupdNannySuspend=1;
    }
    break;
#endif

#ifdef SIGCONT
  case SIGCONT:
    if (!child && AQBackupdNannySuspend) {
      DBG_NOTICE("Resuming daemon");
      AQBackupdNannyResume=1;
    }
    break;
#endif

  default:
    DBG_WARN("Unknown signal  %d",s);
    break;
  } /* switch */
}


void childSignalHandler(int s) {
  return familySignalHandler(s, 1);
}


void fatherSignalHandler(int s) {
  return familySignalHandler(s, 0);
}



struct sigaction saINT,saTERM, saINFO, saHUP;
struct sigaction saUSR1, saUSR2, saALRM, saCHLD;
struct sigaction saTSTP, saCONT;


int setSingleSignalHandler(struct sigaction *sa,
			   int sig,
			   int child) {
  if (child)
    sa->sa_handler=childSignalHandler;
  else
    sa->sa_handler=fatherSignalHandler;
  sigemptyset(&sa->sa_mask);
  sa->sa_flags=0;
  if (sigaction(sig, sa,0)) {
    DBG_ERROR("Could not setup signal handler for signal %d", sig);
    return RETURNVALUE_SETUP;
  }
  return 0;
}


int setSignalHandler(int child) {
  int rv;

  rv=setSingleSignalHandler(&saINT, SIGINT, child);
  if (rv)
    return rv;
  rv=setSingleSignalHandler(&saCHLD, SIGCHLD, child);
  if (rv)
    return rv;
  rv=setSingleSignalHandler(&saTERM, SIGTERM, child);
  if (rv)
    return rv;
#ifdef SIGINFO
  rv=setSingleSignalHandler(&saINFO, SIGINFO, child);
  if (rv)
    return rv;
#endif

#ifdef SIGHUP
  rv=setSingleSignalHandler(&saHUP, SIGHUP, child);
  if (rv)
    return rv;
#endif

#ifdef SIGALRM
  rv=setSingleSignalHandler(&saALRM, SIGALRM, child);
  if (rv)
    return rv;
#endif

#ifdef SIGUSR1
  rv=setSingleSignalHandler(&saUSR1, SIGUSR1, child);
  if (rv)
    return rv;
#endif

#ifdef SIGUSR2
  rv=setSingleSignalHandler(&saUSR2, SIGUSR2, child);
  if (rv)
    return rv;
#endif

#ifdef SIGTSTP
  rv=setSingleSignalHandler(&saTSTP, SIGTSTP, child);
  if (rv)
    return rv;
#endif

#ifdef SIGCONT
  rv=setSingleSignalHandler(&saCONT, SIGCONT, child);
  if (rv)
    return rv;
#endif

  return 0;
}
#endif


int createPidFile(const char *pidfile) {
  FILE *f;
  int pidfd;

  /* the PID file code has been provided by Thomas Viehmann and adapted
   * by me */
  /* create pid file */
  if(remove(pidfile)==0)
    DBG_ERROR("Old PID file existed, removed. (Unclean shutdown?)");

#ifdef HAVE_SYS_STAT_H
  pidfd = open(pidfile, O_EXCL|O_CREAT|O_WRONLY,
	       S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (pidfd < 0) {
    DBG_ERROR("Could not create PID file \"%s\" (%s), aborting.",
	      pidfile,
	      strerror(errno));
    return RETURNVALUE_SETUP;
  }

  f = fdopen(pidfd, "w");
#else /* HAVE_STAT_H */
  f=fopen(pidfile,"w+");
#endif /* HAVE_STAT_H */

  /* write pid */
#ifdef HAVE_GETPID
  fprintf(f,"%d\n",getpid());
#else
  fprintf(f,"-1\n");
#endif
  if (fclose(f)) {
    DBG_ERROR("Could not close PID file \"%s\" (%s), aborting.",
	      pidfile,
	      strerror(errno));
    return RETURNVALUE_SETUP;
  }
  return 0;
}


int execFunction(const char *cmd,
		 const char *args,
		 int *id) {
  int rv;
  char buffer[256];
  char *argv[256];
  int argc;
  const char *p, *p2;
  int i;

#ifdef HAVE_FORK
  rv=fork();
  if (rv>0) {
    /* father */
    *id=rv;
    return 0;
  }
  else if (rv<0) {
    /* error */
    DBG_ERROR("Could not start %s: %s",
	      cmd, strerror(errno));
    return 1;
  }

  argc=0;

  argv[0]=strdup(cmd);
  argc++;
  p=args;
  while(argc<256 && *p) {
    p2=p;
    while(*p2 && *p2!=' ')
      p2++;
    i=p2-p;
    if (i) {
      argv[argc]=(char*) malloc(i+1);
      memmove(argv[argc],p,i);
      argv[argc][i]=0;
      argc++;
    }
    p=p2;
    if (*p)
      p++;
  } /* while */
  argv[argc]=0;
  execvp(cmd, argv);

  DBG_ERROR("Could not execute %s: %s",
	    buffer, strerror(errno));
  return 1;
#else
  DBG_ERROR("Have no fork, will not launch service");
  return 1;
#endif
}



int main(int argc, char **argv) {
  ERRORCODE err;
  CONFIGGROUP *config;
  ARGUMENTS *args;
  const char *pidfile;
  int rv;
  int timeout;
  int maxmsg;
  int enabled;
  CTSERVERDATA *backupd;

#ifdef HAVE_GETTEXT_ENVIRONMENT
  setlocale(LC_ALL,"");
  if (bindtextdomain("aqserver", "/usr/local/share/locale")==0) {
    fprintf(stderr," Error bindtextdomain()\n");
  }
  if (textdomain("aqserver")==0) {
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

  pidfile=args->pidFile;
  if (Logger_Open("aqserver",
		  args->logFile,
		  args->logType,
		  LoggerFacilityDaemon)) {
    fprintf(stderr,I18N("Could not setup logging, aborting.\n"));
    Arguments_free(args);
    return RETURNVALUE_SETUP;
  }
  Logger_SetLevel(args->logLevel);

  DBG_NOTICE("AQBackupd started.");

#ifdef HAVE_FORK
  if (args->daemonMode) {
    rv=fork();
    if (rv==-1) {
      DBG_ERROR("Error on fork, aborting.");
      if (args->logType!=LoggerTypeConsole)
	fprintf(stderr,I18N("Error on fork, aborting.\n"));
      Logger_Close();
      Arguments_free(args);
      remove(pidfile);
      return RETURNVALUE_SETUP;
    }
    else if (rv!=0) {
      /* father process, does nothing more */
      DBG_DEBUG("Daemon forked, father exiting");
      Arguments_free(args);
      remove(pidfile);
      return 0;
    }
  } /* if daemon mode */

  /* this will be executed by the child (or both if not in daemon mode) */
  /* create pid file */
  rv=createPidFile(pidfile);
  if (rv!=0) {
    Logger_Close();
    Arguments_free(args);
    return rv;
  }
#ifdef HAVE_SIGACTION
  /* setup some signal handler */
  if (setSignalHandler(0)) {
    DBG_ERROR("Error setting up signal handler, aborting");
    remove(pidfile);
    Logger_Close();
    Arguments_free(args);
    remove(pidfile);
    return RETURNVALUE_SETUP;
  }
#endif
  /* now spawn a child process which really does the work */

  while (1) {
    AQBackupdStop=0;
    AQBackupdHangup=0;
    rv=fork();
    if (rv==-1) {
      DBG_ERROR("Error on fork, aborting.");
      if (args->logType!=LoggerTypeConsole)
	fprintf(stderr,I18N("Error on fork, aborting.\n"));
      Logger_Close();
      Arguments_free(args);
      remove(pidfile);
      return RETURNVALUE_SETUP;
    }
    else if (rv!=0) {
      /* father process */
      int childPID;

      /* store process id of the child */
      childPID=rv;
      DBG_NOTICE("Nanny now supervising daemon %d", childPID);
      while(1) {
	int status;

	if (AQBackupdNannySuspend) {
	  if (AQBackupdNannyStop) {
	    DBG_NOTICE("Nanny exiting, no daemon.");
	    Logger_Close();
	    Arguments_free(args);
	    remove(pidfile);
	    return 0;
	  }
	  else if (AQBackupdNannyResume) {
	    AQBackupdNannySuspend=0;
	    AQBackupdNannyResume=0;
	    DBG_NOTICE("Resuming daemon");
	    break;
	  }
	  else {
	    DBG_NOTICE("Daemon still suspended.");
	    sleep(300);
	  }
	}
	else {
	  rv=wait(&status);
	  if (rv==-1) {
	    /* error */
	    if (errno==EINTR) {
	      if (AQBackupdNannyStop || AQBackupdNannySuspend) {
		/* nanny is to stop or suspend, so terminate child, wait for
		 * its termination and if we are to stop then go down */
		time_t startedToWait;

                startedToWait=time(0);
		DBG_NOTICE("Terminating daemon.");
		kill(childPID, SIGTERM);
		/* wait for child to go down */
		while(1) {
		  rv=wait(&status);
		  if (rv==-1) {
		    if (errno!=EINTR) {
		      DBG_ERROR("Error while waiting for child's "
				"termination (%s).",
				strerror(errno));
		      Logger_Close();
		      Arguments_free(args);
		      remove(pidfile);
		      return RETURNVALUE_SETUP;
		    }
		  }
		  else
		    break;
		  if (difftime(time(0),startedToWait)>10) {
		    DBG_WARN("Daemon seems to hang, have to kill it");
		    kill(childPID, SIGKILL);
		    break;
		  }
		} /* while waiting for child to go down */
		if (AQBackupdNannySuspend) {
		  DBG_NOTICE("Daemon suspended, "
			     "waiting for SIGCONT to resume.");
		}
		else {
		  DBG_NOTICE("Daemon terminated, exiting.");
		  Logger_Close();
		  Arguments_free(args);
		  remove(pidfile);
		  return 0;
		}
	      }
	    }
	    else if (errno==ECHILD) {
	      DBG_ERROR("No child to wait for (%s), respawning.",
			strerror(errno));
	      break;
	    }
	    else {
	      DBG_ERROR("Error while waiting for child (%s).",
			strerror(errno));
	    }
	  }
	  else {
	    /* ok, child exited */
	    if (WIFEXITED(status)) {
	      /* child exited normally */
	      if (AQBackupdNannySuspend) {
		DBG_NOTICE("Daemon suspended.");
	      }
	      else {
		if (WEXITSTATUS(status)==RETURNVALUE_HANGUP) {
		  DBG_NOTICE("Restarting daemon");
		  break;
		}
		else if ((WEXITSTATUS(status)==RETURNVALUE_SETUP ||
			  WEXITSTATUS(status)==RETURNVALUE_NOSTART) &&
			 args->exitOnSetupError) {
		  DBG_NOTICE("Daemon setup error, exiting.");
		  Logger_Close();
		  Arguments_free(args);
		  remove(pidfile);
		  return WEXITSTATUS(status);
		}
		else if (WEXITSTATUS(status)==RETURNVALUE_SETUP ||
			 WEXITSTATUS(status)==RETURNVALUE_NOSTART) {
		  DBG_NOTICE("Could not init daemon, suspended.");
		  AQBackupdNannySuspend=1;
		  AQBackupdNannyResume=0;
		}
		else {
		  DBG_NOTICE("Daemon exited normally, exiting, too.");
		  Logger_Close();
		  Arguments_free(args);
		  remove(pidfile);
		  return WEXITSTATUS(status);
		}
	      }
	    }
	    else {
	      /* child died unexpectedly */
	      if (WIFSIGNALED(status)) {
		time_t lft;

		lft=LastFailedTime;
		LastFailedTime=time(0);

		DBG_ERROR("Daemon died due to uncaught "
			  "signal %d.",
			  WTERMSIG(status));
		/* check for respawn frequency, don't make the daemon
		 * eat all the processor power */
		if (difftime(LastFailedTime, lft)<2) {
		  ShortFailCounter++;
		  if (ShortFailCounter>10) {
		    DBG_NOTICE("Daemon's respawn frequency too high, "
			       "suspended.");
		    AQBackupdNannySuspend=1;
		    AQBackupdNannyResume=0;
		  }
		  else if (ShortFailCounter>5) {
		    DBG_NOTICE("Daemon dies to often, will wait");
		    sleep(1);
		  }
		  else {
		    DBG_NOTICE("Respawning daemon.");
		  }
		}
		else {
		  /* respawn frequency ok, reset fail counter */
		  ShortFailCounter=0;
		  DBG_NOTICE("Respawning daemon.");
		}
		/* DEBUG end */
	      }
	      else {
		DBG_WARN("Daemon died unexpectedly, respawning.\n");
	      }
	      /* break wait loop */
	      break;
	    }
	  }
	}
      } /* while */
    }
    else {
      /* child process, break spawn loop */
      break;
    }
  } /* while */
#endif /* ifdef HAVE_FORK */

  DBG_NOTICE("Initializing daemon.");

#ifdef HAVE_SIGACTION
  if (setSignalHandler(1)) {
    DBG_ERROR("Error setting up signal handler, aborting");
    Logger_Close();
    Arguments_free(args);
    return RETURNVALUE_SETUP;
  }
#endif
  /* init all modules */
  err=Chameleon_Init();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    Logger_Close();
    Arguments_free(args);
    return RETURNVALUE_SETUP;
  }
  err=CTService_ModuleInit();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    Logger_Close();
    Arguments_free(args);
    return RETURNVALUE_SETUP;
  }

  config=Config_new();
  rv=Config_ReadFile(config, args->configFile,
		     CONFIGMODE_REMOVE_STARTING_BLANKS |
		     CONFIGMODE_REMOVE_TRAILING_BLANKS |
		     CONFIGMODE_REMOVE_QUOTES|
		     CONFIGMODE_ALLOW_PATH_IN_VARS|
		     CONFIGMODE_ALLOW_GROUPS);
  if (rv) {
    fprintf(stderr,I18N("Could not read configuration file, aborting.\n"));
    Logger_Close();
    Arguments_free(args);
    Config_free(config);
    return RETURNVALUE_SETUP;
  }
  enabled=Config_GetIntValue(config,
			     "enabled",
			     0,
			     0);
  if (!enabled) {
    DBG_NOTICE("AQBackupd is disabled by config file, aborting.\n");
    Logger_Close();
    Arguments_free(args);
    Config_free(config);
    return RETURNVALUE_NOSTART;
  }
  timeout=Config_GetIntValue(config,
			     "timeout",
			     750,
			     0);
  maxmsg=Config_GetIntValue(config,
			    "maxmsg",
			    10,
			    0);

  DBG_INFO("Will now initialize server.\n");
  backupd=CTServer_new();
  err=BackupServer_Init(backupd, config);
  if (!Error_IsOk(err)) {
    fprintf(stderr,I18N("Could not initialize server.\n"));
    Arguments_free(args);
    Logger_Close();
    Arguments_free(args);
    Config_free(config);
    CTServer_free(backupd);
    return RETURNVALUE_SETUP;
  }

  /* loop */
  while (AQBackupdStop==0 && AQBackupdHangup==0) {
    err=BackupServer_Work(backupd,timeout, maxmsg);
    if (!Error_IsOk(err)) {
      DBG_DEBUG_ERR(err);
    }
  } /* while */

  DBG_INFO("Will now deinitialize server.\n");
  err=BackupServer_Fini(backupd);
  if (!Error_IsOk(err)) {
    fprintf(stderr,I18N("Could not deinitialize server.\n"));
    Config_free(config);
    Logger_Close();
    Arguments_free(args);
    CTServer_free(backupd);
    return RETURNVALUE_DEINIT;
  }

  Config_free(config);

  /* deinitialize modules */
  err=CTService_ModuleFini();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }
  err=Chameleon_Fini();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }

  DBG_INFO("AQBackupd Daemon exiting.\n");
  Logger_Close();
  Arguments_free(args);
  CTServer_free(backupd);
  if (AQBackupdHangup)
    return RETURNVALUE_HANGUP;
  return 0;
}

