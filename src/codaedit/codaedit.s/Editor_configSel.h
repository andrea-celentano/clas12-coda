/*-----------------------------------------------------------------------------
 * Copyright (c) 1991,1992 Southeastern Universities Research Association,
 *                         Continuous Electron Beam Accelerator Facility
 *
 * This software was developed under a United States Government license
 * described in the NOTICE file included as part of this distribution.
 *
 * CEBAF Data Acquisition Group, 12000 Jefferson Ave., Newport News, VA 23606
 * Email: coda@cebaf.gov  Tel: (804) 249-7101  Fax: (804) 249-7363
 *-----------------------------------------------------------------------------
 * 
 * Description:
 *	Coda Editor Database File name selection dialog
 *	
 * Author:  Jie Chen
 * CEBAF Data Acquisition Group
 *
 * Revision History:
 *   $Log: Editor_configSel.h,v $
 *   Revision 1.1.1.1  1996/08/21 19:36:06  heyes
 *   Imported sources
 *
 *	  
 */
#ifndef _EDITOR_CONFIG_SEL_H
#define _EDITOR_CONFIG_SEL_H

#include <stdio.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include "Editor_database.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _editor_config_sel_
{
  char*   configs_[EDITOR_MAX_CONFIGS];
  int     numConfigs_;
  int     managed_;
  Widget  w_;
  Widget  option_;
  Widget  ok_;
  Widget  cancel_;
  Widget  pushb[EDITOR_MAX_CONFIGS];
} editorConfigSel;


/************************************************************************
 *          void initConfigSel (Widget parent)                          *
 * Description:                                                         *
 *     Initialize the editorConfigSel structure                         *
 ***********************************************************************/
extern void initConfigSel (Widget parent);

/************************************************************************
 *          void configSelpopup (void)                                  *
 * Description:                                                         *
 *     Popup database selection dialog                                  *
 ***********************************************************************/
extern void configSelPopup (void);

/************************************************************************
 *          void configSelpopdown (void)                                *
 * Description:                                                         *
 *     Popdown database selection dialog                                *
 ***********************************************************************/
extern void configSelPopdown (void);

#ifdef __cplusplus
}
#endif


#endif

  
