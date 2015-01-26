/* blaster.c

  ROC:
blaster "129.57.68.21",1234,65536
  clon10:
blastee 1234 65536
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/file.h>

#ifndef Linux
#include <sys/lock.h>
#endif

#include <netinet/tcp.h>

#include "blaster.h"

int blasterStop;
extern int errno;

main(int argc, char *argv[])
{
  struct sockaddr_in sin;
  int s, ix = 0;
  char *buffer;
  int on = 1;
  int y, optval;
  int sendsize;
  int sockbufsize;
  struct hostent *hp, *gethostbyname ();

  struct in_addr **pptr;
  struct sockaddr_in  servaddr;
  char txt[INET_ADDRSTRLEN];

  if(argc != 4)
  {
	printf ("usage: %s targetname port sockbufsize\n", argv [0]);
	exit(1);
  }

  bzero ((char *)&sin, sizeof (sin));

  s = socket (AF_INET, SOCK_STREAM, 0);
  if(s < 0) printf ("cannot open socket\n");

  hp = gethostbyname (argv[1]);

  /*some printfs*/
  printf(">>> hostname >%s<\n",hp->h_name);
  pptr = (struct in_addr **) hp->h_addr_list;
  for( ; *pptr != NULL; pptr++)
  {

    memcpy(&servaddr.sin_addr, *pptr, sizeof(struct in_addr));
    printf(">>> sin_port=%u\n",servaddr.sin_port);
    printf(">>> sin_addr=%u\n",servaddr.sin_addr);
    inet_ntop(AF_INET,*pptr,txt,INET_ADDRSTRLEN);
    printf(">>> txt >%s<\n",txt);

	/*

	 */

	/*
    if ((err = connect(sockfd, (SA *) &servaddr, sizeof(servaddr))) < 0) {
      if (debug) fprintf(stderr, "tcp_connect: error attempting to connect to server\n");
    }
    else {
      fprintf(stderr, "tcp_connect: connected to server\n");
      break;
    }
	*/
  }






  if (hp == 0 && (sin.sin_addr.s_addr = inet_addr (argv [1])) == -1)
  {
	fprintf (stderr, "%s: unkown host\n", argv [1]);
	exit(2);
  }

  if(hp != 0) bcopy (hp->h_addr, &sin.sin_addr, hp->h_length);

  sin.sin_port = htons (atoi (argv [2]));
  sockbufsize  = atoi (argv [3]);

  sin.sin_family 	= AF_INET;

  sendsize = SENDSIZE;
  printf("sendsize=%d\n",sendsize);

  if((buffer = (char *) malloc (sendsize)) == NULL)
  {
	printf ("cannot allocate buffer of sendsize %d\n", sendsize);
	exit (1);
  }

  if(setsockopt(s, SOL_SOCKET, SO_SNDBUF, &sockbufsize, sizeof(sockbufsize))<0)
  {
	printf("setsockopt SO_SNDBUF failed\n");
    free(buffer);
	exit(1);
  }

  {
    int nbytes, lbytes;

    nbytes = 0;
    lbytes = 4;

    getsockopt(s, SOL_SOCKET, SO_SNDBUF, (int *) &nbytes, &lbytes); 
    printf("socket buffer size is %d(0x%08x) bytes\n",nbytes,nbytes);
  }

  /*
  optval = 0;
  if (setsockopt (s, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof (optval)) < 0)
  {
	printf("setsockopt TCP_NODELAY failed\n");
	free(buffer);
	exit(1);
  }
  */

  /*
  if (setsockopt (s, SOL_SOCKET, SO_SND_COPYAVOID, &on, sizeof (on)) < 0)
  {
	printf ("setsockopt SO_RCVBUF failed\n");
	free (buffer);
	exit (1);
  }
  */

  if(connect (s, (const struct sockaddr *)&sin, sizeof (sin)) < 0)
  {
	perror("connect");
    printf("connect failed: host %s port %d\n", inet_ntoa (sin.sin_addr),
	       ntohs (sin.sin_port));

	free(buffer);
	exit(1);
  }
    
  printf("connected and preparing to send...\n");
    
  blasterStop = 0;

  /*if((y = send(s, buffer, sendsize, 0)) < 0)*/
  if((y = write(s, buffer, sendsize)) < 0)
  {
	perror ("blaster write error: ");
  }


  {
    int nbytes, lbytes;

    nbytes = 0;
    lbytes = 4;

    getsockopt(s, SOL_SOCKET, SO_SNDBUF, (int *) &nbytes, &lbytes); 
    printf("socket buffer size is %d(0x%08x) bytes\n",nbytes,nbytes);
  }

  for (;;)
  {
	if(blasterStop == 1) break;

    if ((y = send(s, buffer, sendsize, 0)) < 0)
	/*if ((y = write(s, buffer, sendsize)) < 0)*/
    {
      perror ("blaster write error: ");
      blasterStop = 1;
      break;
    }
    else
    {
      /*printf("sent %d bytes\n",y)*/;
    }
  }
    
  close (s);
    
  free (buffer);
  printf ("blaster exit.\n");
}
