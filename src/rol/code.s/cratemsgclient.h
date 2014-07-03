//
// cratemsgclient.h: interface for the CrateMsgClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CRATEMSGCLIENT_INCLUDED_)
#define CRATEMSGCLIENT_INCLUDED_

#include <stdio.h>
#include <string.h>

typedef int BOOL;
#define TRUE 1
#define FALSE 0















#define DEBUG_NOCONNECTION				0
#define DEBUG_PRINT						0

#define CRATEMSG_LISTEN_PORT			6102
#define MAX_MSG_SIZE						10000

#define CRATEMSG_HDR_ID					0x12345678

#define CRATEMSG_TYPE_READ16			0x01
#define CRATEMSG_TYPE_WRITE16			0x02
#define CRATEMSG_TYPE_READ32			0x03
#define CRATEMSG_TYPE_WRITE32			0x04
#define CRATEMSG_TYPE_DELAY			    0x05
/*#define CRATEMSG_TYPE_READSCALERS	0x06*/


#define SCALER_SERVER_READ_BOARD	    0x100
#define SCALER_SERVER_READ_CHANNEL	    0x101
#define SCALER_SERVER_START             0x111
#define SCALER_SERVER_STOP              0x112
#define SCALER_SERVER_INTERVAL          0x113
#define SCALER_SERVER_MAP               0x114



#define CMD_RSP(cmd)					(cmd | 0x80000000)

#define LSWAP(v)						( ((v>>24) & 0x000000FF) |\
                					      ((v<<24) & 0xFF000000) |\
                					      ((v>>8)  & 0x0000FF00) |\
                					      ((v<<8)  & 0x00FF0000) )

#define HSWAP(v)						( ((v>>8) & 0x00FF) |\
                					      ((v<<8) & 0xFF00) )

/*****************************************************/
/*********** Some Board Generic Commands *************/
/*****************************************************/

#define CRATE_MSG_FLAGS_NOADRINC		0x0
#define CRATE_MSG_FLAGS_ADRINC			0x1
#define CRATE_MSG_FLAGS_USEDMA			0x2








typedef struct
{
	int cnt;
	int addr;
	int flags;
} Cmd_Read16;

typedef struct
{
	int cnt;
	short vals[1];
} Cmd_Read16_Rsp;

typedef struct
{
	int cnt;
	int addr;
	int flags;
	short vals[1];
} Cmd_Write16;

typedef struct
{
	int cnt;
	int addr;
	int flags;
} Cmd_Read32;

typedef struct
{
	int cnt;
	int vals[1];
} Cmd_Read32_Rsp;

typedef struct
{
	int cnt;
	int addr;
	int flags;
	int vals[1];
} Cmd_Write32;

typedef struct
{
	int ms;
} Cmd_Delay;


/*
typedef struct
{
	int cnt;
	unsigned int vals[1];
} Cmd_ReadScalers_Rsp;
*/
typedef struct
{
  int cnt;
  int slot;
} Cmd_ReadScalers;

typedef struct
{
  int cnt;
  unsigned int vals[1];
} Cmd_ReadScalers_Rsp;






typedef struct
{
	int len;
	int type;
  
	union
	{
		Cmd_Read16				m_Cmd_Read16;
		Cmd_Read16_Rsp			m_Cmd_Read16_Rsp;
		Cmd_Write16				m_Cmd_Write16;
		Cmd_Read32				m_Cmd_Read32;
		Cmd_Read32_Rsp			m_Cmd_Read32_Rsp;
		Cmd_Write32				m_Cmd_Write32;
		Cmd_Delay				m_Cmd_Delay;
		Cmd_ReadScalers  		m_Cmd_ReadScalers;
		Cmd_ReadScalers_Rsp		m_Cmd_ReadScalers_Rsp;
		unsigned char			m_raw[MAX_MSG_SIZE];
	} msg;
} CrateMsgStruct;






/* ROOT stuff - temporary !!! */
#define kDefault 0
#define kFALSE 0
#define kTRUE 1




class CrateMsgClient  
{

public:
  char hostname[256];
  int hostport;

  CrateMsgClient(const char *pHost, int port);
  ~CrateMsgClient();

  bool  IsValid();
  void  Close(char *opt = "");
  /*const char* GetUrl() const;*/
  int   SendRaw(const void* buffer, int length/*, ESendRecvOptions opt = kDefault*/);
  int   RecvRaw(void* buffer, int length/*, ESendRecvOptions opt = kDefault*/);
  bool  InitConnection();
  bool  Reconnect();
  bool  CheckConnection(const char *fcn_name);
  bool  RcvRsp(int type);
  bool  Write16(unsigned int addr, unsigned short *val, int cnt = 1, int flags = CRATE_MSG_FLAGS_ADRINC);
  bool  Read16(unsigned int addr, unsigned short *val, int cnt = 1, int flags = CRATE_MSG_FLAGS_ADRINC);
  bool  Write32(unsigned int addr, unsigned int *val, int cnt = 1, int flags = CRATE_MSG_FLAGS_ADRINC);
  bool  Read32(unsigned int addr, unsigned int *val, int cnt = 1, int flags = CRATE_MSG_FLAGS_ADRINC);
  bool  ReadScalers(int slot, unsigned int **val, int *len);
  bool  Delay(unsigned int ms);

  bool  Connect(char *targetname, int port);
  bool  Disconnect();
  bool  DisconnectOnError();
  void  DebugMsg(const char *msg);


private:
  int				swap;
  CrateMsgStruct	Msg;

  /*TSocket			*pSocket;*/
  int sFd;
  bool m_bConnected;

};

#endif // !defined(CRATEMSGCLIENT_INCLUDED_)
