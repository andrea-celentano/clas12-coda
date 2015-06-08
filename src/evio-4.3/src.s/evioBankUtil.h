
/* evioBankUtil.h - evio bank handling routines (Sergey Boyarinov) */

#ifndef __EVIOBANKUTIL_h__
#define __EVIOBANKUTIL_h__






/* prototypes */

#ifdef __cplusplus
extern "C" {
#endif

int evopenFrag(unsigned int *buf, int fragtag, int fragnum);
unsigned int *evOpenBank(unsigned int *buf, int fragtag, int fragnum, int banktag, int banknum, int banktype, char *fmt);
int evCloseBank(unsigned int *buf, int fragtag, int fragnum, int banktag, int banknum, unsigned char *b08);

int evLinkFrag(unsigned int *buf, int fragtag, int fragnum);
int evLinkBank(unsigned int *buf, int fragtag, int fragnum, int banktag, int banknum, int *nbytes);

int evDropFrag(unsigned int *buf, int fragtag, int fragnum);
int evDropBank(unsigned int *buf, int fragtag, int fragnum, int banktag, int banknum);

#ifdef __cplusplus
}
#endif



#endif
