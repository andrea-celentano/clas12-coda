
/* uthbook.h */

#ifdef	__cplusplus
extern "C" {
#endif

void uthbook1(int id, char *title, int nbinx, float xmin, float xmax);
void uthfill(int id, float x, float y, float weight);
void uthprint(int id);
int uthist2ipc(int id, char *myname);

#ifdef	__cplusplus
}
#endif
