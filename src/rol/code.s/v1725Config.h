/****************************************************************************
 *
 *  v1725Config.h  -  configuration library header file for v1725
 *
 *  SP, 07-Nov-2013
 *
 */


#define FNLEN     128       /* length of config. file name */
#define STRLEN    250       /* length of str_tmp */
#define ROCLEN     80       /* length of ROC_name */
#define NBOARD     21
#define NCHAN      16


/** v1725 configuration parameters **/
typedef struct {
  int f_rev;
  int b_rev;
  int b_ID;


  unsigned int winOffset;
  unsigned int winWidth;

  unsigned int chMask;
  unsigned int trigMask;/*not used*/
  unsigned int thr[NCHAN];
  unsigned int dac[NCHAN];
  unsigned int gain[NCHAN];
  unsigned int ped[NCHAN];

} V1725_CONF;


/* functions */

void v1725InitGlobals();
int v1725ReadConfigFile(char *filename);
int v1725DownloadAll();
int v1725Config(char *fname);
void v1725Mon(int slot);
void v1725SetExpid(char *string);
