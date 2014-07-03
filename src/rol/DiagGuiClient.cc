





class CrateMsgClient
{

public:
	char hostname[256];
	int hostport;

	CrateMsgClient(const char *pHost, int port)
	{
	  strncpy(hostname,pHost,255);
	  hostport = port;

#if DEBUG_NOCONNECTION
		pSocket = NULL;
		return;
#endif
		pSocket = new TSocket(pHost,port,32768);
		pSocket->SetOption(kNoDelay, 1);
      
		if(!pSocket->IsValid())
		{
			printf("Failed to connected to host: %s\n", pHost);
			Close();
			delete pSocket;
			pSocket = NULL;
		}
		else
			printf("Successfully connected to host: %s\n" , pHost);

		InitConnection();
	}

  bool IsValid() 
  {
    if(pSocket==NULL)
      return kFALSE;
    else
      return pSocket->IsValid();
  }

  void Close(char *opt = "")
  {
	if(pSocket)
		pSocket->Close();
  }

  const char* GetUrl() const
    {
      return pSocket->GetUrl();
    }

  int SendRaw(const void* buffer, int length, ESendRecvOptions opt = kDefault)
  {
    return pSocket->SendRaw(buffer,length,opt);
  }
  
  int	RecvRaw(void* buffer, int length, ESendRecvOptions opt = kDefault)
  {
    return pSocket->RecvRaw(buffer,length,opt);
  }

  bool InitConnection()
  {
		int val;

		if(!pSocket || !pSocket->IsValid())
			return kFALSE;

		// send endian test word to server
		val = CRATEMSG_HDR_ID;
		SendRaw(&val, 4);

		if(RecvRaw(&val, 4) != 4)
			return kFALSE;

		if(val == CRATEMSG_HDR_ID)
			swap = 0;
		else if(val == LSWAP(CRATEMSG_HDR_ID))
			swap = 1;
		else
		{
			Close();
			return kFALSE;
		}
		return kTRUE;
  }



	bool Reconnect()
	{
		Close();
		delete pSocket;
		printf("Reconnect... %s %d\n",hostname,hostport);
		pSocket = new TSocket(hostname,hostport,32768);

		return InitConnection();
	}



	bool CheckConnection(const char *fcn_name)
	{
#if DEBUG_NOCONNECTION
		return kFALSE;
#endif
		if(!IsValid())
		{
			printf("Function %s FAILED\n", fcn_name);
			return Reconnect();
		}
		return kTRUE;
	}

	bool RcvRsp(int type)
	{
		if(RecvRaw(&Msg, 8) == 8)
		{
			if(swap)
			{
				Msg.len = LSWAP(Msg.len);
				Msg.type = LSWAP(Msg.type);
			}
			if((Msg.len <= MAX_MSG_SIZE) && (Msg.len >= 0) && (Msg.type == (int)CMD_RSP(type)))
			{
				if(!Msg.len)
					return kTRUE;

				if(RecvRaw(&Msg.msg, Msg.len) == Msg.len)
					return kTRUE;
			}
		}
		Close();
		return kFALSE;
	}

	bool Write16(unsigned int addr, unsigned short *val, int cnt = 1, int flags = CRATE_MSG_FLAGS_ADRINC)
	{
		if(!CheckConnection(__FUNCTION__))
			return kFALSE;

		Msg.len = 12+2*cnt;
		Msg.type = CRATEMSG_TYPE_WRITE16;
		Msg.msg.m_Cmd_Write16.cnt = cnt;
		Msg.msg.m_Cmd_Write16.addr = addr;
		Msg.msg.m_Cmd_Write16.flags = flags;
		for(int i = 0; i < cnt; i++)
			Msg.msg.m_Cmd_Write16.vals[i] = val[i];
		SendRaw(&Msg, Msg.len+8);

#if DEBUG_PRINT
		printf("Write16 @ 0x%08X, Count = %d, Flag = %d, Vals = ", addr, cnt, flags);
		for(int i = 0; i < cnt; i++)
			printf("0x%04hX ", val[i]);
		printf("\n");
#endif

		return kTRUE;
	}

	bool Read16(unsigned int addr, unsigned short *val, int cnt = 1, int flags = CRATE_MSG_FLAGS_ADRINC)
	{
		if(!CheckConnection(__FUNCTION__))
			return kFALSE;

		Msg.len = 12;
		Msg.type = CRATEMSG_TYPE_READ16;
		Msg.msg.m_Cmd_Read16.cnt = cnt;
		Msg.msg.m_Cmd_Read16.addr = addr;
		Msg.msg.m_Cmd_Read16.flags = flags;
		SendRaw(&Msg, Msg.len+8);

#if DEBUG_PRINT
		printf("Read16 @ 0x%08X, Count = %d, Flag = %d, Vals = ", addr, cnt, flags);
#endif

		if(RcvRsp(Msg.type))
		{
			if(swap)
			{
				Msg.msg.m_Cmd_Read16_Rsp.cnt = LSWAP(Msg.msg.m_Cmd_Read16_Rsp.cnt);
				for(int i = 0; i < Msg.msg.m_Cmd_Read16_Rsp.cnt; i++)
					val[i] = HSWAP(Msg.msg.m_Cmd_Read16_Rsp.vals[i]);
			}
			else
			{
				for(int i = 0; i < Msg.msg.m_Cmd_Read16_Rsp.cnt; i++)
					val[i] = Msg.msg.m_Cmd_Read16_Rsp.vals[i];
			}
#if DEBUG_PRINT
		for(int i = 0; i < cnt; i++)
			printf("0x%04hX ", val[i]);
		printf("\n");
#endif
			return kTRUE;
		}
#if DEBUG_PRINT
		printf("failed...\n");
#endif
		return kFALSE;
	}

	bool Write32(unsigned int addr, unsigned int *val, int cnt = 1, int flags = CRATE_MSG_FLAGS_ADRINC)
	{
		if(!CheckConnection(__FUNCTION__))
			return kFALSE;

		Msg.len = 12+4*cnt;
		Msg.type = CRATEMSG_TYPE_WRITE32;
		Msg.msg.m_Cmd_Write32.cnt = cnt;
		Msg.msg.m_Cmd_Write32.addr = addr;
		Msg.msg.m_Cmd_Write32.flags = flags;
		for(int i = 0; i < cnt; i++)
			Msg.msg.m_Cmd_Write32.vals[i] = val[i];
		SendRaw(&Msg, Msg.len+8);

#if DEBUG_PRINT
		printf("Write32 @ 0x%08X, Count = %d, Flag = %d, Vals = ", addr, cnt, flags);
		for(int i = 0; i < cnt; i++)
			printf("0x%08X ", val[i]);
		printf("\n");
#endif

		return kTRUE;
	}

	bool Read32(unsigned int addr, unsigned int *val, int cnt = 1, int flags = CRATE_MSG_FLAGS_ADRINC)
	{
		if(!CheckConnection(__FUNCTION__))
			return kFALSE;

		Msg.len = 12;
		Msg.type = CRATEMSG_TYPE_READ32;
		Msg.msg.m_Cmd_Read16.cnt = cnt;
		Msg.msg.m_Cmd_Read16.addr = addr;
		Msg.msg.m_Cmd_Read16.flags = flags;
		SendRaw(&Msg, Msg.len+8);

#if DEBUG_PRINT
		printf("Read32 @ 0x%08X, Count = %d, Flag = %d, Vals = ", addr, cnt, flags);
#endif

		if(RcvRsp(Msg.type))
		{
			if(swap)
			{
				Msg.msg.m_Cmd_Read32_Rsp.cnt = LSWAP(Msg.msg.m_Cmd_Read32_Rsp.cnt);
				for(int i = 0; i < Msg.msg.m_Cmd_Read32_Rsp.cnt; i++)
					val[i] = LSWAP(Msg.msg.m_Cmd_Read32_Rsp.vals[i]);
			}
			else
			{
				for(int i = 0; i < Msg.msg.m_Cmd_Read32_Rsp.cnt; i++)
					val[i] = Msg.msg.m_Cmd_Read32_Rsp.vals[i];
			}
#if DEBUG_PRINT
		for(int i = 0; i < cnt; i++)
			printf("0x%08X ", val[i]);
		printf("\n");
#endif
			return kTRUE;
		}
#if DEBUG_PRINT
		printf("failed...\n");
#endif
		return kFALSE;
	}

	bool ReadScalers(unsigned int **val, int *len)
	{
		if(!CheckConnection(__FUNCTION__))
			return kFALSE;

		Msg.len = 0;
		Msg.type = CRATEMSG_TYPE_READSCALERS;
		SendRaw(&Msg, Msg.len+8);

		if(RcvRsp(Msg.type))
		{
			if(swap)
				Msg.msg.m_Cmd_Read32_Rsp.cnt = LSWAP(Msg.msg.m_Cmd_Read32_Rsp.cnt);

			*val = new unsigned int[Msg.msg.m_Cmd_Read32_Rsp.cnt];
			if(!(*val))
				return kFALSE;

			if(swap)
			{
				for(int i = 0; i < Msg.msg.m_Cmd_Read32_Rsp.cnt; i++)
					(*val)[i] = LSWAP(Msg.msg.m_Cmd_Read32_Rsp.vals[i]);
			}
			else
			{
				for(int i = 0; i < Msg.msg.m_Cmd_Read32_Rsp.cnt; i++)
					(*val)[i] = Msg.msg.m_Cmd_Read32_Rsp.vals[i];
			}
			return kTRUE;
		}
		return kFALSE;
	}

	bool Delay(unsigned int ms)
	{
		if(!CheckConnection(__FUNCTION__))
			return kFALSE;

		Msg.len = 4;
		Msg.type = CRATEMSG_TYPE_DELAY;
		Msg.msg.m_Cmd_Delay.ms = ms;
		SendRaw(&Msg, Msg.len+8);

		return kTRUE;
	}

private:
	int				swap;
	CrateMsgStruct	Msg;
	TSocket			*pSocket;
};






























int
main(int argc, char* argv[])
{
  char host[256];
  int port;

  CrateMsgClient *pCrateMsgClient;

  if(argc!=3)
  {
    printf("Usage: DiagGuiClient <host> <port>\n");
    exit(0);
  }
  strncpy(host, argv[1], 255);
  port = atoi(argv[2]);
  printf("host >%s< port=%d\n",host,port);
exit(0);

  pCrateMsgClient = new CrateMsgClient(paramB, port);
  if(pCrateMsgClient->IsValid())
  {
    pCrateMsgClient->Close();
	printf("Disconnected from host: %s\n");
  }

  exit(0);
}
