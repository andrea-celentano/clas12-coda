
/* blastee.c */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>

#include <sys/file.h>

#ifndef Linux
#include <sys/lock.h>
#endif

#include <netinet/tcp.h>

#include <sys/ioctl.h>
#include <sys/errno.h>

#include "blaster.h"

unsigned long long blastNum, messages, totalMessages;
unsigned long long nreads, mess_num, mess_size;
int errno;

int	blasteeStop;
#define	FALSE	0
#define TRUE	1

void blastRate(int);

main(int argc, char *argv[])
{
  struct sockaddr_in sin, from;
  char *buffer;
  int f, s, len, on=1;
  int sendsize;
  int sockbufsize;

  if(argc != 3)
  {
	printf("usage: %s port sockbufsize\n", argv[0]);
	exit(1);
  }

  sockbufsize = atoi(argv[2]);

  sendsize = SENDSIZE;
  printf("sendsize=%d\n",sendsize);

  buffer = (char *) malloc(sendsize);
  if(buffer == NULL)
  {
	printf ("cannot allocate buffer of sendsize %d\n", sendsize);
	exit(1);
  }

  signal (SIGALRM, blastRate);
  alarm (10);

  bzero((char *)&sin, sizeof(sin));
  bzero((char *)&from, sizeof(from));

  f = socket (DOMAIN, SOCK_STREAM, 0);

  if(f < 0)
  {
	printf("cannot open socket\n");
	free(buffer);
	exit(1);
  }


  sin.sin_family = DOMAIN;

  sin.sin_port = htons (atoi (argv [1]));

  if (bind (f, (const struct sockaddr *)&sin, sizeof(sin)) < 0)
  {
    printf ("bind error\n");
	free (buffer);
	exit (1);
  }

  if (listen (f, 0) < 0)
  {
    printf ("listen failed\n");
	free (buffer);
	exit (1);
  }

  len = sizeof(from);

  while ((s = accept (f, (struct sockaddr *)&from, &len))  == -1) ;

  blastNum = 0;
  blasteeStop = 0;



  printf("trying to set 'snd' socket buffer size to %d(0x%08x) bytes\n",
    sockbufsize,sockbufsize);
  if (setsockopt (s, SOL_SOCKET, SO_SNDBUF, &sockbufsize, sizeof(sockbufsize)) < 0)
  {
	printf("setsockopt SO_SNDBUF failed\n");
	free(buffer);
	exit(1);
  }



  printf("trying to set 'rcv' socket buffer size to %d(0x%08x) bytes\n",
    sockbufsize,sockbufsize);

  /*sergey: kills performance !!!!!!!!!!!!!!!!!!!!!!!!*/

  if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &sockbufsize, sizeof(sockbufsize)) < 0)
  {
	printf("setsockopt SO_RCVBUF failed\n");
	free(buffer);
	exit(1);
  }


  {
    int nbytes, lbytes;

    nbytes = 0;
    lbytes = 4;

    getsockopt(s, SOL_SOCKET, SO_RCVBUF, (int *) &nbytes, &lbytes); 
    printf("actual 'rcv' socket buffer size is %d(0x%08x) bytes\n",
      nbytes,nbytes);
  }



  /*sergey: does not needed for 10G
  {
    int optval = 1;
    if (setsockopt (s, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof (optval)) < 0)
    {
	  printf("setsockopt TCP_NODELAY failed\n");
	  free(buffer);
	  exit(1);
    }
  }
  */




  /*        
  if (setsockopt (s, SOL_SOCKET, SO_RCVLOWAT, &size, sizeof (on)) < 0)
  {
	printf ("setsockopt SO_RCVBUF failed\n");
	free (buffer);
	exit (1);
  }

  if (setsockopt (s, SOL_SOCKET, SO_SND_COPYAVOID, &on, sizeof (on)) < 0)
  {
	printf ("setsockopt SO_RCVBUF failed\n");
	free (buffer);
	exit (1);
  }
  */

  while(1)
  {
	int numRead, rembytes;

	if(blasteeStop == 1) break;

    rembytes = sendsize;
    while(rembytes>0)
    {
	  if((numRead = read(s, buffer, rembytes)) < 0)
      {
		if(errno == EINTR)
        {
          /*printf("EINTR: try again\n");*/
          continue;
        }
	    printf("blastee read error - break\n");
	    blasteeStop = 1;
        break;
	  }
      rembytes -= numRead;
      mess_num ++;
      mess_size += numRead;

      /*if(rembytes>0) printf("numRead=%d rembytes=%d\n",numRead,rembytes);*/
      if(numRead>0) blastNum += numRead;
	}
    nreads ++;
    messages++;
    /*printf("read() is done, rembytes=%d\n",rembytes);*/
    if(rembytes!=0) printf("ERROR: read() is done, but rembytes=%d\n",rembytes);
  }

  close(f);
  close(s);
    
  free(buffer);
  printf("blastee end.\n");
}


/* */

void
blastRate(int dummy)
{
  if(blastNum > 0)
  {
	totalMessages +=messages;
    if(nreads>0 && mess_num>0)
    {
	  printf ("%llu bytes/sec tot %llu, %llu mess/sec tot %llu (<num>=%llu size=%llu)\n",
        blastNum/10, blastNum, messages/10, totalMessages,
        mess_num/nreads, mess_size/mess_num);
    }
    else
    {
      printf("0 nreads=%d mess_num=%d\n",nreads,mess_num);
    }
	blastNum = messages = 0;
    mess_size = mess_num = nreads = 0;
  }
  else
  {
	printf ("No bytes read in the last 10 seconds.\n");
  }

  signal (SIGALRM, blastRate);
  alarm (10);
}
