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


#include <ctserver.h>
#include <stdlib.h>
#include <stdio.h>

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>

int main(int argc, char **argv) {
  CTSERVERDATA *server;
  ERRORCODE err;
  int i;

  if (argc<3) {
    fprintf(stderr,"Usage: %s address port\n", argv[0]);
    return 1;
  }

  /* init all modules */
  err=Chameleon_Init();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }
  err=CTCore_ModuleInit();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }
  err=CTService_ModuleInit();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }

  server=CTServer_new();
  fprintf(stderr,"Will now initialize server.\n");
  err=CTServer_Init(server,argv[1], atoi(argv[2]));
  if (!Error_IsOk(err)) {
    fprintf(stderr,"Could not initialize server.\n");
    CTServer_free(server);
    return 2;
  }

  i=0;
  while (i++<10000000) {
    err=CTServer_Work(server, 1000, 1000);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
    }
  } /* while */
  fprintf(stderr,"Done.\n");
  fprintf(stderr,"Will now deinitialize server.\n");
  err=CTServer_Fini(server);
  if (!Error_IsOk(err)) {
    fprintf(stderr,"Could not deinitialize server.\n");
    CTServer_free(server);
    return 2;
  }
  fprintf(stderr,"Done.\n");

  CTServer_free(server);

  /* deinitialize modules */
  err=CTCore_ModuleFini();
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }
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


