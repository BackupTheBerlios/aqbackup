


#include <backup/server_p.h>
#include <backup/client_direct.h>
#include <backup/aqbackup_p.h>
#include <backup/misc.h>
#include <converter/converter_zip.h>
#include <converter/converter_unzip.h>
#include <converter/converter_rmd160.h>
#include <converter/converter_md5.h>
#include <converter/converter_filein.h>
#include <converter/converter_fileout.h>
#include <converter/converter_dummy.h>

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


int testCreate(int argc, char **argv) {
  int cid;
  int rid;
  int rv;
  CONFIGGROUP *cfg;
  AQB_CLIENT *s;

  Logger_SetLevel(LoggerLevelInfo);

  cfg=Config_new();
  rv=Config_ReadFile(cfg,
		     "test.conf",
		     CONFIGMODE_ALLOW_GROUPS |
		     CONFIGMODE_OVERWRITE_VARS |
		     CONFIGMODE_REMOVE_QUOTES |
		     CONFIGMODE_REMOVE_STARTING_BLANKS |
		     CONFIGMODE_REMOVE_TRAILING_BLANKS
		    );
  if (rv) {
    DBG_ERROR("Could not read file.");
    return 1;
  }

  s=AQBClientDirect_new();
  rv=AQBClient_Init(s, cfg);
  if (rv) {
    DBG_ERROR("Could not init server.");
    return 1;
  }

  cid=AQBClient_Register(s, "martin");
  if (cid==0) {
    DBG_ERROR("Could not register client.");
    return 2;
  }
  DBG_INFO("Client registered as %d", cid);

  rid=AQBClient_CreateRepository(s,
				 "lancelot",
				 "martin");
  if (rid==0) {
    DBG_ERROR("Could not create repository.");
    return 3;
  }
  DBG_INFO("Created repository as %d", rid);

  rv=AQBClient_CloseRepository(s);
  if (rv) {
    DBG_ERROR("Could not close repository.");
    return 4;
  }

  return 0;
}



int testOpen(int argc, char **argv) {
  int cid;
  int rid;
  int did;
  int rv;
  CONFIGGROUP *cfg;
  AQB_CLIENT *s;
  AQB_ENTRY *e, *ewalk;

  Logger_SetLevel(LoggerLevelDebug);

  cfg=Config_new();
  rv=Config_ReadFile(cfg,
		     "test.conf",
		     CONFIGMODE_ALLOW_GROUPS |
		     CONFIGMODE_OVERWRITE_VARS |
		     CONFIGMODE_REMOVE_QUOTES |
		     CONFIGMODE_REMOVE_STARTING_BLANKS |
		     CONFIGMODE_REMOVE_TRAILING_BLANKS
		    );
  if (rv) {
    DBG_ERROR("Could not read file.");
    return 1;
  }

  s=AQBClientDirect_new();
  rv=AQBClient_Init(s, cfg);
  if (rv) {
    DBG_ERROR("Could not init server.");
    return 1;
  }

  cid=AQBClient_Register(s, "martin");
  if (cid==0) {
    DBG_ERROR("Could not register client.");
    return 2;
  }
  DBG_INFO("Client registered as %d", cid);

  rid=AQBClient_OpenRepository(s,
			      "lancelot",
			      "martin",
			       1);
  if (rid==0) {
    DBG_ERROR("Could not open repository.");
    return 3;
  }
  DBG_INFO("Opened repository as %d", rid);

  did=AQBClient_OpenDir(s, "/tmp/dev");
  if (did==0) {
    DBG_ERROR("Could not open directory \"/tmp\"");
    return 1;
  }

  e=0;
  rv=AQBackup_Dir2Entries(&e, "/tmp/dev",0);
  if (rv) {
    DBG_ERROR("Could not scan directory");
    return 2;
  }

  ewalk=e;
  while(ewalk) {
    DBG_INFO("Adding entry \"%s\"", ewalk->name);
    AQBEntry_dump(stderr, ewalk);
    rv=AQBClient_SetEntry(s, did, ewalk);
    if (rv) {
      DBG_ERROR("Could not add entry.");
    }
    ewalk=ewalk->next;
  }

  rv=AQBClient_CloseDir(s, did);
  if (rv) {
    DBG_ERROR("Could not close dir");
    return 2;
  }

  rv=AQBClient_CloseRepository(s);
  if (rv) {
    DBG_ERROR("Could not close repository.");
    return 4;
  }

  return 0;
}


int testFile(int argc, char **argv) {
  int cid;
  int rid;
  int did;
  int rv;
  CONFIGGROUP *cfg;
  AQB_CLIENT *s;
  AQB_ENTRY *e, *ewalk;

  Logger_SetLevel(LoggerLevelDebug);

  cfg=Config_new();
  rv=Config_ReadFile(cfg,
		     "test.conf",
		     CONFIGMODE_ALLOW_GROUPS |
		     CONFIGMODE_OVERWRITE_VARS |
		     CONFIGMODE_REMOVE_QUOTES |
		     CONFIGMODE_REMOVE_STARTING_BLANKS |
		     CONFIGMODE_REMOVE_TRAILING_BLANKS
		    );
  if (rv) {
    DBG_ERROR("Could not read file.");
    return 1;
  }

  s=AQBClientDirect_new();
  rv=AQBClient_Init(s, cfg);
  if (rv) {
    DBG_ERROR("Could not init server.");
    return 1;
  }

  cid=AQBClient_Register(s, "martin");
  if (cid==0) {
    DBG_ERROR("Could not register client.");
    return 2;
  }
  DBG_INFO("Client registered as %d", cid);

  rid=AQBClient_OpenRepository(s,
			      "lancelot",
			      "martin",
			       1);
  if (rid==0) {
    DBG_ERROR("Could not open repository.");
    return 3;
  }
  DBG_INFO("Opened repository as %d", rid);

  did=AQBClient_OpenDir(s, "/home/martin/tmp/test");
  if (did==0) {
    DBG_ERROR("Could not open directory \"/home/martin/tmp/test\"");
    return 1;
  }

  e=0;
  rv=AQBackup_Dir2Entries(&e, "/home/martin/tmp/test",0);
  if (rv) {
    DBG_ERROR("Could not scan directory");
    return 2;
  }

  ewalk=e;
  while(ewalk) {
    AQB_ENTRY_VERSION *v;

    DBG_INFO("Handling file \"%s\"", ewalk->name);
    AQBEntry_dump(stderr, ewalk);

    v=ewalk->versions;
    if (!S_ISLNK(v->mode) && S_ISREG(v->mode)) {
      FILE *f;
      char fullname[256];
      char unescapebuffer[256];
      char databuffer[1024];
      int bytesread;
      char *p;
      int fid;
      int rv;

      DBG_INFO("Opening file \"%s\"", ewalk->name);
      fid=AQBClient_FileOpenOut(s, did, ewalk);
      if (fid==0) {
	DBG_ERROR("Could not open file for writing.");
	return 1;
      }
      strcpy(fullname, "/home/martin/tmp/test/");
      p=Text_Unescape(ewalk->name, unescapebuffer, sizeof(unescapebuffer));
      if (!p) {
	DBG_ERROR("Could not unescape");
        return 1;
      }
      strcat(fullname, p);

      f=fopen(fullname, "rb");
      if (!f) {
	DBG_ERROR("Error on fopen(%s): %s",
		  fullname, strerror(errno));
	return 1;
      }
      while(!feof(f)) {
	bytesread=fread(databuffer, 1, sizeof(databuffer), f);
	if (bytesread<0) {
	  DBG_ERROR("Error on fread(%s): %s",
		    fullname, strerror(errno));
	  return 1;
	}
	else if (bytesread==0)
          break;
	rv=AQBClient_FileWrite(s, fid, databuffer, bytesread);
	if (rv) {
	  DBG_ERROR("Could not write file");
	  return 2;
	}
      } /* while */

      fclose(f);
      DBG_INFO("File written");
      rv=AQBClient_FileCloseOut(s, fid, "");
      if (rv) {
	DBG_ERROR("Could not close file");
	return 2;
      }
    }
    else {
      rv=AQBClient_SetEntry(s, did, ewalk);
      if (rv) {
	DBG_ERROR("Could not add entry.");
      }
    }

    ewalk=ewalk->next;
  }

  rv=AQBClient_CloseDir(s, did);
  if (rv) {
    DBG_ERROR("Could not close dir");
    return 2;
  }

  rv=AQBClient_CloseRepository(s);
  if (rv) {
    DBG_ERROR("Could not close repository.");
    return 4;
  }

  return 0;
}



int testFile2(int argc, char **argv) {
  int cid;
  int rid;
  int did;
  int rv;
  CONFIGGROUP *cfg;
  AQB_CLIENT *s;
  AQB_ENTRY *e;
  AQB_ENTRY *modEntries;
  AQB_ENTRY *modFiles;
  AQB_ENTRY *delEntries;
  STRINGLIST *sl;
  STRINGLISTENTRY *se;
  char unescapebuffer[256];

  Logger_SetLevel(LoggerLevelDebug);

  sl=StringList_new();
  cfg=Config_new();
  rv=Config_ReadFile(cfg,
		     "test.conf",
		     CONFIGMODE_ALLOW_GROUPS |
		     CONFIGMODE_OVERWRITE_VARS |
		     CONFIGMODE_REMOVE_QUOTES |
		     CONFIGMODE_REMOVE_STARTING_BLANKS |
		     CONFIGMODE_REMOVE_TRAILING_BLANKS
		    );
  if (rv) {
    DBG_ERROR("Could not read file.");
    return 1;
  }

  s=AQBClientDirect_new();
  rv=AQBClient_Init(s, cfg);
  if (rv) {
    DBG_ERROR("Could not init server.");
    return 1;
  }

  cid=AQBClient_Register(s, "martin");
  if (cid==0) {
    DBG_ERROR("Could not register client.");
    return 2;
  }
  DBG_INFO("Client registered as %d", cid);

  rid=AQBClient_OpenRepository(s,
			      "lancelot",
			      "martin",
			       1);
  if (rid==0) {
    DBG_ERROR("Could not open repository.");
    return 3;
  }
  DBG_INFO("Opened repository as %d", rid);

  did=AQBClient_OpenDir(s, "/home/martin/tmp/test");
  if (did==0) {
    DBG_ERROR("Could not open directory \"/home/martin/tmp/test\"");
    return 1;
  }

  modEntries=0;
  modFiles=0;
  modEntries=0;
  delEntries=0;
  rv=AQBackup_GetDiffEntriesForBackup(s,
				      did,
				      "/home/martin/tmp/test",
                                      0,
				      sl,
				      &modEntries,
				      &modFiles,
				      &delEntries,
				      0);

  if (rv) {
    DBG_ERROR("Could not get diffs");
    return 2;
  }

  if (sl->count) {
    se=sl->first;
    fprintf(stdout, "Directories:\n");
    while(se) {
      fprintf(stdout, " - %s\n", se->data);
      se=se->next;
    }
  }

  e=modFiles;
  if (e)
    fprintf(stdout, "Modified files:\n");
  while (e) {
    fprintf(stdout, "- %s\n", Text_Unescape(e->name,
					    unescapebuffer,
					    sizeof(unescapebuffer)));
    e=e->next;
  }

  e=modEntries;
  if (e)
    fprintf(stdout, "Modified entries:\n");
  while (e) {
    fprintf(stdout, "- %s\n", Text_Unescape(e->name,
					    unescapebuffer,
					    sizeof(unescapebuffer)));
    e=e->next;
  }

  e=delEntries;
  if (e)
    fprintf(stdout, "Deleted entries:\n");
  while (e) {
    fprintf(stdout, "- %s\n", Text_Unescape(e->name,
					    unescapebuffer,
					    sizeof(unescapebuffer)));
    e=e->next;
  }


  rv=AQBClient_CloseDir(s, did);
  if (rv) {
    DBG_ERROR("Could not close dir");
    return 2;
  }

  rv=AQBClient_CloseRepository(s);
  if (rv) {
    DBG_ERROR("Could not close repository.");
    return 4;
  }

  return 0;
}



int testZip(int argc, char **argv) {
  FILE *fin, *fout;
  CONVERTER *dm;
  int rv;
  int ineof, outeof;
  char inbuffer[1024];
  char outbuffer[444];
  int bytesin;
  int bytesout;

  Logger_SetLevel(LoggerLevelDebug);

  dm=ConverterZip_new(6);
  fin=fopen("/home/martin/tmp/testfile.in", "r");
  if (!fin) {
    DBG_ERROR("Could not open infile (%s)\n",
	      strerror(errno));
    return 1;
  }

  fout=fopen("/home/martin/tmp/testfile.out", "w+");
  if (!fout) {
    DBG_ERROR("Could not open outfile (%s)\n",
	      strerror(errno));
    return 1;
  }

  rv=Converter_Begin(dm);
  if (rv!=0) {
    DBG_ERROR("Error: %d", rv);
    return 1;
  }

  ineof=0;
  outeof=0;
  while (!outeof) {
    /* check if input data is needed */
    DBG_INFO("Data needed ?");
    if (!ineof) {
      bytesin=Converter_NeedsData(dm);
      DBG_INFO("Data needed :%d", bytesin);
      if (bytesin) {
	if (bytesin>sizeof(inbuffer))
	  bytesin=sizeof(inbuffer);
	rv=fread(inbuffer, 1, bytesin, fin);
	if (rv==0) {
	  ineof=1;
	  rv=Converter_SetData(dm, 0, 0);
	}
	else if (rv<0) {
	  DBG_ERROR("Could not read from infile (%s)\n",
		    strerror(errno));
	  return 1;
	}
	else {
	  DBG_INFO("Setting data");
	  rv=Converter_SetData(dm, inbuffer, rv);
	}
	if (rv<0) {
	  DBG_ERROR("Error setting data (%d)", rv);
	  return 1;
	}
      }
    }

    /* now process data */
    DBG_INFO("Processing data");
    rv=Converter_Work(dm);
    if (ineof && rv==CONVERTER_RESULT_EOF)
      outeof=1;
    else {
      if (rv!=CONVERTER_RESULT_OK) {
	DBG_ERROR("Error processing data (%d)", rv);
	return 1;
      }
    }

    /* check if there already is data */
    while ((bytesout=Converter_HasData(dm))) {
      DBG_INFO("Data available :%d", bytesout);
      if (bytesout>sizeof(outbuffer)) {
	DBG_NOTICE("Getting less than available (%d of %d bytes)",
		   sizeof(outbuffer), bytesout);
	bytesout=sizeof(outbuffer);
      }
      DBG_INFO("Getting data");
      rv=Converter_GetData(dm, outbuffer, bytesout);
      if (rv<1) {
	DBG_ERROR("Error getting data");
	return 1;
      }
      if (fwrite(outbuffer, rv, 1, fout)!=1) {
	DBG_ERROR("Could not write to outfile (%s)\n",
		  strerror(errno));
	return 1;
      }
    } /* while has data */
  } /* while */

  fclose(fin);
  if (fclose(fout)) {
    DBG_ERROR("Could not close outfile (%s)\n",
	      strerror(errno));
    return 1;
  }

  return 0;
}


int testUnzip(int argc, char **argv) {
  FILE *fin, *fout;
  CONVERTER *dm;
  int rv;
  int ineof, outeof;
  char inbuffer[1024];
  char outbuffer[2048];
  int bytesin;
  int bytesout;

  Logger_SetLevel(LoggerLevelDebug);

  dm=ConverterUnzip_new();
  fin=fopen("/home/martin/tmp/testfile.out", "r");
  if (!fin) {
    DBG_ERROR("Could not open infile (%s)\n",
	      strerror(errno));
    return 1;
  }

  fout=fopen("/home/martin/tmp/testfile.out.out", "w+");
  if (!fout) {
    DBG_ERROR("Could not open outfile (%s)\n",
	      strerror(errno));
    return 1;
  }

  rv=Converter_Begin(dm);
  if (rv!=0) {
    DBG_ERROR("Error: %d", rv);
    return 1;
  }

  ineof=0;
  outeof=0;
  while (!outeof) {
    /* check if input data is needed */
    DBG_INFO("Data needed ?");
    if (!ineof) {
      bytesin=Converter_NeedsData(dm);
      DBG_INFO("Data needed :%d", bytesin);
      if (bytesin) {
	if (bytesin>sizeof(inbuffer))
	  bytesin=sizeof(inbuffer);
	rv=fread(inbuffer, 1, bytesin, fin);
	if (rv==0) {
	  ineof=1;
          DBG_INFO("Setting eof here");
	  rv=Converter_SetData(dm, 0, 0);
	}
	else if (rv<0) {
	  DBG_ERROR("Could not read from infile (%s)\n",
		    strerror(errno));
	  return 1;
	}
	else {
	  DBG_INFO("Setting data (%d)", rv);
	  rv=Converter_SetData(dm, inbuffer, rv);
	}
	if (rv<0) {
	  DBG_ERROR("Error setting data (%d)", rv);
	  return 1;
	}
      }
    }

    /* now process data */
    DBG_INFO("Processing data");
    rv=Converter_Work(dm);
    if (rv==CONVERTER_RESULT_EOF)
      outeof=1;
    else {
      if (rv!=CONVERTER_RESULT_OK) {
	DBG_ERROR("Error processing data (%d)",rv);
	return 1;
      }
    }

    /* check if there already is data */
    while ((bytesout=Converter_HasData(dm))) {
      DBG_INFO("Data available :%d", bytesout);
      if (bytesout>sizeof(outbuffer)) {
	DBG_NOTICE("Getting less than available (%d of %d bytes)",
		   sizeof(outbuffer), bytesout);
	bytesout=sizeof(outbuffer);
      }
      DBG_INFO("Getting data");
      rv=Converter_GetData(dm, outbuffer, bytesout);
      if (rv<1) {
	DBG_ERROR("Error getting data");
	return 1;
      }
      if (fwrite(outbuffer, rv, 1, fout)!=1) {
	DBG_ERROR("Could not write to outfile (%s)\n",
		  strerror(errno));
	return 1;
      }
    } /* while has data */
  } /* while */

  fclose(fin);
  if (fclose(fout)) {
    DBG_ERROR("Could not close outfile (%s)\n",
	      strerror(errno));
    return 1;
  }

  return 0;
}


int testRmd(int argc, char **argv) {
  FILE *fin, *fout;
  CONVERTER *dm;
  int rv;
  int ineof, outeof;
  char inbuffer[1024];
  char outbuffer[128];
  char hexbuffer[257];
  int bytesin;
  int bytesout;

  Logger_SetLevel(LoggerLevelDebug);

  dm=ConverterRmd160_new();
  fin=fopen("/home/martin/tmp/testfile.in", "r");
  if (!fin) {
    DBG_ERROR("Could not open infile (%s)\n",
	      strerror(errno));
    return 1;
  }

  fout=fopen("/home/martin/tmp/testfile.rmd", "w+");
  if (!fout) {
    DBG_ERROR("Could not open outfile (%s)\n",
	      strerror(errno));
    return 1;
  }

  rv=Converter_Begin(dm);
  if (rv!=0) {
    DBG_ERROR("Error: %d", rv);
    return 1;
  }

  ineof=0;
  outeof=0;
  while (!outeof) {
    /* check if input data is needed */
    DBG_INFO("Data needed ?");
    if (!ineof) {
      bytesin=Converter_NeedsData(dm);
      DBG_INFO("Data needed :%d", bytesin);
      if (bytesin) {
	if (bytesin>sizeof(inbuffer))
	  bytesin=sizeof(inbuffer);
	rv=fread(inbuffer, 1, bytesin, fin);
	if (rv==0) {
	  ineof=1;
	  rv=Converter_SetData(dm, 0, 0);
	}
	else if (rv<0) {
	  DBG_ERROR("Could not read from infile (%s)\n",
		    strerror(errno));
	  return 1;
	}
	else {
	  DBG_INFO("Setting data");
	  rv=Converter_SetData(dm, inbuffer, rv);
	}
	if (rv<0) {
	  DBG_ERROR("Error setting data (%d)", rv);
	  return 1;
	}
      }
    }

    /* now process data */
    DBG_INFO("Processing data");
    rv=Converter_Work(dm);
    if (ineof && rv==CONVERTER_RESULT_EOF)
      outeof=1;
    else {
      if (rv!=CONVERTER_RESULT_OK) {
	DBG_ERROR("Error processing data (%d)", rv);
	return 1;
      }
    }

    /* check if there already is data */
    bytesout=Converter_HasData(dm);
    DBG_INFO("Data available :%d", bytesout);
    if (bytesout) {
      if (bytesout>sizeof(outbuffer)) {
	DBG_ERROR("Buffer too small !!");
	return 1;
      }
      DBG_INFO("Getting data");
      rv=Converter_GetData(dm, outbuffer, bytesout);
      if (rv<1) {
	DBG_ERROR("Error getting data");
        return 1;
      }
      Text_ToHex(outbuffer, rv, hexbuffer, sizeof(hexbuffer));
      if (fwrite(hexbuffer, strlen(hexbuffer), 1, fout)!=1) {
	DBG_ERROR("Could not write to outfile (%s)\n",
		  strerror(errno));
	return 1;
      }
    }

  } /* while */

  fclose(fin);
  if (fclose(fout)) {
    DBG_ERROR("Could not close outfile (%s)\n",
	      strerror(errno));
    return 1;
  }

  return 0;
}


int testMd5(int argc, char **argv) {
  FILE *fin, *fout;
  CONVERTER *dm;
  int rv;
  int ineof, outeof;
  char inbuffer[1024];
  char outbuffer[128];
  char hexbuffer[257];
  int bytesin;
  int bytesout;

  Logger_SetLevel(LoggerLevelDebug);

  dm=ConverterMd5_new();
  fin=fopen("/home/martin/tmp/testfile.in", "r");
  if (!fin) {
    DBG_ERROR("Could not open infile (%s)\n",
	      strerror(errno));
    return 1;
  }

  fout=fopen("/home/martin/tmp/testfile.md5", "w+");
  if (!fout) {
    DBG_ERROR("Could not open outfile (%s)\n",
	      strerror(errno));
    return 1;
  }

  rv=Converter_Begin(dm);
  if (rv!=0) {
    DBG_ERROR("Error: %d", rv);
    return 1;
  }

  ineof=0;
  outeof=0;
  while (!outeof) {
    /* check if input data is needed */
    DBG_INFO("Data needed ?");
    if (!ineof) {
      bytesin=Converter_NeedsData(dm);
      DBG_INFO("Data needed :%d", bytesin);
      if (bytesin) {
	if (bytesin>sizeof(inbuffer))
	  bytesin=sizeof(inbuffer);
	rv=fread(inbuffer, 1, bytesin, fin);
	if (rv==0) {
	  ineof=1;
	  rv=Converter_SetData(dm, 0, 0);
	}
	else if (rv<0) {
	  DBG_ERROR("Could not read from infile (%s)\n",
		    strerror(errno));
	  return 1;
	}
	else {
	  DBG_INFO("Setting data");
	  rv=Converter_SetData(dm, inbuffer, rv);
	}
	if (rv<0) {
	  DBG_ERROR("Error setting data (%d)", rv);
	  return 1;
	}
      }
    }

    /* now process data */
    DBG_INFO("Processing data");
    rv=Converter_Work(dm);
    if (ineof && rv==CONVERTER_RESULT_EOF)
      outeof=1;
    else {
      if (rv!=CONVERTER_RESULT_OK) {
	DBG_ERROR("Error processing data (%d)", rv);
	return 1;
      }
    }

    /* check if there already is data */
    bytesout=Converter_HasData(dm);
    DBG_INFO("Data available :%d", bytesout);
    if (bytesout) {
      if (bytesout>sizeof(outbuffer)) {
	DBG_ERROR("Buffer too small !!");
	return 1;
      }
      DBG_INFO("Getting data");
      rv=Converter_GetData(dm, outbuffer, bytesout);
      if (rv<1) {
	DBG_ERROR("Error getting data");
        return 1;
      }
      Text_ToHex(outbuffer, rv, hexbuffer, sizeof(hexbuffer));
      if (fwrite(hexbuffer, strlen(hexbuffer), 1, fout)!=1) {
	DBG_ERROR("Could not write to outfile (%s)\n",
		  strerror(errno));
	return 1;
      }
    }

  } /* while */

  fclose(fin);
  if (fclose(fout)) {
    DBG_ERROR("Could not close outfile (%s)\n",
	      strerror(errno));
    return 1;
  }

  return 0;
}


int testHex(int argc, char **argv) {
  char hbuffer[256];
  unsigned int n;
  char wbuffer[4];
  char *p;

  if (argc<2) {
    DBG_ERROR("You must give a number");
    return 1;
  }

  sscanf(argv[1], "%i", &n);
  wbuffer[0]=(n>>24) & 0xff;
  wbuffer[1]=(n>>16) & 0xff;
  wbuffer[2]=(n>>8) & 0xff;
  wbuffer[3]=n & 0xff;
  p=Text_ToHexGrouped(wbuffer, sizeof(wbuffer),
		      hbuffer,
                      sizeof(hbuffer),
		      2, '/',
		      1); /* skip leading zeroes */
  if (!p) {
    DBG_ERROR("Conversion error");
    return 0;
  }

  fprintf(stderr, "%8x=%s\n", n, p);
  return 0;
}


int testHex2(int argc, char **argv) {
  char buffer[256];
  unsigned int n1, n2;
  char *p;

  if (argc<3) {
    DBG_ERROR("You must give two numbers");
    return 1;
  }

  sscanf(argv[1], "%i", &n1);
  sscanf(argv[2], "%i", &n2);

  p=AQBServer__MakeStoragePath(n1,n2,
			       buffer,
			       sizeof(buffer));
  if (!p) {
    DBG_ERROR("Conversion error");
    return 0;
  }

  fprintf(stderr, "%8x-%8x=%s\n", n1, n2, p);
  return 0;
}



int testFindRepo(int argc, char **argv) {
  AQBACKUP *b;
  int rv;
  CONFIGGROUP *cfg;
  AQB_BACKUP_REPOSITORY *r;

  Logger_SetLevel(LoggerLevelInfo);

  if (argc<2) {
    DBG_ERROR("Name needed");
    return 1;
  }

  b=AQBackup_new();

  cfg=Config_new();
  rv=Config_ReadFile(cfg,
		     "testc.conf",
		     CONFIGMODE_ALLOW_GROUPS |
		     CONFIGMODE_OVERWRITE_VARS |
		     CONFIGMODE_REMOVE_QUOTES |
		     CONFIGMODE_REMOVE_STARTING_BLANKS |
		     CONFIGMODE_REMOVE_TRAILING_BLANKS
		    );
  if (rv) {
    DBG_ERROR("Could not read file.");
    return 1;
  }

  rv=AQBackup_Init(b, cfg);
  if (rv) {
    DBG_ERROR("Could not init");
    return 2;
  }

  r=AQBackup__FindMatchingRepository(b, argv[1]);
  if (r) {
    fprintf(stdout,
	    "Matching repository is: %s\n",
	    r->name);
    return 0;
  }
  else {
    DBG_ERROR("No matching repository found");
    return 2;
  }
}


int testAdvOpen(int argc, char **argv) {
  AQBACKUP *b;
  int rv;
  CONFIGGROUP *cfg;

  Logger_SetLevel(LoggerLevelDebug);

  if (argc<2) {
    DBG_ERROR("Name needed");
    return 1;
  }

  b=AQBackup_new();

  cfg=Config_new();
  rv=Config_ReadFile(cfg,
		     "testc.conf",
		     CONFIGMODE_ALLOW_GROUPS |
		     CONFIGMODE_OVERWRITE_VARS |
		     CONFIGMODE_REMOVE_QUOTES |
		     CONFIGMODE_REMOVE_STARTING_BLANKS |
		     CONFIGMODE_REMOVE_TRAILING_BLANKS
		    );
  if (rv) {
    DBG_ERROR("Could not read file.");
    return 1;
  }

  rv=AQBackup_Init(b, cfg);
  if (rv) {
    DBG_ERROR("Could not init");
    return 2;
  }

  if (AQBackup_Open(b, argv[1], 1)) {
    DBG_ERROR("Could not open directory \"%s\"", argv[1]);
    return 2;
  }
  return 0;
}


int testAdvStore(int argc, char **argv) {
  AQBACKUP *b;
  int rv;
  CONFIGGROUP *cfg;

  Logger_SetLevel(LoggerLevelNotice);

  if (argc<2) {
    DBG_ERROR("Name needed");
    return 1;
  }

  b=AQBackup_new();

  cfg=Config_new();
  rv=Config_ReadFile(cfg,
		     "testc.conf",
		     CONFIGMODE_ALLOW_GROUPS |
		     CONFIGMODE_OVERWRITE_VARS |
		     CONFIGMODE_REMOVE_QUOTES |
		     CONFIGMODE_REMOVE_STARTING_BLANKS |
		     CONFIGMODE_REMOVE_TRAILING_BLANKS
		    );
  if (rv) {
    DBG_ERROR("Could not read file.");
    return 1;
  }

  rv=AQBackup_Init(b, cfg);
  if (rv) {
    DBG_ERROR("Could not init");
    return 2;
  }

  if (AQBackup_Open(b, argv[1], 1)) {
    DBG_ERROR("Could not open directory \"%s\"", argv[1]);
    return 2;
  }

  rv=AQBackup_HandleDir(b,
			argv[1],
			AQBackupJobStore,
			0,
                        "",
			AQBACKUP_FLAGS_RECURSIVE);
  if (rv) {
    DBG_ERROR("Error handling dir \"%s\"", argv[1]);
  }

  if (AQBackup_Close(b)) {
    DBG_ERROR("Error closing repository");
  }

  return 0;
}


int testShowDiff(int argc, char **argv) {
  AQBACKUP *b;
  int rv;
  CONFIGGROUP *cfg;

  Logger_SetLevel(LoggerLevelNotice);

  if (argc<2) {
    DBG_ERROR("Name needed");
    return 1;
  }

  b=AQBackup_new();

  cfg=Config_new();
  rv=Config_ReadFile(cfg,
		     "testc.conf",
		     CONFIGMODE_ALLOW_GROUPS |
		     CONFIGMODE_OVERWRITE_VARS |
		     CONFIGMODE_REMOVE_QUOTES |
		     CONFIGMODE_REMOVE_STARTING_BLANKS |
		     CONFIGMODE_REMOVE_TRAILING_BLANKS
		    );
  if (rv) {
    DBG_ERROR("Could not read file.");
    return 1;
  }

  rv=AQBackup_Init(b, cfg);
  if (rv) {
    DBG_ERROR("Could not init");
    return 2;
  }

  if (AQBackup_Open(b, argv[1], 1)) {
    DBG_ERROR("Could not open directory \"%s\"", argv[1]);
    return 2;
  }

  rv=AQBackup_HandleDir(b,
			argv[1],
			AQBackupJobShowDiffs,
			0,
			"",
			AQBACKUP_FLAGS_RECURSIVE);
  if (rv) {
    DBG_ERROR("Error handling dir \"%s\"", argv[1]);
  }

  if (AQBackup_Close(b)) {
    DBG_ERROR("Error closing repository");
  }

  return 0;
}


int testRestore(int argc, char **argv) {
  AQBACKUP *b;
  int rv;
  CONFIGGROUP *cfg;

  Logger_SetLevel(LoggerLevelNotice);

  if (argc<3) {
    DBG_ERROR("Path and destdir needed");
    return 1;
  }

  b=AQBackup_new();

  cfg=Config_new();
  rv=Config_ReadFile(cfg,
		     "testc.conf",
		     CONFIGMODE_ALLOW_GROUPS |
		     CONFIGMODE_OVERWRITE_VARS |
		     CONFIGMODE_REMOVE_QUOTES |
		     CONFIGMODE_REMOVE_STARTING_BLANKS |
		     CONFIGMODE_REMOVE_TRAILING_BLANKS
		    );
  if (rv) {
    DBG_ERROR("Could not read file.");
    return 1;
  }

  rv=AQBackup_Init(b, cfg);
  if (rv) {
    DBG_ERROR("Could not init");
    return 2;
  }

  if (AQBackup_Open(b, argv[1], 1)) {
    DBG_ERROR("Could not open directory \"%s\"", argv[1]);
    return 2;
  }

  rv=AQBackup_HandleDir(b,
			argv[1],
			AQBackupJobRestore,
			0,
			argv[2],
			AQBACKUP_FLAGS_RECURSIVE |
			/* AQBACKUP_FLAGS_REMOTE_ALL| */
			AQBACKUP_FLAGS_VERBOUS);
  if (rv) {
    DBG_ERROR("Error handling dir \"%s\"", argv[1]);
  }

  if (AQBackup_Close(b)) {
    DBG_ERROR("Error closing repository");
  }

  return 0;
}


int testConverterGroup(int argc, char **argv) {
  FILE *fin, *fout;
  CONVERTER *dmgroup;
  CONVERTER *dm;
  CONVERTER *dmMD5;
  int rv;
  int ineof, outeof;
  char inbuffer[1024];
  char outbuffer[444];
  int bytesin;
  int bytesout;

  Logger_SetLevel(LoggerLevelNotice);
  DBG_DEBUG("Starting");

  dmgroup=ConverterZip_new(6);
  dm=ConverterUnzip_new();
  ConverterGroup_Append(dmgroup, dm);
  dmMD5=ConverterMd5_new();
  ConverterGroup_Tee(dmgroup, dmMD5);
  fin=fopen("/home/martin/tmp/testfile.in", "r");
  if (!fin) {
    DBG_ERROR("Could not open infile (%s)\n",
	      strerror(errno));
    return 1;
  }

  fout=fopen("/home/martin/tmp/testfile.out", "w+");
  if (!fout) {
    DBG_ERROR("Could not open outfile (%s)\n",
	      strerror(errno));
    return 1;
  }

  rv=ConverterGroup_Begin(dmgroup);
  if (rv!=0) {
    DBG_ERROR("Error: %d", rv);
    return 1;
  }

  ineof=0;
  outeof=0;
  while (!outeof) {
    /* check if input data is needed */
    DBG_INFO("Data needed ?");
    if (!ineof) {
      bytesin=ConverterGroup_NeedsData(dmgroup);
      DBG_INFO("Data needed :%d", bytesin);
      if (bytesin) {
	if (bytesin>sizeof(inbuffer))
	  bytesin=sizeof(inbuffer);
	rv=fread(inbuffer, 1, bytesin, fin);
	if (rv==0) {
	  ineof=1;
	  rv=ConverterGroup_SetData(dmgroup, 0, 0);
	}
	else if (rv<0) {
	  DBG_ERROR("Could not read from infile (%s)\n",
		    strerror(errno));
	  return 1;
	}
	else {
	  DBG_INFO("Setting data");
	  rv=ConverterGroup_SetData(dmgroup, inbuffer, rv);
	}
	if (rv<0) {
	  DBG_ERROR("Error setting data (%d)", rv);
	  return 1;
	}
      }
    }

    /* now process data */
    DBG_INFO("Processing data");
    rv=ConverterGroup_Work(dmgroup);
    if (ineof && rv==CONVERTER_RESULT_EOF)
      outeof=1;
    else {
      if (rv!=CONVERTER_RESULT_OK) {
	DBG_ERROR("Error processing data (%d)", rv);
	return 1;
      }
    }

    /* check if there already is data */
    while ((bytesout=ConverterGroup_HasData(dmgroup))>0) {
      DBG_INFO("Data available :%d", bytesout);
      if (bytesout>sizeof(outbuffer)) {
	DBG_INFO("Getting less than available (%d of %d bytes)",
		 sizeof(outbuffer), bytesout);
	bytesout=sizeof(outbuffer);
      }
      DBG_INFO("Getting data");
      rv=ConverterGroup_GetData(dmgroup, outbuffer, bytesout);
      if (rv<1) {
	DBG_ERROR("Error getting data");
	return 1;
      }
      if (fwrite(outbuffer, rv, 1, fout)!=1) {
	DBG_ERROR("Could not write to outfile (%s)\n",
		  strerror(errno));
	return 1;
      }
    } /* while has data */
  } /* while */

  /* check if there already is data */
  bytesout=ConverterGroup_HasData(dmMD5);
  if (bytesout){
    if (bytesout>sizeof(outbuffer)) {
      DBG_ERROR("Buffer too small");
      return 2;
    }
    DBG_INFO("Data available :%d", bytesout);
    if (bytesout>sizeof(outbuffer)) {
      DBG_NOTICE("Getting less than available (%d of %d bytes)",
		 sizeof(outbuffer), bytesout);
      bytesout=sizeof(outbuffer);
    }
    DBG_INFO("Getting data");
    rv=ConverterGroup_GetData(dmMD5, outbuffer, bytesout);
    if (rv<1) {
      DBG_ERROR("Error getting data");
      return 1;
    }
    else {
      char hexbuffer[256];

      if (Text_ToHex(outbuffer, rv, hexbuffer, sizeof(hexbuffer))==0) {
	DBG_ERROR("Could not transform to hex");
        return 3;
      }
      DBG_NOTICE("MD5 is : %s", hexbuffer);
    }
  } /* if has data */

  fclose(fin);
  if (fclose(fout)) {
    DBG_ERROR("Could not close outfile (%s)\n",
	      strerror(errno));
    return 1;
  }

  ConverterGroup_free(dmgroup);

  return 0;
}


int testConverterGroup2(int argc, char **argv) {
  CONVERTER *dmgroup;
  CONVERTER *dm;
  CONVERTER *dmMD5;
  int rv;
  char outbuffer[444];
  int bytesout;

  Logger_SetLevel(LoggerLevelNotice);
  DBG_DEBUG("Starting");

  dmgroup=ConverterFileIn_new("/home/martin/tmp/testfile.in");
  dm=ConverterZip_new(6);
  ConverterGroup_Append(dmgroup, dm);
  dmMD5=ConverterMd5_new();
  ConverterGroup_Tee(dm, dmMD5);
  dm=ConverterDummy_new();
  ConverterGroup_Append(dmgroup, dm);
  dm=ConverterUnzip_new();
  ConverterGroup_Append(dmgroup, dm);
  dm=ConverterFileOut_new("/home/martin/tmp/testfile.out","wb");
  ConverterGroup_Append(dmgroup, dm);

  rv=ConverterGroup_Begin(dmgroup);
  if (rv!=0) {
    DBG_ERROR("Error: %d", rv);
    return 1;
  }

  for (;;) {
    /* now process data */
    DBG_INFO("Processing data");
    rv=ConverterGroup_Work(dmgroup);
    if (rv==CONVERTER_RESULT_EOF)
      break;
    else {
      if (rv!=CONVERTER_RESULT_OK) {
	DBG_ERROR("Error processing data (%d)", rv);
	return 1;
      }
    }
  } /* for */

  /* check if there already is data */
  bytesout=ConverterGroup_HasData(dmMD5);
  if (bytesout){
    if (bytesout>sizeof(outbuffer)) {
      DBG_ERROR("Buffer too small");
      return 2;
    }
    DBG_INFO("Data available :%d", bytesout);
    if (bytesout>sizeof(outbuffer)) {
      DBG_NOTICE("Getting less than available (%d of %d bytes)",
		 sizeof(outbuffer), bytesout);
      bytesout=sizeof(outbuffer);
    }
    DBG_INFO("Getting data");
    rv=ConverterGroup_GetData(dmMD5, outbuffer, bytesout);
    if (rv<1) {
      DBG_ERROR("Error getting data");
      return 1;
    }
    else {
      char hexbuffer[256];

      if (Text_ToHex(outbuffer, rv, hexbuffer, sizeof(hexbuffer))==0) {
	DBG_ERROR("Could not transform to hex");
        return 3;
      }
      DBG_NOTICE("MD5 is : %s", hexbuffer);
    }
  } /* if has data */

  rv=ConverterGroup_End(dmgroup);
  if (rv) {
    DBG_ERROR("Error on End (%d)", rv);
    return 3;
  }
  ConverterGroup_free(dmgroup);

  return 0;
}


char *SetTimeValue(time_t value, char *buffer){
  struct tm *tm;

  tm=localtime(&value);
  fprintf(stderr,"DLST: %d\n", tm->tm_isdst);

  buffer[0]=0;
  sprintf(buffer,"%4d/%02d/%02d-%02d:%02d:%02d",
	  tm->tm_year+1900,
	  tm->tm_mon,
	  tm->tm_mday,
	  tm->tm_hour,
	  tm->tm_min,
	  tm->tm_sec);
  return buffer;
}


time_t GetTimeValue(const char *p) {
  char buffer[128];
  struct tm tm;
  struct tm *tt;
  time_t val;
  int year;

  val=time(0);
  tt=gmtime(&val);
  memmove(&tm, tt, sizeof(tm));
  fprintf(stderr, "Now: %4d/%02d/%02d-%02d:%02d:%02d\n",
	 tm.tm_year+1900,
	 tm.tm_mon,
	 tm.tm_mday,
	 tm.tm_hour,
	 tm.tm_min,
	 tm.tm_sec);

  buffer[0]=0;
  if (sscanf(p,"%d/%d/%d-%d:%d:%d",
	     &year,
	     &tm.tm_mon,
	     &tm.tm_mday,
	     &tm.tm_hour,
	     &tm.tm_min,
	     &tm.tm_sec)!=6) {
    DBG_ERROR("Bad time");
    return 0;
  }
  tm.tm_year=year-1900;
  tm.tm_isdst=-1;
  val=mktime(&tm);
  return val;
}


int testTime(int argc, char **argv) {
  time_t t;
  const char *p;
  char buffer[256];

  t=time(0);
  p=SetTimeValue(t, buffer);
  fprintf(stderr,"Time is    : %s (%d)\n", p, (int)t);
  t=GetTimeValue(p);
  p=SetTimeValue(t, buffer);
  fprintf(stderr,"Time is now: %s (%d)\n", p, (int)t);
  return 0;

}


int main(int argc, char **argv) {
  //return testCreate(argc, argv);
  //return testZip(argc, argv);
  //return testUnzip(argc, argv);
  //return testRmd(argc, argv);
  //return testMd5(argc, argv);
  //return testOpen(argc, argv);
  //return testHex2(argc, argv);
  //return testFile(argc, argv);
  //return testFile2(argc, argv);
  //return testFindRepo(argc, argv);
  //return testAdvOpen(argc, argv);
  //return testAdvStore(argc, argv);
  //return testShowDiff(argc, argv);
  //return testRestore(argc, argv);
  //return testConverterGroup(argc, argv);
  //return testConverterGroup2(argc, argv);
  return testTime(argc, argv);
}



