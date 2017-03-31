#ifndef SSPCONFIG_H
#define SSPCONFIG_H

#include "sspLib.h"

/****************************************************************************
 *
 *  sspConfig.h  -  configuration library header file for SSP board 
 *
 */


#define FNLEN     128       /* length of config. file name */
#define STRLEN    250       /* length of str_tmp */
#define ROCLEN     80       /* length of ROC_name */
#define NBOARD     22

typedef struct
{
  int emin;
  int emax;
  int nmin;
  int emin_en;
  int emax_en;
  int nmin_en;
  
  int prescale_xmin[7];
  int prescale_xmax[7];
  int prescale[7];
} singles_trig;

typedef struct
{
  int timecoincidence;
  int emin;
  int emax;
  int nmin;
  int summax;
  int summin;
  int summax_en;
  int diffmax;
  int diffmax_en;
  int coplanartolerance;
  int coplanartolerance_en;
  float edfactor;
  int edmin;
  int ed_en;
} pairs_trig;

typedef struct
{
  int en;
  
  int pcal_cluster_emin_en;
  int pcal_cluster_emin;
  int pcal_cluster_width;
  
  int ecalout_cluster_emin_en;
  int ecalout_cluster_emin;
  int ecalout_cluster_width;
  
  int ecalin_cluster_emin_en;
  int ecalin_cluster_emin;
  int ecalin_cluster_width;
  
  int pcal_esum_en;
  int pcal_esum_emin;
  int pcal_esum_width;
  
  int ecal_esum_en;
  int ecal_esum_emin;
  int ecal_esum_width;
} strigger;

typedef struct
{
  int esum_delay;
  int cluster_delay;
  int esum_intwidth;
} ss_ecal;

/** SSP configuration parameters **/
typedef struct {
  int fw_rev;
  int fw_type;
  
  int block_level;
  int bus_error;
  int window_width;
  int window_offset;

  int pulser_freq;
  int ssp_io_mux[SD_SRC_NUM];
  
  struct
  {
    int trigger_latency;
    singles_trig s[2];
    pairs_trig p[2];
    int cosmic_timecoincidence;
    int cosmic_pattern;
  } hps;
  
  struct
  {
    strigger strg[4];
    ss_ecal ecal;
    ss_ecal pcal;
    int gtpif_latency;
  } gt;
  
} SSP_CONF;

/* functions */
void sspInitGlobals();
int sspReadConfigFile(char *filename);
int sspDownloadAll();
int sspConfig(char *fname);
void sspMon(int slot);

#endif
