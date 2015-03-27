#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <sys/types.h>

static int cal_group = 0;
static int run_type = 0;


/* find run type for SVT DAQ */
static int getRunType(char* filename) {
	int type = 0;
	if(strstr(filename,"calib")!=0) type = 1;
	return type;
}




static void getFebConfigFilePath(char* conf, char* type, char* f, int MAX) {

   //initialize
   strcpy(f,"");
   
  if(strlen(conf)>0) {
     printf("Find config file from \"%s\":\n",conf);
     FILE * confFile = fopen(conf,"r");
     if (confFile == NULL) printf("Error opening file \"%s\"\n",conf);
     
     char line[256];
     while( fgets (line , 256 , confFile) != NULL ) {
        //printf("%s\n",line);
        //split by white space
        char **split_line = NULL;
        char* p = strtok(line," ");
        int n_spaces = 0;
        int i;
        while(p) {
           split_line = realloc(split_line, sizeof(char*)*++n_spaces);
           if(split_line==NULL) {
              printf("mem allocation for string failed!");
              exit(1);
           }
           split_line[n_spaces-1] = p;
           p = strtok(NULL," ");
        }
        if(n_spaces==2) {
           
           if((strcmp(split_line[0],"FEB_CONFIG_FILE")==0 && strcmp(type,"CONFIG")==0) || (strcmp(split_line[0],"THR_CONFIG_FILE")==0 && strcmp(type,"THRESHOLDS")==0 )) {
              if(strlen(split_line[1])>255) {
                 printf("config file path is too long: %s", split_line[1]);
                 strcpy(f,"");
                 return;
              }
              //remove carriage return of there
              char* carriage_return = strchr(split_line[1],'\n');
              if(carriage_return!=NULL) {
                 *carriage_return = '\0';
                 //printf("removed carriage return (%s)\n",split_line[1]);
              }
              strcpy(f,split_line[1]);              
           } 
           else {
              printf("[ getFebConfigFile ] : wrong type and config file combination found (type=%s file %s) -> skip\n",type,split_line[0]);                    
           }
        }
        free(split_line);
     }
     fclose(confFile);
  } else {
     printf("CONFFILE is empty.");
  }
  return;
}
     
