/*
--------------------------------------------------------------------------------
-- Company:        IRFU / CEA Saclay
-- Engineers:      Irakli.MANDJAVIDZE@cea.fr (IM)
-- 
-- Project Name:   Clas12 Micromegas Vertex Tracker
-- Design Name:    Common to several designs
--
-- Module Name:    MvtDataFileReader.c
-- Description:    Interprets binary data files
--
-- Target Devices: Windows or Linux PC
-- Tool versions:  Windows Visual C++ or Linux Make
-- 
-- Create Date:    0.0 2014/11/11 IM
-- Revision:       1.0 2015/03/23 IM Add Extendded TI eid & tstp format capability
--
-- Comments:
--
--------------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "Platform.h"
#include "histo.h"

extern char *optarg;
extern int   optind;

/******************
 EVIO Entry macros
 *****************/
#define DEF_ENTRY_DATA_INT    0x01
#define DEF_ENTRY_DATA_ASC    0x03
#define DEF_ENTRY_DATA_BNK_0E 0x0E
#define DEF_ENTRY_DATA_CMP    0x0F
#define DEF_ENTRY_DATA_BNK_10 0x10

#define DEF_ENTRY_TAG_RC_SYNC     0x0010
#define DEF_ENTRY_TAG_RC_PSTART   0x0011
#define DEF_ENTRY_TAG_RC_GO       0x0012
#define DEF_ENTRY_TAG_RC_PAUSE    0x0013
#define DEF_ENTRY_TAG_RC_END      0x0014
#define DEF_ENTRY_TAG_TI          0xE10A
#define DEF_ENTRY_TAG_MVT_RAW     0xE118
#define DEF_ENTRY_TAG_MVT_CMP     0xE11B
#define DEF_ENTRY_TAG_MVT_CFG     0xE10E

#define DEF_GetEvioEntryDat( ent_type ) (((ent_type) >>  8) & 0xFF  )
#define DEF_GetEvioEntryTag( ent_type ) (((ent_type) >> 16) & 0xFFFF)
#define DEF_GetEvioEntryNum( ent_type ) (  ent_type         & 0xFF  )

#define DEF_BinData_FileName "None"

// Global variables
int    verbose           =  0;   // if >0 some debug output

FILE *bin_data_fptr = (FILE *)NULL;
FILE *asc_data_fptr = (FILE *)NULL;
FILE *hst_data_fptr = (FILE *)NULL;
FILE *cfg_data_fptr = (FILE *)NULL;

char bin_data_file_name[128];

int first_phy_blk;
int last_phy_blk;
int do_feu_bin_files = 0;

short feu_smp_val[512][256];
short feu_chan_val[512];
int   feu_chan_num;


unsigned int *rd_buf = (unsigned int *)NULL;
int rd_buf_len = 0;

// Define mux number of FEUs & BEUs
#define Def_MaxNbOfBeu 3
#define Def_MaxNbOfFeu 128
FILE *feu_data_fptr[Def_MaxNbOfFeu];
int feu_data_fptr_cntr = 0;


// Statistics
int net_hdr_cnt;

int rcsync_cnt;
int rcpstart_cnt;
int rcgo_cnt;
int rcend_cnt;

int evt_hdr_cnt;
int evt_blk_cnt;

int ti_evt_cnt;

int mvt_raw_evt_cnt;
int mvt_cmp_evt_cnt;
int cfg_evt_cnt;

int unknown_cmp_cnt;
int unknown_asc_cnt;
int unknown_int_cnt;
int unknown_ent_cnt;

/***********************************************************
 * Histogramms
 ***********************************************************/
typedef struct _TimingHistos
{
	// BEU-s
	unsigned int beu2ti_tstp_offset[Def_MaxNbOfBeu];
	Histo        beu2ti_tstp_histo[Def_MaxNbOfBeu];
	Histo        beu_ftstp_histo[Def_MaxNbOfBeu];
	// FEU-s
	unsigned int feu2beu_tstp_offset[Def_MaxNbOfFeu];
	         int feu_beu_id[Def_MaxNbOfFeu];
	Histo        feu2beu_tstp_histo[Def_MaxNbOfFeu];
	Histo        feu_ftstp_histo[Def_MaxNbOfFeu];

	// Inter trigger delay
	unsigned int ti_tstp_prev;
	Histo        ti_itd_histo;
	
	// Size histos
	Histo phy_evt_blk_size_histo;
	Histo vme_data_size_histo;
	Histo mvt_evt_size_histo;
	Histo feu_smp_size_histo[Def_MaxNbOfFeu];
} TimingHistos;
int TimingHistos_Init( TimingHistos *tim_hist )
{
	int index;
	for( index=0; index<Def_MaxNbOfBeu; index++ )
	{
		tim_hist->beu2ti_tstp_offset[index] = 0xFFFFffff;
		if( Histo_Init(&(tim_hist->beu2ti_tstp_histo[index]), -10, 21, 1) < 0 )
		{
			fprintf( stderr, "%s: Histo_Init failed for beu2ti_tstp_histo[%d]\n", __FUNCTION__, index );
			return -1;
		}
		if( Histo_Init(&(tim_hist->beu_ftstp_histo[index]), 0, 2, 1) < 0 )
		{
			fprintf( stderr, "%s: Histo_Init failed for beu_ftstp_histo[%d]\n", __FUNCTION__, index );
			return -1;
		}
	}
	for( index=0; index<Def_MaxNbOfFeu; index++ )
	{
		tim_hist->feu2beu_tstp_offset[index] = 0xFFFFffff;
		tim_hist->feu_beu_id[index]          = -1;
		if( Histo_Init(&(tim_hist->feu2beu_tstp_histo[index]), -10, 21, 1) < 0 )
		{
			fprintf( stderr, "%s: Histo_Init failed for feu2beu_tstp_histo[%d]\n", __FUNCTION__, index );
			return -1;
		}
		if( Histo_Init(&(tim_hist->feu_ftstp_histo[index]), 0, 7, 1) < 0 )
		{
			fprintf( stderr, "%s: Histo_Init failed for feu_ftstp_histo[%d]\n", __FUNCTION__, index );
			return -1;
		}
		if( Histo_Init(&(tim_hist->feu_smp_size_histo[index]), 0, 1200, 1) < 0 )
		{
			fprintf( stderr, "%s: Histo_Init failed for feu_smp_size_histo[%d]\n", __FUNCTION__, index );
			return -1;
		}
	}

	tim_hist->ti_tstp_prev = 0xFFFFffff;
	if( Histo_Init(&(tim_hist->ti_itd_histo), 0, 100000, 1) < 0 )
	{
		fprintf( stderr, "%s: Histo_Init failed for ti_itd_histo\n", __FUNCTION__ );
		return -1;
	}

	if( Histo_Init(&(tim_hist->phy_evt_blk_size_histo), 0, 100000, 1) < 0 )
	{
		fprintf( stderr, "%s: Histo_Init failed for phy_evt_blk_size_histo\n", __FUNCTION__ );
		return -1;
	}
	if( Histo_Init(&(tim_hist->vme_data_size_histo), 0, 100000, 1) < 0 )
	{
		fprintf( stderr, "%s: Histo_Init failed for vme_data_size_histo\n", __FUNCTION__ );
		return -1;
	}
	if( Histo_Init(&(tim_hist->mvt_evt_size_histo), 0, 100000, 1) < 0 )
	{
		fprintf( stderr, "%s: Histo_Init failed for mvt_evt_size_histo\n", __FUNCTION__ );
		return -1;
	}
	return 0;
}
void TimingHistos_Free( TimingHistos *tim_hist )
{
	int index;
	// Timinh histos
	for( index=0; index<Def_MaxNbOfBeu; index++ )
	{
		Histo_Free(&(tim_hist->beu2ti_tstp_histo[index]));
		Histo_Free(&(tim_hist->beu_ftstp_histo[index]));
	}
	for( index=0; index<Def_MaxNbOfFeu; index++ )
	{
		Histo_Free(&(tim_hist->feu2beu_tstp_histo[index]));
		Histo_Free(&(tim_hist->feu_ftstp_histo[index]));
	}
	Histo_Free(&(tim_hist->ti_itd_histo));

	// Size histos
	Histo_Free(&(tim_hist->phy_evt_blk_size_histo));
	Histo_Free(&(tim_hist->vme_data_size_histo));
	Histo_Free(&(tim_hist->mvt_evt_size_histo));
	for( index=0; index<Def_MaxNbOfFeu; index++ )
	{
		Histo_Free(&(tim_hist->feu_smp_size_histo[index]));
	}
}
void TimingHistos_DumpStat(TimingHistos *tim_hist, FILE *fptr)
{
	int index;
	fprintf( fptr, "TimingHistos = 0x%08x\n", (unsigned int)tim_hist );
	/*
	 * Dump stats
	 */
	// Timing synchro
	fprintf( fptr, " Beu Synchronisation\n" );
	for( index=0; index<Def_MaxNbOfBeu; index++ )
	{
		if( tim_hist->beu2ti_tstp_offset[index] != 0xFFFFffff )
		{
			fprintf( fptr, " beu[%1d]  to ti     tstp Offset = 0x%03x %4d (4ns) ",
				index, tim_hist->beu2ti_tstp_offset[index], tim_hist->beu2ti_tstp_offset[index] );
			Histo_Stat( &(tim_hist->beu2ti_tstp_histo[index]), fptr );
//fprintf( fptr, "\n" );
		}
	}
	fprintf( fptr, "\n" );
	fprintf( fptr, " Feu Synchronisation\n" );
	for( index=0; index<Def_MaxNbOfFeu; index++ )
	{
		if( tim_hist->feu2beu_tstp_offset[index] != 0xFFFFffff )
		{
			fprintf( fptr, " feu[%2d] to beu[%1d] tstp Offset = 0x%03x %4d (8ns) ",
				index, tim_hist->feu_beu_id[index], tim_hist->feu2beu_tstp_offset[index], tim_hist->feu2beu_tstp_offset[index] );
			Histo_Stat( &(tim_hist->feu2beu_tstp_histo[index]), fptr );
//fprintf( fptr, "\n" );
		}
	}
	fprintf( fptr, "\n" );

	// Fine timestamps
	fprintf( fptr, " Beu Fine timestamp (4ns)\n" );
	for( index=0; index<Def_MaxNbOfBeu; index++ )
	{
		if( tim_hist->beu2ti_tstp_offset[index] != 0xFFFFffff )
		{
			fprintf( fptr, " beu[%1d]  fine tstp (4ns) ", index );
			Histo_Stat( &(tim_hist->beu_ftstp_histo[index]), fptr );
//fprintf( fptr, "\n" );
		}
	}
	fprintf( fptr, "\n" );
	fprintf( fptr, " Feu Fine timestamp (8ns)\n" );
	for( index=0; index<Def_MaxNbOfFeu; index++ )
	{
		if( tim_hist->feu2beu_tstp_offset[index] != 0xFFFFffff )
		{
			fprintf( fptr, " feu[%2d] fine tstp Offset (8ns) ", index );
			Histo_Stat( &(tim_hist->feu_ftstp_histo[index]), fptr );
//fprintf( fptr, "\n" );
		}
	}
	fprintf( fptr, "\n" );

	fprintf( fptr, " Feu sample size (16-bit word)\n" );
	for( index=0; index<Def_MaxNbOfFeu; index++ )
	{
		if( tim_hist->feu2beu_tstp_offset[index] != 0xFFFFffff )
		{
			fprintf( fptr, " feu[%2d] Smp (16_bit wrd) ", index );
			Histo_Stat( &(tim_hist->feu_smp_size_histo[index]), fptr );
//fprintf( fptr, "\n" );
		}
	}
	fprintf( fptr, "\n" );

	// Inter triggger delay
	fprintf( fptr, " Ti ITD (1us) " );
	Histo_Stat( &(tim_hist->ti_itd_histo), fptr );
	fprintf( fptr, "\n" );

	// Composite event size
	fprintf( fptr, " Mvt Evt size (32-bit word) " );
	Histo_Stat( &(tim_hist->mvt_evt_size_histo), fptr );
	fprintf( fptr, "\n" );

	// Phy Event Block Size
	fprintf( fptr, " Phy Evt Blk (32-bit word) " );
	Histo_Stat( &(tim_hist->phy_evt_blk_size_histo), fptr );
	if( (tim_hist->phy_evt_blk_size_histo.count!=0) && (tim_hist->ti_itd_histo.sum_x / tim_hist->ti_itd_histo.count != 0) )
	{
		fprintf
		(
			fptr,
			" Average throughput %6.2f Mbyte/s\n",
			4 * tim_hist->phy_evt_blk_size_histo.sum_x / tim_hist->phy_evt_blk_size_histo.count / (tim_hist->ti_itd_histo.sum_x / tim_hist->ti_itd_histo.count)
		);
	}
	fprintf( fptr, "\n" );

	// Vme data Size
/*
	fprintf( fptr, " Vme data size (32-bit word) " );
	Histo_Stat( &(tim_hist->vme_data_size_histo), fptr );
	if( (tim_hist->vme_data_size_histo.count!=0) && (tim_hist->ti_itd_histo.sum_x / tim_hist->ti_itd_histo.count != 0) && (ti_blk_cnt != 0) )
	{
		fprintf
		(
			fptr,
			" Average VME throughput %6.2f Mbyte/s\n",
			4 * tim_hist->vme_data_size_histo.sum_x / tim_hist->vme_data_size_histo.count / (ti_evt_cnt / ti_blk_cnt) / (tim_hist->ti_itd_histo.sum_x / tim_hist->ti_itd_histo.count)
		);
	}
	fprintf( fptr, "\n" );
*/
}

void TimingHistos_DumpHistos(TimingHistos *tim_hist, FILE *fptr)
{
	int index;

	// First dump stats
	TimingHistos_DumpStat(tim_hist, fptr);

	/*
	 * Then Dump histos
	 */
	// Timing synchro
	for( index=0; index<Def_MaxNbOfBeu; index++ )
	{
		if( tim_hist->beu2ti_tstp_offset[index] != 0xFFFFffff )
		{
			fprintf( fptr, " beu[%1d]  to ti     tstp Offset = 0x%03x %4d (4ns) ",
				index, tim_hist->beu2ti_tstp_offset[index], tim_hist->beu2ti_tstp_offset[index] );
			Histo_Stat( &(tim_hist->beu2ti_tstp_histo[index]), fptr );
			Histo_Dump( &(tim_hist->beu2ti_tstp_histo[index]), fptr );
		}
	}
	fprintf( fptr, "\n" );
	for( index=0; index<Def_MaxNbOfFeu; index++ )
	{
		if( tim_hist->feu2beu_tstp_offset[index] != 0xFFFFffff )
		{
			fprintf( fptr, " feu[%2d] to beu[%1d] tstp Offset = 0x%03x %4d (8ns) ",
				index, tim_hist->feu_beu_id[index], tim_hist->feu2beu_tstp_offset[index], tim_hist->feu2beu_tstp_offset[index] );
			Histo_Stat( &(tim_hist->feu2beu_tstp_histo[index]), fptr );
			Histo_Dump( &(tim_hist->feu2beu_tstp_histo[index]), fptr );
		}
	}
	fprintf( fptr, "\n" );

	// Fine timestamps
	fprintf( fptr, " Beu Fine timestamp (4ns)\n" );
	for( index=0; index<Def_MaxNbOfBeu; index++ )
	{
		if( tim_hist->beu2ti_tstp_offset[index] != 0xFFFFffff )
		{
			fprintf( fptr, " beu[%1d]  fine tstp (4ns) ", index );
			Histo_Stat( &(tim_hist->beu_ftstp_histo[index]), fptr );
			Histo_Dump( &(tim_hist->beu_ftstp_histo[index]), fptr );
		}
	}
	fprintf( fptr, "\n" );
	fprintf( fptr, " Feu Fine timestamp (8ns)\n" );
	for( index=0; index<Def_MaxNbOfFeu; index++ )
	{
		if( tim_hist->feu2beu_tstp_offset[index] != 0xFFFFffff )
		{
			fprintf( fptr, " feu[%2d] fine tstp Offset (8ns) ", index );
			Histo_Stat( &(tim_hist->feu_ftstp_histo[index]), fptr );
			Histo_Dump( &(tim_hist->feu_ftstp_histo[index]), fptr );
		}
	}
	fprintf( fptr, "\n" );

	fprintf( fptr, " Feu sample size (16-bit word)\n" );
	for( index=0; index<Def_MaxNbOfFeu; index++ )
	{
		if( tim_hist->feu2beu_tstp_offset[index] != 0xFFFFffff )
		{
			fprintf( fptr, " feu[%2d] Smp (16_bit wrd) ", index );
			Histo_Stat( &(tim_hist->feu_smp_size_histo[index]), fptr );
			Histo_Dump( &(tim_hist->feu_smp_size_histo[index]), fptr );
		}
	}
	fprintf( fptr, "\n" );

	// Inter triggger delay
	fprintf( fptr, " Ti ITD (1us)\n" );
	Histo_Stat( &(tim_hist->ti_itd_histo), fptr );
	Histo_Dump( &(tim_hist->ti_itd_histo), fptr );
	fprintf( fptr, "\n" );

	// Composite event size
	fprintf( fptr, " Mvt Evt size (32-bit word) " );
	Histo_Stat( &(tim_hist->mvt_evt_size_histo), fptr );
	Histo_Dump( &(tim_hist->mvt_evt_size_histo), fptr );
	fprintf( fptr, "\n" );

	// Phy Event Block Size
	fprintf( fptr, " Phy Evt Blk (32-bit word)\n" );
	Histo_Stat( &(tim_hist->phy_evt_blk_size_histo), fptr );
	Histo_Dump( &(tim_hist->phy_evt_blk_size_histo), fptr );
	fprintf( fptr, "\n" );
	
/*
	// Vme data Size
	fprintf( fptr, " Vme data (32-bit word)\n" );
	Histo_Stat( &(tim_hist->vme_data_size_histo), fptr );
	Histo_Dump( &(tim_hist->vme_data_size_histo), fptr );
	fprintf( fptr, "\n" );
*/
}

// Timing histos
TimingHistos timing_histos_raw;
TimingHistos timing_histos_cmp;
/***********************************************************
 * End Histogramms
 ***********************************************************/

/***********************************************************
 * Per Event statistics structure
 ***********************************************************/
#define PHY_EVT_DATA_TYPE_NON -1
#define PHY_EVT_DATA_TYPE_RAW  0
#define PHY_EVT_DATA_TYPE_CMP  1
typedef struct _PhyEvtStat
{
	// General info
	int id;
	int size;
	int data_type;
	// Ti stat
	unsigned int ti_evid_hi;
	unsigned int ti_evid_lo;
	unsigned int ti_tstp_hi;
	unsigned int ti_tstp_lo;
	// Beu stat
	int nb_of_beu;
	int beu_evid_hi[Def_MaxNbOfBeu];
	int beu_evid_mi[Def_MaxNbOfBeu];
	int beu_evid_lo[Def_MaxNbOfBeu];
	int beu_evid_ac[Def_MaxNbOfBeu];
	int beu_tstp_hi[Def_MaxNbOfBeu];
	int beu_tstp_mi[Def_MaxNbOfBeu];
	int beu_tstp_lo[Def_MaxNbOfBeu];
	int beu_tstp_fn[Def_MaxNbOfBeu];
	int beu_smp_num[Def_MaxNbOfBeu];
	// Feu stat
	int nb_of_feu;
	int feu_evid[Def_MaxNbOfFeu];
	int feu_tstp[Def_MaxNbOfFeu];
	int feu_ftstp[Def_MaxNbOfFeu];
	int feu_size[Def_MaxNbOfFeu];
	int feu_smp_num[Def_MaxNbOfFeu];
	int feu_beu_id[Def_MaxNbOfFeu];
} PhyEvtStat;
void PhyEvtStat_Init(PhyEvtStat *phy_evt_stat)
{
	int index;
	phy_evt_stat->ti_evid_hi = 0xFFFFffff;
	phy_evt_stat->ti_evid_lo = 0xFFFFffff;
	phy_evt_stat->ti_tstp_hi = 0xFFFFffff;
	phy_evt_stat->ti_tstp_lo = 0xFFFFffff;
	phy_evt_stat->nb_of_beu = 0;
	for( index=0; index<Def_MaxNbOfBeu; index++ )
	{
		phy_evt_stat->beu_evid_hi[index] = -1;
		phy_evt_stat->beu_evid_mi[index] = -1;
		phy_evt_stat->beu_evid_lo[index] = -1;
		phy_evt_stat->beu_evid_ac[index] = -1;
		phy_evt_stat->beu_tstp_hi[index] = -1;
		phy_evt_stat->beu_tstp_mi[index] = -1;
		phy_evt_stat->beu_tstp_lo[index] = -1;
		phy_evt_stat->beu_tstp_fn[index] = -1;
		phy_evt_stat->beu_smp_num[index] =  0;
	}
	phy_evt_stat->nb_of_feu = 0;
	for( index=0; index<Def_MaxNbOfFeu; index++ )
	{
		phy_evt_stat->feu_evid[index]  = -1;
		phy_evt_stat->feu_tstp[index]  = -1;
		phy_evt_stat->feu_ftstp[index] = -1;
		phy_evt_stat->feu_size[index]  = -1;
		phy_evt_stat->feu_smp_num[index] = 0;
		phy_evt_stat->feu_beu_id[index]  = -1;
	}
	phy_evt_stat->id   = 0;
	phy_evt_stat->size = 0;
	phy_evt_stat->data_type = PHY_EVT_DATA_TYPE_NON;

}
void PhyEvtStat_Dump(PhyEvtStat *phy_evt_stat, FILE *fptr)
{
	int index;
	fprintf( fptr, "  PhyEvtStat = 0x%08x\n", (unsigned int)phy_evt_stat );
	fprintf( fptr, "   id   = %d\n", phy_evt_stat->id );
	fprintf( fptr, "   size = %d\n", phy_evt_stat->size );
	fprintf( fptr, "   data_type = %d\n", phy_evt_stat->data_type );
	if( phy_evt_stat->ti_evid_hi != 0xFFFFFFFF )
		fprintf( fptr, "   ti_evid_hi = 0x%08x\n", phy_evt_stat->ti_evid_hi );
	if( phy_evt_stat->ti_evid_lo != 0xFFFFFFFF )
		fprintf( fptr, "   ti_evid_lo = 0x%08x\n", phy_evt_stat->ti_evid_lo );
	if( phy_evt_stat->ti_tstp_hi != 0xFFFFFFFF )
		fprintf( fptr, "   ti_tstp_hi = 0x%08x\n", phy_evt_stat->ti_tstp_hi );
	if( phy_evt_stat->ti_tstp_lo != 0xFFFFFFFF )
		fprintf( fptr, "   ti_tstp_lo = 0x%08x\n", phy_evt_stat->ti_tstp_lo );
	fprintf( fptr, "   nb_of_beu = %d\n", phy_evt_stat->nb_of_beu );
	if( phy_evt_stat->nb_of_beu > 0 )
	{
		for( index=0; index<Def_MaxNbOfBeu; index++ )
		{
			if( phy_evt_stat->beu_smp_num[index] > 0 )
			{
				if( phy_evt_stat->beu_evid_hi[index] != 0xFFFFFFFF )
					fprintf( fptr, "     beu %d evid_hi = 0x%04x\n", index, phy_evt_stat->beu_evid_hi[index] );
				if( phy_evt_stat->beu_evid_mi[index] != 0xFFFFFFFF )
					fprintf( fptr, "     beu %d evid_mi = 0x%04x\n", index, phy_evt_stat->beu_evid_mi[index] );
				if( phy_evt_stat->beu_evid_lo[index] != 0xFFFFFFFF )
					fprintf( fptr, "     beu %d evid_lo = 0x%04x\n", index, phy_evt_stat->beu_evid_lo[index] );
				if( phy_evt_stat->beu_evid_ac[index] != 0xFFFFFFFF )
					fprintf( fptr, "     beu %d evid_ac = 0x%04x %d\n", index, phy_evt_stat->beu_evid_ac[index], phy_evt_stat->beu_evid_ac[index] );

				if( phy_evt_stat->beu_tstp_hi[index] != 0xFFFFFFFF )
					fprintf( fptr, "     beu %d tstp_hi = 0x%04x\n", index, phy_evt_stat->beu_tstp_hi[index] );
				if( phy_evt_stat->beu_tstp_mi[index] != 0xFFFFFFFF )
					fprintf( fptr, "     beu %d tstp_mi = 0x%04x\n", index, phy_evt_stat->beu_tstp_mi[index] );
				if( phy_evt_stat->beu_tstp_lo[index] != 0xFFFFFFFF )
					fprintf( fptr, "     beu %d tstp_lo = 0x%04x\n", index, phy_evt_stat->beu_tstp_lo[index] );
				if( phy_evt_stat->beu_tstp_fn[index] != 0xFFFFFFFF )
					fprintf( fptr, "     beu %d tstp_fn = 0x%04x\n", index, phy_evt_stat->beu_tstp_fn[index] );
				fprintf( fptr, "     beu %d smp_num = %d\n", index, phy_evt_stat->beu_smp_num[index] );
			}
		}
	}
	fprintf( fptr, "   nb_of_feu = %d\n", phy_evt_stat->nb_of_feu );
	if( phy_evt_stat->nb_of_feu > 0 )
	{
		for( index=0; index<Def_MaxNbOfFeu; index++ )
		{
			if( phy_evt_stat->feu_smp_num[index] > 0 )
			{
				if( phy_evt_stat->feu_evid[index] >= 0 )
					fprintf( fptr, "     feu %2d evid = 0x%03x %4d\n", index, phy_evt_stat->feu_evid[index], phy_evt_stat->feu_evid[index] );
				if( phy_evt_stat->feu_tstp[index] >= 0 )
					fprintf( fptr, "     feu %2d tstp = 0x%03x %4d\n", index, phy_evt_stat->feu_tstp[index], phy_evt_stat->feu_tstp[index] );
				if( phy_evt_stat->feu_ftstp[index] >= 0 )
					fprintf( fptr, "     feu %2d ftst =       %4d\n", index, phy_evt_stat->feu_ftstp[index] );
				if( phy_evt_stat->feu_size[index] >= 0 )
					fprintf( fptr, "     feu %2d size =       %4d\n", index, phy_evt_stat->feu_size[index] );
				fprintf( fptr, "     feu %2d smp_num = %d\n", index, phy_evt_stat->feu_smp_num[index] );
				fprintf( fptr, "     feu %2d beu_id  = %d\n", index, phy_evt_stat->feu_beu_id[index] );
			}
		}
	}
}
int PhyEvtStat_Validate(PhyEvtStat *phy_evt_stat)
{
	int beu;
	int feu;
	int error;
	unsigned int beu2ti_tstp_offset;
	unsigned int feu2beu_tstp_offset;

	if( phy_evt_stat->ti_evid_lo == 0xFFFFFFFF )
	{
		fprintf( stderr, "%s: ERROR ti_evid_lo=-1\n", __FUNCTION__ );
		return -1;
	}

	if( phy_evt_stat->nb_of_beu == 0 )
	{
		fprintf( stderr, "%s: ERROR nb_of_beu==0\n", __FUNCTION__ );
		return -2;
	}

	error = 0;

	// BEU-s
	for( beu=0; beu<Def_MaxNbOfBeu; beu++ )
	{
		// Validate beu event ID
		if( phy_evt_stat->beu_smp_num[beu] > 0 )
		{
			if( phy_evt_stat->beu_evid_lo[beu] != (phy_evt_stat->ti_evid_lo & 0x7FFF) )
			{
				fprintf( stderr, "%s: ERROR beu %d evid_lo = 0x%04x != ti_evid_lo[15:0] = 0x%04x\n",
					__FUNCTION__, beu, phy_evt_stat->beu_evid_lo[beu], (phy_evt_stat->ti_evid_lo & 0xFFFF) );
				error |= 4;
			}
			if( phy_evt_stat->beu_evid_mi[beu] != ((phy_evt_stat->ti_evid_lo >> 15)& 0x7FFF) )
			{
				fprintf( stderr, "%s: ERROR beu %d evid_mi = 0x%04x != ti_evid_lo[31:16] = 0x%04x\n",
					__FUNCTION__, beu, phy_evt_stat->beu_evid_mi[beu], ((phy_evt_stat->ti_evid_lo >> 15)& 0x7FFF) );
				error |= 8;
			}
			if( phy_evt_stat->beu_evid_ac[beu] != (phy_evt_stat->beu_evid_lo[beu] & 0xFFF) )
			{
				fprintf( stderr, "%s: WARNING beu %d beu_evid_ac = 0x%03x != beu_evid_lo[11:0] = 0x%03x\n",
					__FUNCTION__, beu, phy_evt_stat->beu_evid_ac[beu], (phy_evt_stat->beu_evid_lo[beu] & 0xFFF) );
			}

			// Timestamp
			if( phy_evt_stat->data_type == PHY_EVT_DATA_TYPE_RAW )
			{
				beu2ti_tstp_offset = ( ((((unsigned int)phy_evt_stat->beu_tstp_lo[beu])<<1)&0xFFF) - (phy_evt_stat->ti_tstp_lo & 0xFFF) ) & 0xFFF;
	//			beu2ti_tstp_offset = (  (((unsigned int)phy_evt_stat->beu_tstp_lo[beu])&0xFFF) - ((phy_evt_stat->ti_tstp_lo >> 1) & 0xFFF) ) & 0xFFF;
				if( timing_histos_raw.beu2ti_tstp_offset[beu] == 0xFFFFffff )
				{
					timing_histos_raw.beu2ti_tstp_offset[beu] = beu2ti_tstp_offset;
				}
				if( timing_histos_raw.beu2ti_tstp_offset[beu] == 0xFFFFffff )
				{
					timing_histos_raw.beu2ti_tstp_offset[beu] = beu2ti_tstp_offset;
				}
/*
printf("ti=0x%08x beu(%d)=0x%04x offset=0x%08x first_offset=0x%08x diff=0x%04x\n",
phy_evt_stat->ti_tstp_lo,
beu, phy_evt_stat->beu_tstp_lo[beu],
beu2ti_tstp_offset, timing_histos.beu2ti_tstp_offset[beu],
beu2ti_tstp_offset - timing_histos.beu2ti_tstp_offset[beu]);
//getchar();
*/
				Histo_Add(&(timing_histos_raw.beu2ti_tstp_histo[beu]), (beu2ti_tstp_offset - timing_histos_raw.beu2ti_tstp_offset[beu]) );
				// Fine timestamp
				if( phy_evt_stat->beu_tstp_fn[beu] == 0x0700 )
					Histo_Add(&(timing_histos_raw.beu_ftstp_histo[beu]), 0 );
				else if( phy_evt_stat->beu_tstp_fn[beu] == 0x0f00 )
					Histo_Add(&(timing_histos_raw.beu_ftstp_histo[beu]), 1 );
				else
				{
					fprintf( stderr, "%s: WARNING beu %d beu_evid_ac = 0x%03x unknown ftstp=0x%04x\n",
						__FUNCTION__, beu, phy_evt_stat->beu_evid_ac[beu], phy_evt_stat->beu_tstp_fn[beu] );
				}
			}
			else if( phy_evt_stat->data_type == PHY_EVT_DATA_TYPE_CMP )
			{
				beu2ti_tstp_offset = ( ((((unsigned int)phy_evt_stat->beu_tstp_lo[beu])<<1)&0xFFF) - (phy_evt_stat->ti_tstp_lo & 0xFFF) ) & 0xFFF;
	//			beu2ti_tstp_offset = (  (((unsigned int)phy_evt_stat->beu_tstp_lo[beu])&0xFFF) - ((phy_evt_stat->ti_tstp_lo >> 1) & 0xFFF) ) & 0xFFF;
				if( timing_histos_cmp.beu2ti_tstp_offset[beu] == 0xFFFFffff )
				{
					timing_histos_cmp.beu2ti_tstp_offset[beu] = beu2ti_tstp_offset;
				}
				if( timing_histos_cmp.beu2ti_tstp_offset[beu] == 0xFFFFffff )
				{
					timing_histos_cmp.beu2ti_tstp_offset[beu] = beu2ti_tstp_offset;
				}
				Histo_Add(&(timing_histos_cmp.beu2ti_tstp_histo[beu]), (beu2ti_tstp_offset - timing_histos_cmp.beu2ti_tstp_offset[beu]) );
				// Fine timestamp
				if( phy_evt_stat->beu_tstp_fn[beu] == 0x0700 )
					Histo_Add(&(timing_histos_cmp.beu_ftstp_histo[beu]), 0 );
				else if( phy_evt_stat->beu_tstp_fn[beu] == 0x0f00 )
					Histo_Add(&(timing_histos_cmp.beu_ftstp_histo[beu]), 1 );
				else
				{
					fprintf( stderr, "%s: WARNING beu %d beu_evid_ac = 0x%03x unknown ftstp=0x%04x\n",
						__FUNCTION__, beu, phy_evt_stat->beu_evid_ac[beu], phy_evt_stat->beu_tstp_fn[beu] );
				}
/*
				beu2ti_tstp_offset = ( (((unsigned int)phy_evt_stat->beu_tstp_lo[beu])&0xFFF) - (phy_evt_stat->ti_tstp_lo & 0xFFF) ) & 0xFFF;
				if( timing_histos_cmp.beu2ti_tstp_offset[beu] == 0xFFFFffff )
				{
					timing_histos_cmp.beu2ti_tstp_offset[beu] = beu2ti_tstp_offset;
				}
				if( timing_histos_cmp.beu2ti_tstp_offset[beu] == 0xFFFFffff )
				{
					timing_histos_cmp.beu2ti_tstp_offset[beu] = beu2ti_tstp_offset;
				}
				Histo_Add(&(timing_histos_cmp.beu2ti_tstp_histo[beu]), (beu2ti_tstp_offset - timing_histos_cmp.beu2ti_tstp_offset[beu]) );
				// Fine timestamp
				Histo_Add(&(timing_histos_cmp.beu_ftstp_histo[beu]), phy_evt_stat->beu_tstp_fn[beu] );
*/
			}
			else
			{
				fprintf( stderr, "%s: WARNING phy %d unknown data type=%d\n", __FUNCTION__, phy_evt_stat->id, phy_evt_stat->data_type );
			}
		} // if( phy_evt_stat->beu_smp_num[beu] > 0 )
	} // for( beu=0; beu<Def_MaxNbOfBeu; beu++ )

	// FEU-s
	if( phy_evt_stat->nb_of_feu > 0 )
	{
		for( feu=0; feu<Def_MaxNbOfFeu; feu++ )
		{
			if( phy_evt_stat->feu_smp_num[feu] > 0 )
			{
				// Validate FEU event ID
				beu = phy_evt_stat->feu_beu_id[feu];
				if( phy_evt_stat->feu_evid[feu] != phy_evt_stat->beu_evid_ac[beu] )
				{
					fprintf( stderr, "%s: ERROR feu %d evid = 0x%03x != beu_evid_ac = 0x%03x of beu %d\n",
						__FUNCTION__, feu, phy_evt_stat->feu_evid[feu], phy_evt_stat->beu_evid_ac[beu], beu );
					error |= 16;
				}

				// Timestamp
				feu2beu_tstp_offset = ( ((unsigned int)phy_evt_stat->feu_tstp[feu]) - (((unsigned int)phy_evt_stat->beu_tstp_lo[beu])&0xFFF) ) & 0xFFF;
				if( timing_histos_raw.feu2beu_tstp_offset[feu] == 0xFFFFffff )
				{
					timing_histos_raw.feu2beu_tstp_offset[feu] = feu2beu_tstp_offset;
					timing_histos_raw.feu_beu_id[feu] = beu;
				}
				if( timing_histos_cmp.feu2beu_tstp_offset[feu] == 0xFFFFffff )
				{
					timing_histos_cmp.feu2beu_tstp_offset[feu] = feu2beu_tstp_offset;
					timing_histos_cmp.feu_beu_id[feu] = beu;
				}
				Histo_Add(&(timing_histos_raw.feu2beu_tstp_histo[feu]), (feu2beu_tstp_offset - timing_histos_raw.feu2beu_tstp_offset[feu]) );
				Histo_Add(&(timing_histos_cmp.feu2beu_tstp_histo[feu]), (feu2beu_tstp_offset - timing_histos_cmp.feu2beu_tstp_offset[feu]) );
/*
printf("beu(%d)=0x%04x 0x%04x 0x%04x feu(%2d)=0x%04x offset=0x%08x first_offset=0x%08x diff=0x%08x\n",
beu, phy_evt_stat->beu_tstp_hi[beu], phy_evt_stat->beu_tstp_mi[beu], phy_evt_stat->beu_tstp_lo[beu],
feu, phy_evt_stat->feu_tstp[feu], feu2beu_tstp_offset, timing_histos.feu2beu_tstp_offset[feu],
(feu2beu_tstp_offset - timing_histos.feu2beu_tstp_offset[feu]) );
//getchar();
*/

				Histo_Add(&(timing_histos_raw.feu_ftstp_histo[feu]), phy_evt_stat->feu_ftstp[feu] );
				Histo_Add(&(timing_histos_cmp.feu_ftstp_histo[feu]), phy_evt_stat->feu_ftstp[feu] );
			} // if( phy_evt_stat->feu_smp_num[feu] > 0 )
		} // for( feu=0; feu<Def_MaxNbOfFeu; feu++ )
	} // if( phy_evt_stat->nb_of_feu > 0 )

	return( -error );
}

// Per event statistics
PhyEvtStat phy_evt_stat_raw;
PhyEvtStat phy_evt_stat_cmp;

int FeuWordWrite( unsigned short word, FILE* fptr )
{
	unsigned char feu_word[2];
	int feu_wr_len;

	feu_word[0] = (word >> 8) & 0xFF;
	feu_word[1] =  word       & 0xFF;
	if( ( feu_wr_len = fwrite(feu_word, 1, 2, fptr) ) != 2 )
	{
		if( ferror( fptr ) )
		{
			fprintf( stderr, "%s: fwrite failed with %s for feu binary file\n", __FUNCTION__, strerror(errno) );
			return -1;
		}
	}
	return feu_wr_len;
};

int PhyEvtStat_SaveFdf(PhyEvtStat *phy_evt_stat, int feu_id, int zero_sup, char *base_file_name)
{
	int wr_len = 0;
	char feu_bin_data_file_name[128];
	unsigned short word;
	unsigned short feu_data_len;
	short feu_chan;
	short dream;
	short cur_dream;
	short dream_chan;
	int samples;
	int chan;

	// Open File if necessary
	if( feu_data_fptr[feu_id] == (FILE *)NULL )
	{
		if( (first_phy_blk != 0) || (last_phy_blk != 0x7fffFFFF) )
			sprintf( feu_bin_data_file_name, "%s_%dTo%d_%02d.fdf", base_file_name, first_phy_blk, last_phy_blk, feu_id );
		else
			sprintf( feu_bin_data_file_name, "%s_%02d.fdf", base_file_name, feu_id );
//fprintf( stderr, "feu_id=%d file=%s to be opend\n", feu_id, base_file_name);
		if( (feu_data_fptr[feu_id]=fopen(feu_bin_data_file_name, "wb")) == ((FILE *)NULL) )
		{
			fprintf( stderr, "%s: fopen failed for file %s in binary write mode with %s\n",
				__FUNCTION__, feu_bin_data_file_name, strerror(errno) );
			return -1;
		}
		feu_data_fptr_cntr++;
	}
	// Feu ID
//fprintf( stdout, "%s: feu_id=%d zs=%d nsmp=%d feu_chan_num=%d\n", __FUNCTION__, feu_id, zero_sup, phy_evt_stat->feu_smp_num[feu_id], feu_chan_num);
	for( samples=0; samples<phy_evt_stat->feu_smp_num[feu_id]; samples++ )
	{
		feu_data_len = 0;
		word = 0x6000 | (zero_sup<<10) | (feu_id & 0xFF);         FeuWordWrite( word, feu_data_fptr[feu_id] ); feu_data_len++;
		word = 0x6000 | (phy_evt_stat->feu_evid[feu_id] & 0xFFF); FeuWordWrite( word, feu_data_fptr[feu_id] ); feu_data_len++;
		word = 0x6000 | (phy_evt_stat->feu_tstp[feu_id] & 0xFFF); FeuWordWrite( word, feu_data_fptr[feu_id] ); feu_data_len++;
		word = 0x6000 | (( (samples << 3) | phy_evt_stat->feu_ftstp[feu_id]) & 0xFFF); FeuWordWrite( word, feu_data_fptr[feu_id] ); feu_data_len++;
		cur_dream = -1;
		for( feu_chan=0; feu_chan<feu_chan_num; feu_chan++ )
		{
			chan = feu_chan_val[feu_chan]-1;
			dream      = chan / 64;
			dream_chan = chan % 64;
//printf("%s: feu_id=%d feu_chan_num=%d feu_chan=%d chan=%d dream=%d dream_chan=%d ", __FUNCTION__, feu_id, feu_chan_num, feu_chan, chan, dream, dream_chan);
			if( zero_sup == 0 )
			{
				if( dream != cur_dream )
				{
					if( (cur_dream >= 0) )
					{
						// write current dream trailer
						word = 0x4000 | (cur_dream<<9); FeuWordWrite( word, feu_data_fptr[feu_id] ); feu_data_len++;
					}
					// write new dream header
					cur_dream = dream;
					word = 0x2000 | (cur_dream<<9); FeuWordWrite( word, feu_data_fptr[feu_id] ); feu_data_len++;
				}
				word = feu_smp_val[feu_chan][samples] & 0xFFF; FeuWordWrite( word, feu_data_fptr[feu_id] ); feu_data_len++;
			}
			else
			{
				word = 0x1000 | (dream<<6) | chan;    FeuWordWrite( word, feu_data_fptr[feu_id] ); feu_data_len++;
//printf(" chan=0x%04x", word);
				word = feu_smp_val[feu_chan][samples] & 0xFFF; FeuWordWrite( word, feu_data_fptr[feu_id] ); feu_data_len++;
//printf(" smp=0x%04x\n", word);
			}
		}
		// write current dream trailer if needed
		if( (zero_sup == 0) && (cur_dream >= 0) )
		{
				word = 0x4000 | (cur_dream<<9); FeuWordWrite( word, feu_data_fptr[feu_id] ); feu_data_len++;
		}
		// Feu Trailler
		word = 0x7000 | ((feu_data_len+1) & 0xFFF);
		// Set end of event flag for last sample
		if( samples == (phy_evt_stat->feu_smp_num[feu_id] - 1) )
			word |= 0x800;
		 FeuWordWrite( word, feu_data_fptr[feu_id] );
		// VEP emulator
		word = 0; FeuWordWrite( word, feu_data_fptr[feu_id] );
//fprintf( stdout, "   feu_id=%d smp=%d feu_data_len=%d\n", feu_id, samples, feu_data_len);
	}
	return wr_len;
}

/***************
 Network Header
 **************/
typedef struct _NetHdr
{
	int size;
	int blk_num;
	int hdr_len;
	int evt_cnt;
	int rsvd1;
	int bit_and_ver;
	int rsvd2;
	int magic;
} NetHdr;
#define Def_NetHdrSize (sizeof(NetHdr)/sizeof(int))
int NetHdr_Printf( FILE *fptr, NetHdr *net_hdr )
{
	// Check for NULL parameters
	if( fptr == (FILE *)NULL )
	{
		fprintf( stderr, "%s: fptr = NULL\n", __FUNCTION__ );
		return -1;
	}
	if( net_hdr == (NetHdr *)NULL )
	{
		fprintf( stderr, "%s: net_hdr = NULL\n", __FUNCTION__ );
		return -1;
	}
	// Dump net_hdr
	fprintf( fptr, "NetHdr =0x%08x\n",      (unsigned int)net_hdr );
	fprintf( fptr, " size    =0x%08x %d\n", (unsigned int)(net_hdr->size),        net_hdr->size );
	fprintf( fptr, " blk_num =0x%08x %d\n", (unsigned int)(net_hdr->blk_num),     net_hdr->blk_num );
	fprintf( fptr, " hdr_len =0x%08x %d\n", (unsigned int)(net_hdr->hdr_len),     net_hdr->hdr_len );
	fprintf( fptr, " evt_cnt =0x%08x %d\n", (unsigned int)(net_hdr->evt_cnt),     net_hdr->evt_cnt );
	fprintf( fptr, " rsvd1   =0x%08x\n",    (unsigned int)(net_hdr->rsvd1) );
	fprintf( fptr, " bit&ver =0x%08x %d\n", (unsigned int)(net_hdr->bit_and_ver), net_hdr->bit_and_ver );
	fprintf( fptr, " Rsvd2   =0x%08x\n",    (unsigned int)(net_hdr->rsvd2) );
	fprintf( fptr, " magic   =0x%08x\n",    (unsigned int)(net_hdr->magic),       net_hdr->magic );
	return 0;
}

/***************
 Entry Header
 **************/
typedef struct _EntryHdr
{
	int size;
	int type;
} EntryHdr;
#define Def_EntryHdrSize (sizeof(EntryHdr)/sizeof(int))
int EntryHdr_Printf( FILE *fptr, EntryHdr *ent_hdr )
{
	// Check for NULL parameters
	if( fptr == (FILE *)NULL )
	{
		fprintf( stderr, "%s: fptr = NULL\n", __FUNCTION__ );
		return -1;
	}
	if( ent_hdr == (EntryHdr *)NULL )
	{
		fprintf( stderr, "%s: ent_hdr = NULL\n", __FUNCTION__ );
		return -1;
	}
	// Dump net_hdr
	fprintf( fptr, "EntryHdr =0x%08x\n", (unsigned int)ent_hdr );
	fprintf( fptr, " size=0x%08x %d\n",  (unsigned int)(ent_hdr->size), ent_hdr->size );
	fprintf
	(
		fptr,
		" type=0x%08x tag=0x%04x dat=0x%02x num=0x%02x\n",
		(unsigned int)(ent_hdr->type),
		DEF_GetEvioEntryTag(ent_hdr->type),
		DEF_GetEvioEntryDat(ent_hdr->type),
		DEF_GetEvioEntryNum(ent_hdr->type)
	);
	return 0;
}

/***********************************************************
 * Run Control Entries
 ***********************************************************/
#define Def_RcEntrySize (sizeof(RcPreStart)/sizeof(int))
typedef struct _RcPreStart
{
	int size;
	int type;
	int time;
	int run_num;
	int run_typ;
} RcPreStart;
int RcPreStart_Printf( FILE *fptr, RcPreStart *rc_ent )
{
	// Check for NULL parameters
	if( fptr == (FILE *)NULL )
	{
		fprintf( stderr, "%s: fptr = NULL\n", __FUNCTION__ );
		return -1;
	}
	if( rc_ent == (RcPreStart *)NULL )
	{
		fprintf( stderr, "%s: rc_ent = NULL\n", __FUNCTION__ );
		return -1;
	}
	// Dump net_hdr
	fprintf( fptr, "RcPreStart =0x%08x\n",  (unsigned int)rc_ent );
	fprintf( fptr, " size    =0x%08x %d\n", (unsigned int)(rc_ent->size),     rc_ent->size );
	fprintf( fptr, " type    =0x%08x\n",    (unsigned int)(rc_ent->type) );
	fprintf( fptr, " time    =0x%08x %d\n", (unsigned int)(rc_ent->time),     rc_ent->time );
	fprintf( fptr, " run_num =0x%08x %d\n", (unsigned int)(rc_ent->run_num),  rc_ent->run_num );
	fprintf( fptr, " run_typ =0x%08x %d\n", (unsigned int)(rc_ent->run_typ),  rc_ent->run_typ );
	return 0;
}
typedef struct _RcGo
{
	int size;
	int type;
	int time;
	int rsvd;
	int evt_num;
} RcGo;
int RcGo_Printf( FILE *fptr, RcGo *rc_ent )
{
	// Check for NULL parameters
	if( fptr == (FILE *)NULL )
	{
		fprintf( stderr, "%s: fptr = NULL\n", __FUNCTION__ );
		return -1;
	}
	if( rc_ent == (RcGo *)NULL )
	{
		fprintf( stderr, "%s: rc_ent = NULL\n", __FUNCTION__ );
		return -1;
	}
	// Dump net_hdr
	fprintf( fptr, "RcGo =0x%08x\n",        (unsigned int)rc_ent );
	fprintf( fptr, " size    =0x%08x %d\n", (unsigned int)(rc_ent->size),     rc_ent->size );
	fprintf( fptr, " type    =0x%08x\n",    (unsigned int)(rc_ent->type) );
	fprintf( fptr, " time    =0x%08x %d\n", (unsigned int)(rc_ent->time),     rc_ent->time );
	fprintf( fptr, " rsvd    =0x%08x\n",    (unsigned int)(rc_ent->rsvd) );
	fprintf( fptr, " evt_num =0x%08x %d\n", (unsigned int)(rc_ent->evt_num),  rc_ent->evt_num );
	return 0;
}

typedef struct _RcEnd
{
	int size;
	int type;
	int time;
	int rsvd;
	int evt_num;
} RcEnd;
int RcEnd_Printf( FILE *fptr, RcEnd *rc_ent )
{
	// Check for NULL parameters
	if( fptr == (FILE *)NULL )
	{
		fprintf( stderr, "%s: fptr = NULL\n", __FUNCTION__ );
		return -1;
	}
	if( rc_ent == (RcEnd *)NULL )
	{
		fprintf( stderr, "%s: rc_ent = NULL\n", __FUNCTION__ );
		return -1;
	}
	// Dump net_hdr
	fprintf( fptr, "RcEnd =0x%08x\n",       (unsigned int)rc_ent );
	fprintf( fptr, " size    =0x%08x %d\n", (unsigned int)(rc_ent->size),     rc_ent->size );
	fprintf( fptr, " type    =0x%08x\n",    (unsigned int)(rc_ent->type) );
	fprintf( fptr, " time    =0x%08x %d\n", (unsigned int)(rc_ent->time),     rc_ent->time );
	fprintf( fptr, " rsvd    =0x%08x\n",    (unsigned int)(rc_ent->rsvd) );
	fprintf( fptr, " evt_num =0x%08x %d\n", (unsigned int)(rc_ent->evt_num),  rc_ent->evt_num );
	return 0;
}
typedef struct _RcSync
{
	int size;
	int type;
	int time;
	int lst_num;
	int evt_num;
} RcSync;
int RcSync_Printf( FILE *fptr, RcSync *rc_ent )
{
	// Check for NULL parameters
	if( fptr == (FILE *)NULL )
	{
		fprintf( stderr, "%s: fptr = NULL\n", __FUNCTION__ );
		return -1;
	}
	if( rc_ent == (RcSync *)NULL )
	{
		fprintf( stderr, "%s: rc_ent = NULL\n", __FUNCTION__ );
		return -1;
	}
	// Dump net_hdr
	fprintf( fptr, "RcSync =0x%08x\n",      (unsigned int)rc_ent );
	fprintf( fptr, " size    =0x%08x %d\n", (unsigned int)(rc_ent->size),    rc_ent->size );
	fprintf( fptr, " type    =0x%08x\n",    (unsigned int)(rc_ent->type) );
	fprintf( fptr, " time    =0x%08x %d\n", (unsigned int)(rc_ent->time),    rc_ent->time );
	fprintf( fptr, " lst_num =0x%08x %d\n", (unsigned int)(rc_ent->lst_num), rc_ent->lst_num );
	fprintf( fptr, " evt_num =0x%08x %d\n", (unsigned int)(rc_ent->evt_num), rc_ent->evt_num );
	return 0;
}

/***********************************************************
 * Event Block
 ***********************************************************/
typedef struct _EvtBlk
{
	int size;
	int type;
	int num;
	int clas;
	int stat;
} EvtBlk;
EvtBlk evt_blk;
int EvtBlk_Fill( EvtBlk *evt_blk, unsigned int *buf )
{
	unsigned int *ptr;

	// Check for NULL parameters
	if( buf == (unsigned int *)NULL )
	{
		fprintf( stderr, "%s: buf = NULL\n", __FUNCTION__ );
		return -1;
	}
	if( evt_blk == (EvtBlk *)NULL )
	{
		fprintf( stderr, "%s: evt_blk = NULL\n", __FUNCTION__ );
		return -1;
	}
	ptr = (int *)buf;
	evt_blk->size = *ptr++;
	evt_blk->type = *ptr++;
	evt_blk->num  = *ptr++;
	evt_blk->clas = *ptr++;
	evt_blk->stat = *ptr;
	return sizeof(EvtBlk)/sizeof(int);
}
int EvtBlk_Printf( EvtBlk *evt_blk, FILE *fptr )
{
	// Check for NULL parameters
	if( fptr == (FILE *)NULL )
	{
		fprintf( stderr, "%s: fptr = NULL\n", __FUNCTION__ );
		return -1;
	}
	if( evt_blk == (EvtBlk *)NULL )
	{
		fprintf( stderr, "%s: evt_blk = NULL\n", __FUNCTION__ );
		return -1;
	}
	// Dump net_hdr
	fprintf( fptr, "EvtBlk=0x%08x\n",    (unsigned int)evt_blk );
	fprintf( fptr, " size =0x%08x %d\n", (unsigned int)(evt_blk->size),     evt_blk->size );
	fprintf( fptr, " type =0x%08x %d\n", (unsigned int)(evt_blk->type),     evt_blk->type );
	fprintf( fptr, " num  =0x%08x %d\n", (unsigned int)(evt_blk->num),      evt_blk->num  );
	fprintf( fptr, " clas =0x%08x %d\n", (unsigned int)(evt_blk->clas),     evt_blk->clas );
	fprintf( fptr, " stat =0x%08x %d\n", (unsigned int)(evt_blk->stat),     evt_blk->stat );
	return 0;
}


/***********************************************************
 * Ti Bank
 ***********************************************************/
typedef struct _TiBank_Dis
{
	int size;
	unsigned int header;
	unsigned int type_count;
	unsigned int evid_lo;
	unsigned int tstp_lo;
	unsigned int evid_tstp_hi;
} TiBank_Dis;
TiBank_Dis ti_bank;
int TiBank_Disent_Fill( TiBank_Dis *ti_bank, unsigned int *buf )
{
	unsigned int *ptr;

	// Check for NULL parameters
	if( buf == (unsigned int *)NULL )
	{
		fprintf( stderr, "%s: buf = NULL\n", __FUNCTION__ );
		return -1;
	}
	if( ti_bank == (TiBank_Dis *)NULL )
	{
		fprintf( stderr, "%s: ti_bank = NULL\n", __FUNCTION__ );
		return -1;
	}
	ptr = buf;
	ti_bank->size         = *(int *)ptr++;
	ti_bank->header       = *ptr++;
	ti_bank->type_count   = *ptr++;
	ti_bank->evid_lo      = *ptr++;
	ti_bank->tstp_lo      = *(int *)ptr++;
	ti_bank->evid_tstp_hi = *(int *)ptr;
	if( (ti_bank->header & 0xFFFFFF00) != 0xe10a0100 )
	{
		fprintf( stderr, "%s: Unxexpected TI bank header 0x%08x should be 0xe10a0100\n", __FUNCTION__, ti_bank->header );
		return -2;
	}
	if
	(
		((ti_bank->type_count & 0xFFFFFF00) != 0X03010000)
		&&
		((ti_bank->type_count & 0xFFFFFF00) != 0X02010000)
		&&
		((ti_bank->type_count & 0xFFFFFF00) != 0X01010000)
		&&
		((ti_bank->type_count & 0xFFFFFF00) != 0X00010000)
		&&
		((ti_bank->type_count & 0xFFFFFF00) != 0Xfe010000)
	)
	{
		fprintf( stderr, "%s: Unexpected TI banck type 0x%08x\n", __FUNCTION__, ti_bank->type_count );
		return -3;
	}
	return( sizeof(TiBank_Dis) / sizeof(int) );
}

int TiBank_Disent_Printf( TiBank_Dis *ti_bank, FILE *fptr )
{
	// Check for NULL parameters
	if( ti_bank == (TiBank_Dis *)NULL )
	{
		fprintf( stderr, "%s: ti_bank = NULL\n", __FUNCTION__ );
		return -1;
	}
	if( fptr == (FILE *)NULL )
	{
		fprintf( stderr, "%s: fptr = NULL\n", __FUNCTION__ );
		return -1;
	}

	fprintf( fptr, "TiBank =0x%08x\n",   (unsigned int)ti_bank );
	fprintf( fptr, " size          =0x%08x %d\n", ti_bank->size, ti_bank->size );
	fprintf( fptr, " header        =0x%08x\n", ti_bank->header );
	fprintf( fptr, "   type_count  =0x%08x size=%d\n",  ti_bank->type_count, ti_bank->type_count & 0xFF );
	fprintf( fptr, "   evid_lo     =0x%08x %d\n",  ti_bank->evid_lo,  ti_bank->evid_lo );
	fprintf( fptr, "   tstp_lo     =0x%08x\n",     ti_bank->tstp_lo );
	fprintf( fptr, "   evid_tstp_hi=0x%08x eid=0x%04x tstp=0x%04x\n", ti_bank->evid_tstp_hi, ti_bank->evid_tstp_hi & 0xFFFF, (ti_bank->evid_tstp_hi >> 16) & 0xFFFF );
	return 0;
}

/***********************************************************
 * Composite MVT Data 
 ***********************************************************/
#define CMP_GetBeuId( crate ) ((((crate)>>5)&0x07)+1)
#define CMP_GetLnkId( crate ) (( (crate)    &0x1F)-1)

#define CMP_GetFeuFineTstp(  tstp_lo ) (  (tstp_lo)     &0x0007)
#define CMP_GetFeuTstp(      tstp_lo ) ( ((tstp_lo)>> 3)&0x0FFF)
#define CMP_GetFeuZs(        tstp_lo ) ( ((tstp_lo)>>15)&0x01)
#define CMP_GetBeuTstp16Lsb( tstp_lo ) ( ((tstp_lo)>>16)&0xFFFF)
#define CMP_GetBeuTstp30Msb( tstp_hi ) (  (tstp_hi)     &0x3FFFFFFF)

int MvtCmpDataRead( unsigned int *buf, int buf_len )
{
	unsigned int *ptr;
	int index;
	// Evio header
	int size;
	unsigned int type;

	// Format descriptor
	unsigned int desc;
	int desc_size;
	int desc_type;
	int desc_bytes;
	unsigned char *desc_ptr;
	char desc_string[16];

	// Composite data
	unsigned char  *u08ptr;
	unsigned short *u16ptr;
	unsigned int   *u32ptr;
	int cmp_size;
	unsigned int cmp_type;
	int crate;
	int evnt_id;
	int tstp_lo;
	int tstp_hi;

	int beu_id;
	int lnk_id;
	int feu_fine_tstp;
	int feu_tstp;
	unsigned short beu_tstp_fn;
	unsigned short beu_tstp_lo;
	unsigned short beu_tstp_mi;
	unsigned short beu_tstp_hi;

	int   nb_of_chan;
	short feu_chan;
	int   nb_of_samples;
	int   sample;
	unsigned short sample_value;

	int dream;
	short dream_chan;

	int zero_sup;

	int ret;

	int cur_beu = -1;
	int cur_feu = -1;
	int cur_drm = -1;

	// Check for NULL parameters
	if( buf == (unsigned int *)NULL )
	{
		fprintf( stderr, "%s: buf = NULL\n", __FUNCTION__ );
		return -1;
	}

//fprintf( stdout, "MvtCmpBuf =0x%08x size=%d\n",   (unsigned int)buf, buf_len );

	ptr = buf;
	size = (int)(*ptr++);
	type = *ptr++;
//fprintf( stdout, " size          =0x%08x %d\n", size, size );
//fprintf( stdout, " type          =0x%08x\n",    type );
	// Get Format descriprot string
	desc = *ptr++;
	desc_size  = desc & 0xFF;
	desc_type  = (desc>>16) & 0xF;
	desc_bytes = (desc>>20) & 0xF;
	if( desc_size != 4 )
	{
		fprintf( stderr, "%s: wrong format descriptor 0x%08x desc_size=%d != 4\n", __FUNCTION__, desc, desc_size );
		return -2;
	}
	if( desc_type != 0x6 )
	{
		fprintf( stderr, "%s: wrong format descriptor 0x%08x desc_type=%d != 0x6 int8\n", __FUNCTION__, desc, desc_type );
		return -3;
	}
	if( desc_bytes != 13 )
	{
		fprintf( stderr, "%s: wrong format descriptor 0x%08x desc_bytes=%d != 13\n", __FUNCTION__, desc, desc_type );
		return -3;
	}
	desc_ptr = (unsigned char *)ptr;
	desc_bytes++;
	for( index=0; index<desc_bytes; index++ )
		desc_string[index] = (char)*desc_ptr++;
//	fprintf( stdout, "desc_string=%s\n", desc_string );

	ptr += desc_size;

	cmp_size = (int)(*ptr++);
	cmp_type = *ptr++;
//fprintf( stdout, " cmp_size          =0x%08x %d\n", cmp_size, cmp_size );
//fprintf( stdout, " cmp_size          =0x%08x\n",    cmp_type );
	u08ptr = (unsigned char*)ptr;
	cmp_size *= sizeof(unsigned int);
	while( cmp_size > (sizeof(char)+3*sizeof(int)) )
	{
		crate = *u08ptr;
		u08ptr++;
		cmp_size--;

		u32ptr = (unsigned int *)u08ptr;
		evnt_id = *u32ptr++;
		u08ptr += sizeof(unsigned int);
		cmp_size -= sizeof(unsigned int);

		tstp_lo = *u32ptr++;
		u08ptr += sizeof(unsigned int);
		cmp_size -= sizeof(unsigned int);
//fprintf( stdout, " tstp_lo=0x%08x\n", tstp_lo );
		tstp_hi = *u32ptr++;
		u08ptr += sizeof(unsigned int);
		cmp_size -= sizeof(unsigned int);
//fprintf( stdout, " tstp_hi=0x%08x\n", tstp_hi );

		beu_id = CMP_GetBeuId( crate );
		lnk_id = CMP_GetLnkId( crate );
		zero_sup      =   CMP_GetFeuZs(        tstp_lo );
		feu_tstp      =   CMP_GetFeuTstp(      tstp_lo );
		feu_fine_tstp =   CMP_GetFeuFineTstp(  tstp_lo );
		beu_tstp_lo   = ((CMP_GetBeuTstp30Msb( tstp_hi ) & 0x1) << 15) | (CMP_GetBeuTstp16Lsb( tstp_lo ) >> 1);
//fprintf( stdout, " beu_tstp_lo=0x%08x\n", beu_tstp_lo );
		beu_tstp_mi   =  (CMP_GetBeuTstp30Msb( tstp_hi ) >>  1 ) & 0xFFFF;
//fprintf( stdout, " beu_tstp_mi=0x%08x\n", beu_tstp_mi );
		beu_tstp_hi   =  (CMP_GetBeuTstp30Msb( tstp_hi ) >> 17 ) & 0x1FFF;
//fprintf( stdout, " beu_tstp_hi=0x%08x\n", beu_tstp_hi );
		if( ( CMP_GetBeuTstp16Lsb(  tstp_lo ) & 0x1 ) == 0 )
			beu_tstp_fn   =   0x0700;
		else
			beu_tstp_fn   =   0x0F00;

		if( beu_id != cur_beu )
		{
			phy_evt_stat_cmp.nb_of_beu++;
			cur_beu = beu_id;
			phy_evt_stat_cmp.beu_evid_ac[beu_id] =   evnt_id & 0x0FFF;
			phy_evt_stat_cmp.beu_evid_lo[beu_id] =   evnt_id & 0x7FFF;
			phy_evt_stat_cmp.beu_evid_mi[beu_id] = ( evnt_id >> 15 ) & 0x7FFF;
			phy_evt_stat_cmp.beu_evid_hi[beu_id] = ( evnt_id >> 30 ) & 0x3;
			phy_evt_stat_cmp.beu_tstp_hi[beu_id] = beu_tstp_hi;
			phy_evt_stat_cmp.beu_tstp_mi[beu_id] = beu_tstp_mi;
			phy_evt_stat_cmp.beu_tstp_lo[beu_id] = beu_tstp_lo;
			phy_evt_stat_cmp.beu_tstp_fn[beu_id] = beu_tstp_fn;
		}
		if( crate != cur_feu )
		{
			phy_evt_stat_cmp.nb_of_feu++;
			cur_feu = crate;
			cur_drm = -1;
			phy_evt_stat_cmp.feu_evid[crate]   = evnt_id & 0xFFF;
			phy_evt_stat_cmp.feu_ftstp[crate]  = feu_fine_tstp;
			phy_evt_stat_cmp.feu_tstp[crate]   = feu_tstp;
			phy_evt_stat_cmp.feu_beu_id[crate] = beu_id;
		}
//fprintf( stdout, " \n\ncrate %d; evt_id=0x%08x %d; tstp_lo=0x%08x tstp_hi=0x%08x\n", crate, evnt_id, evnt_id, tstp_lo, tstp_hi );
//fprintf( stdout, " Beu %d Link %d\n", beu_id, lnk_id );
//fprintf( stdout, " Feu tstp 0x%03x fine=%d\n", feu_tstp, feu_fine_tstp );
//fprintf( stdout, " Beu tstp hi=0x%04x lo=0x%08x\n", beu_tstp_hi, beu_tstp_lo );

		nb_of_chan = *u32ptr++;
		u08ptr += sizeof(unsigned int);
		cmp_size -= sizeof(unsigned int);
//fprintf( stdout, " nb_of_chan = %d\n", nb_of_chan );
		u16ptr = (unsigned short *)u08ptr;
		for( index=0; index<nb_of_chan; index++ )
		{
			feu_chan = *u16ptr++;
//fprintf( stdout, " chan(%d)", chan );
			u08ptr += sizeof(unsigned short);
			cmp_size -= sizeof(unsigned short);
			dream = feu_chan / 64;
			dream_chan = feu_chan % 64;
			feu_chan_val[index] = feu_chan;

			u32ptr = (unsigned int *)u08ptr;
			nb_of_samples = *u32ptr++;
//fprintf( stdout, " samples(%d) val", nb_of_samples );
			u08ptr += sizeof(unsigned int);
			cmp_size -= sizeof(unsigned int);

			u16ptr = (unsigned short *)u08ptr;
			for( sample=0; sample<nb_of_samples; sample++ )
			{
				sample_value = *u16ptr++;
				u08ptr += sizeof(unsigned short);
				cmp_size -= sizeof(unsigned short);
//fprintf( stdout, " 0x%03x", sample_value );
				feu_smp_val[index][sample] = sample_value;
			}
//fprintf( stdout, "\n" );
		} // for( index=0; index<nb_of_chan; index++ )
//fprintf( stdout, " cmp_size =%d after beu=%d\n", cmp_size, beu_id );
		feu_chan_num = nb_of_chan;
		phy_evt_stat_cmp.feu_smp_num[crate] = nb_of_samples;
		phy_evt_stat_cmp.beu_smp_num[phy_evt_stat_cmp.feu_beu_id[crate]] = nb_of_samples;
		Histo_Add( &(timing_histos_cmp.feu_smp_size_histo[crate]), nb_of_chan*(1+2+nb_of_samples)+6);
		if( do_feu_bin_files && ( cur_feu >= 0 ) )
			if( (first_phy_blk <= evt_blk_cnt) && (evt_blk_cnt <= last_phy_blk) )
				if( (ret = PhyEvtStat_SaveFdf(&phy_evt_stat_cmp, cur_feu, zero_sup, rootfilename(bin_data_file_name))) < 0 )
				{
					fprintf( stderr, "%s: PhyEvtStat_SaveFdf failed for evt=%d feu=%d\n", __FUNCTION__, phy_evt_stat_cmp.id, cur_feu );
					return -4;
				}
	} // while( cmp_size )
/*
	while( cmp_size )
	{
//fprintf( stdout, " cmp_size=%d val=0x%1x\n", cmp_size, *u08ptr);
//u08ptr++;
//cmp_size--;
	}
*/
	return(rd_buf_len);
}


// Read num words from file
int ReadNoumWords( FILE *fptr, int num, unsigned int *rd_buffer )
{
	int rd_len;
	// Check for NULL parameters
	if( fptr == (FILE *)NULL )
	{
		fprintf( stderr, "%s: fptr = NULL\n", __FUNCTION__ );
		return -1;
	}
	if( rd_buffer == (unsigned int *)NULL )
	{
		fprintf( stderr, "%s: rd_buffer = NULL\n", __FUNCTION__ );
		return -1;
	}
	// Read num words
	if( ( rd_len = fread(rd_buffer, sizeof(int), num, bin_data_fptr) ) != num )
	{
		if( ferror(bin_data_fptr) )
		{
			// File IO Error
			fprintf( stderr, "%s: fread failed with %s\n", __FUNCTION__, strerror(errno) );
			return( rd_len ) ;
		}
		if( feof(bin_data_fptr) )
		{
			if( rd_len == 0 )
			{
				fprintf( stderr, "%s: EoF reached\n", __FUNCTION__ );
				return 0; // End of File on event boundary
			}
			else
			{
				// Error: Unexpected End of File
				fprintf( stderr, "%s: unexpected EoF reached rd_len=%d!=%d\n", __FUNCTION__, num );
				return -2;
			}
		}
	}
	return rd_len;
}


/*
 * Usage function
 */
void usage( char *name )
{
	printf( "\nUsage: %s", name );
	printf( " [-b BinData_FileName]" );
	printf( " [-f first_phy_blk]" );
	printf( " [-l last_phy_blk]" );
	printf( " [-A]" );
	printf( " [-a]" );
	printf( " [-F]" );
	printf( " [-s]" );
	printf( " [-p]" );
	printf( " [-v [-v]]" );
	printf( " [-h]" );
	printf( "\n" );
	
	printf( "\n" );
	printf( "-b BinData_FileName     - Binary data file to process; default: %s\n", DEF_BinData_FileName );
	printf( "-f first_phy_blk        - First physics event block to process\n" );
	printf( "-l last_phy_blk         - Last physics event block to process\n" );
	printf( "-F                      - Produce FEU binary data files\n" );
	printf( "-A                      - Produce ASCII data file of headers\n" );
	printf( "-a                      - Produce ASCII data on stdout (-A overtakes)\n" );
	printf( "-s                      - Produce histogram statistics file\n" );
	printf( "-p                      - Pause between each entity; press CR to process next\n" );
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
	int feu_id;
	if( param )
		fprintf( stderr, "cleanup: Entering with %d\n", param );

	// Print statistics
	fprintf(stdout, "%s: %s statistics\n", __FUNCTION__, bin_data_file_name);
	fprintf(stdout, " net_hdr_cnt=%d\n",    net_hdr_cnt);

	fprintf(stdout, " rcsync_cnt  =%d\n",   rcsync_cnt);
	fprintf(stdout, " rcpstart_cnt=%d\n",   rcpstart_cnt);
	fprintf(stdout, " rcgo_cnt    =%d\n",   rcgo_cnt);
	fprintf(stdout, " rcend_cnt   =%d\n",   rcend_cnt);

	fprintf(stdout, " evt_hdr_cnt=%d\n",    evt_hdr_cnt);
	fprintf(stdout, " evt_blk_cnt=%d\n",    evt_blk_cnt);

	fprintf(stdout, " ti_evt_cnt=%d\n",     ti_evt_cnt);

	fprintf(stdout, " mvt_raw_evt_cnt=%d\n", mvt_raw_evt_cnt);
	fprintf(stdout, " mvt_cmp_evt_cnt=%d\n", mvt_cmp_evt_cnt);
	fprintf(stdout, " cfg_evt_cnt=%d\n", cfg_evt_cnt);

	fprintf(stdout, " unknown_asc_cnt=%d\n",     unknown_asc_cnt);
	fprintf(stdout, " unknown_int_cnt=%d\n",     unknown_int_cnt);
	fprintf(stdout, " unknown_cmp_cnt=%d\n",     unknown_cmp_cnt);
	fprintf(stdout, " unknown_ent_cnt=%d\n",     unknown_ent_cnt);

	if( mvt_raw_evt_cnt )
	{
		fprintf(stdout, "\nRAW data stat histos for %d events\n", mvt_raw_evt_cnt);
		TimingHistos_DumpStat(&timing_histos_raw, stdout);
		fprintf(stdout, "\n", mvt_raw_evt_cnt);
	}
	if( mvt_cmp_evt_cnt )
	{
		fprintf(stdout, "\nCMP data stat histos for %d events\n", mvt_cmp_evt_cnt);
		TimingHistos_DumpStat(&timing_histos_cmp, stdout);
		fprintf(stdout, "\n", mvt_raw_evt_cnt);
	}

	// Close data files
	if( bin_data_fptr != (FILE *)NULL )
	{
		fclose( bin_data_fptr );
		bin_data_fptr = (FILE *)NULL;
		if( verbose > 1 )
			printf( "cleanup: bin_data_fptr closed\n" );
	}

	// Close data files
	if( asc_data_fptr != (FILE *)NULL )
	{
		fflush( asc_data_fptr );
		fclose( asc_data_fptr );
		asc_data_fptr = (FILE *)NULL;
		if( verbose > 1 )
			printf( "cleanup: asc_data_fptr closed\n" );
	}

	// Close data files
	if( hst_data_fptr != (FILE *)NULL )
	{
		fflush( hst_data_fptr );
		fclose( hst_data_fptr );
		hst_data_fptr = (FILE *)NULL;
		if( verbose > 1 )
			printf( "cleanup: hst_data_fptr closed\n" );
	}

	// Close config file
	if( cfg_data_fptr != (FILE *)NULL )
	{
		fflush( cfg_data_fptr );
		fclose( cfg_data_fptr );
		cfg_data_fptr = (FILE *)NULL;
		if( verbose > 1 )
			printf( "cleanup: cfg_data_fptr closed\n" );
	}

	for( feu_id=0; feu_id<Def_MaxNbOfFeu; feu_id++ )
	{
		if( feu_data_fptr[feu_id] != (FILE *)NULL )
		{
			fflush( feu_data_fptr[feu_id] );
			fclose( feu_data_fptr[feu_id] );
			feu_data_fptr[feu_id] = (FILE *)NULL;
		}
	}

	// Free readout buffer allocated by malloc
	if( rd_buf != (unsigned int *)NULL )
	{
		free( (void *)rd_buf );
		rd_buf = (unsigned int *)NULL;
		if( verbose > 1 )
			printf( "cleanup: rd_buf freed\n" );
	}

	// Free allocated timing histos
	TimingHistos_Free( &timing_histos_raw );
	TimingHistos_Free( &timing_histos_cmp );
	if( verbose > 1 )
		printf( "cleanup: TimingHistos_Free called\n" );

	if( param == 0 )
	{
		if( verbose )
			printf( "cleanup: Returning\n" );
		return;
	}
	if( verbose > 1 )
		printf( "cleanup: Exiting\n" );
	exit( 0 );
}

/*
 * Signal hendler
 */
void sig_hndlr( int param )
{
	cleanup(param);
}


/*
 * Main
 */
int main( int argc, char* *argv )
{
	// Internal variables
	int  opt;
	char optformat[128];
	char progname[128];
	int index;

	char feu_bin_data_file_name[128];
	int do_asc_file;
	char asc_data_file_name[128];
	int do_hst_file;
	char hst_data_file_name[128];
	int do_cfg_file;
	char cfg_data_file_name[128];
	int pause;

	int ret;
	int feu_wr_len;

	int eof;
	int end;

	int rd_len;
	int rd_word_cnt;
	unsigned int *rd_buf_ptr;
	int rd_buf_ptr_len;

	int net_hdr_expected;
	NetHdr net_hdr;
	int net_buf_len;
	int next_ent_len_to_be_read;

	RcSync     *rc_sync;
	RcGo       *rc_go;
	RcEnd      *rc_end;
	RcPreStart *rc_pstart;

	int ent_len;
	int ent_proc;
	int next_ent_len;

	int cur_ent_type;
	int cur_ent_len;

	int itd;
	int prescale_event;

	// Initialization
	sprintf(progname, "%s", basename(argv[0]));
	sprintf(bin_data_file_name, "%s", DEF_BinData_FileName );
	first_phy_blk = 0;
	last_phy_blk  = 0x7FFFffff;
	do_feu_bin_files = 0;
	for( index=0; index<Def_MaxNbOfFeu; index++ )
	{
		feu_data_fptr[index] = (FILE *)NULL;
	}
	feu_data_fptr_cntr = 0;
	do_asc_file = 0;
	do_hst_file = 0;
	do_cfg_file = 0;
	pause       = 0;
	prescale_event = 0;

	/******************************/
	/* Check for input parameters */
	/******************************/
	sprintf( optformat, "b:f:l:saAcFvhTp" );
	while( ( opt = getopt( argc, argv, optformat ) ) != -1 )
	{
		switch( opt )
		{
			case 'b':
				sprintf( bin_data_file_name, "%s", optarg );
			break;

			case 'f':
				first_phy_blk = atoi( optarg );
			break;

			case 'l':
				last_phy_blk = atoi( optarg );
			break;

			case 'F':
				do_feu_bin_files = 1;
				do_cfg_file = 1;
			break;

			case 'A':
				do_asc_file = 1;
			break;

			case 'a':
				if( do_asc_file == 0 )
					do_asc_file = 2;
			break;

			case 's':
					do_hst_file = 1;
			break;

			case 'c':
					do_cfg_file = 1;
			break;

			case 'p':
				pause = 1;
			break;

			case 'v':
				verbose++;
			break;

			case 'h':
			default:
				usage( progname );
				return 0;
		}
	}
	// Check if bin file name was given directly without -b key
	if( (argc - optind) == 1 )
		sprintf( bin_data_file_name, "%s", argv[optind] );
	if( verbose )
	{
		printf( "%s: Input parameters:\n", progname );
		printf( " bin_data_file_name = %s\n", bin_data_file_name );
		printf( " first_phy_blk      = %d\n", first_phy_blk );
		printf( " last_phy_blk       = %d\n", last_phy_blk );
		printf( " do_feu_bin_files   = %d\n", do_feu_bin_files );
		printf( " do_asc_file        = %d\n", do_asc_file );
		printf( " do_hst_file        = %d\n", do_hst_file );
		printf( " do_cfg_file        = %d\n", do_cfg_file );
		printf( " pause              = %d\n", pause );
		printf( " verbose            = %d\n", verbose );
	}
	
	// Check input parameters
	if( strcmp( bin_data_file_name, "None" ) == 0 )
	{
		printf( "%s: bin_data_file_name must be different from None\n", progname );
		usage( progname );
		return( 0 );
	}
	/***********************/
	/* Set signal hendler  */
	/***********************/
	signal( SIGABRT, sig_hndlr);
	signal( SIGFPE,  sig_hndlr);
	signal( SIGILL,  sig_hndlr);
	signal( SIGINT,  sig_hndlr);
	signal( SIGSEGV, sig_hndlr);
	signal( SIGTERM, sig_hndlr);
	if( verbose > 1 )
		printf( "%s: signal handler set\n", progname );

	/*********************/
	/* Open binary file  */
	/*********************/
	if( (bin_data_fptr=fopen(bin_data_file_name, "rb")) == NULL )
	{
		fprintf( stderr, "%s: fopen failed for file %s in read mode with %d %s\n",
			progname, bin_data_file_name, errno, strerror(errno) );
		cleanup(8);
	}
	else
		if( verbose > 1 )
			printf( "%s: binary data file %s opend in read mode\n", progname, bin_data_file_name );

	// Open Ascii file if requested
	if( do_asc_file == 1 )
	{
		if( (first_phy_blk != 0) || (last_phy_blk != 0x7fffFFFF) )
			sprintf( asc_data_file_name, "%s_%dTo%d.asc", rootfilename(bin_data_file_name), first_phy_blk, last_phy_blk );
		else
			sprintf( asc_data_file_name, "%s.asc", rootfilename(bin_data_file_name) );
		if( (asc_data_fptr=fopen(asc_data_file_name, "w")) == NULL )
		{
			fprintf( stderr, "%s: fopen failed for file %s in ascii write mode with %s\n",
				progname, asc_data_file_name, strerror(errno) );
			cleanup(9);
		}
	}
	else if ( do_asc_file == 2 )
		asc_data_fptr = stdout;
	else
		asc_data_fptr = (FILE *)NULL;

	if( do_hst_file == 1 )
	{
		sprintf( hst_data_file_name, "%s.hst", rootfilename(bin_data_file_name) );
		if( (hst_data_fptr=fopen(hst_data_file_name, "w")) == NULL )
		{
			fprintf( stderr, "%s: fopen failed for file %s in ascii write mode with %s\n",
				progname, hst_data_file_name, strerror(errno) );
			cleanup(9);
		}
	}
	else hst_data_fptr = (FILE *)NULL;

	// Allocated timing histos
	TimingHistos_Init( &timing_histos_raw );
	TimingHistos_Init( &timing_histos_cmp );

	/*
	 * Main loop
	 */
	eof = 0;
	end = 0;

	// Statistics
	net_hdr_cnt = 0;

	rcsync_cnt   = 0;
	rcpstart_cnt = 0;
	rcgo_cnt     = 0;
	rcend_cnt    = 0;
	evt_hdr_cnt  = 0;
	evt_blk_cnt  = 0;
	ti_evt_cnt   = 0;
	mvt_raw_evt_cnt = 0;
	mvt_cmp_evt_cnt = 0;
	cfg_evt_cnt = 0;
	unknown_cmp_cnt = 0;
	unknown_int_cnt = 0;
	unknown_asc_cnt = 0;
	unknown_ent_cnt = 0;

	rd_word_cnt = 0;
	ent_len = 0;
	ent_proc = 0;
	net_hdr_expected = 1;
	net_buf_len = 0;
	next_ent_len = 0;
	while( (eof == 0) && (end == 0) )
	{
		// First get a new entry
		if( ent_len == 0 )
		{
			// Check if nework header has to be stripped
			if( net_hdr_expected == 1 )
			{
				// Skip network header
				if( ( rd_len = ReadNoumWords(bin_data_fptr, Def_NetHdrSize, (unsigned int *)(&net_hdr)) ) != Def_NetHdrSize )
				{
					if( rd_len == 0 )
					{
						fprintf( stderr, "%s: EoF reached\n", __FUNCTION__ );
						eof = 1;
						break;
					}
					else
					{
						fprintf( stderr, "%s: ReadNoumWords failed for net_hdr with rd_len=%d != %d\n", __FUNCTION__, rd_len, Def_NetHdrSize );
						cleanup(-1);
					}
				}
				rd_word_cnt += Def_NetHdrSize;
				if( verbose > 1 )
					NetHdr_Printf( stdout, &net_hdr );
				net_hdr_cnt++;
				// Network header has been stripped
				net_buf_len = net_hdr.size - net_hdr.hdr_len;
				if( net_buf_len )
					net_hdr_expected = 0;
				else
				{
					fprintf( stderr, "%s: Empty Network buffer\n", __FUNCTION__ );
					cleanup(-2);
				}
			}
			if( next_ent_len == 0 )
			{
				// Get next entry length
				if( ( rd_len = ReadNoumWords(bin_data_fptr, 1, &next_ent_len) ) != 1 )
				{
					fprintf( stderr, "%s: ReadNoumWords failed for next_ent_len with rd_len=%d != 1\n", __FUNCTION__, rd_len );
					cleanup(-3);
				}
				rd_word_cnt += 1;
				if( verbose > 2 )
					fprintf(stdout, "%s: next_ent_len = %d net_buf_len = %d\n", __FUNCTION__, next_ent_len, net_buf_len);
				// determine buffer length to allocate
				if( next_ent_len <= 0 )
				{
					fprintf( stderr, "%s: unexpected next_ent_len=%d 0x%08x must be positive\n", __FUNCTION__, next_ent_len, next_ent_len );
					// try to get few more words to understand their nature
					rd_word_cnt = 0;
					do
					{
						net_buf_len=0;
						do
						{
							if( ( rd_len = ReadNoumWords(bin_data_fptr, 1, &next_ent_len) ) != 1 )
							{
								fprintf( stderr, "%s: ReadNoumWords failed for tst word %d with rd_len=%d != 1\n", __FUNCTION__, net_buf_len, rd_len );
								cleanup(-3);
							}
							net_buf_len++;
						} while ( next_ent_len == 0 );
						rd_word_cnt++;
						fprintf( stderr, "%s: %4d word after negative and %2d 0 words : 0x%08x %d\n", __FUNCTION__, rd_word_cnt, net_buf_len-1, next_ent_len, next_ent_len );
					} while( rd_word_cnt < 200 );
					cleanup(-4);
				}
				// One word has been read
				net_buf_len--;
				// Allocate requested buffer
				if( (rd_buf = (unsigned int *)calloc(next_ent_len+1, sizeof(int))) == (unsigned int *)NULL )
				{
					fprintf( stderr, "%s: calloc failed for %d words\n", __FUNCTION__, next_ent_len+1 );
					cleanup(-5);
				}
				// Now the buffer has to be filled from the file stripping if necessary the network header
				// first wright the entity length in the buffer
				rd_buf_ptr = rd_buf;
				if( verbose > 2 )
					fprintf(stdout, "%s: rd_buf_ptr = 0x%08x\n", __FUNCTION__, (unsigned int)rd_buf_ptr);
				*rd_buf_ptr++ = next_ent_len;
				next_ent_len_to_be_read = next_ent_len;
				if( verbose > 2 )
					fprintf(stdout, "%s: next_ent_len_to_be_read = %d\n", __FUNCTION__, next_ent_len_to_be_read);
			}
			// Compute what needs to be read from the current network buffer
			rd_buf_len = next_ent_len_to_be_read < net_buf_len ? next_ent_len_to_be_read : net_buf_len;
			if( verbose > 2 )
				fprintf(stdout, "%s: rd_buf_len = %d\n", __FUNCTION__, rd_buf_len);
			// Now read requested number of words
			if( ( rd_len = ReadNoumWords(bin_data_fptr, rd_buf_len, rd_buf_ptr) ) != rd_buf_len )
			{
				fprintf( stderr, "%s: ReadNoumWords failed with rd_len=%d != rd_buf_len\n", __FUNCTION__, rd_len, rd_buf_len );
				cleanup(-6);
			}
			rd_word_cnt += rd_buf_len;
			// Update network buffer
			rd_buf_ptr += rd_len;
			net_buf_len -= rd_len;
			if( net_buf_len == 0 )
				net_hdr_expected = 1;
			// Update remained next entity data to be read
			next_ent_len_to_be_read -= rd_len;
			if( next_ent_len_to_be_read == 0 )
			{
				ent_len = next_ent_len;
				if( verbose > 2 )
					fprintf(stdout, "%s: rd_buf = 0x%08x rd_buf_len=%d\n", __FUNCTION__, (unsigned int)rd_buf, ent_len);
				next_ent_len = 0;
			}
		}
		else // if( ent_len == 0 )
		{
			rd_buf_ptr = rd_buf;
			rd_buf_ptr_len = ent_len;
			do
			{
				cur_ent_type = *(rd_buf_ptr+1);
				if( asc_data_fptr != ((FILE *)NULL) )
					EntryHdr_Printf(asc_data_fptr, (EntryHdr *)rd_buf_ptr);
				if( (DEF_GetEvioEntryDat(cur_ent_type) == DEF_ENTRY_DATA_BNK_0E) || (DEF_GetEvioEntryDat(cur_ent_type) == DEF_ENTRY_DATA_BNK_10) )
				{
					if( (cur_ent_type == 0x000010cc) || (cur_ent_type == 0x008210cc ) || (cur_ent_type == 0x008110cc) || (cur_ent_type == 0x00C110cc) || (cur_ent_type == 0x00C210cc) ||
						(DEF_GetEvioEntryTag(cur_ent_type) == 0x45) || ((DEF_GetEvioEntryTag(cur_ent_type) == 0x4B)) )
					{
//printf("%s: Physics header in cur_ent_type=0x%08x of len=%d\n", __FUNCTION__, cur_ent_type, *rd_buf_ptr);
						// Looks to be Physics event header
						evt_hdr_cnt++;
						PhyEvtStat_Init( &phy_evt_stat_raw );
						PhyEvtStat_Init( &phy_evt_stat_cmp );
						phy_evt_stat_raw.size = (*rd_buf_ptr + 1);
						phy_evt_stat_cmp.size = (*rd_buf_ptr + 1);
						// This is a bank of bank: skip the header
						rd_buf_ptr = rd_buf_ptr + Def_EntryHdrSize;
						rd_buf_ptr_len -= Def_EntryHdrSize;
					}
					else if
					(
						(DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_RC_PSTART)
						||
						(DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_RC_SYNC)
						||
						(DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_RC_GO)
						||
						(DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_RC_PAUSE)
						||
						(DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_RC_END)
						)
					{
						// This is a bank of bank for run control messages: skip the header
						rd_buf_ptr = rd_buf_ptr + Def_EntryHdrSize;
						rd_buf_ptr_len -= Def_EntryHdrSize;
					}
					else
					{
//						fprintf(stdout, "%s: Unknown cur_ent_type=0x%08x of size %d taf 0x%04x skipping!\n", __FUNCTION__, cur_ent_type, (*rd_buf_ptr+1), DEF_GetEvioEntryTag(cur_ent_type) );
						cur_ent_len = (*rd_buf_ptr + 1);
						rd_buf_ptr = rd_buf_ptr + cur_ent_len;
						rd_buf_ptr_len -= cur_ent_len;
					}
				}
				else if( DEF_GetEvioEntryDat(cur_ent_type) == DEF_ENTRY_DATA_CMP )
				{
					if( DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_MVT_CMP )
					{
						// Looks to be composite MVT data
						if( (ret = MvtCmpDataRead( rd_buf_ptr, (*rd_buf_ptr + 1) ) ) <= 0 )
						{
							fprintf(stderr, "%s: MvtCmpDataRead failed with %d for event count %d\n", __FUNCTION__, ret, evt_blk_cnt);
							break;
						}
						phy_evt_stat_cmp.data_type = PHY_EVT_DATA_TYPE_CMP;
						Histo_Add( &(timing_histos_cmp.phy_evt_blk_size_histo), phy_evt_stat_cmp.size);
						mvt_cmp_evt_cnt++;
						if( prescale_event == 0 )
						{
							if( (ret = PhyEvtStat_Validate( &phy_evt_stat_cmp )) < 0 )
							{
								fprintf(stderr, "%s: PhyEvtBlkStat_Validate failed with %d\n", __FUNCTION__, ret);
							}
						}

						if( (first_phy_blk <= evt_blk_cnt) && (evt_blk_cnt <= last_phy_blk) && (asc_data_fptr != ((FILE *)NULL)) )
						{
							PhyEvtStat_Dump( &phy_evt_stat_cmp, asc_data_fptr );
						}
						if( verbose )
							PhyEvtStat_Dump( &phy_evt_stat_cmp, stdout );

						cur_ent_len = (*rd_buf_ptr + 1);
						Histo_Add( &(timing_histos_cmp.mvt_evt_size_histo), cur_ent_len );
						rd_buf_ptr = rd_buf_ptr + cur_ent_len;
						rd_buf_ptr_len -= cur_ent_len;
					}
					else
					{
//						fprintf(stdout, "%s: Unknown tag for rd_buf = 0x%08x of len=%d of composite type=0x%08x skipping!\n", __FUNCTION__, (unsigned int)rd_buf_ptr, *rd_buf_ptr, *(rd_buf_ptr+1) );
						unknown_cmp_cnt++;
						cur_ent_len = (*rd_buf_ptr + 1);
						rd_buf_ptr = rd_buf_ptr + cur_ent_len;
						rd_buf_ptr_len -= cur_ent_len;
					}
				}
				else if( DEF_GetEvioEntryDat(cur_ent_type) == DEF_ENTRY_DATA_ASC )
				{
					if( DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_MVT_CFG )
					{
						// Looks to be MVT config data
						if( do_cfg_file )
						{
							sprintf( cfg_data_file_name, "%s_%d.cfg", rootfilename(bin_data_file_name), cfg_evt_cnt );
							if( (cfg_data_fptr=fopen(cfg_data_file_name, "w")) == NULL )
							{
								fprintf( stderr, "%s: fopen failed for file %s in ascii write mode with %s\n",
									progname, cfg_data_file_name, strerror(errno) );
								cleanup(9);
							}
							if( ( feu_wr_len = fwrite((char *)(rd_buf_ptr+2), 1, (*rd_buf_ptr-2)*sizeof(int), cfg_data_fptr) ) != (*rd_buf_ptr-2)*sizeof(int) )
							{
								if( ferror(cfg_data_fptr) )
								{
									fprintf( stderr, "%s: fwrite failed with %s for ascii cfg file %s\n", __FUNCTION__, strerror(errno), feu_bin_data_file_name );
									cleanup(10);
								}
							}
							fflush(cfg_data_fptr);
							fclose(cfg_data_fptr);
							cfg_data_fptr = (FILE *)NULL;
						}
						cfg_evt_cnt++;
						cur_ent_len = (*rd_buf_ptr + 1);
						rd_buf_ptr = rd_buf_ptr + cur_ent_len;
						rd_buf_ptr_len -= cur_ent_len;
					}
					else
					{
//						fprintf(stdout, "%s: Unknown tag for rd_buf = 0x%08x of len=%d of ascii type=0x%08x skipping!\n", __FUNCTION__, (unsigned int)rd_buf_ptr, *rd_buf_ptr, *(rd_buf_ptr+1) );
						unknown_asc_cnt++;
						cur_ent_len = (*rd_buf_ptr + 1);
						rd_buf_ptr = rd_buf_ptr + cur_ent_len;
						rd_buf_ptr_len -= cur_ent_len;
					}
				}
				else if( DEF_GetEvioEntryDat(cur_ent_type) == DEF_ENTRY_DATA_INT )
				{
					// Check for data tags
					if( DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_RC_SYNC )
					{
						// Looks to be PreSync
						rc_sync = (RcSync *)rd_buf_ptr;
						if( asc_data_fptr != ((FILE *)NULL) )
							RcSync_Printf( asc_data_fptr, rc_sync );
						rcsync_cnt++;
						rd_buf_ptr = rd_buf_ptr + Def_RcEntrySize;
						rd_buf_ptr_len -= Def_RcEntrySize;
					}
					else if( DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_RC_PSTART )
					{
						rc_pstart = (RcPreStart *)rd_buf_ptr;
						if( asc_data_fptr != ((FILE *)NULL) )
							RcPreStart_Printf( asc_data_fptr, rc_pstart );
						rcpstart_cnt++;
						rd_buf_ptr = rd_buf_ptr + Def_RcEntrySize;
						rd_buf_ptr_len -= Def_RcEntrySize;
					}
					else if( DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_RC_GO )
					{
						// Looks to be Go
						rc_go = (RcGo *)rd_buf_ptr;
						if( asc_data_fptr != ((FILE *)NULL) )
							RcGo_Printf( asc_data_fptr, rc_go );
						rcgo_cnt++;
						rd_buf_ptr = rd_buf_ptr + Def_RcEntrySize;
						rd_buf_ptr_len -= Def_RcEntrySize;
					}
					else if( DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_RC_END )
					{
						// Looks to be End
						rc_end = (RcEnd *)rd_buf_ptr;
						if( asc_data_fptr != ((FILE *)NULL) )
							RcEnd_Printf( asc_data_fptr, rc_end );
						rcend_cnt++;
						end = 1;
						rd_buf_ptr = rd_buf_ptr + Def_RcEntrySize;
						rd_buf_ptr_len -= Def_RcEntrySize;
					}
					else if( DEF_GetEvioEntryTag(cur_ent_type) == 0xc000 )
					{
						if( (ret = EvtBlk_Fill( &evt_blk, rd_buf_ptr ) ) <= 0 )
						{
							fprintf(stderr, "%s: EvtBlk_Fill failed with %d\n", __FUNCTION__, ret);
							break;
						}
						// Looks to be EvtHdr
						// Initialize event stat
						phy_evt_stat_raw.id = evt_blk.num;
						phy_evt_stat_cmp.id = evt_blk.num;
						evt_blk_cnt++;
						if( (first_phy_blk <= evt_hdr_cnt) && (evt_hdr_cnt <= last_phy_blk) && (asc_data_fptr != ((FILE *)NULL)) )
							EvtBlk_Printf( (EvtBlk *)rd_buf_ptr, asc_data_fptr );
						rd_buf_ptr = rd_buf_ptr + ret;
						rd_buf_ptr_len -= ret;
					}
					else if( DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_TI )
					{
						// Looks to be Disentangled TI
						if( (ret = TiBank_Disent_Fill( &ti_bank, rd_buf_ptr ) ) <= 0 )
						{
							fprintf(stderr, "%s: TiBank_Disent_Fill failed with %d for event count %d\n", __FUNCTION__, ret, evt_blk_cnt);
							break;
						}
						ti_evt_cnt++;
						if( (first_phy_blk <= evt_hdr_cnt) && (evt_hdr_cnt <= last_phy_blk) && (asc_data_fptr != ((FILE *)NULL)) )
							TiBank_Disent_Printf( &ti_bank, asc_data_fptr );
						// Fill Phy event stat
						phy_evt_stat_raw.ti_evid_lo = ti_bank.evid_lo;
						phy_evt_stat_raw.ti_tstp_lo = ti_bank.tstp_lo;
						phy_evt_stat_raw.ti_evid_hi = ti_bank.evid_tstp_hi        & 0xFFFF;
						phy_evt_stat_raw.ti_tstp_hi =(ti_bank.evid_tstp_hi >> 16) & 0xFFFF;
						// Inter trigger delay
						if( ti_bank.evid_lo > 1 || timing_histos_raw.ti_tstp_prev != 0xFFFFffff)
						{
							itd = (int)(((((ti_bank.tstp_lo & 0xFFffff) - (timing_histos_raw.ti_tstp_prev & 0xFFffff)) & 0xFFffff) * 4. / 1000.)+.5);
							Histo_Add(&(timing_histos_raw.ti_itd_histo), itd );
						}
						timing_histos_raw.ti_tstp_prev = ti_bank.tstp_lo;

						phy_evt_stat_cmp.ti_evid_lo = ti_bank.evid_lo;
						phy_evt_stat_cmp.ti_tstp_lo = ti_bank.tstp_lo;
						phy_evt_stat_cmp.ti_evid_hi = ti_bank.evid_tstp_hi        & 0xFFFF;
						phy_evt_stat_cmp.ti_tstp_hi =(ti_bank.evid_tstp_hi >> 16) & 0xFFFF;
						rd_buf_ptr = rd_buf_ptr + ret;
						rd_buf_ptr_len -= ret;
						// Inter trigger delay
						if( ti_bank.evid_lo > 1 || timing_histos_cmp.ti_tstp_prev != 0xFFFFffff)
						{
							itd = (int)(((((ti_bank.tstp_lo & 0xFFffff) - (timing_histos_cmp.ti_tstp_prev & 0xFFffff)) & 0xFFffff) * 4. / 1000.)+.5);
							Histo_Add(&(timing_histos_cmp.ti_itd_histo), itd );
						}
						timing_histos_cmp.ti_tstp_prev = ti_bank.tstp_lo;
					}
					else if( DEF_GetEvioEntryTag(cur_ent_type) == DEF_ENTRY_TAG_MVT_RAW )
					{
						// Looks to be Raw MVT data
						mvt_raw_evt_cnt++;
						Histo_Add( &(timing_histos_raw.phy_evt_blk_size_histo), phy_evt_stat_raw.size);
						cur_ent_len = (*rd_buf_ptr + 1);
						Histo_Add(&(timing_histos_raw.mvt_evt_size_histo), cur_ent_len );
						rd_buf_ptr = rd_buf_ptr + cur_ent_len;
						rd_buf_ptr_len -= cur_ent_len;
					}
					else
					{
//						fprintf(stdout, "%s: Unknown tag for rd_buf = 0x%08x of len=%d of integers type=0x%08x skipping!\n", __FUNCTION__, (unsigned int)rd_buf_ptr, *rd_buf_ptr, *(rd_buf_ptr+1) );
						unknown_int_cnt++;
						cur_ent_len = (*rd_buf_ptr + 1);
						rd_buf_ptr = rd_buf_ptr + cur_ent_len;
						rd_buf_ptr_len -= cur_ent_len;
					}
				}
				else
				{
					fprintf(stdout, "%s: Unknown type rd_buf = 0x%08x of len=%d 0x%08x type=0x%08x skipping!\n", __FUNCTION__, (unsigned int)rd_buf_ptr, *rd_buf_ptr, *rd_buf_ptr, *(rd_buf_ptr+1) );
					unknown_ent_cnt++;
					cur_ent_len = (*rd_buf_ptr + 1);
					rd_buf_ptr = rd_buf_ptr + cur_ent_len;
					rd_buf_ptr_len -= cur_ent_len;
				}
			} while( rd_buf_ptr_len > 0 );
//fprintf(stdout, "%s: rd_buf = 0x%08x rd_buf_len=%d type=0x%08x 0x%08x 0x%08x 0x%08x\n", __FUNCTION__, (unsigned int)rd_buf, ent_len, *(rd_buf+1), *(rd_buf+2), *(rd_buf+3), *(rd_buf+4));
			// Entity has been processed
			ent_proc++;
			free( (void *)rd_buf );
			rd_buf = (unsigned int *)NULL;
			if( (asc_data_fptr != ((FILE *)NULL)) || (ent_len == 4) || (ent_len == 6) || (evt_hdr_cnt % 100) == 0 )
				fprintf(stdout, "%s: ent %d of size %d processed\n\n", __FUNCTION__, ent_proc, ent_len);
			ent_len = 0;
			if( pause )
			{
				getchar();
			}
			// Do not go byond requested events
			if( last_phy_blk <= evt_blk_cnt )
				end = 1;
		} // if( ent_len == 0 )
	} // while(eof == 0)
	/********************/
	/* Cleanup and stop */
	/********************/
	if( verbose > 2 )
		printf( "%s: Done after %d word reads\n", progname, rd_word_cnt );
	cleanup(0);
	return 0;
}
