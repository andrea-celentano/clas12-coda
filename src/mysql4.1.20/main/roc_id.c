
/* roc_id.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mysql.h>

#define STRLEN 80

int
main(int argc, char **argv)
{
  char *mysql_database = getenv("EXPID");
  int id;
  char *name = argv[1];

  get_roc_id(mysql_database, name, &id);

  printf("name=%s -> id=%d\n",name,id);

  return(0);
}
