#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "TiConfigParams.h"

#include "SysConfig.h"


#include "sdLib.h"


#include "jvme.h"
#include "beusspLib.h"
#include "BeuConfig.h"
#include "tiLib.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "ReturnCodes.h"
#include "SysConfigParams.h"
#include "SysConfig.h"

/* lib pour test getc non bloquant */
#include <sys/types.h>
#include <fcntl.h>
 
// GLOBAL EXTERNAL VARIABLES 
DMA_MEM_ID vmeIN,vmeOUT;
extern DMANODE *the_event;
//extern unsigned int *dma_dabufp;
unsigned int *dma_dabufp;
unsigned int i2_from_rol1;


extern char *optarg;
extern int   optind;

// For the moment an ugly declaration
extern SysParams *sys_params_ptr;					// DEFINED IN SysConfig.h



// Default values
#define DEF_ConfFileName        "Sys.cfg"

// Global variables
int verbose = 0; // if >0 some debug output



/*
 * Usage function
 */
void usage( char *name )
{
	printf( "\nUsage: %s", name );
	printf( " [-c Conf_FileName]" );
	printf( " [-v [-v]]" );
	printf( " [-h]" );
	printf( "\n" );
	
	printf( "\n" );
	printf( "-c Conf_FileName        - name for config file; default: %s; \"None\" no file consulted\n", DEF_ConfFileName );
	printf( "-v [-v]                 - forces debug output\n" );
	printf( "-h                      - help\n" );

	printf( "  Get this text:\n"); 
	printf( "   %s -h\n\n", name);
}

/*
 * Cleanup function
 */
void cleanup(int param)
{
	if( param )
	{
		fprintf(stderr, "cleanup: Entering with %d\n", param);
	}
	if( verbose )
		printf( "cleanup: Entering with %d\n", param );

	// Close mamory configuration file if any open
	SysConfig_CleanUp();

	if( verbose )
		printf( "cleanup: Exiting\n" );
	exit( param );
}

/*
 * Signal handler
 */
void sig_hndlr( int param )
{
	cleanup(param);
}



// ----------------------------------------------------------------
// --  Main
// ----------------------------------------------------------------
int main(void ) 
{

		char progname[128];

		// Parameters
		char conf_file_name[128];

		int ret;

//volatile        unsigned int         *BEUSSPmblk=NULL;  	/* pointer to BEUSSP multi block adress window*/
/* Global Variables */
//int beusspA32Base_0  =0x09000000;                   /* Minimum VME A32 Address for use by BEUSSP */
//int beusspA32Base_1  =0x0A000000;                   /* Minimum VME A32 Address for use by BEUSSP */
//TI has address 0x08000000;
// need to build an adresse from the slot number
// need to have all a32 addresses within the specified default a32 window width.

 int  cargc = 0;
 char * cargv[MAX_ARGC];
 int len;
 char line[128];

 char cc=0;
 int n = 0;
 int tem = 0;
 
 unsigned int size=0;
 unsigned int counter = 0;

 int MAX_BLOCK_SIZE =0x300000;
 //int MAX_BLOCK_SIZE =0x4000;

 unsigned int data1,data2,data3,data4,data5,data6,data7,data8, data_1;
 unsigned int numFeu, numBeu, nbbytes ;	

 unsigned int blocksize_0;  
 unsigned int blocksize_1; 
 unsigned int readme_0, readme_1, tireadme,tokenstatus; 
 unsigned int nb_blocks_h, nb_blocks_l;
 unsigned int nb_events_h, nb_events_l;				
 unsigned int nb_samples_h, nb_samples_l;				
 unsigned int nb_filler_h, nb_filler_l;				
 unsigned int nb_eob0_h, nb_eob0_l;				
 unsigned int nb_eob1_h, nb_eob1_l, nb_eob;	
 unsigned int nb_parity_l, nb_parity_h;	

 unsigned int nb_words_in_block, num_word_in_block,result;

 double deltatime;
 
 
 FILE * fp;

  int dCnt;
  DMANODE *outEvent;
  unsigned int beusspIntCnt=0;
  
	int run = 1;
	int do_post_reads = 0;

 printf("\n IRFU BEUSSP Library Test\n");
 printf("----------------------------\n");

 //vmeOpenDefaultWindows();

//// stat = vmeOpenDefaultWindows();
////  if(stat != OK) goto CLOSE;

 	/***********************/
	/* Set signal hendler  */
	/***********************/
	signal( SIGABRT, sig_hndlr);
	signal( SIGFPE,  sig_hndlr);
	signal( SIGILL,  sig_hndlr);
	signal( SIGINT,  sig_hndlr);
	signal( SIGSEGV, sig_hndlr);
	signal( SIGTERM, sig_hndlr);
	if( verbose )
		printf( "%s: signal handler set\n", progname );

	// ----------------------------------------------------------------
		// ----------------------------------------------------------------
			// ----------------------------------------------------------------
 
 printf("-- Entering main() --\r\n");

 
 handle_help();
 printf(PROMPT);
 for (;;) {
	//get a string from stdin	
	my_get_line(line, sizeof(line));
	len = strlen(line);
	if (line[len-1] == '\n') line[len-1] = 0;
	tokenize(line, &cargc, cargv, MAX_ARGC);
	// ----------------------------------------------------------------
	if (cargc == 0) {
		printf(PROMPT);
		continue;
	}
	// ----------------------------------------------------------------
	if (strcmp(cargv[0], "quit") == 0) {
		printf("Closing ... \r\n");
		break;
	}	
	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "readall") == 0) {
	
  				beusspDisplayAllReg(beu_reg_control[1]);
  				printf("\r\n");
  				beusspDisplayAllReg(beu_reg_control[2]);
				printf("\r\n");
				printf(PROMPT);					
				
			}
	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "resetgtx") == 0) {

				beusspResetMultiGTX(beu_reg_control[1]);
				beusspResetMultiGTX(beu_reg_control[2]);
 
				printf(PROMPT);	
			}	

	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "resetpll") == 0) {

				beusspResetGClkPll(beu_reg_control[1]);
				beusspResetGClkPll(beu_reg_control[2]);
				
				printf(PROMPT);	
			}
	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "flush") == 0) {

  				beusspFlushPipeline(beu_reg_control[1]);
  				beusspFlushPipeline(beu_reg_control[2]);

				printf(PROMPT);	
			}	
			
	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "tisync") == 0) {

				beusspEnSyncCom(beu_reg_control[1]);
				beusspEnSyncCom(beu_reg_control[2]);
				tiEnableVXSSignals();
				taskDelay(1);
				tiSyncReset(0);
				taskDelay(1);	
				tiStatus(1);		
				printf(PROMPT);	
			}

		
			
	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "cfg") == 0) {
	
		// Initialization
			verbose =  1;
			sprintf(progname, "%s", cargv[0] );
			
			if ( cargc == 1) { 	
				sprintf(conf_file_name, DEF_ConfFileName );	
				}
			else { 
				sprintf(conf_file_name, cargv[1] );	
				}

		
		if( verbose )
		{
			printf( "conf_file_name          = %s\n",    conf_file_name );
			printf( "verbose                 = %d\n",    verbose );
		}
			
			if( (ret=SysConfigFromFile( conf_file_name )) != D_RetCode_Sucsess )
			{
				fprintf( stderr, "%s: SysConfigFromFile failed for file %s with %d\n", progname, conf_file_name, ret );
			}
			if( verbose )
			printf( "%s: SysConfig OK\n", progname );

				
	
	  /* Setup Address and data modes for DMA transfers
   *   
   *  vmeDmaConfig(addrType, dataType, sstMode);
   *
   *  addrType = 0 (A16)    1 (A24)    2 (A32)
   *  dataType = 0 (D16)    1 (D32)    2 (BLK32) 3 (MBLK) 4 (2eVME) 5 (2eSST)
   *  sstMode  = 0 (SST160) 1 (SST267) 2 (SST320)
   */
 /* vmeDmaConfig(2,5,1); */
//usrVmeDmaSetConfig(2,5,1);


{
    int i1, i2, i3;

    usrVmeDmaInit();

    usrVmeDmaMemory(&i1, &i2, &i3);
    i2_from_rol1 = i2;
    printf("tiprimarytinit: i2_from_rol1 = 0x%08x\n",i2_from_rol1);
    i2_from_rol1 = (i2_from_rol1 & 0xFFFFFFF0);
    printf("tiprimarytinit: i2_from_rol1 = 0x%08x\n",i2_from_rol1);
    i2_from_rol1 = i2_from_rol1 + 0x10;
    printf("tiprimarytinit: i2_from_rol1 = 0x%08x\n",i2_from_rol1);
  }



  /* dma */
  usrVmeDmaSetConfig(2,5,1); /*A32,MBLT*/
  printf("!!!!!!!!! set DMA as A32,MBLT\n");

  dma_dabufp=(unsigned int *)i2_from_rol1;
  // static unsigned int *tdcbuf;





  /* Initialize the event buffers (required for DMA) */
  
//dmaPFreeAll();
//  vmeIN  = dmaPCreate("vmeIN",MAX_BLOCK_SIZE,50,0);
//  vmeOUT = dmaPCreate("vmeOUT",0,0,0);
    
//  dmaPStatsAll();
//  dmaPReInitAll();

	//After tiInit, some default values may need to be corrected to fit specific needs.
	// tiDisableDataReadout();
	// tiDisableA32();

	// if TI fifos fill up it generates a busy that blocks triggers.
	// if I want to use TI as a trigger source, I have to block all busy sources.
	//tiSetBusySource(0,1);
	//tiSetBusySource(TI_BUSY_SWB,1);		
	//tiSetBusySource(TI_BUSY_LOOPBACK | TI_BUSY_SWB,1);
	//tiSetBlockLimit(0);		
	//tiSetBlockBufferLevel(5);	
	printf(PROMPT);	
	}

	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "tkcfg") == 0) {
	

		// Initialization
			verbose =  1;
		
			sprintf(progname, "%s", cargv[0] );

			if ( cargc == 1) { 	
				sprintf(conf_file_name, DEF_ConfFileName );	
				}
			else { 
				sprintf(conf_file_name, cargv[1] );	
				}

		
		if( verbose )
		{
			printf( "conf_file_name          = %s\n",    conf_file_name );
			printf( "verbose                 = %d\n",    verbose );
		}
			
			if( (ret=SysConfigFromFile( conf_file_name )) != D_RetCode_Sucsess )
			{
				fprintf( stderr, "%s: SysConfigFromFile failed for file %s with %d\n", progname, conf_file_name, ret );
			}
			if( verbose )
			printf( "%s: SysConfig OK\n", progname );

				
	
	  /* Setup Address and data modes for DMA transfers
   *   
   *  vmeDmaConfig(addrType, dataType, sstMode);
   *
   *  addrType = 0 (A16)    1 (A24)    2 (A32)
   *  dataType = 0 (D16)    1 (D32)    2 (BLK32) 3 (MBLK) 4 (2eVME) 5 (2eSST)
   *  sstMode  = 0 (SST160) 1 (SST267) 2 (SST320)
   */
/*  vmeDmaConfig(2,5,1); */
//usrVmeDmaSetConfig(2,5,1);


{
    int i1, i2, i3;

    usrVmeDmaInit();

    usrVmeDmaMemory(&i1, &i2, &i3);
    i2_from_rol1 = i2;
    printf("tiprimarytinit: i2_from_rol1 = 0x%08x\n",i2_from_rol1);
    i2_from_rol1 = (i2_from_rol1 & 0xFFFFFFF0);
    printf("tiprimarytinit: i2_from_rol1 = 0x%08x\n",i2_from_rol1);
    i2_from_rol1 = i2_from_rol1 + 0x10;
    printf("tiprimarytinit: i2_from_rol1 = 0x%08x\n",i2_from_rol1);
  }



  /* dma */
  usrVmeDmaSetConfig(2,5,1); /*A32,MBLT*/
  printf("!!!!!!!!! set DMA as A32,MBLT\n");

  dma_dabufp=(unsigned int *)i2_from_rol1;
  // static unsigned int *tdcbuf;





  /* Initialize the event buffers (required for DMA) */
//  dmaPFreeAll();
//  vmeIN  = dmaPCreate("vmeIN",MAX_BLOCK_SIZE,50,0);
//  vmeOUT = dmaPCreate("vmeOUT",0,0,0);
    
//  dmaPStatsAll();
//  dmaPReInitAll();

 			/*	beusspInitMblk(beu_reg_control, &BEUSSPmblk);
  				beusspEnableA32m(beu_reg_control[1]);
 				beusspEnableA32m(beu_reg_control[2]);
				beusspEnBerrTerm(beu_reg_control[1]);
 				beusspEnBerrTerm(beu_reg_control[2]);
			*/
			
	//tiSetBusySource(TI_BUSY_LOOPBACK | TI_BUSY_SWB,1);
	//tiSetBlockLimit(0);		
	//tiSetBlockBufferLevel(5);	
	
	
	


	printf(PROMPT);	
			}

	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "tkstart") == 0) {
			if ( cargc == 1) { fp = fopen ("tk.bin", "wb");}
			else 		 { fp = fopen (cargv[1], "wb");}
			
			my_tiIntEnable(1);
			
			 //tiSetBusySource(TI_BUSY_SWB ,1);
			 //tiDisableDataReadout();
 			 //tiDisableA32();

		// Putting all FEU in running mode
			sprintf(progname, "%s", cargv[0] );
			if( (ret=SysRun()) != D_RetCode_Sucsess )
			{
				fprintf( stderr, "%s: SysRun failed with %d\n", progname, ret );
			}
			if( verbose )
			printf( "%s: SysRun OK\n", progname );



		/* permet une boucle sans fin sauf apres appui sur touche return */
		tem = fcntl(0, F_GETFL, 0);                                                       
		fcntl (0, F_SETFL, (tem | O_NDELAY));
		
		printf("\n\r");
		usleep(50);

		//start trigger

			tiStatus(1);	
			if( (ret=SysGo()) != D_RetCode_Sucsess )
			{
				fprintf( stderr, "%s: SysGo failed with %d\n", __FUNCTION__, ret );
			}
			if( verbose )
			printf( "%s: SysGo OK\n", __FUNCTION__ );
		
	tiEnableBusError();
	
		// Enable trigger sources 
		if( tiEnableTriggerSource() != OK )
		{
			fprintf( stderr, "%s: tiEnableTriggerSource failed\n", __FUNCTION__ );
		}



		beusspIntCnt = 0;
		run = 1;
		do_post_reads = 0;
		while (run) {
			// CE SERAIT BIEN DE RANGER LES CARTES PAR ORDRE CROISSANT DE SLOT NUMBER
			//first board needs to take token 
			beusspTakeToken(beu_reg_control[2]);
			
			//check for readme
			readme_0 = beusspBReady(beu_reg_control[1]);
			readme_1 = beusspBReady(beu_reg_control[2]);	
 			tokenstatus = beusspGetTokenStatus(beu_reg_control[2]);
 			tireadme = tiBReady();
	
			if  (( (readme_0 & 0x00010000) != 0 ) && ( (readme_1 & 0x00010000) != 0 ) && ((tokenstatus & 0x80000000)!=0  )&& ( tiBReady != 0 ) ) {

				beusspIntCnt += 1;

				//set bzrd high ( acknowledge )
				beusspBZRDHigh(beu_reg_control[1]);
				beusspBZRDHigh(beu_reg_control[2]);
			
 				/* Grab an event buffer off of the queue */
 //				GETEVENT(vmeIN,beusspIntCnt);
			  dma_dabufp=(unsigned int *)i2_from_rol1;

 				
 				dCnt = tiReadBlock( dma_dabufp, 8+5*30, 1 );
				//printf("%d	",dCnt);
				if(dCnt<=0)
				{
					printf("No data or error.  dCnt = %d\n",dCnt);
				}
				else
				{
                          dma_dabufp += dCnt; 
                   
				}
	
				dCnt = beusspTokenReadBlock(beu_reg_control[2], BEUSSPmblk, dma_dabufp,( MAX_BLOCK_SIZE>>2) -1024,1);


  				//first board needs to take token 
				beusspTakeToken(beu_reg_control[2]);

				
  				if(dCnt<=0)
    				{
      					printf("No data or error.  dCnt = %d\n",dCnt);
   				 }
  				else
    				{
      					dma_dabufp += dCnt;
					
						counter ++;
						size+= dCnt;
						if (counter == 1000)
						{
							
							
							// print deadtime

							
							printf("****** size=%d for %d events; average %d; last dCnt = %d; ",size, counter, size/counter, dCnt);
							my_tiDeadtime();
							deltatime = my_tiLive();
							printf("VME = %6.2f (MB/s)  rate = %7.3f ", (double) size*4.*1000. / (((double) deltatime )*30.*256.)  ,  ((double) ( counter * sys_params_ptr->NbOfEvtPerBlk)*1000.*1000.*1000.) / (((double) deltatime )*30.*256.) ) ;
							deltatime = tiLive(0);
							printf("diffdeadtime = %7.3f \n", 100. - (double) deltatime / 1000. ) ;
						//sys_params_ptr->NbOfEvtPerBlk
							size=0;
							counter = 0;
						}
					
    				}

  				/* Push this buffer to the outgoing queue */
  //				PUTEVENT(vmeOUT);
  				/* Grab a buffer off of the outgoing queue */
//				outEvent = dmaPGetItem(vmeOUT);

				//fwrite( outEvent->data, 4, outEvent->length, fp);

  				/* Free up the buffer (puts it back into the incoming queue) */
//				dmaPFreeItem(outEvent);

				//set bzrd low ( acknowledge second step : fin relecture data bloc)
				beusspBZRDLow(beu_reg_control[1]);
				beusspBZRDLow(beu_reg_control[2]);			
				my_tiIntAck();	
			}			
			//else  my_tiSyncReset();			
			
			
			if( do_post_reads == 0 )
			{
				n = read(0, &cc, 1);
				if (n > 0)
				{
					do_post_reads = 1000;
					// Disable trigger sources 
					if( (ret=tiDisableTriggerSource(0)) != OK )
					{
						fprintf( stderr, "%s: tiDisableTriggerSource failed with %d\n", __FUNCTION__, ret );
					}
						fprintf( stderr, "%s: tiDisableTriggerSource OK  \n", __FUNCTION__ );
				}
			}
			else
			{
				do_post_reads--;
				if( do_post_reads == 0 )
					run = 0;
			}


		} //end while

		tiStatus(1);	

		// Stop !
			if( (ret=SysStop()) != D_RetCode_Sucsess )
			{
				fprintf( stderr, "%s: SysStop failed with %d\n", __FUNCTION__, ret );
			}
			if( verbose )
			printf( "%s: SysStop OK\n",  __FUNCTION__ );


		tiStatus(1);	


		fclose(fp);
			
		fcntl(0, F_SETFL, tem);					
		printf(PROMPT);	
}

	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "start") == 0) {
			if ( cargc == 1) { fp = fopen ("data.bin", "wb");}
			else 		 { fp = fopen (cargv[1], "wb");}
			
			
			my_tiIntEnable(1);
			//tiSetBusySource(TI_BUSY_SWB ,1);
			//tiDisableDataReadout();
 			//tiDisableA32();
 			//tiDisableBusError();
	
 			//beusspDisableA32m(beu_reg_control[2]);
			beusspDisBerrTerm(beu_reg_control[1]);
 			//beusspDisBerrTerm(beu_reg_control[2]);
			beusspEnableA32(beu_reg_control[1]);
 			//beusspEnableA32(beu_reg_control[2]);
			

	
		// Putting all FEU in running mode
			sprintf(progname, "%s", cargv[0] );
			if( (ret=SysRun()) != D_RetCode_Sucsess )
			{
				fprintf( stderr, "%s: SysRun failed with %d\n", progname, ret );
			}
			if( verbose )
			printf( "%s: SysRun OK\n", progname );



		/* permet une boucle sans fin sauf apres appui sur touche return */
		tem = fcntl(0, F_GETFL, 0);                                                       
		fcntl (0, F_SETFL, (tem | O_NDELAY));
		
		printf("\n\r");
		usleep(50);

		//start trigger

			tiStatus(1);	
			if( (ret=SysGo()) != D_RetCode_Sucsess )
			{
				fprintf( stderr, "%s: SysGo failed with %d\n", __FUNCTION__, ret );
			}
			if( verbose )
			printf( "%s: SysGo OK\n", __FUNCTION__ );
		

		// Enable trigger sources 
		if( tiEnableTriggerSource() != OK )
		{
			fprintf( stderr, "%s: tiEnableTriggerSource failed\n", __FUNCTION__ );
		}

		tiEnableBusError();
				run = 1;
		beusspIntCnt = 0;
		while (run) {
			//check for readme
			readme_0 = beusspBReady(beu_reg_control[1]);
			//readme_1 = beusspBReady(beu_reg_control[2]);	
			tireadme = tiBReady();
			

			//if  (( (readme_0 & 0x00010000) != 0 ) && ( (readme_1 & 0x00010000) != 0 ) && ( tiBReady != 0 )) 
			if  (( (readme_0 & 0x00010000) != 0 ) && ( tiBReady != 0 )) 
			{
				beusspIntCnt += 1;
				
				//set bzrd high ( acknowledge )
				beusspBZRDHigh(beu_reg_control[1]);
				//beusspBZRDHigh(beu_reg_control[2]);

				//get block size from each beussp
				beusspGetBlockSize(beu_reg_control[1], &blocksize_0);		
				//beusspGetBlockSize(beu_reg_control[2], &blocksize_1);
										
 				/* Grab an event buffer off of the queue */
// 				GETEVENT(vmeIN,beusspIntCnt);
// 			printf("dma_dabufp = 0x%08x\n",	dma_dabufp);
			  dma_dabufp=(unsigned int *)i2_from_rol1;

				dCnt = tiReadBlock( dma_dabufp, 8+5*30, 1 );
				if(dCnt<=0)
				{
					printf("No data or error.  dCnt = %d\n",dCnt);
				}
				else
				{
                          dma_dabufp += dCnt;             
				}

				dCnt = beusspReadBlock(beu_reg_control[1], beu_fifo[1], dma_dabufp, blocksize_0,1);
  				if(dCnt<=0)
    				{
      					printf("No data or error.  dCnt = %d\n",dCnt);
   				 }
  				else
    				{
      					dma_dabufp += dCnt;

						counter ++;
						size+= dCnt;
						if (counter == 1000)
						{
							printf("****** size=%d for %d events; average %d; last dCnt = %d; ",size, counter, size/counter, dCnt);
							my_tiDeadtime();
							deltatime = my_tiLive();
							printf("VME = %6.2f (MB/s)  rate = %7.3f ", (double) size*4.*1000. / (((double) deltatime )*30.*256.)  ,  ((double) ( counter * sys_params_ptr->NbOfEvtPerBlk)*1000.*1000.*1000.) / (((double) deltatime )*30.*256.) ) ;
							deltatime = tiLive(0);
							printf("diffdeadtime = %7.3f \n", 100. - (double) deltatime / 1000. ) ;
							size=0;
							counter = 0;
						}

    				}





	/*			dCnt = beusspReadBlock(beu_reg_control[2], beu_fifo[2], dma_dabufp, blocksize_1,1);
			printf("%d\n",dCnt);
  				if(dCnt<=0)
    				{
      					printf("No data or error.  dCnt = %d\n",dCnt);
   				 }
  				else
    				{
      					dma_dabufp += dCnt;

    				 }
*/

  				/* Push this buffer to the outgoing queue */
//  				PUTEVENT(vmeOUT);
  				/* Grab a buffer off of the outgoing queue */
//				outEvent = dmaPGetItem(vmeOUT);
				//fwrite( outEvent->data, 4, outEvent->length, fp);

  				/* Free up the buffer (puts it back into the incoming queue) */
//				dmaPFreeItem(outEvent);
				//set bzrd low ( acknowledge second step : fin relecture data bloc)
				beusspBZRDLow(beu_reg_control[1]);
				//beusspBZRDLow(beu_reg_control[2]);			
				my_tiIntAck();	

			}			
					
					
			if( do_post_reads == 0 )
			{
				n = read(0, &cc, 1);
				if (n > 0)
				{
					do_post_reads = 1000;
					// Disable trigger sources 
					if( (ret=tiDisableTriggerSource(0)) != OK )
					{
						fprintf( stderr, "%s: tiDisableTriggerSource failed with %d\n", __FUNCTION__, ret );
					}
						fprintf( stderr, "%s: tiDisableTriggerSource OK  \n", __FUNCTION__ );
				}
			}
			else
			{
				do_post_reads--;
				if( do_post_reads == 0 )
					run = 0;
			}



		} //end while
		
		tiStatus(1);	

		// Stop !
			if( (ret=SysStop()) != D_RetCode_Sucsess )
			{
				fprintf( stderr, "%s: SysStop failed with %d\n", __FUNCTION__, ret );
			}
			if( verbose )
			printf( "%s: SysStop OK\n",  __FUNCTION__ );


		tiStatus(1);	


		fclose(fp);
			
		fcntl(0, F_SETFL, tem);					
		printf(PROMPT);	
	}
	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "numfeu") == 0) {
				if ( cargc == 2 ) {
				
					my_hex_string_to_int( cargv[1], &numFeu ) ;
					beusspSetTargetFeu(beu_reg_control[1], numFeu);
					beusspSetTargetFeu(beu_reg_control[2], numFeu);
				}	
			else {
					printf("Unable to execute command : %s  .\r\n", cargv[0] );
				}

				printf(PROMPT);	
			}					
	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "anabin") == 0) {
			fp = fopen (cargv[1], "r");
			if (fp==NULL) {printf("ERROR : File does not exist !\n\r");}
			else {
  			// obtain file size:
  			fseek (fp , 0 , SEEK_END);
 			nb_words_in_block = ftell (fp);
  			rewind (fp);
			printf("nb bytes in file : %d  nb words in file : %d \n\r", nb_words_in_block, nb_words_in_block/4 );
			nb_words_in_block = nb_words_in_block/4;

			nb_blocks_h=0;
			nb_blocks_l=0;
			nb_events_h=0;
 			nb_events_l=0;				
			nb_samples_h=0; nb_samples_l=0;				
			nb_filler_h=0; nb_filler_l=0;				
			nb_eob0_h=0;nb_eob0_l=0;			
			nb_eob1_h=0;nb_eob1_l=0;
			nb_eob=0;
			nb_parity_h = 0;
			nb_parity_l = 0;

			for( num_word_in_block = 0; num_word_in_block < 100; num_word_in_block++) {
			//for( num_word_in_block = 0; num_word_in_block < nb_words_in_block; num_word_in_block++) {
			result = fread(&data1, 4, 1, fp);
			printf("%x %x \n\r", num_word_in_block,  data1 );}
			
			rewind (fp);
			for( num_word_in_block = 0; num_word_in_block < nb_words_in_block ; num_word_in_block ++ ) {

				
				result = fread(&data1, 4, 1, fp);
				if( ((data1 & 0xFFFF0000)>>16)== 0xBBF3 ) { nb_blocks_h +=1;}
				if( (data1 & 0x0000FFFF)== 0xBBF3 ) { nb_blocks_l +=1;}
				if( ((data1 & 0xFFFF0000)>>16)== 0xEEF3 ) { nb_events_h +=1;}
				if( (data1 & 0x0000FFFF)== 0xEEF3 ) { nb_events_l +=1;}
				if( ((data1 & 0xFFFF0000)>>16)== 0x55F3 ) { nb_samples_h +=1;}
				if( (data1 & 0x0000FFFF)== 0x55F3 ) { nb_samples_l +=1;}
				if( ((data1 & 0xFFFF0000)>>16)== 0xAAFA ) { nb_filler_h +=1;}
				if( (data1 & 0x0000FFFF)== 0xAAFA ) { nb_filler_l +=1;}
				
				if( ((data1 & 0xFFFF0000)>>16)== 0xCAFC ) { nb_eob0_h +=1;}
				if( (data1 & 0x0000FFFF)== 0xCAFC ) { nb_eob0_l +=1;}
				
				if( ((data1 & 0xFFFF0000)>>16)== 0xAAFC ) { nb_eob1_h +=1;}
				if( (data1 & 0x0000FFFF)== 0xAAFC ) { nb_eob1_l +=1;}

				if( data1 == 0xAAFCCAFC ) { nb_eob +=1;}
				
				data2 = ((data1 & 0x00010000)>>16) + ((data1 & 0x00020000)>>17) + ((data1 & 0x00040000)>>18)+ ((data1 & 0x00080000)>>19) +
					((data1 & 0x00100000)>>20) + ((data1 & 0x00200000)>>21) + ((data1 & 0x00400000)>>22)+ ((data1 & 0x00800000)>>23) +
					((data1 & 0x01000000)>>24) + ((data1 & 0x02000000)>>25) + ((data1 & 0x04000000)>>26)+ ((data1 & 0x08000000)>>27) +
					((data1 & 0x10000000)>>28) + ((data1 & 0x20000000)>>29) + ((data1 & 0x40000000)>>30)+ ((data1 & 0x80000000)>>31) ;
				if ( (( data2 > 1)<1) ==data2 ) {nb_parity_h +=1;}

				data3 = ((data1 & 0x00000001)    ) + ((data1 & 0x00000002)>> 1) + ((data1 & 0x00000004)>> 2)+ ((data1 & 0x00000008)>> 3) +
					((data1 & 0x00000010)>> 4) + ((data1 & 0x00000020)>> 5) + ((data1 & 0x00000040)>> 6)+ ((data1 & 0x00000080)>> 7) +
					((data1 & 0x00000100)>> 8) + ((data1 & 0x00000200)>> 9) + ((data1 & 0x00000400)>>10)+ ((data1 & 0x00000800)>>11) +
					((data1 & 0x00001000)>>12) + ((data1 & 0x00002000)>>11) + ((data1 & 0x00004000)>>14)+ ((data1 & 0x00008000)>>15) ;
				if ( (( data3 > 1)<1) ==data3) {nb_parity_l +=1;}





				}
				printf("block headers : %d %d \n\r", nb_blocks_h,  nb_blocks_l );
				printf("event headers : %d %d \n\r", nb_events_h,  nb_events_l );
				printf("sample headers : %d %d \n\r", nb_samples_h,  nb_samples_l );
				printf("filler words : %d %d \n\r", nb_filler_h,  nb_filler_l );
				printf("block trailer 0 : %d %d \n\r", nb_eob0_h,  nb_eob0_l );
				printf("block trailer 1 : %d %d \n\r", nb_eob1_h,  nb_eob1_l );
				printf("block trailer: %d  \n\r", nb_eob);
				printf("parity : %d  %d  \n\r", nb_parity_h, nb_parity_l);

		fclose(fp); }
		
		printf(PROMPT); }
	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "sendslwctrl") == 0) {
				if ( cargc == 11 ) {
				
					my_hex_string_to_int( cargv[1], &numBeu ) ;
					my_hex_string_to_int( cargv[2], &numFeu ) ;
					my_hex_string_to_int( cargv[3], &data1 ) ;
					my_hex_string_to_int( cargv[4], &data2 ) ;
					my_hex_string_to_int( cargv[5], &data3 ) ;
					my_hex_string_to_int( cargv[6], &data4 ) ;
					my_hex_string_to_int( cargv[7], &data5 ) ;
					my_hex_string_to_int( cargv[8], &data6 ) ;
					my_hex_string_to_int( cargv[9], &data7 ) ;
					my_hex_string_to_int( cargv[10], &data8 ) ;

					data1 =   (0x000000FF&data4) | (  (0x000000FF&data3) <<8 )| (  (0x000000FF&data2) <<16 )| (  (0x000000FF&data1) <<24 );		
					data2 =   (0x000000FF&data8) | (  (0x000000FF&data7) <<8 )| (  (0x000000FF&data6) <<16 )| (  (0x000000FF&data5) <<24 );
					
					if ( numBeu == 0 ){
						beusspSendSlowControl(beu_reg_control[1], numFeu, &data1, &data2, &data3, &data4);
						printf("%08X %08X \r\n", data3, data4);
					} else 
					if ( numBeu == 1 ){
						beusspSendSlowControl(beu_reg_control[2], numFeu, &data1, &data2, &data3, &data4);
						printf("%08X %08X \r\n", data3, data4); }
					else {
						printf("Unable to execute command : invalid beussp number  .\r\n"); 
						}
				}
				else {
					printf("Unable to execute command : %s  .\r\n", cargv[0] );
				}

				printf(PROMPT);	
			}
	// ----------------------------------------------------------------
		else if (strcmp(cargv[0], "flashstatus") == 0) { 
			//D7 = 11010111
	    
		data_1 = beusspFlashSatus(beu_reg_control[1]);
		printf(" data =  %08x \n",data_1);
		beusspFlashSatus(beu_reg_control[1]);
		data_1 = beusspFlashSatusReg(beu_reg_control[1]);
		printf(" data =  %08x \n",data_1);

		

		printf(PROMPT);	
	}			
		// ----------------------------------------------------------------
		else if (strcmp(cargv[0], "flashid") == 0) { 
			//9F = 11010111
	    
			data_1  =  beusspFlashID(beu_reg_control[1]);
			printf(" data =  %08x \n",data_1);

		printf(PROMPT);	
	}			
	
	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "localclock") == 0) {
	    
		beusspSetClkSrc(beu_reg_control[1], 0);
 		beusspSetClkSrc(beu_reg_control[2], 0);

		printf(PROMPT);	
	}
	
	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "ticlock") == 0) {
	
		beusspSetClkSrc(beu_reg_control[1], 1);
 		beusspSetClkSrc(beu_reg_control[2], 1);
	    
		printf(PROMPT);	
	}
	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "loadflash") == 0) {
			fp = fopen ("ssp_V4.bin", "r"); 
			if (fp==NULL) {printf("ERROR : File does not exist !"); break;}
			
  			// obtain file size:
  			fseek (fp , 0 , SEEK_END);
 			nbbytes = ftell (fp);
  			rewind (fp);
			printf("nb bytes in file : %d  nb words in file : %d \n\r", nbbytes, nbbytes/4 );


			/*
			for( num_word_in_block = 0; num_word_in_block < 100; num_word_in_block++) {
			result = fread(&data1, 4, 1, fp);
			printf("%x %x \n\r", num_word_in_block,  data1 );}
			
			rewind (fp);
			for( num_word_in_block = 0; num_word_in_block < nb_words_in_block ; num_word_in_block ++ ) {
				result = fread(&data1, 4, 1, fp);
				}
			*/

		fclose(fp);
		printf(PROMPT); }		

	// ----------------------------------------------------------------
	else if (strcmp(cargv[0], "help") == 0) {
				handle_help();

				usage( progname );
				printf(PROMPT);	
			}
	// ----------------------------------------------------------------
	else {
		printf("Unrecognized command \" %s \".\r\n", cargv[0]);
		printf(PROMPT);			
	}
}//for loop
	
 
 //CLOSE:
  vmeBusLock();
  dmaPFreeAll();
  vmeBusUnlock();
  vmeCloseDefaultWindows();


  cleanup(0);	
  exit(0);
}

