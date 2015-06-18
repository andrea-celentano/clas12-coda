#define _GNU_SOURCE
#define DEVEL

#include <sys/prctl.h>
#include <unistd.h>
#include "jvme.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "beusspLib.h"
#include <pthread.h>


// ------------------------------------------------------------------------------------------------
// general purpose declarations for BEUSSP 
// ------------------------------------------------------------------------------------------------
/* Mutex to guard BEUSSP read/writes */
pthread_mutex_t   beusspMutex = PTHREAD_MUTEX_INITIALIZER;
#define BEUSSPLOCK     if(pthread_mutex_lock(&beusspMutex)<0) perror("pthread_mutex_lock");
#define BEUSSPUNLOCK   if(pthread_mutex_unlock(&beusspMutex)<0) perror("pthread_mutex_unlock");
// ------------------------------------------------------------------------------------------------
/* VXS Payload Port to VME Slot map */
#define MAX_VME_SLOTS 21    /* This is either 20 or 21 */
unsigned short myPayloadPort[MAX_VME_SLOTS+1] =
  {
    0,     /* Filler for mythical VME slot 0 */ 
#if MAX_VME_SLOTS == 21
    0,     /* VME Controller */
#endif
    17, 15, 13, 11, 9, 7, 5, 3, 1,  
    0,     /* Switch Slot A - SD */
    0,     /* Switch Slot B - CTP/GTP */
    2, 4, 6, 8, 10, 12, 14, 16, 
    18     /* VME Slot Furthest to the Right - TI */ 
  };
// ------------------------------------------------------------------------------------------------
// declaration of utility functions and macros used in beusspLibTest.c
// ------------------------------------------------------------------------------------------------
 void delay(unsigned int ms)
 {
 	volatile unsigned int i;
 	while(ms--)
 		for(i = 0; i < 1000; i++);
 }
// ------------------------------------------------------------------------------------------------
void handle_help( void )
{
	printf ( "help \r\n"); 	
	printf(
	"\r\nMenu:\r\n"
	"resetpll \t\t Reset global clock pll \r\n"
	"resetgtx \t\t Reset multigtx \r\n"
	"flush \t\t\t Flush pipeline \r\n"
	"configure \t\t Configure active GTX, emulator bypass, trigger source, trigger frequency, etc. \r\n"
	"readall \t\t Prints the contents of all registers.\r\n"
	"start \t\t\t Starts / enables triggers and data acquisition. \r\n"
	"sendslwctrl \t\t send slow control request to specific FEU target : sendslwctrl numfeu data1 data2 data3 data4\r\n"
	"localclock \t\t set 250MHz MGT reference clock to local \r\n"	
	"numfeu \t\t set target  hexa decimal feu number either for slow control or packet counter display \r\n"
	"ticlock \t\t set 250MHz MGT reference clock distributed TI clock \r\n"
	"quit \t\t\t Quit\r\n"
	"help \t\t\t View this menu\r\n"
	);
}
// ------------------------------------------------------------------------------------------------
int my_hex_string_to_int(char * hex_string, unsigned int * pval)
{
	char * p;
	unsigned int nybble;

	*pval = 0;
	if (!*hex_string)
		return 0;

	p = hex_string;
	if (p[0] == '0') {
		if ((p[1] == 'x') || (p[1] == 'X'))
			p += 2;
	}

	for (; *p; p++) {
		if ((*p >= '0') && (*p <= '9'))
			nybble = *p - '0';
		else if ((*p >= 'a') && (*p <= 'f'))
			nybble = *p - 'a' + 0xa;
		else if ((*p >= 'A') && (*p <= 'F'))
			nybble = *p - 'A' + 0xa;
		else
			return 0;
		*pval <<= 4;
		*pval |= nybble;
	}

	return 1;
}
// ------------------------------------------------------------------------------------------------
void my_get_line( char * line, unsigned int maxlen) {
	
char   c = 0;
char * p = line;
int n;

	*p = 0;
	for (n = 0; n < maxlen-1; n++) {
		c = getchar();
		if (c == '\n') {
			//printf("new line caracter received");
			putchar('\r');
			putchar('\n');
			break;
		}
		else if (c == '\r') {
			printf("carriage return received");
			putchar('\r');
			putchar('\n');
			break;
		}

		// Check for backspace or delete key.
		else if ((c == '\b') || (c == 0x7F)) {
			if (p > line) {
				putchar('\b'); // Write backspace
				putchar(' ');  // Write space
				putchar('\b'); // Write backspace
				p--;
				*p = 0;
			}
		}

		// Check for escape key or control-U.
		else if ((c == 0x1b) || (c == 0x15)) {
			while (p > line) {
				putchar('\b');
				putchar(' ');
				putchar('\b');
				p--;
				*p = 0;
			}
		}
		
		else {
			*p = c;
			p++;
			*p = 0;
		}
	}
	*p = 0;
}
// ------------------------------------------------------------------------------------------------
void tokenize(char * line, int * pargc, char ** argv, unsigned int max_arguments){
	char * readp;
	int    inside_token = 0;

	*pargc = 0;

	for (readp = line; *readp; readp++) {
		if (!inside_token) {
			if ((*readp == ' ') || (*readp == '\t')) {
				// Whitespace is not copied.
			}
			else {
				// Start of token
				inside_token = 1;
				argv[*pargc] = readp;
				(*pargc)++;
			}
		}
		else { // inside token
			if ((*readp == ' ') || (*readp == '\t')) {
				// End of token
				inside_token = 0;
				*readp = 0;
			}
			else {
				// Continuation of token
			}
		}
	}

	if (inside_token) {
		// End of input line terminates a token.
		*readp = 0;
		readp++;
	}

	argv[*pargc] = 0; // Null-terminate just to be nice.
}


//------------------------------------------------------------------------------------------------
//
//  beusspInit - Initialize the beussp register space into local memory,
//  and setup registers given user input
//
//  ARGs: 
//    vmebaseaddr - A24 VME Address of the BEUSSP , should match address set by hardware on beussp board
//    BEUSSPreg   - map of all beussp registers 
//    BEUSSPconf  - structure filled by software containing values for all the configuration registers of the beussp 
//
//  RETURNS: OK if successful, otherwise ERROR.
//
//------------------------------------------------------------------------------------------------
int  beusspInit(unsigned int vmebaseaddr, volatile struct BEUSSP_A24RegStruct  * * BEUSSPreg, volatile unsigned int * *BEUSSPfifo, BeuSspConf  * BEUSSPconf){

	unsigned int laddr;
	unsigned int rval;
	int stat;

	/* Check VME address */
	if(vmebaseaddr<0 || vmebaseaddr>0xffffff)
	{
		fprintf( stderr, "%s: ERROR: Invalid VME Address  %d \n", __FUNCTION__, vmebaseaddr );
		return ERROR;
	}

	stat = vmeBusToLocalAdrs(0x39,(char *)vmebaseaddr,(char **)&laddr);
	if (stat != 0) 
	{
		fprintf( stderr,"%s: ERROR: Error in vmeBusToLocalAdrs res=%d \n",__FUNCTION__,stat);
		return ERROR;
	} 

	/* Check that it is a BEUSSP */
	stat = vmeMemProbe((char *)(laddr),4,(char *)&rval);
	if (stat != 0) 
	{
		fprintf( stderr, "%s: BEUSSP not found at 0x%08x\n",__FUNCTION__,vmebaseaddr);
		return ERROR;
	}
	if(  rval != BEUSSP_BOARDID_TYPE_BEUSSP)
	{
		fprintf( stderr, "%s: Not a BEUSSP board at 0x%08x\n",__FUNCTION__,vmebaseaddr);
		return ERROR;
	}
	
	printf("%s: Found BEUSSP at 0x%08x\n",__FUNCTION__,vmebaseaddr);
	printf("BEUSSP VME (USER) address = 0x%.8x (0x%.8x)\n",vmebaseaddr,laddr);	
	//beusspA24Offset = laddr-vmebaseaddr;
	/* Set Up pointer */
	*BEUSSPreg = (volatile struct BEUSSP_A24RegStruct *)laddr;		// adresses allouees au peripherique VME


	//stat = vmeMemProbe((char *)( &BEUSSPreg->boardID ),4,(char *)&rval);
	stat = vmeMemProbe((char *)( &(* BEUSSPreg)->boardID ),4,(char *)&rval);
	if (stat != 0) 
	{
		fprintf( stderr,"%s: ERROR: BEUSSP card not addressable\n",__FUNCTION__);
		*BEUSSPreg=NULL;
		return ERROR;
	}
	
	if(  rval != BEUSSP_BOARDID_TYPE_BEUSSP)
	{
	fprintf( stderr,"%s: ERROR: Invalid Board ID: 0x%x (rval = 0x%08x)\n",__FUNCTION__,rval,rval);
	return ERROR ;
	}   

	/* Check if this board has a valid slot number */
	BEUSSPLOCK;
		rval = vmeRead32( &(* BEUSSPreg)->adrgeo     );
	BEUSSPUNLOCK;
	if(  rval != BEUSSPconf->Slot)
		{
		fprintf( stderr,"%s: ERROR: slot number mismatch ! From configuration  file %d  Read from board = %d \n",__FUNCTION__,BEUSSPconf->Slot,rval);
		return ERROR ;
	}   
	

	/*Make sure all control registers are in proper state */
	beusspInitCtrlRegs(* BEUSSPreg);
	
	/*flush the pipeline - soft reset*/
	beusspFlushPipeline(* BEUSSPreg);
	
	/* Configure BEUSSP with provided configuration structure */
	beusspWriteConf(* BEUSSPreg, BEUSSPfifo, BEUSSPconf);
	
	return OK;
}

/*******************************************************************************
 *
 *  beusspInitCtrlRegs - Initialize control registers for slow control and daq 
 *
 *  ARGs: 
 *    BEUSSPreg   - map of all beussp registers 
 *
 *  RETURNS: OK if successful, otherwise ERROR.
 *
*/
int beusspInitCtrlRegs(volatile struct BEUSSP_A24RegStruct * BEUSSPreg)
{
	if(BEUSSPreg == NULL) 
	{
     fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  	BEUSSPLOCK;   
  	vmeWrite32( &BEUSSPreg->regin_4,  0x00000000 );
	vmeWrite32( &BEUSSPreg->regin_6,  0x00000000 );
	BEUSSPUNLOCK;   
	
    return OK;
}


/*******************************************************************************
 *
 *  beusspSetTargetFeu - set the FEU target number for subsequent packet counter display
 *
 *  ARGs: 
 *    BEUSSPreg   - map of all beussp registers 
 *
 *  RETURNS: OK if successful, otherwise ERROR.
 *
*/
int beusspSetTargetFeu(volatile struct BEUSSP_A24RegStruct * BEUSSPreg, int numFeu)
{
	if(BEUSSPreg == NULL) 
	{
     fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
  	BEUSSPLOCK;   
	vmeWrite32( &BEUSSPreg->regin_6,   (0x000000FF&numFeu)<< 8   );
	BEUSSPUNLOCK;   
	
    return OK;
}


/*******************************************************************************
 *
 *  beusspSendSlowControl - send a R/W slow control request to specified FEU 
 *
 *  ARGs: 
 *    BEUSSPreg   - map of all beussp registers 
 *
 *  RETURNS: OK if successful, otherwise ERROR.
 *
*/
int beusspSendSlowControl(volatile struct BEUSSP_A24RegStruct * BEUSSPreg, unsigned int numFeu, unsigned  int * datain1, unsigned int * datain2, unsigned int * dataout1, unsigned int * dataout2)
{
  unsigned int timeout = 0, slwctrlstatus = 0, mltgtxstatus = 0;
  unsigned int feu_is_there = 0, feu_is_active = 0;

   // check that BEU is there
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
 
// check that FEU is there and active 
	BEUSSPLOCK;
	
	feu_is_there = vmeRead32(  &(BEUSSPreg->regout_11)     );
	feu_is_active = vmeRead32( &(BEUSSPreg->regin_0)     );
	
	BEUSSPUNLOCK;
	if (((feu_is_there >> numFeu) & 0x00000001 ) != 1 )  
		{
		fprintf( stderr,"%s: ERROR: there is no FEU on link %d  Feu present 0x%08x.  \n",__FUNCTION__, numFeu,feu_is_there );
		return ERROR ;
	}   
	if ((( feu_is_active >> numFeu) & 0x00000001 ) != 1 ) 
		{
		fprintf( stderr,"%s: ERROR: FEU on link %d is not marked as active.  \n",__FUNCTION__, numFeu);
		return ERROR ;
	}   
 
  	BEUSSPLOCK;   

	//check that peripheral is ready to send a slwctrl request
	slwctrlstatus = vmeRead32( &BEUSSPreg->regout_B );
	mltgtxstatus = vmeRead32( &BEUSSPreg->regout_10);
	
//	if ( (( (slwctrlstatus & 0x0000003E) != (unsigned int) 0  ) || ((mltgtxstatus & 0x00000001)  ==  (unsigned int) 0 ) )) {
	if ( ( (slwctrlstatus & 0x0000003E) != (unsigned int) 0  ) ) {
		fprintf( stderr,"%s: ERROR: Peripheral not ready for slow control : scstat=0x%08x scstat&0x3E=0x%02x mgtstat=0x%08x mgtstat&0x1=0x%01x.\r\n",
			__FUNCTION__,  slwctrlstatus, slwctrlstatus & 0x3E, mltgtxstatus, mltgtxstatus & 0x1 );
		BEUSSPUNLOCK;   		 
		return ERROR;
	}
	
	
	
		//write slowctrl data to be sent to FEU 	
		vmeWrite32( &BEUSSPreg->regin_8, * datain1);  
		vmeWrite32( &BEUSSPreg->regin_9, * datain2); 		
		//write numfeu  
		vmeWrite32( &BEUSSPreg->regin_6,   (0x000000FF&numFeu)<< 8   );
		//request send
		vmeWrite32( &BEUSSPreg->regin_6,   ( (0x000000FF&numFeu)<< 8 )  | (0x00000001)  ); //request send !
#define SLCTRL_TO_CNTR 10000
		timeout = 0;
		do{
			slwctrlstatus = vmeRead32( &BEUSSPreg->regout_B);
			timeout += 1;
		} while ( ( timeout <= SLCTRL_TO_CNTR) && ( (slwctrlstatus & 0x00000008) == (unsigned int)0) ) ;

	
		if ( timeout > SLCTRL_TO_CNTR )  {
			//set low the slowctrl request bit
			vmeWrite32( &BEUSSPreg->regin_6,  ( (0x000000FF&numFeu)<< 8 )  | (0x00000000) );

		 	fprintf( stderr,"%s: ERROR: TIMEOUT ! Slow control operation timed out by software. \n\r",__FUNCTION__);
		 	BEUSSPUNLOCK;   		 
		 	return ERROR;
		}

		//These two error types are not exclusive : there will be a timeout if no Ack is receivd, there can be an Ack received and a timeout.
		//The following is sufficient for now.

		if ( (slwctrlstatus & 0x00000010) == 0x00000000 )  {
			//set low the slowctrl request bit
			vmeWrite32( &BEUSSPreg->regin_6,  ( (0x000000FF&numFeu)<< 8 )  | (0x00000000) );

		 	fprintf( stderr,"%s: ERROR:  no Acknowledge received for slow control request. \n\r",__FUNCTION__);
		 	BEUSSPUNLOCK;   		 
		 	return ERROR;
		}
		
		if ( (slwctrlstatus & 0x00000020) == 0x00000020 )  {
			//set low the slowctrl request bit
			vmeWrite32( &BEUSSPreg->regin_6,  ( (0x000000FF&numFeu)<< 8 )  | (0x00000000) );

		 	fprintf( stderr,"%s: ERROR: TIMEOUT ! Slow control operation timed out by hardware. \n\r",__FUNCTION__);
		 	BEUSSPUNLOCK;   		 
		 	return ERROR;
		}

		* dataout1 = vmeRead32( &BEUSSPreg->regout_12);
		* dataout2 = vmeRead32( &BEUSSPreg->regout_13);

		//set low the slowctrl request bit
		vmeWrite32( &BEUSSPreg->regin_6,  ( (0x000000FF&numFeu)<< 8 )  | (0x00000000) );
		BEUSSPUNLOCK;   
	   	return OK;
}

/*******************************************************************************
 *
 *  beusspDisplayAllReg - display all beussp registers to screen
 *
 *  ARGs: 
 *    BEUSSPreg   - map of all beussp registers 
 *
 *  RETURNS: OK if successful, otherwise ERROR.
 *
*/
int  beusspDisplayAllReg(volatile struct BEUSSP_A24RegStruct  * BEUSSPreg)
{
  unsigned int reg_num=0;
  unsigned int memadr;
  unsigned int data, ddata;
  
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
	
	memadr =  ( unsigned int ) BEUSSPreg;
	
	BEUSSPLOCK;
	
	for (reg_num = 0; reg_num < 20 ; reg_num ++)
			{

			printf("%08X %08x %08x %08x %08x \r\n",memadr, 
							vmeRead32( (volatile unsigned int * ) memadr    ), 
							vmeRead32( (volatile unsigned int * )(memadr + 0x4)), 
							vmeRead32( (volatile unsigned int * )(memadr + 0x8)),
							vmeRead32( (volatile unsigned int * )(memadr + 0xc))  ); 
			memadr += 0x10;
			}
	
	printf("------------------ BEUSSP REGISTERS ------------------------\r\n" );  
	printf("board Soft ID   %08X board ID        %08X FWRevision   %08X \r\n",  vmeRead32( &BEUSSPreg->regin_11 ),  vmeRead32( &BEUSSPreg->boardID ),  vmeRead32( &BEUSSPreg->FWRevision ) );
	printf("ADR32M          %08X ADR32           %08X ADRGEO       %08X \r\n",  vmeRead32( &BEUSSPreg->adr32m ),  vmeRead32( &BEUSSPreg->adr32 ), vmeRead32( &BEUSSPreg->adrgeo )  );
	printf("Active links    %08X Masked FEU Emu  %08X NELoopback   %08X \r\n",  vmeRead32( &BEUSSPreg->regin_0 ),  vmeRead32( &BEUSSPreg->regin_1 ), vmeRead32( &BEUSSPreg->regin_C ) );
	printf("Trigger Period  %08X \r\n",  vmeRead32( &BEUSSPreg->regin_2 ) );
	printf("MGTX status     %08X MGTX aligned    %08X \r\n", vmeRead32( &BEUSSPreg->regout_10 ) , vmeRead32( &BEUSSPreg->regout_11 )  );
	data = vmeRead32( &BEUSSPreg->regin_3 ) ;
	printf("Samples         %08X Events/Block    %08X \r\n",  (data&0xFFFF0000)>>16 , (data&0x0000FFFF) );
	data = vmeRead32( &BEUSSPreg->regin_5 ) ;
	printf("Busy Source     %08X Sync Source     %08X Trig Source  %08X \r\n",  (data&0xFF000000)>>24 , (data&0x00FF0000)>>16 , (data&0x0000FF00)>>8  );
	data = vmeRead32( &BEUSSPreg->regin_D) ;	
	ddata = vmeRead32( &BEUSSPreg->regin_E) ;
	printf("FEU High Thr    %08X FEU Low Thr     %08X FEU Full Thr %08X \r\n",  (data&0x0000FFFF) , (data&0xFFFF0000)>>16 , (ddata&0x0000FFFF)  );
	printf("Trigger Fifo Maximum Occupancy       %08X \r\n",  vmeRead32( &BEUSSPreg->regout_16 ) );
	printf("Timeout setting for concentrator     %08X \r\n", vmeRead32( &BEUSSPreg->regin_F )  );
	data = vmeRead32( &BEUSSPreg->regout_C) ;	
	ddata = vmeRead32( &BEUSSPreg->regin_6) ;
	printf("Rx SlwCtrl Cntr %08X Rx Data Cntr    %08X Link number  %08X \r\n",(data&0xFFFF0000)>>16 , (data&0x0000FFFF), (ddata&0x0000FF00)>>8    );
	printf("Tx SlwCtrl Cntr %08X Tx Trg Cntr     %08X SyncCmd Cntr %08X \r\n", vmeRead32( &BEUSSPreg->regout_A ) , vmeRead32( &BEUSSPreg->regout_8 ), vmeRead32( &BEUSSPreg->regout_9 )   );
  	printf("Packet Errors (7:unexp. char. 6: EOP timeout 5: misp. IFG 4: crc mismatch 3-2: invalid data 1-0: invalid slwctrl) \r\n" );
  	printf("(links 31 downto 0) : \r\n" );  
  	printf("%08X  %08X %08X %08X  \r\n",vmeRead32( &BEUSSPreg->regout_7 ), vmeRead32( &BEUSSPreg->regout_6 ), vmeRead32( &BEUSSPreg->regout_5 ), vmeRead32( &BEUSSPreg->regout_4 ) );
  	printf("%08X  %08X %08X %08X  \r\n",vmeRead32( &BEUSSPreg->regout_3 ), vmeRead32( &BEUSSPreg->regout_2 ), vmeRead32( &BEUSSPreg->regout_1 ), vmeRead32( &BEUSSPreg->regout_0 ) );
	//printf("BEUSSP STATUS (23-trgfifoalmostfull 22-trgfifofull 21-ORBRAMBZRD 20-ORBRAMREADME 19-BRAMINFOBZRD 18:BRAMINFOREADME 17:TRGFIFOBZRD 16:TRGFIFOREADME 0:braminfowe 1:bramdatawe 2:braminfoweifspace %08X \r\n",  vmeRead32( &BEUSSPreg->regout_15 ) );
	data = vmeRead32( &BEUSSPreg->regout_14 );
	printf("BEUSSP ERROR - TRIGGER FIFO OVERFLOW        %08X \r\n", (data&0x00020000)>>17 );
	printf("BEUSSP ERROR - INFERRED FEU OVERFLOW        %08X \r\n", (data&0x00010000)>>16 );
	printf("BEUSSP ERROR - OVERFLOW IN BRAM2DDR         %08X \r\n", (data&0x00000001) );
	printf("BEUSSP ERROR - OVERFLOW IN BRAMINFO         %08X \r\n", (data&0x00000002)>>1 );	
	printf("BEUSSP ERROR - OVERFLOW IN HEAD BRAM (31>0) %08X \r\n",  vmeRead32( &BEUSSPreg->regout_18 ) ); 
	printf("BEUSSP ERROR - DATA CONCENTRATOR TIMEOUT    %08X ( counter : %08X) \r\n", (data&0x00000004)>>2, (data&0x000000F0)>>4  );	
	printf("BEUSSP ERROR - TIMEDOUT LINKS (31>0)        %08X \r\n",  vmeRead32( &BEUSSPreg->regout_17 ) );	
	printf("BEUSSP ERROR - DATA SYNCHRONIZATION ERROR   %08X ( counter : %08X) \r\n", (data&0x00000008)>>3, (data&0x00000F00)>>8  );		
  	printf("------------------------------------------------------------\r\n" );  


	BEUSSPUNLOCK;
  return OK;
}

/*******************************************************************************
 *
 *  beusspWriteConf - set configuration registers of BEUSSP according to provided configuration parameters
 *
 *  ARGs: 
 *    BEUSSPreg   - map of all beussp registers 
 *    BEUSSPconf  - structure filled by software containing values for all the configuration registers of the beussp 
 *
 *  RETURNS: OK if successful, otherwise ERROR.
 *
*/
int  beusspWriteConf(volatile struct BEUSSP_A24RegStruct  * BEUSSPreg, volatile unsigned int * * BEUSSPfifo, BeuSspConf  * BEUSSPconf)
{
	unsigned int timeout;
	
  if(BEUSSPreg == NULL) 
    {
     fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

	timeout = BEUSSPconf->timeout;
	if( BEUSSPconf->InMemBusyThr ) {
		timeout |= ( (BEUSSPconf->InMemBusyThr << 16) | 0x10000000 );
	}
	if( BEUSSPconf->MaskToInput ) {
		timeout |= 0x8000;
	}



               beusspSetAdr32(BEUSSPreg,BEUSSPfifo,BEUSSPconf->base_adr_fifo );
         beusspSetSampleBlock(BEUSSPreg,BEUSSPconf->NbOfSamples,BEUSSPconf->NbOfEventsPerBlock);
         beusspSetActiveLinks(BEUSSPreg,BEUSSPconf->rol_enb);
              beusspSetClkSrc(BEUSSPreg,BEUSSPconf->ClkSrc);           
     beusspSetNearEndLoopBack(BEUSSPreg,BEUSSPconf->near_end_loop_enb);
      beusspSetBypassEmulator(BEUSSPreg,BEUSSPconf->feu_emu_msk);
         beusspSetEmuDatasize(BEUSSPreg,BEUSSPconf->emu_data_size);
       beusspSetTrgFifoThresh(BEUSSPreg,BEUSSPconf->TrgFifo_Lwm,BEUSSPconf->TrgFifo_Hwm,BEUSSPconf->TrgFifo_Full);
 beusspSetConcentratorTimeout(BEUSSPreg,timeout);
 			//beusspSetConcentratorTimeout(BEUSSPreg,0x1AFF7FFF);
         beusspSetSBTSource(BEUSSPreg,BEUSSPconf->TrgSrc,BEUSSPconf->BsySrc,BEUSSPconf->SyncSrc);
           //beusspSetSBTSource(BEUSSPreg,BEUSSPconf->TrgSrc,6,BEUSSPconf->SyncSrc);
              beusspSetSoftId(BEUSSPreg,BEUSSPconf->Id);       
              beusspSetAdr32m(BEUSSPreg, BEUSSPconf->base_adr_com_min, BEUSSPconf->base_adr_com_max, BEUSSPconf->mblk_rank);

  return OK;
}

/*******************************************************************************
 *
 *  beusspResetMultiGTX - 
 *
 *  ARGs: 
 *    BEUSSPreg   - map of all beussp registers 
 *    BEUSSPconf  - structure filled by software containing values for all the configuration registers of the beussp 
 *
 *  RETURNS: OK if successful, otherwise ERROR.
 *
*/
int  beusspResetMultiGTX(volatile struct BEUSSP_A24RegStruct  * BEUSSPreg)
{

  unsigned int res=0, timeout = 0;

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

		BEUSSPLOCK;
	  
		vmeWrite32( &BEUSSPreg->regin_7, 0x00010000);  //multigtx reset high
		printf(" Resetting multigigabit transceivers. \n\r" );
		do{
			res = vmeRead32( &BEUSSPreg->regout_10  );
		} while  ( (res & 0x00000001) == 1 )   ;        // check multigtx ready low        


		vmeWrite32( &BEUSSPreg->regin_7, 0x00000000);  //multigtx reset low
		timeout = 0;
		usleep(500);
		do{
			res = vmeRead32( &BEUSSPreg->regout_10  );
			timeout += 1;
		} while ( ( timeout < 0x001FFFFF ) && ((res & 0x00000001) == 0 ) ) ;  	// check multigtx ready high


		if ( timeout < 0x001FFFFF )  {
			printf("%s: Multi Gigabit tranceivers ready . \n",__FUNCTION__);
			BEUSSPUNLOCK;
		}
		else
		{
			fprintf( stderr,"%s: ERROR: Multi Gigabit tranceivers not ready . \n",__FUNCTION__);
			BEUSSPUNLOCK;
			return ERROR;
		}
			
		return OK;
}
/*******************************************************************************
 *
 *  beusspResetGClkPll - 
 *
 *  ARGs: 
 *    BEUSSPreg   - map of all beussp registers 

 *
 *  RETURNS: OK if successful, otherwise ERROR.
 *
*/
int  beusspResetGClkPll(volatile struct BEUSSP_A24RegStruct  * BEUSSPreg)
{

  unsigned int res=0, timeout=0;
	  if(BEUSSPreg == NULL) 
		{
		  fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
		  return ERROR;
		}
				
		BEUSSPLOCK;
	  
		vmeWrite32( &BEUSSPreg->regin_7, 0x00020000);  //glclk pll reset high
		printf(" Resetting global clock pll. \n\r" );
		do{
			res = vmeRead32( &BEUSSPreg->regout_B  );
		} while (  ( (res & 0x00000001) == 1 ) ||  ( (res & 0x00000100) == 1 )  ) ;                

		vmeWrite32( &BEUSSPreg->regin_7, 0x00000000); //glclk pll reset low
		timeout = 0;
		usleep(500);
		do{
			res = vmeRead32( &BEUSSPreg->regout_B  );
			timeout += 1;
		} while ( ( timeout < 0x001FFFFF ) && ((res & 0x00000100) == 0 ) ) ;  


		if ( timeout < 0x001FFFFF )  {
			printf("%s: GLOBAL clock PLL locked. \n\r",__FUNCTION__); 
			BEUSSPUNLOCK;
		}
		else
		{
			printf("%s: ERROR: GLOBAL clock PLL NOT locked \n\r",__FUNCTION__); 
			fprintf( stderr,"%s: ERROR: GLOBAL clock PLL NOT locked \n",__FUNCTION__);
			BEUSSPUNLOCK;
			return ERROR;
		}
			
		return OK;
}


/*******************************************************************************
 *
 *  beusspFlushPipeline - 
 *
 *  ARGs: 
 *    BEUSSPreg   - map of all beussp registers 

 *
 *  RETURNS: OK if successful, otherwise ERROR.
 *
*/
int  beusspFlushPipeline(volatile struct BEUSSP_A24RegStruct  * BEUSSPreg)
{

	  if(BEUSSPreg == NULL) 
		{
		  fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
		  return ERROR;
		}
				
		BEUSSPLOCK;

				vmeWrite32( &BEUSSPreg->regin_7,  0x00040000);  //flush bit set high
				printf("%s: Flushing pipeline ... \n\r",__FUNCTION__ );
				usleep(100);

				vmeWrite32(&BEUSSPreg->regin_7,  0x00000000);  //flush bit set low
				usleep(500);
				printf("%s: ... pipeline flushed ! ! \n\r",__FUNCTION__ );
				
		BEUSSPUNLOCK;

	return OK;
}




/*******************************************************************************
 *
 *  beusspSetAdr32
 *  - Routine to set the A32 Base
 *
 */

int beusspSetAdr32(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg,  volatile unsigned int * * BEUSSPfifo,  unsigned int a32base)
{
  unsigned int laddr=0;
  int res=0,a32Enabled=0;

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  if(a32base<0x00800000)
    {
      fprintf( stderr,"%s: ERROR: a32base out of range (0x%08x)\n",
	     __FUNCTION__,a32base);
      return ERROR;
    }

  BEUSSPLOCK;

    vmeWrite32(&BEUSSPreg->adr32, (a32base>>16) & BEUSSP_ADR32_BASE_MASK );
    //enable 
  vmeWrite32(&BEUSSPreg->adr32, (  ((vmeRead32(&BEUSSPreg->adr32))& BEUSSP_ADR32_BASE_MASK) | BEUSSP_EN_ADR32_MASK ) );

  a32Enabled = (vmeRead32(&BEUSSPreg->adr32))  & BEUSSP_EN_ADR32_MASK;
  if(!a32Enabled)
    {
     fprintf( stderr,"%s: ERROR: Failed to enable A32 Address\n",__FUNCTION__);
      BEUSSPUNLOCK;
      return ERROR;
    }

  res = vmeBusToLocalAdrs(0x09,(char *)a32base,(char **)&laddr);
  if (res != 0) 
    {
      fprintf( stderr,"%s: ERROR in vmeBusToLocalAdrs(0x09,0x%x,&laddr) \n", __FUNCTION__,a32base);
      BEUSSPUNLOCK;
      return ERROR ;
    }
  * BEUSSPfifo = (unsigned int *)(laddr);  /* Set a pointer to the FIFO */

  BEUSSPUNLOCK;

  printf("%s: A32 Base address set to 0x%08x  ( 0x%08x ) \n", __FUNCTION__,a32base,laddr );
  printf("%s: A32 Base address space enabled   \n", __FUNCTION__ );


  return OK;
}


//------------------------------------------------------------------------------------------------
//  beusspEnableA32 - Enable the A32 address space for block readout
//
//  ARGs: 
//       - 
//       - 
//
//  RETURNS: OK if successful, otherwise ERROR.
//
//------------------------------------------------------------------------------------------------
int beusspEnableA32(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg){
 
 int  a32Enabled=0;

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;
  
  vmeWrite32(&BEUSSPreg->adr32, (  ((vmeRead32(&BEUSSPreg->adr32))& BEUSSP_ADR32_BASE_MASK) | BEUSSP_EN_ADR32_MASK ) );

  a32Enabled = (vmeRead32(&BEUSSPreg->adr32))  & BEUSSP_EN_ADR32_MASK;
  if(!a32Enabled)
    {
     fprintf( stderr,"%s: ERROR: Failed to enable A32 Address\n",__FUNCTION__);
      BEUSSPUNLOCK;
      return ERROR;
    }

  BEUSSPUNLOCK;
  return OK;
}


//------------------------------------------------------------------------------------------------
//  beusspDisableA32 - Disable the A32 address space for block readout
//
//  ARGs: 
//       - 
//       - 
//
//  RETURNS: OK if successful, otherwise ERROR.
//
//------------------------------------------------------------------------------------------------
int beusspDisableA32(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg){
 
 int a32Enabled=0;

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->adr32, (  ((vmeRead32(&BEUSSPreg->adr32)) & BEUSSP_ADR32_BASE_MASK) ) );

  a32Enabled = (vmeRead32(&BEUSSPreg->adr32))  & BEUSSP_EN_ADR32_MASK;
  if(a32Enabled)
    {
     fprintf( stderr,"%s: ERROR: Failed to disable A32 Address\n",__FUNCTION__);
      BEUSSPUNLOCK;
      return ERROR;
    }

  BEUSSPUNLOCK;
  return OK;
}


/*******************************************************************************
 *
 *  beusspGetBlockSize - get the size of the next block to read from the data fifo
 *
 *  ARGs: 
 *    BEUSSPreg   - map of all beussp registers 
 *
 *  RETURNS: OK if successful, otherwise ERROR.
 *
*/
int  beusspGetBlockSize(volatile struct BEUSSP_A24RegStruct  * BEUSSPreg, unsigned int * blocksize)
{

unsigned int res=0, data=0, nb_words_in_block=0;
  
  if(BEUSSPreg == NULL) 
    {
     fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
	
BEUSSPLOCK;

	// relecture taille des données utiles ( relecture indirecte)
	//set address  
	res = vmeRead32( &BEUSSPreg->regout_F );
	vmeWrite32(&BEUSSPreg->infobram_adr, ( res & 0x000000FF ) * 0x40);
	//read  data 
	data = vmeRead32( &BEUSSPreg->infobram_rdata );
	
	BEUSSPUNLOCK;

	//compute number of data words in current data block (including padding) 
	nb_words_in_block = ((data & 0x0000FFFF) << 16 ) | ((data & 0xFFFF0000) >> 16 );

	if ( (((nb_words_in_block + 2)>>4 )<< 4) == (nb_words_in_block + 2)) {
		nb_words_in_block = (nb_words_in_block + 2) >> 4; }
	else {
		nb_words_in_block = ((((nb_words_in_block + 2)>> 4 ) + 1) ) ; 
	}

	//block size in bytes ?
	nb_words_in_block = nb_words_in_block << 3;
	* blocksize = nb_words_in_block;
		
return OK;
}
/*******************************************************************************
 *
 * beusspReadBlock - Read a block of events from the BEUSSP
 *
 *    data  - local memory address to place data
 *    nwrds - Max number of words to transfer
 *    rflag - Readout Flag
 *              0 - programmed I/O from the specified board
 *              1 - DMA transfer using Universe/Tempe DMA Engine 
 *                    (DMA VME transfer Mode must be setup prior)
 *
 * RETURNS: Number of words transferred to data if successful, ERROR otherwise
 *
 */

int beusspReadBlock(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg,  volatile unsigned int *BEUSSPfifo, volatile unsigned int * data, unsigned int nwrds, int rflag)
{
  int retVal, xferCount;
  volatile unsigned int *laddr;
	unsigned int vmeAdr;
	unsigned int a32base;
	unsigned int a32Enabled=0;

  if(BEUSSPreg==NULL)
    {
     fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
    }

  if(BEUSSPfifo==NULL)
    {
      fprintf( stderr,"%s: ERROR: BEUSSP A32 not initialized\n",__FUNCTION__);
      return ERROR;
    }

  if(data==NULL) 
    {
          fprintf( stderr,"%s: ERROR: Invalid Destination address\n",__FUNCTION__);
      return(ERROR);
    }


  BEUSSPLOCK;
  if(rflag >= 1)
    { /* Block transfer */

      /* Assume that the DMA programming is already setup. 
	 Don't Bother checking if there is valid data - that should be done prior
	 to calling the read routine */
      
      /* Check for 8 byte boundary for address - insert dummy word (Slot 0 FADC Dummy DATA)*/
      
	laddr = data;      
	a32base = vmeRead32( &BEUSSPreg->adr32 );
	a32Enabled = a32base  & BEUSSP_EN_ADR32_MASK;
  	if(!a32Enabled) 
    {
      fprintf( stderr,"%s: ERROR: A32 not enabled \n",__FUNCTION__);
      return(ERROR);
    }

	
	vmeAdr = (unsigned int)  ( (a32base & BEUSSP_ADR32_BASE_MASK ) << 16) ; 

/*	printf("vmeAdr=0x%08x\n",vmeAdr);  */

	/*retVal = vmeDmaSend((UINT32)laddr, vmeAdr, (nwrds<<2));*/
	retVal = usrVme2MemDmaStart(vmeAdr, (unsigned int *)laddr, (nwrds << 2));

      if(retVal |= 0) 
	{
	  fprintf( stderr,"%s: ERROR in DMA transfer Initialization 0x%x\n",__FUNCTION__, retVal);
	  BEUSSPUNLOCK;
	  return(retVal);
	}

      /* Wait until Done or Error */
    /*retVal = vmeDmaDone();*/
    retVal = usrVme2MemDmaDone();
    if(retVal > 0)
	{

	  xferCount = ((retVal>>2) ); /* Number of longwords transfered */
	  BEUSSPUNLOCK;
	  return(xferCount);
	}
      else if (retVal == 0) 
	{
	  fprintf( stderr,"%s: WARN: DMA transfer returned zero word count 0x%x\n",__FUNCTION__, nwrds);
	  BEUSSPUNLOCK;
	  return(nwrds);
	}
      else 
	{  /* Error in DMA */
	  
	  fprintf( stderr,"%s: ERROR: vmeDmaDone returned an Erro\n",__FUNCTION__);
	  BEUSSPUNLOCK;
	  return(retVal>>2);	  
	}
    }

  BEUSSPUNLOCK;

  return OK;
}


/*******************************************************************************
 *
 *  beusspSetSampleBlock
 *  - Routine to set the number of DREAM samples per event and the number of events per block
 *
 */

int beusspSetSampleBlock(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, unsigned int NbOfSamples, unsigned int NbOfEventsPerBlock)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
  
  if(NbOfSamples<=0 || NbOfSamples>0x0000ffff)
    {
      fprintf( stderr,"%s: ERROR: NbOfSamples is out of range (0x%08x)\n",
	     __FUNCTION__,NbOfSamples);
      return ERROR;
    }
    
  if(NbOfEventsPerBlock<=0 || NbOfEventsPerBlock>0x0000ffff)
    {
      fprintf( stderr,"%s: ERROR: NbOfEventsPerBlock is out of range (0x%08x)\n",
	     __FUNCTION__,NbOfEventsPerBlock);
      return ERROR;
    }
    

  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_3, (NbOfSamples << 16) |  NbOfEventsPerBlock	);

  BEUSSPUNLOCK;

  return OK;
}




/*******************************************************************************
 *
 *  beusspSetSBTSource
 *  - Routine to configure the trigger source, the busy type and the Sync source
 *		int TrgSrc; //-- 0 - soft; 1-Internal periodic; 2-external Trg1 of TI; 3 - external Trg2 of TI
	int BsySrc; //-- 0 - No busy; 1-In Busy constantly for test; 2-Internal TrigFifo; 3 - FEU bUsy; 4 - 2 or 3
	int SyncSrc; //-- 0 - Soft; 1-Ti Synch; governs RstEvtCntr, RstTstp end ReSync synchronous commands
	
 */

int beusspSetSBTSource(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, unsigned int TrgSrc, unsigned int BsySrc, unsigned int SyncSrc)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  if(TrgSrc< 0 || TrgSrc>3)
    {
      fprintf( stderr,"%s: ERROR: TrgSrc is out of range (0x%08x)\n",
	     __FUNCTION__,TrgSrc);
      return ERROR;
    } 

 /* if(BsySrc< 0 || BsySrc>4)
    {
      fprintf( stderr,"%s: ERROR: BsySrc is out of range (0x%08x)\n",
	     __FUNCTION__,BsySrc);
      return ERROR;
    } 
*/

  if(SyncSrc< 0 || SyncSrc>1)
    {
      fprintf( stderr,"%s: ERROR: SyncSrc is out of range (0x%08x)\n",
	     __FUNCTION__,SyncSrc);
      return ERROR;
    } 

  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_5, (BsySrc << 24) |  (SyncSrc << 16) |  (TrgSrc << 8)	  );

  BEUSSPUNLOCK;

  return OK;
}
/*******************************************************************************
 *
 *  beusspSetActiveLinks
 *  - Routine to set the active links register
 *	
 */

int beusspSetActiveLinks(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, unsigned int rol_enb)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_0, rol_enb );

  BEUSSPUNLOCK;

  return OK;
}
/*******************************************************************************
 *
 *  beusspSetNearEndLoopBack
 *  - Routine to set the register used to configure links in near end loop back
 *	
 */

int beusspSetNearEndLoopBack(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, unsigned int near_end_loop_enb)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_C, near_end_loop_enb );

  BEUSSPUNLOCK;

  return OK;
}
/*******************************************************************************
 *
 *  beusspSetSoftId
 *  - Routine to set the beussp soft ID register, included in the data blocks
 *	
 */

int beusspSetSoftId(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, unsigned int soft_id)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_11,  soft_id );

  BEUSSPUNLOCK;

  return OK;
}

/*******************************************************************************
 *
 *  beusspSetBypassEmulator
 *  - Routine to set the register used to mask the FEU emulator on the input data links 
 *	
 */

int beusspSetBypassEmulator(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, unsigned int feu_emu_msk)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_1, feu_emu_msk);

  BEUSSPUNLOCK;

  return OK;
}
/*******************************************************************************
 *
 *  beusspSetEmuDatasize
 *  - Routine to set the register used to mask the FEU emulator on the input data links 
 *	
 */
int beusspSetEmuDatasize(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, unsigned int emu_data_size)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_B, emu_data_size);

  BEUSSPUNLOCK;

  return OK;
}
/*******************************************************************************
 *
 *  beusspSetTrgFifoThresh
 *  - Routine to set the register used to mask the FEU emulator on the input data links 
 *	
 */
int beusspSetTrgFifoThresh(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, unsigned int TrgFifo_Lwm,  unsigned int TrgFifo_Hwm, unsigned int TrgFifo_Full)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  if(TrgFifo_Lwm<=0 || TrgFifo_Lwm>0x0000ffff)
    {
      fprintf( stderr,"%s: ERROR: TrgFifo_Lwm is out of range (0x%08x)\n",
	     __FUNCTION__,TrgFifo_Lwm);
      return ERROR;
    }
    
  if(TrgFifo_Hwm<=0 || TrgFifo_Hwm>0x0000ffff)
    {
      fprintf( stderr,"%s: ERROR: TrgFifo_Hwm is out of range (0x%08x)\n",
	     __FUNCTION__,TrgFifo_Hwm);
      return ERROR;
    }
    
  if(TrgFifo_Full<=0 || TrgFifo_Full>0x0000ffff )
    {
      fprintf( stderr,"%s: ERROR: TrgFifo_Full is out of range (0x%08x)\n",
	     __FUNCTION__,TrgFifo_Full);
      return ERROR;
    }
    
  if ( ! ( (TrgFifo_Full > TrgFifo_Hwm ) || ( TrgFifo_Lwm < TrgFifo_Hwm ) ) )
    {
      fprintf( stderr,"%s: ERROR: invalid trigger fifo threshold values  (0x%08x 0x%08x 0x%08x)\n",
	     __FUNCTION__,TrgFifo_Lwm,TrgFifo_Hwm,TrgFifo_Full);
      return ERROR;
    }
    
    
    
  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_D,(TrgFifo_Lwm << 16) |  TrgFifo_Hwm	 );
  vmeWrite32(&BEUSSPreg->regin_E, TrgFifo_Full);

  BEUSSPUNLOCK;

  return OK;
}
/*******************************************************************************
 *
 *  beusspSetConcentratorTimeout
 *  - Routine to set the value of the data concentrator time out register
 *	
 */
int beusspSetConcentratorTimeout(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, unsigned int timeout)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
 /* if(timeout<=0 || timeout>0x0000ffff)
    {
      fprintf( stderr,"%s: ERROR: timeout is out of range (0x%08x)\n",
	     __FUNCTION__,timeout);
      return ERROR;
    }
  */  

  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_F, timeout);

  BEUSSPUNLOCK;

  return OK;
}
/*******************************************************************************
 *
 *  beusspEnSyncCom
 *  - Routine to enable synchronous commands 
 *	
 */
int beusspEnSyncCom(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_7, 0x00000001);			// all other control bits in this register will be set to 0

  BEUSSPUNLOCK;

  return OK;
}
/*******************************************************************************
 *
 *  beusspDisSyncCom
 *  - Routine to enable synchronous commands 
 *	
 */
int beusspDisSyncCom(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_7, 0x00000000);			// all other control bits in this register will be set to 0

  BEUSSPUNLOCK;

  return OK;
}
/*******************************************************************************
 *
 *  beusspEnableTriggerSource
 *  - Routine to enable triggers
 *	
 */
int beusspEnableTriggerSource(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_7, 0x00000003);			

  BEUSSPUNLOCK;

  return OK;
}
/*******************************************************************************
 *
 *  beusspDisableTriggerSource
 *  - Routine to disable triggers
 *	
 */
int beusspDisableTriggerSource(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->regin_7, 0x00000001);			

  BEUSSPUNLOCK;

  return OK;
}






/*******************************************************************************
 *
 *  beusspVmeResyncRtsRec
 *  - Routine to configure the clock 4x4 cross switch 
 *	
 */
int beusspVmeResyncRtsRec(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg)
{
unsigned int regin;

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  BEUSSPLOCK;

		regin = vmeRead32( &BEUSSPreg->regin_5  );		
		//Re Sync
		vmeWrite32( &BEUSSPreg->regin_5, ( regin & 0xFFFFFF00 ) | 0x00000000);
		vmeWrite32( &BEUSSPreg->regin_5, ( regin & 0xFFFFFF00 ) | 0x00000040);
		vmeWrite32( &BEUSSPreg->regin_5, ( regin & 0xFFFFFF00 ) | 0x00000000);
		usleep(50);
		//reset time stamp 
		vmeWrite32( &BEUSSPreg->regin_5, ( regin & 0xFFFFFF00 ) | 0x00000000);
		vmeWrite32( &BEUSSPreg->regin_5, ( regin & 0xFFFFFF00 ) | 0x00000002);
		vmeWrite32( &BEUSSPreg->regin_5, ( regin & 0xFFFFFF00 ) | 0x00000000);
		usleep(50);

		//reset event counter
		vmeWrite32( &BEUSSPreg->regin_5, ( regin & 0xFFFFFF00 ) | 0x00000000);
		vmeWrite32( &BEUSSPreg->regin_5, ( regin & 0xFFFFFF00 ) | 0x00000001);
		vmeWrite32( &BEUSSPreg->regin_5, ( regin & 0xFFFFFF00 ) | 0x00000000);
		usleep(50);

  BEUSSPUNLOCK;

  return OK;
}



/*******************************************************************************
 *
 *  beusspSetClkSrc
 *  - Routine to configure the clock 4x4 cross switch 
 *	
 */
int beusspSetClkSrc(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, unsigned int ClkSrc)
{
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
  
  if(ClkSrc == 0) {  
  printf("Using BEUSSP 250 MHz on board oscillator as global clock ( 0, 1, 2) and MGT clock.\n\r");
  BEUSSPLOCK;
		//select output 0 and input 3 ( local clock) 
		vmeWrite32( &BEUSSPreg->regin_A, 0x00000003);
	 	//load bit high
		vmeWrite32( &BEUSSPreg->regin_A, 0x00000023);
	 	//load bit low
		vmeWrite32( &BEUSSPreg->regin_A, 0x00000003);
	 	//configure bit high
		vmeWrite32( &BEUSSPreg->regin_A, 0x00000013);
	 	//configure bit low
		vmeWrite32( &BEUSSPreg->regin_A, 0x00000003);
		
		//select output 1 and input 3
		vmeWrite32( &BEUSSPreg->regin_A, 0x00000007);
	 	//load bit high
		vmeWrite32( &BEUSSPreg->regin_A, 0x00000027);
	 	//load bit low
		vmeWrite32( &BEUSSPreg->regin_A, 0x00000007);
	 	//configure bit high
		vmeWrite32( &BEUSSPreg->regin_A, 0x00000017);
	 	//configure bit low
		vmeWrite32( &BEUSSPreg->regin_A, 0x00000007);

		//select output 2 and input 3
		vmeWrite32( &BEUSSPreg->regin_A, 0x0000000B);
	 	//load bit high
		vmeWrite32( &BEUSSPreg->regin_A, 0x0000002B);
	 	//load bit low
		vmeWrite32( &BEUSSPreg->regin_A, 0x0000000B);
	 	//configure bit high
		vmeWrite32( &BEUSSPreg->regin_A, 0x0000001B);
	 	//configure bit low
		vmeWrite32( &BEUSSPreg->regin_A, 0x0000000B);

		//select output 3 and input 3
		vmeWrite32( &BEUSSPreg->regin_A, 0x0000000F);
	 	//load bit high
		vmeWrite32( &BEUSSPreg->regin_A, 0x0000002F);
	 	//load bit low
		vmeWrite32( &BEUSSPreg->regin_A, 0x0000000F);
	 	//configure bit high
		vmeWrite32( &BEUSSPreg->regin_A, 0x0000001F);
	 	//configure bit low
		vmeWrite32( &BEUSSPreg->regin_A, 0x0000000F);
  BEUSSPUNLOCK;
  return OK;};

  if(ClkSrc == 1) {  
  printf("Using VXS SWB TI clock as global clock ( 0, 1, 2)  and MGT clock.\n\r");
  BEUSSPLOCK;
		//select output 0 and input 1 (SWB CLK from TI through SD) 
		vmeWrite32(&BEUSSPreg->regin_A,0x00000001);
	 	//load bit high
		vmeWrite32(&BEUSSPreg->regin_A, 0x00000021);
	 	//load bit low
		vmeWrite32(&BEUSSPreg->regin_A, 0x00000001);
	 	//configure bit high
		vmeWrite32(&BEUSSPreg->regin_A, 0x00000011);
	 	//configure bit low
		vmeWrite32( &BEUSSPreg->regin_A, 0x00000001);
		
		//select output 1 and input 1
		vmeWrite32(&BEUSSPreg->regin_A, 0x00000005);
	 	//load bit high
		vmeWrite32(&BEUSSPreg->regin_A, 0x00000025);
	 	//load bit low
		vmeWrite32(&BEUSSPreg->regin_A,0x00000005);
	 	//configure bit high
		vmeWrite32(&BEUSSPreg->regin_A,0x00000015);
	 	//configure bit low
		vmeWrite32(&BEUSSPreg->regin_A,0x00000005);

		//select output 2 and input 1
		vmeWrite32(&BEUSSPreg->regin_A,0x00000009);
	 	//load bit high
		vmeWrite32(&BEUSSPreg->regin_A,0x00000029);
	 	//load bit low
		vmeWrite32(&BEUSSPreg->regin_A,0x00000009);
	 	//configure bit high
		vmeWrite32(&BEUSSPreg->regin_A,0x00000019);
	 	//configure bit low
		vmeWrite32(&BEUSSPreg->regin_A,0x00000009);

		//select output 3 and input 1
		vmeWrite32(&BEUSSPreg->regin_A,0x0000000D);
	 	//load bit high
		vmeWrite32(&BEUSSPreg->regin_A,0x0000002D);
	 	//load bit low
		vmeWrite32(&BEUSSPreg->regin_A,0x0000000D);
	 	//configure bit high
		vmeWrite32(&BEUSSPreg->regin_A,0x0000001D);
	 	//configure bit low
		vmeWrite32(&BEUSSPreg->regin_A,0x0000000D);
  BEUSSPUNLOCK;
  return OK;};

  if(ClkSrc > 1)
  	{
      fprintf( stderr,"%s: ERROR: invalid specified clock configuration \n",__FUNCTION__);
      return ERROR;
    };

  
return OK;
}


/*******************************************************************************
 *
 *  beusspFlashSatus
 *  - Routine to read the FLASH status register 
 *  - Command sent to FLASH is  0xD7 = 0b11010111
 *	
 */
int beusspFlashSatus(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg)
{

int retval = 0 ;

  if(BEUSSPreg == NULL) 
    {
     fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  BEUSSPLOCK;
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );	 // CLK LOW
				vmeWrite32( &BEUSSPreg->regin_14,  0xFFFFFFFF );	 // CSB HIGH - should be high already
				vmeWrite32( &BEUSSPreg->regin_14,  0x00000000 );	 // CSB LOW
				
				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );	 // MSB first : 1 
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );   // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF ); 
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );   // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0x00000000 );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );  // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );  // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0x00000000 );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );  // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );  // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );  // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );  // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );
				retval = vmeRead32(&BEUSSPreg->regout_1C);		// read output MSB which was pulled out of the FLASH on the falling clock edge

				
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );  // CLK LOW
				vmeWrite32( &BEUSSPreg->regin_14,  0xFFFFFFFF );  // CSB HIGH
	

  BEUSSPUNLOCK;

  return retval;
}

/*******************************************************************************
 *
 *  beusspFlashSatusReg
 *  - Routine to read the FLASH status register 
 *  - Command sent to FLASH is  0xD7 = 0b11010111
 *	
 */
int beusspFlashSatusReg(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg)
{

int retval = 0 ;
int retval_bis = 0 ;


  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  BEUSSPLOCK;
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );	 // CLK LOW
				vmeWrite32( &BEUSSPreg->regin_14,  0xFFFFFFFF );	 // CSB HIGH - should be high already
				vmeWrite32( &BEUSSPreg->regin_14,  0x00000000 );	 // CSB LOW
				
				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );	 // MSB first : 1 
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );   // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF ); 
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );   // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0x00000000 );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );  // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );  // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0x00000000 );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );  // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );  // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );  // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );  // CLK CYLCE
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );
				retval = vmeRead32(&BEUSSPreg->regout_1C);		// read output MSB which was pulled out of the FLASH on the falling clock edge

				
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );  // CLK CYLCE
				retval_bis = vmeRead32(&BEUSSPreg->regout_1C);
				retval = (retval << 1 ) | ( retval_bis);

				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );  // CLK CYLCE
				retval_bis = vmeRead32(&BEUSSPreg->regout_1C);
				retval = (retval << 1 ) | ( retval_bis);


				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );  // CLK CYLCE
				retval_bis = vmeRead32(&BEUSSPreg->regout_1C);
				retval = (retval << 1 ) | ( retval_bis);


				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );  // CLK CYLCE
				retval_bis = vmeRead32(&BEUSSPreg->regout_1C);
				retval = (retval << 1 ) | ( retval_bis);


				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );  // CLK CYLCE
				retval_bis = vmeRead32(&BEUSSPreg->regout_1C);
				retval = (retval << 1 ) | ( retval_bis);
				retval = (retval << 1 ) | ( retval_bis);


				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );  // CLK CYLCE
				retval_bis = vmeRead32(&BEUSSPreg->regout_1C);
				retval = (retval << 1 ) | ( retval_bis);


				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );  // CLK CYLCE
				retval_bis = vmeRead32(&BEUSSPreg->regout_1C);
				retval = (retval << 1 ) | ( retval_bis);
		
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );  // CLK LOW
				vmeWrite32( &BEUSSPreg->regin_14,  0xFFFFFFFF );  // CSB HIGH
	

  BEUSSPUNLOCK;

  return retval;
}

/*******************************************************************************
 *
 *  beusspFlashID
 *  - Routine to read the FLASH status register 
 *  - Command sent to FLASH is  0x9F = 0b11010111
 *	
 */
int beusspFlashID(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg)
{

int retval = 0 ;
int retval_bis = 0 ;
int ii = 0 ;

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  BEUSSPLOCK;
	
				
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );	 // CLK LOW
				vmeWrite32( &BEUSSPreg->regin_14,  0xFFFFFFFF );	 // CSB HIGH - should be high already
				vmeWrite32( &BEUSSPreg->regin_14,  0x00000000 );	 // CSB LOW
	

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0x00000000 );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0x00000000 );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );

				vmeWrite32( &BEUSSPreg->regin_15,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );
				retval = vmeRead32(&BEUSSPreg->regout_1C);		
						
 			for (ii = 0; ii < 31; ii++){
				vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );
				retval_bis = vmeRead32(&BEUSSPreg->regout_1C);
				retval = (retval << 1 ) | ( retval_bis);
				
			}

				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );  // CLK LOW
				vmeWrite32( &BEUSSPreg->regin_14,  0xFFFFFFFF );  // CSB HIGH


  BEUSSPUNLOCK;

  return retval;
}

/*******************************************************************************
 *
 *  beusspFlashLoadbuffer
 *  - Routine to load one of the two 1056 byte buffers on the Flash
 *	
 */
int beusspFlashLoadbuffer(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, int NumBuf, int data[1056] )
{

int cmd = 0 ;
int ii = 0 ;
int jj = 0 ;
int tmpdata = 0; 

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
  
  if (NumBuf == 1) {cmd = 0x84;} 
  else if (NumBuf == 2) {cmd = 0x87;} 
  else {
      fprintf( stderr,"%s: ERROR: Specified Flash buffer number should be either 1 or 2.\n",__FUNCTION__);
      return ERROR;
    }
    
	
  BEUSSPLOCK;
				
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );	 // CLK LOW
				vmeWrite32( &BEUSSPreg->regin_14,  0xFFFFFFFF );	 // CSB HIGH - should be high already
				vmeWrite32( &BEUSSPreg->regin_14,  0x00000000 );	 // CSB LOW
	
				for (ii = 1; ii <= 8; ii++){
					vmeWrite32( &BEUSSPreg->regin_15, (cmd & 0x00000080) >> 7 );
					vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
					vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );
					cmd  = (cmd << 1 ); 
					}
				
				for (ii = 1; ii <= 1056; ii++){
					
					tmpdata = data[ii];
					
					for (jj = 1; jj <= 8; jj++){
					
					vmeWrite32( &BEUSSPreg->regin_15, (tmpdata & 0x00000080) >> 7 );
					vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
					vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );
					tmpdata  = (tmpdata << 1 ); 
					} 
				}


				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );  // CLK LOW
				vmeWrite32( &BEUSSPreg->regin_14,  0xFFFFFFFF );  // CSB HIGH


  BEUSSPUNLOCK;

  return OK;
}

/*******************************************************************************
 *
 *  beusspFlashBufferToMemory
 *  - Routine to send the command to transfer one of the two 1056 byte buffer contents 
 *  - to the  on the Flash
 *	
 */
int beusspFlashBufferToMemory(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, int NumBuf, int data[1056] )
{

int cmd = 0 ;
int ii = 0 ;


  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
  
  if (NumBuf == 1) {cmd = 0x83;} 
  else if (NumBuf == 2) {cmd = 0x86;} 
  else {
      fprintf( stderr,"%s: ERROR: Specified Flash buffer number should be either 1 or 2.\n",__FUNCTION__);
      return ERROR;
    }
    
	
  BEUSSPLOCK;
				
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );	 // CLK LOW
				vmeWrite32( &BEUSSPreg->regin_14,  0xFFFFFFFF );	 // CSB HIGH - should be high already
				vmeWrite32( &BEUSSPreg->regin_14,  0x00000000 );	 // CSB LOW
	
				for (ii = 1; ii <= 8; ii++){
					vmeWrite32( &BEUSSPreg->regin_15, (cmd & 0x00000080) >> 7 );
					vmeWrite32( &BEUSSPreg->regin_16,  0xFFFFFFFF );
					vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );
					cmd  = (cmd << 1 ); 
					}
				
				vmeWrite32( &BEUSSPreg->regin_16,  0x00000000 );  // CLK LOW
				vmeWrite32( &BEUSSPreg->regin_14,  0xFFFFFFFF );  // CSB HIGH


  BEUSSPUNLOCK;

  return OK;
}


//------------------------------------------------------------------------------------------------
//
//  beusspSetAdr32m - set the beussp A32m register : A32mMin, A32mMax, Rank in token passing scheme
//
//  ARGs: 
//    vmebaseaddr - A24 VME Address of the BEUSSP , should match address set by hardware on beussp board
//    BEUSSPreg   - map of all beussp registers 
//    BEUSSPconf  - structure filled by software containing values for all the configuration registers of the beussp 
//
//  RETURNS: OK if successful, otherwise ERROR.
//
//------------------------------------------------------------------------------------------------
int  beusspSetAdr32m(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, unsigned int A32mMin, unsigned int A32mMax, unsigned int MblkRank){

	unsigned int Adr32m;


  if(BEUSSPreg==NULL)
    {
     fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

	Adr32m = (A32mMin >> 23 ) | (A32mMax >> 7 ) | (MblkRank << 26); 
	
	BEUSSPLOCK;
 		vmeWrite32(&BEUSSPreg->adr32m,  Adr32m );
	BEUSSPUNLOCK;
	
	
	return OK;
}


//------------------------------------------------------------------------------------------------
//
//  beusspInitMblk - Initialize the beussp multiblk readout mode
//
//  ARGs: 
//    vmebaseaddr - A24 VME Address of the BEUSSP , should match address set by hardware on beussp board
//    BEUSSPreg   - map of all beussp registers 
//------------------------------------------------------------------------------------------------
int beusspInitMblk(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg[], volatile unsigned int * * BEUSSPmblk){

	unsigned int laddr,vmeaddr;
	int stat;
	
// Need to verify that all ssp boards involved in the MBLK transfer are correctly configured.
// Do I have to read the SD registers to know which boards are there ?

//Check that the FIRST board is in the right slot
// check that the last is in the right slot

  if(BEUSSPreg[1]==NULL)
    {
     fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

	BEUSSPLOCK;
	vmeaddr =  ( (vmeRead32(&BEUSSPreg[1]->adr32m)) & BEUSSP_ADR32_MBLK_ADDR_MIN_MASK) << 23 ;
	BEUSSPUNLOCK;


    stat = vmeBusToLocalAdrs(0x09,(char *)vmeaddr,(char **)&laddr);
 	if (stat != 0) 
	{
		fprintf( stderr,"%s: ERROR: Error in vmeBusToLocalAdrs stat=%d \n",__FUNCTION__,stat);
		return ERROR;
	} 
 	* BEUSSPmblk = (unsigned int *)(laddr);  

	return OK;
}


//------------------------------------------------------------------------------------------------
int beusspInitMblkTESTMODE(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg, volatile unsigned int * * BEUSSPmblk){

	unsigned int laddr,vmeaddr;
	int stat;
	
// Need to verify that all ssp boards involved in the MBLK transfer are correctly configured.
// Do I have to read the SD registers to know which boards are there ?

//Check that the FIRST board is in the right slot
// check that the last is in the right slot

  if(BEUSSPreg==NULL)
    {
     fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

	BEUSSPLOCK;
	vmeaddr =  ( (vmeRead32(&BEUSSPreg->adr32m)) & BEUSSP_ADR32_MBLK_ADDR_MIN_MASK) << 23 ;
	BEUSSPUNLOCK;


    stat = vmeBusToLocalAdrs(0x09,(char *)vmeaddr,(char **)&laddr);
 	if (stat != 0) 
	{
		fprintf( stderr,"%s: ERROR: Error in vmeBusToLocalAdrs stat=%d \n",__FUNCTION__,stat);
		return ERROR;
	} 
 	* BEUSSPmblk = (unsigned int *)(laddr);  

	return OK;
}




//------------------------------------------------------------------------------------------------
//  beusspEnableA32m - Enable the A32m address space for block readout
//
//  ARGs: 
//       - 
//       - 
//
//  RETURNS: OK if successful, otherwise ERROR.
//
//------------------------------------------------------------------------------------------------
int beusspEnableA32m(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg){
 
 int a32mEnabled=0;

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->adr32m, (  (vmeRead32(&BEUSSPreg->adr32m)) | BEUSSP_ADR32_MBLK_EN_MASK ) );

  a32mEnabled = (vmeRead32(&BEUSSPreg->adr32m)) & BEUSSP_ADR32_MBLK_EN_MASK;
  if(!a32mEnabled)
    {
     fprintf( stderr,"%s: ERROR: Failed to enable A32m Address\n",__FUNCTION__);
      BEUSSPUNLOCK;
      return ERROR;
    }

  BEUSSPUNLOCK;
  return OK;
}

//------------------------------------------------------------------------------------------------
//  beusspDisableA32m - Disable the A32m address space for block readout
//
//  ARGs: 
//       - 
//       - 
//
//  RETURNS: OK if successful, otherwise ERROR.
//
//------------------------------------------------------------------------------------------------
int beusspDisableA32m(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg){
 
 int a32mEnabled=0;

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->adr32m, (  (vmeRead32(&BEUSSPreg->adr32m)) & BEUSSP_ADR32_MBLK_EN_MASK_COMP ) );

  a32mEnabled = (vmeRead32(&BEUSSPreg->adr32m)) & BEUSSP_ADR32_MBLK_EN_MASK;
  if(a32mEnabled)
    {
     fprintf( stderr,"%s: ERROR: Failed to disable A32m Address\n",__FUNCTION__);
      BEUSSPUNLOCK;
      return ERROR;
    }

  BEUSSPUNLOCK;
  return OK;
}


//------------------------------------------------------------------------------------------------
//  beusspTakeToken - Tell board to take token. This function assumes the board is first in the chain
//						and does not check rank.
//  ARGs: 
//       - 
//       - 
//
//  RETURNS: OK if successful, otherwise ERROR.
//
//------------------------------------------------------------------------------------------------
int beusspTakeToken(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg){
 
 int status=0;

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;

  vmeWrite32(&BEUSSPreg->adr32m, (  (vmeRead32(&BEUSSPreg->adr32m)) | BEUSSP_ADR32_MBLK_TAKE_MASK ) );

  status = (vmeRead32(&BEUSSPreg->adr32m)) & BEUSSP_ADR32_MBLK_STATUS_MASK ;
  if(!status)
    {
     fprintf( stderr,"%s: ERROR: Failed to take token\n",__FUNCTION__);
      BEUSSPUNLOCK;
      return ERROR;
    }

  BEUSSPUNLOCK;
  return OK;
}

//------------------------------------------------------------------------------------------------
//  beusspEnBerrTerm - Enables Bus Error Termination of VME Block and MultiBlock read
//  ARGs: 
//       - 
//       - 
//
//  RETURNS: OK if successful, otherwise ERROR.
//
//------------------------------------------------------------------------------------------------
int beusspEnBerrTerm(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg){
 
 int status=0;

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;
     	vmeWrite32( &BEUSSPreg->tokenin_cfg,  0x00000001 );		//ENABLE BERR TERMINATION			
  		status = (vmeRead32(&BEUSSPreg->tokenin_cfg)) & 0x00000001;
  		
  if(!status)
    {
     fprintf( stderr,"%s: ERROR: Failed to enable bus error termination \n",__FUNCTION__);
      BEUSSPUNLOCK;
      return ERROR;
    }

  BEUSSPUNLOCK;
  return OK;
}
//------------------------------------------------------------------------------------------------
//  beusspDisBerrTerm - Disables Bus Error Termination of VME Block and MultiBlock read
//  ARGs: 
//       - 
//       - 
//
//  RETURNS: OK if successful, otherwise ERROR.
//
//------------------------------------------------------------------------------------------------
int beusspDisBerrTerm(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg){
 
 int status=0;

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;
     	vmeWrite32( &BEUSSPreg->tokenin_cfg,  0x00000000 );		//Disable BERR TERMINATION			
  		status = (vmeRead32(&BEUSSPreg->tokenin_cfg)) & 0x00000001;
  		
  if(status)
    {
     fprintf( stderr,"%s: ERROR: Failed to disable bus error termination \n",__FUNCTION__);
      BEUSSPUNLOCK;
      return ERROR;
    }

  BEUSSPUNLOCK;
  return OK;
}
//------------------------------------------------------------------------------------------------
//  beusspGetTokenStatus - Returns board status with respect to token passing
//  ARGs: 
//       - 
//       - 
//
//  RETURNS: OK if successful, otherwise ERROR.
//
//------------------------------------------------------------------------------------------------
int beusspGetTokenStatus(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg){
 
 int status=0;

  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  BEUSSPLOCK;
  	status = (vmeRead32(&BEUSSPreg->adr32m)) & BEUSSP_ADR32_MBLK_STATUS_MASK ;
  BEUSSPUNLOCK;
  return status;
}


/*******************************************************************************
 *
 * beusspTokenReadBlock - Read a block of events from the BEUSSP
 *
 *    data  - local memory address to place data
 *    nwrds - Max number of words to transfer
 *    rflag - Readout Flag
 *              0 - programmed I/O from the specified board
 *              1 - DMA transfer using Universe/Tempe DMA Engine 
 *                    (DMA VME transfer Mode must be setup prior)
 *
 * RETURNS: Number of words transferred to data if successful, ERROR otherwise
 *
 */

int beusspTokenReadBlock(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg,  volatile unsigned int *BEUSSPmblk, volatile unsigned int * data, unsigned int nwrds, int rflag)
{
  int retVal, nbwords;
  volatile unsigned int *laddr;
	unsigned int vmeAdr;
	unsigned int a32mblk;

	//need to check for all beussp involved in the readout 
	//check that the BusErr occured on the LAST board and not for some other reason
	//Check that the first board has the token
	//check that boards involved are correctly enbaled fro block transfer.
	
	//Does this mean that I have to change the parameters to this function too ??
	
  if(BEUSSPreg==NULL)
    {
     fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }

  if(BEUSSPmblk==NULL)
    {
      fprintf( stderr,"%s: ERROR: BEUSSP A32 not initialized\n",__FUNCTION__);
      return ERROR;
    }

  if(data==NULL) 
    {
          fprintf( stderr,"%s: ERROR: Invalid Destination address\n",__FUNCTION__);
      return(ERROR);
    }

  BEUSSPLOCK;
  if(rflag >= 1)
    { /* Block transfer */

      /* Assume that the DMA programming is already setup. 
	 Don't Bother checking if there is valid data - that should be done prior
	 to calling the read routine */
	 
	laddr = data; 
	     
	a32mblk = vmeRead32( &BEUSSPreg->adr32m );					// c'est quand meme bete de ne pas passer l'adresse directement en argument 
											//au lieu de recalculer à chaque transfer!
	vmeAdr = (unsigned int)  ( a32mblk & BEUSSP_ADR32_MBLK_ADDR_MIN_MASK) << 23 ;

/*	retVal = vmeDmaSend          */
        retVal = usrVme2MemDmaStart(vmeAdr, (unsigned int *)laddr, (nwrds << 2));
	
	
      if(retVal |= 0) 
	{
	  fprintf( stderr,"%s: ERROR in DMA transfer Initialization 0x%x\n",__FUNCTION__, retVal);
	  BEUSSPUNLOCK;
	  return(retVal);
	}

      /* Wait until Done or Error */
     /* retVal = vmeDmaDone();*/
        retVal = usrVme2MemDmaDone();
	
      if(retVal > 0)
	{
	  nbwords = ((retVal>>2) ); /* Number of longwords transfered */
	  BEUSSPUNLOCK;
	  return(nbwords);
	}
      else if (retVal == 0) 
	{
	  fprintf( stderr,"%s: WARN: DMA transfer returned zero word count 0x%x\n",__FUNCTION__, retVal);
	  BEUSSPUNLOCK;
	  return(retVal);
	}
      else 
	{  /* Error in DMA */
	  
	  fprintf( stderr,"%s: ERROR: vmeDmaDone returned an Erro\n",__FUNCTION__);
	  BEUSSPUNLOCK;
	  return(retVal>>2);	  
	}
    }

  BEUSSPUNLOCK;

  return OK;
}

/*******************************************************************************
 *
 * beusspCheckFeuLink - check that the gigabit link to a FEU is active 
 *
 *     - 
 *     - 
 *     
 *         
 * RETURNS: -1 : bad parameters, 1 : link is up and ready, 0 - link is not 
 * (RETURNS: OK if successful, otherwise ERROR)
 *
 */

int beusspCheckFeuLink( volatile struct BEUSSP_A24RegStruct *BEUSSPreg, int numFeu)
{

  unsigned int feu_is_there = 0;
  

  if(BEUSSPreg==NULL)
    {
     fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return -1;
    }
  
  if( (numFeu < 0 ) || (numFeu >=0x0000001f))
    {
      fprintf( stderr,"%s: ERROR: target Feu is out of range (0 to 31) \n",__FUNCTION__);
      return -1;
    }

	BEUSSPLOCK;

	feu_is_there = vmeRead32(  &(BEUSSPreg->regout_11)     );

	BEUSSPUNLOCK;
	
	if (((feu_is_there >> numFeu) & 0x00000001 ) != 1 )  
		{
		return 0 ;
		}   
	else 
		{
		return 1;
 		}	
}
/*******************************************************************************
 *
 *  beusspBReady
 *  - Routine to check if a block is ready for readout 
 *	
 */
int beusspBReady(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg)
{
  unsigned int readme = 0;
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  BEUSSPLOCK;
  readme = vmeRead32( &(BEUSSPreg->regout_F) );
  BEUSSPUNLOCK;

  return readme;
}
/*******************************************************************************
 *
 *  beusspBZRDHigh
 *  - Routine to set high the Busy Reading bit
 *	- WILL CAUSE AN ERROR IF TRYING TO SET HIGH WHEN ALREADY HIGH
 */
int beusspBZRDHigh(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg)
{
  unsigned int bzrd = 0;
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
   
  BEUSSPLOCK;
  bzrd = vmeRead32( &(BEUSSPreg->regin_4) );
  if(bzrd == 0x00000001) 
    {
      fprintf( stderr,"%s: ERROR: Trying to mark busy a BEUSSP already in busy state !\n",__FUNCTION__);
      BEUSSPUNLOCK;
      return ERROR;
    } 
  
  vmeWrite32( &(BEUSSPreg->regin_4),  0x00000001 );
  BEUSSPUNLOCK;
  return bzrd;
}
/*******************************************************************************
 *
 *  beusspBZRDLow
 *  - Routine to set low the Busy Reading bit
 *	- WILL CAUSE AN ERROR IF TRYING TO SET LOW WHEN ALREADY LOW
 */
int beusspBZRDLow(volatile struct BEUSSP_A24RegStruct  *BEUSSPreg)
{
  unsigned int bzrd = 0;
  if(BEUSSPreg == NULL) 
    {
      fprintf( stderr,"%s: ERROR: BEUSSP not initialized\n",__FUNCTION__);
      return ERROR;
    }
    
  BEUSSPLOCK;
  bzrd = vmeRead32( &(BEUSSPreg->regin_4) );
  if(bzrd == 0x00000000) 
    {
      fprintf( stderr,"%s: ERROR: Trying to mark NOT BUSY a BEUSSP already in NOT BUSY state !\n",__FUNCTION__);
      BEUSSPUNLOCK;
      return ERROR;
    } 
  
  vmeWrite32( &(BEUSSPreg->regin_4),  0x00000000 );
  BEUSSPUNLOCK;
  return bzrd;
}





