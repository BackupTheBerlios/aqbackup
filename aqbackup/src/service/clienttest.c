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


#include <ctclient.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>
#include <assert.h>



int waitForResponse(CTCLIENTDATA *cd,
		    int requestid){
  ERRORCODE err;
  int i;

  i=0;
  while (i++<1000) {
    err=CTClient_CheckResponse(cd,
			       requestid);
    if (!Error_IsOk(err)) {
      if (Error_GetType(err)!=Error_FindType(CTSERVICE_ERROR_TYPE) ||
	  Error_GetCode(err)!=CTSERVICE_ERROR_NO_MESSAGE)
	DBG_ERROR_ERR(err);
      fprintf(stderr,".");
    }
    else {
      fprintf(stderr,"\n");
      break;
    }
    err=CTClient_Work(cd, 500, 1000);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
    }
  } /* while */
  return i;
}


int waitForPing(CTCLIENTDATA *client, int reqid) {
  int i;
  ERRORCODE err;

  i=0;
  while (i++<1000) {
    err=CTClient_Work(client, 500, 1000);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
    }
    err=CTClient_CheckPing(client, reqid);
    if (!Error_IsOk(err)) {
      if (Error_GetType(err)!=Error_FindType(CTSERVICE_ERROR_TYPE) ||
	  Error_GetCode(err)!=CTSERVICE_ERROR_NO_MESSAGE)
	DBG_ERROR_ERR(err);
      fprintf(stderr,".");
    }
    else {
      fprintf(stderr,
	      "\nI got the response ;-) (at loop %d, Request ID %d)\n",
	      i, reqid);
      break;
    }
  } /* while */
  return i;
}


int waitForAlloc(CTCLIENTDATA *client, int reqid) {
  int i;
  ERRORCODE err;
  int tid;
  int serviceid;

  i=0;
  i=waitForResponse(client, reqid);
  err=CTClient_CheckAllocReader(client, reqid, &tid, &serviceid);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }
  else {
    fprintf(stderr,
	    "\nRESPONSE: Loop %d, Request ID %d, Terminal ID %d\n",
	    i, reqid, tid);
  }
  return tid;
}


int waitForFind(CTCLIENTDATA *client, int reqid,
		unsigned int *tids, int *max) {
  int i;
  int j;
  ERRORCODE err;
  int found;
  int localtcount;
  int terms[32];

  i=0;
  found=0;
  while (found<*max) {
    fprintf(stderr,"Loop started\n");
    i=waitForResponse(client, reqid);
    localtcount=*max-found;
    if (localtcount>32)
      localtcount=32;
    err=CTClient_CheckFindReader(client, reqid, &terms, localtcount);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      if (!Error_IsOk(err)) {
	if (Error_GetType(err)!=Error_FindType(CTSERVICE_ERROR_TYPE) ||
	    Error_GetCode(err)!=CTSERVICE_ERROR_NO_REQUEST) {
	  break;
	}
	else {
	  DBG_ERROR_ERR(err);
	}
      }
      else {
	for (j=0; j<localtcount; j++) {
	  tids[j+found]=terms[j];
	}
	found+=localtcount;
      }
    }
  } /* while */
    fprintf(stderr,
	  "\nRESPONSE: Loop %d, Request ID %d, Terminal ID %d\n",
	  i, reqid, *tids);
  *max=found;
  return i;
}


int waitForRelease(CTCLIENTDATA *client, int reqid) {
  int i;
  ERRORCODE err;

  i=0;
  while (i++<1000) {
    err=CTClient_Work(client, 500, 1000);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
    }
    err=CTClient_CheckReleaseReader(client, reqid);
    if (!Error_IsOk(err)) {
      if (Error_GetType(err)!=Error_FindType(CTSERVICE_ERROR_TYPE) ||
	  Error_GetCode(err)!=CTSERVICE_ERROR_NO_MESSAGE)
	DBG_ERROR_ERR(err);
      fprintf(stderr,".");
    }
    else {
      fprintf(stderr,
	      "\nRESPONSE: Loop %d, Request ID %d\n",
	      i, reqid);
      break;
    }
  } /* while */
  return i;
}


int waitForConnect(CTCLIENTDATA *client, int reqid) {
  int i;
  int result;
  ERRORCODE err;
  char atr[300];
  int atrlen;

  i=0;
  while (i++<1000) {
    err=CTClient_Work(client, 500, 1000);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
    }
    atrlen=sizeof(atr);
    err=CTClient_CheckConnectReader(client, reqid, &result,
				    atr, &atrlen);
    if (!Error_IsOk(err)) {
      if (Error_GetType(err)!=Error_FindType(CTSERVICE_ERROR_TYPE) ||
	  Error_GetCode(err)!=CTSERVICE_ERROR_NO_MESSAGE)
	DBG_ERROR_ERR(err);
      fprintf(stderr,".");
    }
    else {
      fprintf(stderr,
	      "\nRESPONSE: Loop %d, Request ID %d, Result %d.\n",
	      i, reqid, result);
      if (result==0) {
	fprintf(stderr,"ATR is:\n");
	Chameleon_DumpString(atr, atrlen);
      }
      break;
    }
  } /* while */
  return i;
}


int waitForDisconnect(CTCLIENTDATA *client, int reqid) {
  int i;
  int result;
  ERRORCODE err;

  i=0;
  while (i++<1000) {
    err=CTClient_Work(client, 500, 1000);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
    }
    err=CTClient_CheckDisconnectReader(client, reqid, &result);
    if (!Error_IsOk(err)) {
      if (Error_GetType(err)!=Error_FindType(CTSERVICE_ERROR_TYPE) ||
	  Error_GetCode(err)!=CTSERVICE_ERROR_NO_MESSAGE)
	DBG_ERROR_ERR(err);
      fprintf(stderr,".");
    }
    else {
      fprintf(stderr,
	      "\nRESPONSE: Loop %d, Request ID %d, Result %d\n",
	      i, reqid, result);
      break;
    }
  } /* while */
  return i;
}


int waitForCommand(CTCLIENTDATA *client, int reqid) {
  int i;
  int result;
  ERRORCODE err;
  char atr[300];
  int atrlen;

  i=0;
  while (i++<1000) {
    err=CTClient_Work(client, 500, 1000);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
    }
    atrlen=sizeof(atr);
    err=CTClient_CheckCommandReader(client, reqid, &result,
				    atr, &atrlen);
    if (!Error_IsOk(err)) {
      if (Error_GetType(err)!=Error_FindType(CTSERVICE_ERROR_TYPE) ||
	  Error_GetCode(err)!=CTSERVICE_ERROR_NO_MESSAGE)
	DBG_ERROR_ERR(err);
      fprintf(stderr,".");
    }
    else {
      fprintf(stderr,
	      "\nRESPONSE: Loop %d, Request ID %d, Result %d.\n",
	      i, reqid, result);
      if (result==0) {
	fprintf(stderr,"Result is:\n");
	Chameleon_DumpString(atr, atrlen);
      }
      break;
    }
  } /* while */
  return i;
}


int doCommand(CTCLIENTDATA *client, int sid,
	      int tid, const char *adpu, int adpusize) {
  ERRORCODE err;
  int requestid;

  fprintf(stderr,"\nWill now try to connect the terminal.\n");
  err=CTClient_RequestConnectReader(client, &requestid, 1, tid);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    fprintf(stderr,"Could not send message.\n");
    CTClient_free(client);
    return 2;
  }
  fprintf(stderr,"Will wait for response.\n");
  waitForConnect(client, requestid);
  fprintf(stderr,"Done.\n");
  fprintf(stderr,"Will now sleep for some seconds\n");
  sleep(10);

  fprintf(stderr,"\nWill now try to execute a command.\n");
  err=CTClient_RequestCommandReader(client, &requestid, 1, tid,
				    adpu, adpusize);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    fprintf(stderr,"Could not send message.\n");
    CTClient_free(client);
    return 2;
  }
  fprintf(stderr,"Will wait for response.\n");
  waitForCommand(client, requestid);
  fprintf(stderr,"Done.\n");
  fprintf(stderr,"Will now sleep for some seconds\n");
  sleep(10);


  fprintf(stderr,"\nWill now try to disconnect the terminal.\n");
  err=CTClient_RequestDisconnectReader(client, &requestid, 1, tid);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    fprintf(stderr,"Could not send message.\n");
    CTClient_free(client);
    return 2;
  }
  fprintf(stderr,"Will wait for response.\n");
  waitForDisconnect(client, requestid);
  fprintf(stderr,"Done.\n");
  fprintf(stderr,"Will now sleep for some seconds\n");
  sleep(10);
  return 0;
}


int doTest(int argc, char **argv) {
  CTCLIENTDATA *client;
  ERRORCODE err;
  int i;
  int requestid;
  int tid;
  char adpu[300];
  int adpusize;
  int readerBufferLength;

  if (argc<4) {
    fprintf(stderr,"Usage: %s address port terminal\n", argv[0]);
    return 1;
  }

  /* init all modules */
  err=Chameleon_Init();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }
  err=CTService_ModuleInit();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }

  client=CTClient_new();
  fprintf(stderr,"Will now initialize client.\n");
  err=CTClient_Init(client);
  if (!Error_IsOk(err)) {
    fprintf(stderr,"Could not initialize client.\n");
    DBG_ERROR_ERR(err);
    CTClient_free(client);
    return 2;
  }
  fprintf(stderr,"Done.\n");

  fprintf(stderr,"Will add server %s:%d\n", argv[1], atoi(argv[2]));
  err=CTClient_AddServer(client, argv[1], atoi(argv[2]));
  if (!Error_IsOk(err)) {
    fprintf(stderr,"Could not add server.\n");
    DBG_ERROR_ERR(err);
    CTClient_free(client);
    return 2;
  }
  fprintf(stderr,"Done.\n");


  fprintf(stderr,"\nWill now look for a terminal.\n");
  err=CTClient_RequestFindReader(client, &requestid, 1,
				 -1, -1, 0, 0);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    fprintf(stderr,"Could not send message.\n");
    CTClient_free(client);
    return 2;
  }
  fprintf(stderr,"Will wait for response.\n");
  readerBufferLength=1;
  waitForFind(client, requestid,&tid, &readerBufferLength);
  fprintf(stderr,"Done (using termin %d).\n", tid);
  fprintf(stderr,"Will now sleep for some seconds\n");
  sleep(10);


  fprintf(stderr,"\nWill now try to allocate a terminal.\n");
  err=CTClient_RequestAllocReader(client, &requestid, 1, tid);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    fprintf(stderr,"Could not send message.\n");
    CTClient_free(client);
    return 2;
  }
  fprintf(stderr,"Will wait for response.\n");
  tid=waitForAlloc(client, requestid);
  fprintf(stderr,"Done.\n");
  fprintf(stderr,"Will now sleep for some seconds\n");
  sleep(10);

  adpu[0]=0x00;
  adpu[1]=0xa4;
  adpu[2]=0x00;
  adpu[3]=0x00;
  adpu[4]=0xff;
  adpusize=5;
  fprintf(stderr,"Will try that command twice.\n");
  doCommand(client, 1, tid, adpu, adpusize);
  fprintf(stderr,"\n\nFirst walk done.\n\n");
  doCommand(client, 1, tid, adpu, adpusize);
  fprintf(stderr,"\n\nSecond walk done.\n\n");

  fprintf(stderr,"\nWill now try to release terminal.\n");
  err=CTClient_RequestReleaseReader(client, &requestid, 1, tid);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    fprintf(stderr,"Could not send message.\n");
    CTClient_free(client);
    return 2;
  }
  fprintf(stderr,"Will wait for response.\n");
  waitForRelease(client, requestid);
  fprintf(stderr,"Done.\n");

  fprintf(stderr,"Will now sleep for some seconds\n");
  sleep(30);


  if (0) {
    fprintf(stderr,"Will now ping the server 1 multiple times.\n");
    for (i=0; i<10; i++) {
      fprintf(stderr,"Will ping server.\n");
      err=CTClient_RequestPing(client, &requestid, 1);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	fprintf(stderr,"Could not send message.\n");
	CTClient_free(client);
	return 2;
      }
      fprintf(stderr,"Will wait for pong.\n");
      waitForPing(client, requestid);
    }
    fprintf(stderr,"Done.\n");
  }

  fprintf(stderr,"Will now deinitialize client.\n");
  err=CTClient_Fini(client);
  if (!Error_IsOk(err)) {
    fprintf(stderr,"Could not deinitialize client.\n");
    CTClient_free(client);
    return 2;
  }
  fprintf(stderr,"Done.\n");

  CTClient_free(client);

  err=CTService_ModuleFini();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }
  /* deinit all modules */
  err=CTService_ModuleFini();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }
  err=Chameleon_Fini();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }

  fprintf(stderr,"Program finished.\n");
  return 0;
}



int main(int argc, char **argv) {
  return doTest(argc, argv);
}


