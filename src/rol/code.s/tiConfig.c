#if defined(VXWORKS) || defined(Linux_vme)

/****************************************************************************
 *
 *  tiConfig.c  -  configuration library file for ti board 
 *
 
config file format:

CRATE      <tdcecal1>     <- crate name, usually IP name

TI_ADD_SLAVE 1                                 # for every slave need to be added

TI_FIBER_DELAY_OFFSET 0x80 0xcf                # fiber delay and offsets

TI_BLOCK_LEVEL 1                               # the number of events in readout block

TI_BUFFER_LEVEL 1                              # 0 - pipeline mode, 1 - ROC Lock mode, >=2 - buffered mode

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef Linux_vme
#include "jvme.h"
#endif

#include "tiLib.h"

#define FNLEN     128       /* length of config. file name */
#define STRLEN    250       /* length of str_tmp */
#define ROCLEN     80       /* length of ROC_name */

#define MAXSLAVES 100
static int is_slave;
static nslave, slave_list[MAXSLAVES];
static unsigned int delay, offset;
static int block_level;
static int buffer_level;

/* tiInit() have to be called before this function */
int  
tiConfig(char *fname)
{
  int res;

  /* set defaults */
  tiInitGlobals();

  /* read config file */
  if( (res = tiReadConfigFile(fname)) < 0 ) return(res);

  /* download to all boards */
  tiDownloadAll();

  return(0);
}


int
tiInitGlobals()
{
  int ii, jj;

  is_slave = 0;
  delay = 0x80;
  offset = 0xcf;
  block_level = 1;
  buffer_level = 1;
  nslave = 0;
  for(ii=0; ii<MAXSLAVES; ii++) slave_list[ii] = 0;

  return(0);
}


/**/
int
tiReadConfigFile(char *filename)
{
  FILE   *fd;
  char   fname[FNLEN] = { "" };  /* config file name */
  int    ii, jj, ch;
  char   str_tmp[STRLEN], keyword[ROCLEN];
  char   host[ROCLEN], ROC_name[ROCLEN];
  char   str2[2];
  int    args, i1, i2;
  int    slot, chan;
  unsigned int  ui1, ui2;
  char *getenv();
  char *clonparms;
  char *expid;

  clonparms = getenv("CLON_PARMS");
  expid = getenv("EXPID");
  if(strlen(filename)!=0) /* filename specified */
  {
    if ( filename[0]=='/' || (filename[0]=='.' && filename[1]=='/') )
	{
      sprintf(fname, "%s", filename);
	}
    else
	{
      sprintf(fname, "%s/ti/%s", clonparms, filename);
	}

    if((fd=fopen(fname,"r")) == NULL)
    {
      printf("\nReadConfigFile: Can't open config file >%s<\n",fname);
      return(-1);
    }
  }
  else /* filename does not specified */
  {
    /* obtain our hostname */
    gethostname(host,ROCLEN);
    sprintf(fname, "%s/ti/%s.cnf", clonparms, host);
    if((fd=fopen(fname,"r")) == NULL)
    {
      sprintf(fname, "%s/ti/%s.cnf", clonparms, expid);
      if((fd=fopen(fname,"r")) == NULL)
      {
        printf("\nReadConfigFile: Can't open config file >%s<\n",fname);
        return(-2);
	  }
	}

  }
  printf("\nReadConfigFile: Using configuration file >%s<\n",fname);

  /* Parsing of config file */
  while ((ch = getc(fd)) != EOF)
  {
    if ( ch == '#' || ch == ' ' || ch == '\t' )
    {
      while (getc(fd) != '\n') {}
    }
    else if( ch == '\n' ) {}
    else
    {
      ungetc(ch,fd);
      fgets(str_tmp, STRLEN, fd);
      sscanf (str_tmp, "%s %s", keyword, ROC_name);


      /* Start parsing real config inputs */
      if(strcmp(keyword,"CRATE") == 0)
      {
	    if(strcmp(ROC_name,host) != 0)
        {
	      printf("\nReadConfigFile: Wrong crate name in config file, %s\n",str_tmp);
          return(-3);
        }
	    printf("\nReadConfigFile: conf_CRATE_name = %s  host = %s\n",ROC_name,host);
      }

      else if((strcmp(keyword,"TI_ADD_SLAVE")==0))
      {
        sscanf (str_tmp, "%*s %d", &i1);
        slave_list[nslave++] = i1;
      }

      else if((strcmp(keyword,"TI_FIBER_DELAY_OFFSET")==0))
      {
        sscanf (str_tmp, "%*s %x %x", &i1, &i2);
        delay = i1;
        offset = i2;
      }

      else if((strcmp(keyword,"TI_BLOCK_LEVEL")==0))
      {
        sscanf (str_tmp, "%*s %d", &i1);
        block_level = i1;
      }

      else if((strcmp(keyword,"TI_BUFFER_LEVEL")==0))
      {
        sscanf (str_tmp, "%*s %d", &i1);
        buffer_level = i1;
      }

      else
      {
        printf("ReadConfigFile: Unknown Field or Missed Field in\n");
        printf("   %s \n", fname);
        printf("   str_tmp=%s", str_tmp);
        printf("   keyword=%s \n\n", keyword);
        return(-10);
      }
    }
  } /* end of while */

  fclose(fd);

  return(0);
}


int
tiConfigGetBlockLevel()
{
  printf("block_level = %d\n",block_level);
  return(block_level);
}

int
tiDownloadAll()
{
  int ii;

  /*
  tiIntDisable();
  tiDisableVXSSignals();
  */

  for(ii=0; ii<nslave; ii++) tiAddSlave(slave_list[ii]);

  tiSetFiberDelay(delay, offset);

  tiSetBlockLevel(block_level);

  tiSetBlockBufferLevel(buffer_level);

  return(0);
}

void
tiMon(int slot)
{
  tiStatus(1);
}

#else /* dummy version*/

void
tiConfig_dummy()
{
  return;
}

#endif
