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
 *	CODA Editor and mysql interface
 *	
 * Mar 2008: Sergey Boyarinov: migrate to mysql
 *
 * Author:  Jie Chen
 * CEBAF Data Acquisition Group
 *
 * Revision History:
 *   $Log: Editor_database.h,v $
 *   Revision 1.1.1.2  1996/11/05 17:45:26  chen
 *   coda source
 *
 *	  
 */
#ifndef _EDITOR_DATABASE_H
#define _EDITOR_DATABASE_H

#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>

#include "Editor.h"



#define EDITOR_MAX_DATABASES 100
#define EDITOR_MAX_CONFIGS 200
#define EDITOR_MAX_DATABASE_NAMELEN 64



#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 *              int connectToDatabase (char* host)                           *
 * Description:                                                              *
 *      Connect to a database on a host 'host'                               *
 *      if host == NULL, connect to local host                               *
 *      return -1: failure, return > 0 success                               *
 ****************************************************************************/
extern int connectToDatabase (char* host);


/*****************************************************************************
 *              void closeDatabase (void)                                    *
 * Description:                                                              *
 *      Close connection to a database                                       *
 ****************************************************************************/
extern void closeDatabase    (void);

/*****************************************************************************
 *              int databaseIsOpen (void)                                    *
 * Description:                                                              *
 *      return database is open or not flag                                  *
 ****************************************************************************/
extern int databaseIsOpen (void);


/*****************************************************************************
 *              void cleanDatabaseMiscInfo (void)                            *
 * Description:                                                              *
 *      Clean out misc information such as current database name and current *
 *      run type selection. 
 ****************************************************************************/
extern void cleanDatabaseMiscInfo (void);

/*****************************************************************************
 *              int createNewDatabase (char *name)                           *
 * Description:                                                              *
 *      Create a new database with name 'name'                               *
 *      return 0: success, return -1: failure                                *
 ****************************************************************************/
extern int createNewDatabase (char *name);

/*****************************************************************************
 *              int listAllDatabases (char **names, int *num)                *
 * Description:                                                              *
 *      list all database names in the buffer names with number of database  *
 *      stored in num. free memory pointed by names[i]                       *
 *      return 0: success, return -1: failure                                *
 ****************************************************************************/
extern int listAllDatabases (char** names, int *num);

/*****************************************************************************
 *              int selectDatabase (char *name)                              *
 * Description:                                                              *
 *      select a database with name 'name'                                   *
 *      return 0: success, return -1: failure                                *
 ****************************************************************************/
extern int selectDatabase (char* name);


/*****************************************************************************
 *              int databaseSelected (void)                                  *
 * Description:                                                              *
 *      return 1: yes, return 0: not yet                                     *
 ****************************************************************************/
extern int databaseSelected (void);

/*****************************************************************************
 *              char* currentDatabase (void)                                 *
 * Description:                                                              *
 *      return current database. caller should copy the result               *
 ****************************************************************************/
extern char* currentDatabase (void);

/*****************************************************************************
 *              int removeDatabase (char* name)                              *
 * Description:                                                              *
 *      remove a database pointed by 'name'                                  *
 *      return 0: success, return -1: failure                                *
 ****************************************************************************/
extern int  removeDatabase (char* name);

/*****************************************************************************
 *              int createConfigTable (void)                                 *
 * Description:                                                              *
 *      Create Configuration Table Template in the database                  *
 *      return -1: failure, return anything else: ok                         *
 ****************************************************************************/
extern int createConfigTable (char* config);

/*****************************************************************************
 *              int createExpInfoTable (void)                                *
 * Description:                                                              *
 *      Create expt info Table Template in the database                      *
 *      return -1: failure, return anything else: ok                         *
 ****************************************************************************/
extern int createExpInfoTable (void);


/*****************************************************************************
 *              int createProcessTable (void)                                *
 * Description:                                                              *
 *      Create Process Table Template in the database                        *
 *      return -1: failure, return anything else: ok                         *
 ****************************************************************************/
extern int createProcessTable (void);

/*****************************************************************************
 *              int createRunTypeTable (void)                                *
 * Description:                                                              *
 *      Create Run Type Table Template in the database                       *
 *      return -1: failure, return anything else: ok                         *
 ****************************************************************************/
extern int createRunTypeTable (void);

/*****************************************************************************
 *              int createPositionTable (void)                               *
 * Description:                                                              *
 *      Create Component position Table Template in the database             *
 *      return -1: failure, return anything else: ok                         *
 ****************************************************************************/
extern int createPositionTable (char* config);

/*****************************************************************************
 *              int createPriorityTable (void)                               *
 * Description:                                                              *
 *      Create create priority Table Template in the database                *
 *      return -1: failure, return anything else: ok                         *
 ****************************************************************************/
extern int createPriorityTable (void);

/*****************************************************************************
 *      int createOptionTable (char* config)                                 *
 * Description:                                                              *
 *      Create create priority Table Template in the database                *
 *      return -1: failure, return anything else: ok                         *
 ****************************************************************************/
extern int createOptionTable (char* config);

/*****************************************************************************
 *      int createScriptTable (char* config)                                 *
 * Description:                                                              *
 *      Create create priority Table Template in the database                *
 *      return -1: failure, return anything else: ok                         *
 ****************************************************************************/
extern int createScriptTable (char* config);

/*****************************************************************************
 *              int listAllTables (char* tables[], int* num)                 *
 * Description:                                                              *
 *      List all the tables in the database                                  *
 *      must be called after database has been selected                      *
 *      caller must free memory allocated by tables[i]                       *
 *      return -1: failure, return anything else: ok                         *
 ****************************************************************************/
extern int listAllTables (char* tables[], int* num);

/*****************************************************************************
 *              int listAllConfigs (char* configs[], int* num)               *
 * Description:                                                              *
 *      List all the configuration names in the database                     *
 *      must be called after database has been selected                      *
 *      caller must free memory allocated by configs[i]                      *
 *      return -1: failure, return anything else: ok                         *
 ****************************************************************************/
extern int listAllConfigs (char* configs[], int* num);

/*****************************************************************************
 *              int numberConfigs (void)                                     *
 * Description:                                                              *
 *      Return number of configuration databases                             *
 ****************************************************************************/
extern int numberConfigs (void);

/*****************************************************************************
 *              int isConfigCreated (char* config)                           *
 * Description:                                                              *
 *      check a particular configuration that has been created               *
 *      return 1: yes. return 0 : not yet, return -1: error                  *
 ****************************************************************************/
extern int isConfigCreated (char* config);

/*****************************************************************************
 *              int isTableCreated (char* name)                              *
 * Description:                                                              *
 *      check a particular tabel that has been created                       *
 *      return 1: yes. return 0 : not yet, return -1: error                  *
 ****************************************************************************/
extern int isTableCreated (char* config);

/*****************************************************************************
 *              int insertValToPosTable (char* config, char* name,           *
 *                                       int row, int col)                   *
 * Description:                                                              *
 *      update position table using data 'name, row, col'                    *
 *      return -1: error, return 0: success                                  *
 ****************************************************************************/
extern int insertValToPosTable (char* config, char* name, int row, int col);


/*****************************************************************************
 *              int insertValToOptionTable (char* config, char* name,        *
 *                                          char* value)                     *
 * Description:                                                              *
 *      update option table using data 'name, value'                         *
 *      return -1: error, return 0: success                                  *
 ****************************************************************************/
extern int insertValToOptionTable (char* config, char* name, char* value);


/*****************************************************************************
 *              int insertValToScriptTable (char* config, char* name,        *
 *                                          codaScript* list)                *
 * Description:                                                              *
 *      update script table using data 'name, all list'                      *
 *      return -1: error, return 0: success                                  *
 ****************************************************************************/
extern int insertValToScriptTable (char* config, char* name, codaScript* list);

/*****************************************************************************
 *              int insertValToConfigTable (char* config, char* name,        *
 *                 char* code, char* input, char* output, char* next,        *
 *                 int first )                                               *
 * Description:                                                              *
 *      update config table using data                                       *
 *      return -1: error, return 0: success                                  *
 ****************************************************************************/
extern int insertValToConfigTable (char* config, char* name, char* code,
				   char* inputs, char* outputs, char* next,
								   int first, short order_num);

/*****************************************************************************
 *              int insertDaqcompToProcTable (daqComp* comp)                 *
 * Description:                                                              *
 *      Insert a daqComponent into the process table                         *
 *      return 0: success, return -1: failure                                *
 ****************************************************************************/
extern int insertDaqCompToProcTable (daqComp* comp);


/*****************************************************************************
 *              int updateDaqcompToProcTable (daqComp* comp)                 *
 * Description:                                                              *
 *      update a daqComponent into the process table                         *
 *      return 0: success, return -1: failure                                *
 ****************************************************************************/
extern int updateDaqCompToProcTable (daqComp* comp);

/*****************************************************************************
 *              int removeDaqCompFromProcTable (char* name)                  *
 * Description:                                                              *
 *      remove daqComponent from the process table                           *
 *      return 0: success, return -1: failure                                *
 ****************************************************************************/
extern int removeDaqCompFromProcTable (char *name);

/*****************************************************************************
 *              int isDaqCompInProcTable (char* name)                        *
 * Description:                                                              *
 *      check whether a component in the process table or not                *
 *      return 1: yes, return 0: no                                          *
 ****************************************************************************/
extern int isDaqCompInProcTable (char *name);

/*****************************************************************************
 *              int removePositionTable (char* config)                       *
 * Description:                                                              *
 *      Remove position table                                                *
 *      return -1: error, return 0: success                                  *
 ****************************************************************************/
extern int removePositionTable (char *config);

/*****************************************************************************
 *              int removeScriptTable (char* config)                         *
 * Description:                                                              *
 *      Remove Script table                                                  *
 *      return -1: error, return 0: success                                  *
 ****************************************************************************/
extern int removeScriptTable (char *config);

/*****************************************************************************
 *              int removeConfigTable (char* config)                         *
 * Description:                                                              *
 *      Remove configuration table                                           *
 *      return -1: error, return 0: success                                  *
 ****************************************************************************/
extern int removeConfigTable (char *config);

/*****************************************************************************
 *              int removeOptionTable (char* config)                         *
 * Description:                                                              *
 *      Remove configuration optiontable                                     *
 *      return -1: error, return 0: success                                  *
 ****************************************************************************/
extern int removeOptionTable (char *config);

/*****************************************************************************
 *              int selectConfigTable (char* config)                         *
 * Description:                                                              *
 *      Select this config table as current table                            *
 *      return -1: error, return 0: success                                  *
 ****************************************************************************/
extern int selectConfigTable (char* config);

/*****************************************************************************
 *              char* currentConfigTable (void)                              *
 * Description:                                                              *
 *      current configuration table                                          *
 *      return current runType, return 0: not selected yet                   *
 ****************************************************************************/
extern char* currentConfigTable (void);

/*****************************************************************************
 *              void  removeMiscConfigInfo (void)                            *
 * Description:                                                              *
 *      remove related information with current config table                 *
 ****************************************************************************/
extern void removeMiscConfigInfo (void);


/*****************************************************************************
 *              int createRcNetCompsFromDbase (rcNetComp** comp, int *num)   *
 * Description:                                                              *
 *      constructRcNetComp from process table                                *
 *      return -1: error, return 0: success                                  *
 *      Caller has full control of memory of comp[i] if not null             *
 ****************************************************************************/
extern int createRcNetCompsFromDbase (rcNetComp** comp, int* num);

/*****************************************************************************
 *              int retrieveConfigInfoFromDbase (char*, ConfigInfo* comp)    *
 * Description:                                                              *
 *      retrieve configuration information from database                     *
 *      return -1: error, return 0: success                                  *
 ****************************************************************************/
extern int retrieveConfigInfoFromDbase (char* config, ConfigInfo** comp,
					int *num);


/*sergey: get 'code' from 'defaults' table */
extern int getDefaultCodeFromDbase (char* type, char *rols[3]);


/*****************************************************************************
 *              int compInConfigTable (char* name)                           *
 * Description:                                                              *
 *      check whether a component in any configuration tables                *
 *      return 1: yes, return 0: no                                          *
 ****************************************************************************/
extern int compInConfigTables (char* name);



#ifdef __cplusplus
}
#endif



#endif
