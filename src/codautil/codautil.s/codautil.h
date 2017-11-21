/* codautil.h */

#ifdef  __cplusplus
extern "C" {
#endif

int   get_run_number(char *mysql_database, char *session);
char *get_run_operators(char *mysql_database, char *session);
char *get_run_status(char *mysql_database, char *session);

#ifdef  __cplusplus
}
#endif
