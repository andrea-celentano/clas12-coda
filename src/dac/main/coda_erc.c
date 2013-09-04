/* coda_erc.c - CODA format */

/* following flag (if defined) will force CODA format functions to be linked
   directly rather then loaded dynamicaly; need it to check function
   prototypes etc; if used, BOS format will not work !!! */

#include "tcpServer.c"

#define LINK_CODA_FORMAT

#include "cinclude/coda_er_inc.c"
