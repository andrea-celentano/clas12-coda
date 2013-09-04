
/* vmeserver.c - tcp server for trigger-like clients */


#ifdef VXWORKS

#include <vxWorks.h>
#include <sockLib.h>
#include <inetLib.h>
#include <taskLib.h>
#include <stdioLib.h>
#include <strLib.h>
#include <ioLib.h>
#include <sys/types.h>
#include <sys/socket.h>

#define EIEIO    __asm__ volatile ("eieio")
#define SYNC     __asm__ volatile ("sync")

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

#ifdef Linux
#include <linux/prctl.h>
#endif

#define EIEIO 
#define SYNC 

#endif

#include "libtcp.h" 
#include "libdb.h" 

#include "V1495VMERemote.h"








/* scaler stuff - dummy for now */
#define MAXSCALERS 8
static int vmeScalersReadInterval = 0;
#ifdef VXWORKS
static SEM_ID vmescalers_lock;
#else
static pthread_mutex_t vmescalers_lock;
#endif
static unsigned int  vmescalersnum = 0;
static unsigned int  vmescalersbegin[MAXSCALERS];   /*VME address of the first scaler (by switches on board) */
static unsigned int  vmescalersend[MAXSCALERS];     /*last scaler, the same address space */
static unsigned int  vmescalerslen[MAXSCALERS];     /*scalers space (the number of bytes) */
static unsigned int  vmescalersaddr[MAXSCALERS];    /*actual VME address (after sysBusToLocalAdrs()) */
static unsigned int  vmescalersenable[MAXSCALERS];  /*actual VME address (after sysBusToLocalAdrs()) */
static unsigned int  vmescalersdisable[MAXSCALERS]; /*actual VME address (after sysBusToLocalAdrs()) */
static unsigned int *vmescalers[MAXSCALERS];        /*scalers memory space address*/









#ifndef VXWORKS
char localname[128];
#endif




#define DEBUG_REMOTE_CMD	0


#define SERVER_WORK_PRIORITY 250  /* priority of server's work task (<ROC !)*/ 
#define SERVER_STACK_SIZE 100000  /* stack size of server's work task */ 
#define SERVER_MAX_CONNECTIONS 4  /* max clients connected at a time */


/* readiness flag */
static int request_in_progress;

/* task flag */
static int iTaskTCP;

/*defined in libtcp.h
typedef struct twork
{
  int newFd;
  char *address;
  unsigned short port;
} TWORK;
*/

/* function declarations */ 
char *targetName(); /* from roc_component.c */ 
void vmetcpServerWorkTask(TWORK *targ); 
int vmetcpServer(void);

#ifdef VXWORKS
int sysBusToLocalAdrs(int, char *, char **);
#else
int vmeBusToLocalAdrs(int, char *, char **);
#endif


/**************************************************************************** 
* * tcpServerWorkTask - process client requests 
* * This routine reads from the server's socket, and processes client 
* requests. If the client requests a reply message, this routine 
* will send a reply to the client. 
* * RETURNS: N/A. */ 

/*int gClientSocket;*/
RemoteMsgStruct OutgoingMsg;

/*int gListenSocket;*/
RemoteMsgStruct IncomingMsg;

int
vmeProcessRemoteMsg(RemoteMsgStruct *pRemoteMsgStruct)
{
  int readfrommemory;

#if DEBUG_REMOTE_CMD
  printf("Message: Type=0x%08X, Len=%u, Board=%u\n", pRemoteMsgStruct->MsgType, pRemoteMsgStruct->Length, pRemoteMsgStruct->BoardType);
#endif

  switch(pRemoteMsgStruct->MsgType)
  {
	case REMOTE_CMD_READ16:
	{
	  Cmd_Read16 *pCmd_Read16 = (Cmd_Read16 *)pRemoteMsgStruct->Msg;
	  Cmd_Read16_Rsp *pCmd_Read16_Rsp = (Cmd_Read16_Rsp *)OutgoingMsg.Msg;
	  unsigned short *pData;

#ifdef VXWORKS
	  sysBusToLocalAdrs(0x39, ntohl(pCmd_Read16->Address) & 0xFFFFFF, (unsigned long *)&pData);
#else
	  vmeBusToLocalAdrs(0x39, ntohl(pCmd_Read16->Address) & 0xFFFFFF, (unsigned long *)&pData);
#endif

	  OutgoingMsg.BoardType = pRemoteMsgStruct->BoardType;
	  OutgoingMsg.Length = REMOTE_MSG_SIZE(Cmd_Read16_Rsp)-4;
	  OutgoingMsg.MsgType = CMD_RSP(REMOTE_CMD_READ16);

	  pCmd_Read16_Rsp->Value = htons(vmeRead16(pData));

#if DEBUG_REMOTE_CMD
	  printf("Cmd_Read16: Address = 0x%08x (0x%08x), Data = 0x%04x\n", ntohl(pCmd_Read16->Address), pData, ntohs(pCmd_Read16_Rsp->Value));
#endif
	  return(OutgoingMsg.Length + 4);
	}

	case REMOTE_CMD_WRITE16:
	{
	  Cmd_Write16 *pCmd_Write16 = (Cmd_Write16 *)pRemoteMsgStruct->Msg;
	  unsigned short *pData;

#ifdef VXWORKS
	  sysBusToLocalAdrs(0x39, ntohl(pCmd_Write16->Address) & 0xFFFFFF, (unsigned long *)&pData);
#else
	  vmeBusToLocalAdrs(0x39, ntohl(pCmd_Write16->Address) & 0xFFFFFF, (unsigned long *)&pData);
#endif

	  vmeWrite16(pData, ntohs(pCmd_Write16->Value));
#if DEBUG_REMOTE_CMD
	  printf("Cmd_Write16: Address = 0x%08X (0x%08x), Data = 0x%04hX\n", ntohl(pCmd_Write16->Address), pData, ntohs(pCmd_Write16->Value));
#endif
	  break;
	}

	case REMOTE_CMD_BLKREAD16:
	{
	  Cmd_BlkRead16 *pCmd_BlkRead16 = (Cmd_BlkRead16 *)pRemoteMsgStruct->Msg;
	  Cmd_BlkRead16_Rsp *pCmd_BlkRead16_Rsp = (Cmd_BlkRead16_Rsp *)OutgoingMsg.Msg;
	  volatile unsigned short *pData;
      unsigned short *sData;
	  int i;

#ifdef VXWORKS
	  sysBusToLocalAdrs(0x39, ntohl(pCmd_BlkRead16->Address) & 0xFFFFFF, (unsigned long *)&pData);
#else
	  vmeBusToLocalAdrs(0x39, ntohl(pCmd_BlkRead16->Address) & 0xFFFFFF, (unsigned long *)&pData);
#endif

#if DEBUG_REMOTE_CMD
	  printf("Cmd_BlkRead16: Address = 0x%08x (0x%08x), count=%d\n", ntohl(pCmd_BlkRead16->Address), pData, ntohl(pCmd_BlkRead16->Count));
#endif

	  OutgoingMsg.BoardType = pRemoteMsgStruct->BoardType;
	  OutgoingMsg.Length = REMOTE_MSG_SIZE(Cmd_BlkRead16_Rsp)-sizeof(Cmd_BlkRead16_Rsp)
                              + ntohl(pCmd_BlkRead16->Count) * sizeof(unsigned short) - 4;
	  OutgoingMsg.MsgType = CMD_RSP(REMOTE_CMD_BLKREAD16);

	  if( ntohl(pCmd_BlkRead16->NoAddressInc)) /* fifo-style readout (scope feature) */
	  {
		for(i=0; i<ntohl(pCmd_BlkRead16->Count); i++)
		{
		  pCmd_BlkRead16_Rsp->Values[i] = htons(vmeRead16( &pData[0]));
#if DEBUG_REMOTE_CMD
		  printf("{0x%08X => 0x%08X}\n", ntohl(pCmd_BlkRead16->Address) + i*4, ntohs(pCmd_BlkRead16_Rsp->Values[i]));
#endif
		}
	  }
	  else /* array readout */
	  {
        /* check if specified space is scaler's space; if so, read scaler's image
        from memory space instead - if interval is NOT zero !! */
        readfrommemory = 0;
        if(vmeScalersReadInterval>0)
        {
          for(i=0; i<vmescalersnum; i++)
          {
            if( (ntohl(pCmd_BlkRead16->Address) == vmescalersbegin[i])  && ((ntohl(pCmd_BlkRead16->Count)/4) == vmescalerslen[i]) )
            {
              sData = (unsigned int *)(vmescalers[i]);
#if DEBUG_REMOTE_CMD
              printf("found 0x%08x %6d -> read from memory 0x%08x\n",vmescalersbegin[i],vmescalerslen[i],pData);
#endif
              for(i=0; i<ntohl(pCmd_BlkRead16->Count); i++) pCmd_BlkRead16_Rsp->Values[i] = htons(sData[i]);
              readfrommemory = 1;
              break;
            }
          }
        }

        /* read from VME space */
        if(readfrommemory==0)
        {
#if DEBUG_REMOTE_CMD
          printf("normal blk16 from 0x%08x, len=%d\n",pData,pCmd_BlkRead16->Count);
#endif
          for(i = 0; i<ntohl(pCmd_BlkRead16->Count); i++)
		  {
		    pCmd_BlkRead16_Rsp->Values[i] = htons(vmeRead16(&pData[i]));
#if DEBUG_REMOTE_CMD
		    printf("{0x%08X => 0x%08X}\n", ntohl(pCmd_BlkRead16->Address) + i*4, ntohs(pCmd_BlkRead16_Rsp->Values[i]));
#endif
		  }
	    }

	  }

	  return(OutgoingMsg.Length + 4); 
	}


	case REMOTE_CMD_READ32:
	{
	  Cmd_Read32 *pCmd_Read32 = (Cmd_Read32 *)pRemoteMsgStruct->Msg;
	  Cmd_Read32_Rsp *pCmd_Read32_Rsp = (Cmd_Read32_Rsp *)OutgoingMsg.Msg;
	  unsigned int *pData;
	  unsigned int *sData;
      int i;

#ifdef VXWORKS
	  sysBusToLocalAdrs(0x39, ntohl(pCmd_Read32->Address) & 0xFFFFFF, (unsigned long *)&pData);
#else
	  vmeBusToLocalAdrs(0x39, ntohl(pCmd_Read32->Address) & 0xFFFFFF, (unsigned long *)&pData);
#endif

	  OutgoingMsg.BoardType = pRemoteMsgStruct->BoardType;
	  OutgoingMsg.Length = REMOTE_MSG_SIZE(Cmd_Read32_Rsp)-4;
	  OutgoingMsg.MsgType = CMD_RSP(REMOTE_CMD_READ32);

      readfrommemory = 0;
      /* check if specified space is scaler's space; if so, read scaler's image
        from memory space instead - if interval is NOT zero !! */
      if(vmeScalersReadInterval>0)
      {
        for(i=0; i<vmescalersnum; i++)
        {
          if( (ntohl(pCmd_Read32->Address) >= vmescalersbegin[i]) && (ntohl(pCmd_Read32->Address) <= vmescalersend[i]) )
          {
            sData = (unsigned int *)(vmescalers[i]);
#if DEBUG_REMOTE_CMD
            printf("found 0x%08x %6d -> read from memory 0x%08x\n",vmescalersbegin[i],vmescalerslen[i],pData);
#endif
            pCmd_Read32_Rsp->Value = htonl(sData[(ntohl(pCmd_Read32->Address)-vmescalersbegin[i])/4]);
            readfrommemory = 1;
            break;
          }
        }
      }

      /* read from VME space */
      if(readfrommemory==0)
      {
#if DEBUG_REMOTE_CMD
        printf("normal read32 from 0x%08x\n",pData);
#endif
        pCmd_Read32_Rsp->Value = htonl(vmeRead32(pData));
      }

#if DEBUG_REMOTE_CMD
	  printf("Cmd_Read32: Address = 0x%08X, Data = 0x%08X\n", ntohl(pCmd_Read32->Address), ntohl(pCmd_Read32_Rsp->Value));
#endif

	  return(OutgoingMsg.Length + 4);
	}

	case REMOTE_CMD_WRITE32:
	{
	  Cmd_Write32 *pCmd_Write32 = (Cmd_Write32 *)pRemoteMsgStruct->Msg;
	  unsigned int *pData;
      int i, flag;

#ifdef VXWORKS
	  sysBusToLocalAdrs(0x39, ntohl(pCmd_Write32->Address) & 0xFFFFFF, (unsigned long *)&pData);
#else
	  vmeBusToLocalAdrs(0x39, ntohl(pCmd_Write32->Address) & 0xFFFFFF, (unsigned long *)&pData);
#endif

	  /*sergey: ignore scalers enable/disable*/
      flag = 0;
      if(vmeScalersReadInterval>0)
      {
        for(i=0; i<vmescalersnum; i++)
        {
          if(pData == vmescalersenable[i])
          {
            flag = 1;
            break;
		  }
        }
      }
      if(flag) break;

	  vmeWrite32( pData, ntohl(pCmd_Write32->Value));

#if DEBUG_REMOTE_CMD
	  printf("Cmd_Write32: Address = 0x%08X, Data = 0x%08X\n", ntohl(pCmd_Write32->Address), ntohl(pCmd_Write32->Value));
#endif
	  break;
	}

	case REMOTE_CMD_BLKREAD32:
	{
	  Cmd_BlkRead32 *pCmd_BlkRead32 = (Cmd_BlkRead32 *)pRemoteMsgStruct->Msg;
	  Cmd_BlkRead32_Rsp *pCmd_BlkRead32_Rsp = (Cmd_BlkRead32_Rsp *)OutgoingMsg.Msg;
	  volatile unsigned int *pData;
      unsigned int *sData;
	  int i;

#ifdef VXWORKS
	  sysBusToLocalAdrs(0x39, ntohl(pCmd_BlkRead32->Address) & 0xFFFFFF, (unsigned long *)&pData);
#else
	  vmeBusToLocalAdrs(0x39, ntohl(pCmd_BlkRead32->Address) & 0xFFFFFF, (unsigned long *)&pData);
#endif

	  OutgoingMsg.BoardType = pRemoteMsgStruct->BoardType;
	  OutgoingMsg.Length = REMOTE_MSG_SIZE(Cmd_BlkRead32_Rsp)-sizeof(Cmd_BlkRead32_Rsp)+ ntohl(pCmd_BlkRead32->Count)*sizeof(unsigned int)-4;
	  OutgoingMsg.MsgType = CMD_RSP(REMOTE_CMD_BLKREAD32);
	  if( ntohl(pCmd_BlkRead32->NoAddressInc)) /* fifo-style readout (scope feature) */
	  {
		for(i=0; i<ntohl(pCmd_BlkRead32->Count); i++)
		{
		  pCmd_BlkRead32_Rsp->Values[i] = htonl(vmeRead32(&pData[0]));
#if DEBUG_REMOTE_CMD
		  printf("{0x%08X => 0x%08X}\n", ntohl(pCmd_BlkRead32->Address) + i*4, ntohl(pCmd_BlkRead32_Rsp->Values[i]));
#endif
		}
	  }
	  else /* array readout */
	  {
        /* check if specified space is scaler's space; if so, read scaler's image
        from memory space instead - if interval is NOT zero !! */
        readfrommemory = 0;
        if(vmeScalersReadInterval>0)
        {
          for(i=0; i<vmescalersnum; i++)
          {
            if( (ntohl(pCmd_BlkRead32->Address) == vmescalersbegin[i])  && ((ntohl(pCmd_BlkRead32->Count)/4) == vmescalerslen[i]) )
            {
              sData = (unsigned int *)(vmescalers[i]);
#if DEBUG_REMOTE_CMD
              printf("found 0x%08x %6d -> read from memory 0x%08x\n",vmescalersbegin[i],vmescalerslen[i],pData);
#endif
              for(i=0; i<ntohl(pCmd_BlkRead32->Count); i++) pCmd_BlkRead32_Rsp->Values[i] = htonl(sData[i]);
              readfrommemory = 1;
              break;
            }
          }
        }

        /* read from VME space */
        if(readfrommemory==0)
        {
#if DEBUG_REMOTE_CMD
          printf("normal blk32 from 0x%08x, len=%d\n",pData,ntohl(pCmd_BlkRead32->Count));
#endif
          for(i=0; i < ntohl(pCmd_BlkRead32->Count); i++)
		  {
		    pCmd_BlkRead32_Rsp->Values[i] = htonl(vmeRead32(&pData[i]));
#if DEBUG_REMOTE_CMD
		    printf("{0x%08X => 0x%08X}\n", ntohl(pCmd_BlkRead32->Address) + i*4, ntohl(pCmd_BlkRead32_Rsp->Values[i]));
#endif
		  }
	    }
	  }

	  return(OutgoingMsg.Length + 4); 
	}

    case REMOTE_CMD_WRITEPAGE: /*for USER FPGA firmware update only*/
      break;

	case REMOTE_CMD_WRITEPAGE_VME: /*for VME FPGA firmware update only*/
	case REMOTE_CMD_BLKWRITE16:
	case REMOTE_CMD_BLKWRITE32:
	default:
	  printf("Unhandled Message Type = 0x%08X\n", pRemoteMsgStruct->MsgType);
	  break;
  }

  return(0);
}


/* startup programs */

#ifdef VXWORKS
int
vmeServer(void)
{
  int iii;

  /* enter infinite loop */
  while(1)
  {
    taskDelay(sysClkRateGet()); /* 1 sec sleep */

    /* check if VMESERVER task exist; if it does, check it health and kill
     if it is suspended */
    iii = taskIdVerify(iTaskTCP);
    /*printf("taskIdVerify returns %d\n",iii);*/
    if(iii==OK) /* check task status */
    {
      /* if task suspended, delete it */
      iii = taskIsSuspended(iTaskTCP); /* returns TRUE if suspended */
  	  if(iii==TRUE)
      {
        printf("INFO(vmeServer): task is suspended, calling taskDelete(%d)\n",iTaskTCP);
        iii = taskDelete(iTaskTCP);
        printf("taskDelete() returns %d\n",iii);
        iTaskTCP = 0;
        taskDelay(5*sysClkRateGet());
	  }
      else
	  {
        /*printf("INFO(vmeServer): task is Ok\n")*/;
	  }
    }
    else
    {
      iTaskTCP = taskSpawn("VMESERVER", 251, 0, 100000, vmetcpServer,
                          0, 0, 0,0,0,0,0,0,0,0);
      printf("taskSpawn(\"VMESERVER\") returns %d\n",iTaskTCP);
    }
  }
}
#else
int
vmeServer(char *name)
{
  int id;

  printf("vmeServer called with name >%s<\n",name);

  strcpy(localname,name);

  pthread_attr_t attr;
  pthread_attr_init(&attr); /* initialize attr with default attributes */
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  pthread_create(&id, &attr, vmetcpServer, NULL);
}
#endif


#ifndef VXWORKS
#define TRUE  1
#define FALSE 0
#define OK 0
#define ERROR (-1)
#define STATUS int
#endif



STATUS 
vmetcpServer(void)
{ 
  struct sockaddr_in serverAddr; 
  struct sockaddr_in clientAddr; 
  int sockAddrSize;              /* size of socket address structure */ 
  int gListenSocket;             /* socket file descriptor */ 
  int gClientSocket;             /* socket descriptor from accept */ 
  int ix = 0;                    /* counter for work task names */ 
  int portnum;                   /* desired port number; can be changed if that number in use enc */
  char workName[16];             /* name of work task */ 
  int on = TRUE;  /* non-blocking */
  int off = FALSE; /* blocking */
  int status;
  static TWORK targ;
  MYSQL *dbsock = NULL;
  MYSQL_RES *result;
  int numRows;
  char tmp[1000], temp[100], *myname, *hname;
  int itmp;

  printf("vmetcpServer called\n");

#ifdef Linux
  prctl(PR_SET_NAME,"vmetcpServer");
#endif

  /* some cleanup */
  sockAddrSize = sizeof(struct sockaddr_in); 
  bzero((char *)&serverAddr, sockAddrSize); 
  bzero((char *)&clientAddr, sockAddrSize); 

  portnum = VMESERVER_PORT;

  /* creates an endpoint for communication and returns a socket file descriptor */
  if((gListenSocket = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
  {
    perror("socket"); 
    return(ERROR); 
  } 

  /* set up the local address */ 
  serverAddr.sin_family = AF_INET; 
  serverAddr.sin_port = htons(portnum); 
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* create a TCP-based socket */ 

  /* bind socket to local address */
  itmp = portnum;
  while(bind(gListenSocket, (struct sockaddr *)&serverAddr, sockAddrSize) == ERROR)
  {
    perror("bind");

    /* try another port (just increment on one) */
    portnum ++;
    if((portnum-itmp) > 50)
    {
      close(gListenSocket); 
      return(ERROR);
    }

    serverAddr.sin_port = htons(portnum);
  }
  printf("bind on port %d\n",portnum);

  /* create queue for client connection requests */ 
  if(listen (gListenSocket, SERVER_MAX_CONNECTIONS) == ERROR)
  {
    perror ("listen"); 
    close (gListenSocket); 
    return (ERROR);
  }





  /* update database with port number */
#ifdef VXWORKS
  myname = targetName();
#else
  myname = localname;
#endif
  printf("myname >%s<\n",myname);

  /* update daq database 'Ports' table with port number and host name */
  dbsock = dbConnect(getenv("MYSQL_HOST"), "daq");
  /* trying to select our name from 'Ports' table */
  sprintf(tmp,"SELECT Name FROM Ports WHERE Name='%s'",myname);
  if(mysql_query(dbsock, tmp) != 0)
  {
	printf("mysql error (%s)\n",mysql_error(dbsock));
    return(ERROR);
  }

  /* gets results from previous query */
  /* we assume that numRows=0 if our Name does not exist,
     or numRows=1 if it does exist */
  if( !(result = mysql_store_result(dbsock)) )
  {
    printf("ERROR in mysql_store_result (%)\n",mysql_error(dbsock));
    return(ERROR);
  }
  else
  {
    numRows = mysql_num_rows(result);
    mysql_free_result(result);

    /* NOTE: for VXWORKS 'Host' the same as 'Name' */
#ifdef VXWORKS
    hname = myname; /*assume they are the same for vxworks*/
#else
	hname = getenv("HOST");
#endif

    printf("hname >%s<, nrow=%d\n",hname,numRows);
    if(numRows == 0)
    {
      sprintf(tmp,"INSERT INTO Ports (Name,Host,Trigger_tcp) VALUES ('%s','%s',%d)",
        myname,hname,portnum);
    }
    else if(numRows == 1)
    {
      sprintf(tmp,"UPDATE Ports SET Host='%s',Trigger_tcp=%d WHERE Name='%s'",hname,portnum,myname);
    }
    else
    {
      printf("ERROR: unknown nrow=%d",numRows);
      return(ERROR);
    }

    if(mysql_query(dbsock, tmp) != 0)
    {
	  printf("ERROR\n");
      return(ERROR);
    }
    else
    {
      printf("Query >%s< succeeded\n",tmp);
    }
  }
  dbDisconnect(dbsock);







  request_in_progress = 0;
  /* accept new connect requests and spawn tasks to process them */ 
  while(1)
  {
#ifdef VXWORKS
    /* check for free net buffers */
    usrNetStackSysPoolStatus("tcpServer",0);
    usrNetStackDataPoolStatus("tcpServer",0);
#endif

    /* do not accept new request if current one is not finished yet; too
    many requests may create network buffer shortage */
    if(request_in_progress)
    {
#if DEBUG_REMOTE_CMD
      printf("wait: request in progress\n");
#endif

#ifdef VXWORKS
      taskDelay(100);
#else
      sleep(1);
#endif
      continue;
    }

    if((gClientSocket = accept(gListenSocket, (struct sockaddr *) &clientAddr, &sockAddrSize)) == ERROR)
    {
      perror ("accept"); 
      close (gListenSocket); 
      return (ERROR); 
    }

    sprintf (workName, "vmeWork%d", ix++);
	
    request_in_progress = 1;
    /* spawn with floating point flag VX_FP_TASK, just in case if some code needs it */


    /* fill the structure for thread */
    targ.newFd = gClientSocket;
    targ.address = (int) inet_ntoa(clientAddr.sin_addr);
    targ.port = ntohs (clientAddr.sin_port);


#ifdef VXWORKS
    if(taskSpawn(workName, SERVER_WORK_PRIORITY, VX_FP_TASK/*0*/, SERVER_STACK_SIZE,
				 (FUNCPTR) vmetcpServerWorkTask, &targ, 0, 0, 0, 0, 0, 0, 0, 0, 0)
          == ERROR)
    {
	  perror("taskSpawn"); 
	  close(gClientSocket);
      request_in_progress = 0;
    }
#else
    /*printf("TcpServer: start work thread\n");*/
	{
      int ret;
	  pthread_t id;
      pthread_attr_t detached_attr;

      pthread_attr_init(&detached_attr);
      pthread_attr_setdetachstate(&detached_attr, PTHREAD_CREATE_DETACHED);
      pthread_attr_setscope(&detached_attr, PTHREAD_SCOPE_SYSTEM);
	  /*
      printf("befor: socket=%d address>%s< port=%d\n",
        targ.newFd, targ.address, targ.port); fflush(stdout);
	  */
      /* block annoying IP address(es) */
      if(!strncmp((int) inet_ntoa (clientAddr.sin_addr),"129.57.71.",10))
	  {
        printf("WARN: ignore request from %s\n",targ.address);
        close(targ.newFd);
        request_in_progress = 0;
	  }
      else
	  {
        ret = pthread_create(&id, &detached_attr, vmetcpServerWorkTask, &targ);
        if(ret!=0)
        {
          printf("ERROR: pthread_create(vmetcpServerWorkTask) returned %d\n",
            ret);
          close(targ.newFd);
          request_in_progress = 0;
        }
	  }
	}
#endif	

    /* sleep 1 sec before processing next request; we do not want to process
    too many requests per minute to avoid network buffers shortage */
#ifdef VXWORKS
    taskDelay(sysClkRateGet());
#endif
  }

} 



void
vmetcpServerWorkTask(TWORK *targ) 
{
  STATUS ret;
  int len;  /* number of bytes read */ 
  int size;
  int oldStdErr, oldStdOut;
  int res1,res2,res3,size1,ii1,ii2,ii3;


again:


  if( (len=recv(targ->newFd, (char *)&IncomingMsg, 4, NULL)) == 4)
  {
	IncomingMsg.Length = ntohl(IncomingMsg.Length);
#if DEBUG_REMOTE_CMD
	printf("IncomingMsg.Length: %d(0x%08x)\n",IncomingMsg.Length,IncomingMsg.Length);
#endif

	if(IncomingMsg.Length <= sizeof(RemoteMsgStruct) - 4)
	{
	  if( (len=recv(targ->newFd, (char *)&IncomingMsg.BoardType, IncomingMsg.Length, NULL))
            == IncomingMsg.Length)
	  {

        /************************/
        /* Executing the message */

        if(len)
	    {
		  IncomingMsg.BoardType = ntohl(IncomingMsg.BoardType);
		  IncomingMsg.MsgType = ntohl(IncomingMsg.MsgType);
#if DEBUG_REMOTE_CMD
          printf ("FROM CLIENT (Internet Address %s, port %d, length %d): message 0x%02x 0x%02x ..\n", 
	        targ->address, targ->port, len, IncomingMsg.BoardType, IncomingMsg.MsgType);
#endif
	      size = vmeProcessRemoteMsg(&IncomingMsg);
#if DEBUG_REMOTE_CMD
          printf("size1: size=%d\n",size);
#endif
          if(size > 0)
          {
#ifdef VXWORKS
	        /* save and redirect IO */
            fflush(stderr);
            fflush(stdout);
            /*oldStdErr = ioTaskStdGet(0, STD_ERR);*/
            oldStdOut = ioTaskStdGet(0, STD_OUT);
            /*ioTaskStdSet(0, STD_ERR, targ->newFd);*/
            ioTaskStdSet(0, STD_OUT, targ->newFd);
#endif

			ii1=OutgoingMsg.Length;
            ii2=OutgoingMsg.BoardType;
            ii3=OutgoingMsg.MsgType;

  		    OutgoingMsg.Length = htonl(OutgoingMsg.Length);
  		    OutgoingMsg.BoardType = htonl(OutgoingMsg.BoardType);
		    OutgoingMsg.MsgType = htonl(OutgoingMsg.MsgType);

#ifdef VXWORKS
            size1 = fwrite(&OutgoingMsg, 1, size, stdout);
#else
	        size1 = write(targ->newFd, &OutgoingMsg, size);
#endif


#ifdef VXWORKS
            /* restore IO settings */
            fflush(stderr);
            fflush(stdout);
            /*ioTaskStdSet(0, STD_ERR, oldStdErr);*/
            ioTaskStdSet(0, STD_OUT, oldStdOut);
#endif

#if DEBUG_REMOTE_CMD
			printf("ERROR: res1=%d, res2=%d, res3=%d\n",res1,res2,res3);
			printf("call fwrite: len=%d btype=%d mtype=%d\n",ii1,ii2,ii3);
            printf("fwrite returns %d\n",size1);
#endif
		  }
		  /*
          else goto socket() again ??? 
		  */
#if DEBUG_REMOTE_CMD
          printf("size2: size=%d size1=%d\n",size,size1);
#endif
          goto again;
	    }

#if DEBUG_REMOTE_CMD
        printf("OUT: len=%d\n",len);
#endif

        /************************/
        /************************/

	  }
      else
	  {
        printf("ERROR2: unexpected len=%d\n",len);
	  }
	}
    else
	{
      printf("message too long (%d) - exit\n",IncomingMsg.Length);
	}

    request_in_progress = 0;
  }
  else if(len == ERROR) /* error from recv() */ 
  {
    printf("ERROR in recv\n");
    perror("ERROR (recv)"); 
  }
  else if(len == 0)
  {
    printf("connection closed, exit thread\n");
  }
  else
  {
    printf("ERROR1: unexpected len=%d\n",len);
  }

  ret = close(targ->newFd/*gClientSocket*/);    /* close server socket connection */ 
  if(ret==ERROR) printf("ERROR in close()\n");

#ifdef VXWORKS
  free(targ->address); /* free malloc from inet_ntoa() */
#endif

  /*
  ret = shutdown(gClientSocket,2);
  if(ret==ERROR) printf("ERROR in shutdown()\n");
  */

  request_in_progress = 0;

  return;
}




