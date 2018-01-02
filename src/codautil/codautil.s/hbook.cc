
/* hbook.cc - mini-hbook package (Sergey Boyarinov) */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <strstream>
#include <iostream>
#include <iomanip>
using namespace std;

#include "ipc_lib.h"
#include "MessageActionHist.h"

#include "hbook.h"
#include "evio.h"
#include "evioBankUtil.h"

#define MIN(x,y) ((x) < (y) ? (x) : (y))


void
Hbook::hbook1(int id, char *title, int nbinx, float xmin, float xmax)
{
  int i, nch;

  if(id <=0 || id >=NHIST)
  {
    printf("hbook1: ERROR id=%d, must be from 1 to %d\n",id,NHIST);
    return;
  }

  if(hist[id].nbinx > 0)
  {
    /*printf("hbook1: WARN id=%d already exist - will be replaced\n",id);*/
    hist[id].nbinx = 0;
    /*printf("hbook1: free histogram buffer ... ");*/
    free(hist[id].buf);
    /*printf("done.\n");*/
  }

  hist[id].id = id;
  hist[id].entries = 0;
  hist[id].xunderflow = 0;
  hist[id].xoverflow = 0;
  hist[id].xmin = xmin;

  hist[id].dx = (xmax - xmin)/(float)nbinx;
  /* check bin size consistency */
  if(hist[id].dx <= HZERO)
  {
    printf("hbook1: ERROR dx=%d must be > 0\n",hist[id].dx);
    return;
  }
  if( (xmin + nbinx*hist[id].dx) < xmax)
  {
    hist[id].nbinx = (xmax - xmin)/hist[id].dx + 1;
    hist[id].xmax = xmin + ((float)nbinx)*hist[id].dx;
    printf("hbook1: WARN rebining for [%d]: nbinx=%d xmax=%d\n",
      id,hist[id].nbinx,hist[id].xmax);
  }
  else
  {
    hist[id].nbinx = nbinx;
    hist[id].xmax = xmax;
  }

  hist[id].nbiny = 0; /* to recognize 1-dim from 2-dim */

  nch = strlen(title);
  hist[id].ntitle = nch;
  hist[id].title = strdup(title);

  hist[id].buf = (float *) calloc(hist[id].nbinx,sizeof(float));
  if(hist[id].buf==NULL) {printf("hbook: ERROR in calloc()\n");exit(0);}

#ifdef VXWORKS
  if(hist[id].nbinx <= MAXI2 && hist[id].nbiny <= MAXI2 &&
     hist[id].xmin <= MAXI2 && hist[id].xmax <= MAXI2 &&
     hist[id].ymin <= MAXI2 && hist[id].ymax <= MAXI2)
  {
    hist[id].ifi2_book = 1;
    hist[id].ifi2_fill = 1;
  }
  else
  {
    hist[id].ifi2_book = 0;
    hist[id].ifi2_fill = 0;
  }
#endif
  /*printf("hbook1: [%d] nbinx=%d xmin=%d xmax=%d ntitle=%d\n",
    id,hist[id].nbinx,hist[id].xmin,hist[id].xmax,hist[id].ntitle);*/

  return;
}




void
Hbook::hbook2(int id, char *title, int nbinx, float xmin, float xmax, int nbiny, float ymin, float ymax)
{
  int i, nch;

  if(id <=0 || id >=NHIST)
  {
    printf("hbook2: ERROR id=%d, must be from 1 to %d\n",id,NHIST);
    return;
  }

  if(hist[id].nbinx > 0 || hist[id].nbiny > 0)
  {
    /*printf("hbook2: WARN id=%d already exist - will be replaced\n",id);*/
    hist[id].nbinx = 0;
    hist[id].nbiny = 0;
    return;
  }

  hist[id].id = id;
  hist[id].entries = 0;
  hist[id].xunderflow = 0;
  hist[id].xoverflow = 0;
  hist[id].yunderflow = 0;
  hist[id].yoverflow = 0;
  hist[id].xmin = xmin;
  hist[id].ymin = ymin;

  hist[id].dx = (xmax - xmin)/(float)nbinx;
  /* check bin size consistency */
  if(hist[id].dx <= HZERO)
  {
    printf("hbook2: ERROR dx=%d must be > 0\n",hist[id].dx);
    return;
  }
  hist[id].dy = (ymax - ymin)/(float)nbiny;
  if(hist[id].dy <= HZERO)
  {
    printf("hbook2: ERROR dy=%d must be > 0\n",hist[id].dy);
    return;
  }

  if( (xmin + nbinx*hist[id].dx) < xmax)
  {
    hist[id].nbinx = (xmax - xmin)/hist[id].dx + 1;
    hist[id].xmax = xmin + ((float)nbinx)*hist[id].dx;
    printf("hbook1: WARN rebining for [%d]: nbinx=%d xmax=%d\n",
      id,hist[id].nbinx,hist[id].xmax);
  }
  else
  {
    hist[id].nbinx = nbinx;
    hist[id].xmax = xmax;
  }


  if( (ymin + nbiny*hist[id].dy) < ymax)
  {
    hist[id].nbiny = (ymax - ymin)/hist[id].dy + 1;
    hist[id].ymax = ymin + ((float)nbiny)*hist[id].dy;
    printf("hbook1: WARN rebining for [%d]: nbiny=%d ymax=%d\n",
      id,hist[id].nbiny,hist[id].ymax);
  }
  else
  {
    hist[id].nbiny = nbiny;
    hist[id].ymax = ymax;
  }


  nch = strlen(title);
  hist[id].ntitle = nch;
  hist[id].title = strdup(title);

  hist[id].buf2 = (float **) calloc(nbinx,sizeof(float));
  if(hist[id].buf2==NULL) {printf("hbook: ERROR1 in calloc()\n");exit(0);}
  for(i=0; i<nbinx; i++)
  {
    hist[id].buf2[i] = (float *) calloc(nbiny,sizeof(float));
    if(hist[id].buf2[i]==NULL) {printf("hbook: ERROR2 in calloc()\n");exit(0);}
  }
  printf("hbook2: [%d] nbinx=%d xmin=%f xmax=%f nbiny=%d ymin=%f ymax=%f\n",
    id,hist[id].nbinx,hist[id].xmin,hist[id].xmax,
       hist[id].nbiny,hist[id].ymin,hist[id].ymax);

  return;
}



void
Hbook::hfill(int id, float x, float y, float weight)
{
  int ibinx, ibiny;

  if(id <=0 || id >=NHIST)
  {
    /*printf("uthfill: ERROR id=%d, must be from 1 to %d\n",id,NHIST);*/
    fflush(stdout);
    return;
  }

  if(hist[id].nbinx <= 0)
  {
    printf("uthfill: ERROR id=%d is not booked\n",id);
    fflush(stdout);
    return;
  }

  hist[id].entries += weight;
  if(x < hist[id].xmin)
  {
    hist[id].xunderflow += weight;
#ifdef VXWORKS
    if(hist[id].xunderflow > MAXI2) hist[id].ifi2_fill = 0;
#endif
  }
  else if(x >= hist[id].xmax)
  {
    hist[id].xoverflow += weight;
#ifdef VXWORKS
    if(hist[id].xoverflow > MAXI2) hist[id].ifi2_fill = 0;
#endif
  }
  else
  {
    ibinx = (int)((x - hist[id].xmin) / hist[id].dx);
    if(hist[id].nbiny == 0) /* 1-dim */
    {
      hist[id].buf[ibinx] += weight;
#ifdef VXWORKS
      if(hist[id].buf[ibinx] > MAXI2) hist[id].ifi2_fill = 0;
#endif
    }
    else                    /* 2-dim */
    {
      if(y < hist[id].ymin)
      {
        hist[id].yunderflow += weight;
      }
      else if(y >= hist[id].ymax)
      {
        hist[id].yoverflow += weight;
      }
      else
      {
        ibiny = (int)((y - hist[id].ymin) / hist[id].dy);
        hist[id].buf2[ibinx][ibiny] += weight;
      }
    }
  }

  return;
}




/* reset histogram contents and change title optionaly (not yet !) */

void
Hbook::hreset(int id, char *title)
{
  int ibinx, ibiny;

  if(id <=0 || id >=NHIST)
  {
    printf("uthreset: ERROR id=%d, must be from 1 to %d\n",id,NHIST);
    return;
  }

  if(hist[id].nbinx <= 0)
  {
    printf("uthreset: ERROR id=%d is not booked\n",id);
    return;
  }

  hist[id].entries = 0;
  hist[id].xunderflow = 0;
  hist[id].xoverflow = 0;
  hist[id].yunderflow = 0;
  hist[id].yoverflow = 0;

  if(hist[id].nbiny == 0) /* 1-dim */
  {
    for(ibinx=0; ibinx<hist[id].nbinx; ibinx++)
      hist[id].buf[ibinx] = 0;
  }
  else                    /* 2-dim */
  {
    for(ibinx=0; ibinx<hist[id].nbinx; ibinx++)
      for(ibiny=0; ibiny<hist[id].nbiny; ibiny++)
        hist[id].buf2[ibinx][ibiny] = 0;
  }

  return;
}


void
Hbook::hprint(int id)
{
  int i, a;
  float h_max;

  if(id <=0 || id >=NHIST)
  {
    printf("uthprint: ERROR id=%d, must be from 1 to %d\n",id,NHIST);
    return;
  }

  if(hist[id].nbinx <= 0)
  {
    printf("uthprint: ERROR id=%d is not booked\n",id);
    return;
  }
  
  if(hist[id].nbiny > 0)
  {
    printf("uthprint: ERROR id=%d: 2-dim cannot be printed  - sorry\n",id);
    return;
  }

  for(i=0; i<hist[id].nbinx; i++)
  {
    printf("[%d] --> %f\n",i,hist[id].buf[i]);
  }
  printf("hist %d: xunderflow=%f xoverflow=%f\n",
    id,hist[id].xunderflow,hist[id].xoverflow);


  /* print histogram vertically */

  h_max = 0.0;
  for(i=0; i<hist[id].nbinx; i++) if(hist[id].buf[i]>h_max) h_max=hist[id].buf[i];
  printf("\nh_max=%f\n",h_max);

  for(i=(int)(h_max+1.0); i>=0; i--)
  {
    for (a = 0; a < hist[id].nbinx; ++a)
    {
      if (hist[id].buf[a] > i)
      {
        printf("*"); 
      }
      else
      {
        printf(" ");
      }
    }
    printf("\n");
  }
  printf("\n");


  return;
}


/* report histogram contents as array */
/* 'choice' and 'num' for 2-dim hists only - not done yet !!! */

void
Hbook::hunpak(int id, float *content, char *choice, int num)
{
  int i;

  if(id <=0 || id >=NHIST)
  {
    printf("uthunpak: ERROR id=%d, must be from 1 to %d\n",id,NHIST);
    return;
  }

  if(hist[id].nbinx <= 0)
  {
    printf("uthunpak: ERROR id=%d is not booked\n",id);
    return;
  }
  
  if(hist[id].nbiny > 0)
  {
    printf("uthunpak: ERROR id=%d: 2-dim is not suppported, use 'uthij'\n",id);
    return;
  }

  for(i=0; i<hist[id].nbinx; i++)
  {
    content[i] = hist[id].buf[i];
  }

  return;
}


/* return contents of the bin (i,j) of 2-dim histogram */

float
Hbook::hij(int id, int ibinx, int ibiny)
{
  float tmp;
  /*printf("=> %d %d %d\n",id,ibinx,ibiny);fflush(stdout);*/
  tmp = hist[id].buf2[ibinx][ibiny];
  /*printf("=> %f\n",tmp);fflush(stdout);*/

  return(tmp);
}


/* returns histogram parameters */

void
Hbook::hgive(int id, char *title, int *nbinx, float *xmin,
        float *xmax, int *nbiny, float *ymin, float *ymax, int *titlelen)
{
  int i, len;

  if(id <=0 || id >=NHIST)
  {
    printf("uthgive: ERROR id=%d, must be from 1 to %d\n",id,NHIST);
    return;
  }

  len = MIN(*titlelen,hist[id].ntitle);
  for(i=0; i<*titlelen; i++) title[i] = ' '; /* fill all 'title' by ' ' */
  strncpy(title,hist[id].title,len);
  *titlelen = len;

  *nbinx = hist[id].nbinx;
  *xmin = hist[id].xmin;
  *xmax = hist[id].xmax;

  if(hist[id].nbiny > 0)
  {
    *nbiny = hist[id].nbiny;
    *ymin = hist[id].ymin;
    *ymax = hist[id].ymax;
  }
  else
  {
    *nbiny = 0;
  }

  return;
}


void
Hbook::hidall(int *ids, int *n)
{
  int id, nid;

  nid = 0;
  for(id=0; id<NHIST; id++)
  {
    if(hist[id].nbinx > 0) ids[nid++] = id; 
  }
  *n = nid;
  /*printf("uthidall: nid=%d (%d %d %d ...)\n",nid,ids[0],ids[1],ids[2]);*/

  return;
}


/* returns the number of entries */

int
Hbook::hentries(int id)
{
  if(id <=0 || id >=NHIST)
  {
    printf("uthentries: ERROR id=%d, must be from 1 to %d\n",id,NHIST);
    return(0);
  }

  return(hist[id].entries);
}







/* save histograms in a form of BOS banks */


/*
 !!! THE HISTOGRAM WILL BE CLEANED UP !!!
 !!! TITLE CAN BE LOST IF READING ON ANOTHER PLATFORM !!! 

  HISF bank format:

    bank number = histogram id
    bank format :
       1-dim: "2iNc4fNf"
       2-dim: "2iNc4f4fNf"
    bank contents:

c    bufptr[ 0] - histogram format (3-normal, 13-"bin-contents")

i    bufptr[ 1] - nbinx
f    bufptr[ 2] - xmin
f    bufptr[ 3] - xmax
f    bufptr[ 4] - xunderflow
f    bufptr[ 5] - xoverflow

i    bufptr[ 6] - nbiny
f    bufptr[ 7] - ymin
f    bufptr[ 8] - ymax
f    bufptr[ 9] - yunderflow
f    bufptr[10] - yoverflow

i    bufptr[11] - entries

Nc    bufptr[12] - title length (words)
    bufptr[13] - bufptr[xx] - title

(f)    bufptr[xx+1] - bufptr[yy] - data

 */

int
Hbook::hist2evio(int id, long *bufptr)
{
  GET_PUT_INIT;
  int ibinx, ibiny, ind, len, ncol, nbins, packed;
  float *buf;

  /*printf("hist=0x%08x\n",hist);*/

  if(id <=0 || id >=NHIST)
  {
    printf("hist2evio: ERROR id=%d, must be from 1 to %d\n",id,NHIST);
    return(0);
  }

  len = 0;
  ncol = 1;

  /*printf("0 %d %d\n",id,ncol);*/

  ind = 0; /* was if( (ind = etNcreate(bufptr,"HISF",id,ncol,nrow)) > 0) - for now, put histogram bank from the beginning of bufptr[] */
  {
    b08out = (unsigned char *)&bufptr[ind];

    PUT32(NORMAL_F);
	PUT32(hist[id].id);
	PUT32(hist[id].entries);

    PUT32(hist[id].ntitle);
    for(int i=0; i<hist[id].ntitle; i++) PUT8(hist[id].title[i]);

    PUTF32(hist[id].xmin);
	PUTF32(hist[id].xmax);
    PUTF32(hist[id].xunderflow);
	PUTF32(hist[id].xoverflow);
    PUT32(hist[id].nbinx);

    if(hist[id].nbiny == 0) /* 1-dim */
    {

      for(ibinx=0; ibinx<hist[id].nbinx; ibinx++)
      {
        PUTF32(hist[id].buf[ibinx]);
        hist[id].buf[ibinx] = 0.0;
      }
    }
    else                    /* 2-dim */ /* NEED CHECK !!! */
    {
	  PUTF32(hist[id].ymin);
	  PUTF32(hist[id].ymax);
	  PUTF32(hist[id].yunderflow);
	  PUTF32(hist[id].yoverflow);
	  PUT32(hist[id].nbinx * hist[id].nbiny);
      for(ibinx=0; ibinx<hist[id].nbinx; ibinx++)
      {
        for(ibiny=0; ibiny<hist[id].nbiny; ibiny++)
        { 
          PUTF32(hist[id].buf2[ibinx][ibiny]);
          hist[id].buf2[ibinx][ibiny] = 0.0;
        }
      }
    }
  }

  /*printf("uth2bos: id=%d ncol=%d nrow=%d entries=%d\n",
  id,ncol,nrow,hist[id].entries);*/
  hist[id].entries = 0;
  hist[id].xunderflow = 0.0;
  hist[id].xoverflow = 0.0;

  return(len);
}


/* restore histograms from BOS banks; see also CMgetHISF() in root */
/* NEED CHECK !!! */

int
Hbook::evio2hist(int id, long *bufptr)
{
  GET_PUT_INIT;
  int i, ind, nw2, nch, ibinx, ibiny, nbinxy, packed;

  if(id <=0 || id >=NHIST)
  {
    printf("evio2hist: ERROR id=%d, must be from 1 to %d\n",id,NHIST);
    return(0);
  }

#if 0
  if((ind=etNlink(bufptr,"HISF",id)) <= 0)
  {
    /*printf("uth2bos: HISF [%d] not found\n",id);*/
    return(0);
  }
#endif
  /*printf("uth2bos: HISF [%d] found\n",id);*/

  /*printf("uth2bos: HISF [%d] 1\n",id);fflush(stdout);*/
  if(hist[id].nbinx > 0 || hist[id].nbiny > 0)
  {
    /*printf("uth2bos: WARN id=%d already exist -  replace it\n",id);*/
    free(hist[id].title);
    if(hist[id].nbiny == 0)
    {
      free(hist[id].buf);
    }
    else
    {
      for(i=0; i<hist[id].nbinx; i++) free(hist[id].buf2[i]);
      free(hist[id].buf2);
    }
    hist[id].nbinx = 0;
  }

#if 0 /*ERROR: get it from bank itself !!!???  */
  nw2 = etNcol(bufptr,ind)*etNrow(bufptr,ind);
#endif
  ind = 0;



  /*printf("uth2bos: nw2=%d (ncol=%d nrow=%d)\n",
    nw2,etNcol(bufptr,ind),etNrow(bufptr,ind));fflush(stdout);*/

  b08 = (unsigned char *)&bufptr[ind];

  GET32(packed);
  GET32(hist[id].id);
  GET32(hist[id].entries);

  GET32(hist[id].ntitle);
  nch = hist[id].ntitle + 1;
  hist[id].title = (char *) malloc(nch);
  for(int i=0; i<nch-1; i++) GET8(hist[id].title[i]);
  hist[id].title[nch-1] = '\0';

  GETF32(hist[id].xmin);
  GETF32(hist[id].xmax);
  GETF32(hist[id].xunderflow);
  GETF32(hist[id].xoverflow);
  GET32(hist[id].nbinx);
  hist[id].dx = (hist[id].xmax - hist[id].xmin)/(float)hist[id].nbinx;

  if(packed == NORMAL_F)
  {
    if(hist[id].nbiny == 0) /* 1-dim */
    {
      hist[id].buf = (float *) calloc(hist[id].nbinx,sizeof(int));
      for(ibinx=0; ibinx<hist[id].nbinx; ibinx++)
      {
        GETF32(hist[id].buf[ibinx]);
      }
    }
    else                    /* 2-dim */ /* NEED CHECK !!! */
    {
      GETF32(hist[id].ymin);
      GETF32(hist[id].ymax);
      GETF32(hist[id].yunderflow);
      GETF32(hist[id].yoverflow);

      GET32(nbinxy);
      hist[id].nbiny = nbinxy / hist[id].nbinx;

      hist[id].dy = (hist[id].ymax - hist[id].ymin)/(float)hist[id].nbiny;

      hist[id].buf2 = (float **) calloc(hist[id].nbinx,sizeof(int));
      for(i=0; i<hist[id].nbinx; i++)
        hist[id].buf2[i] = (float *) calloc(hist[id].nbiny,sizeof(int));
      for(ibinx=0; ibinx<hist[id].nbinx; ibinx++)
      {
        for(ibiny=0; ibiny<hist[id].nbiny; ibiny++)
        {
          GETF32(hist[id].buf2[ibinx][ibiny]);
        }
      }
    }
  }
  else
  {
    printf("evio2hist: PACKED format for HISF is not supported yet\n");
    return(0);
  }

  return(id);
}




/***************************************/
/* send/receive histograms as messages */

int
Hbook::hist2ipc(int id, char *myname)
{
  int ibinx, ibiny, ind, len, ncol, nbins, packed;
  float *buf;

  /*printf("hist2ipc: hist=0x%08x\n",hist);*/

  if(id <=0 || id >=NHIST)
  {
    printf("hist2evio: ERROR id=%d, must be from 1 to %d\n",id,NHIST);
    return(0);
  }

  len = 0;
  ncol = 1;

  /*printf("hist2ipc: %d %d\n",id,ncol);*/

  IpcServer &server = IpcServer::Instance();
  server << clrm;

  std::string str = "hist:";
  str = str+myname;
  server << clrm << str;

  server << (int)NORMAL_F;
  server << hist[id].id;
  server << hist[id].entries;

  server << hist[id].ntitle;
  server << hist[id].title;

  server << hist[id].nbinx;
  server << hist[id].xmin;
  server << hist[id].xmax;
  server << hist[id].xunderflow;
  server << hist[id].xoverflow;
  server << hist[id].nbiny;

  if(hist[id].nbiny == 0) /* 1-dim */
  {
    for(ibinx=0; ibinx<hist[id].nbinx; ibinx++)
    {
      server << hist[id].buf[ibinx];
      hist[id].buf[ibinx] = 0.0;
    }
  }
  else                    /* 2-dim */ /* NEED CHECK !!! */
  {
	server << hist[id].ymin;
	server << hist[id].ymax;
	server << hist[id].yunderflow;
	server << hist[id].yoverflow;
    for(ibinx=0; ibinx<hist[id].nbinx; ibinx++)
    {
      for(ibiny=0; ibiny<hist[id].nbiny; ibiny++)
      { 
        server << hist[id].buf2[ibinx][ibiny];
        hist[id].buf2[ibinx][ibiny] = 0.0;
      }
    }
  }

  server << endm;


  /*printf("uth2bos: id=%d ncol=%d entries=%d\n",
  id,ncol,hist[id].entries);*/
  hist[id].entries = 0;
  hist[id].xunderflow = 0.0;
  hist[id].xoverflow = 0.0;

  return(len);
}
