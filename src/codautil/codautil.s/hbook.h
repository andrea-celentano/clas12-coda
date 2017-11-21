#ifndef _UTHBOOK_


#define NHIST 1000
#define NTITLE 64
#define NBUF 10000
#define MAXI2      65536
#define HZERO 0.001

#define NORMAL_I2  0x01
#define PACKED_I2  0x11
#define NORMAL_I4  0x02
#define PACKED_I4  0x12
#define NORMAL_F   0x03
#define PACKED_F   0x13


#ifdef	__cplusplus
extern "C" {
#endif

typedef struct hist
{
  int id;
  int entries;    /* total number of entries */

  int ifi2_book; /* set to 1 in booking if I2 permitted */
  int ifi2_fill; /* set to 1 in filling if I2 permitted */

  int nbinx;
  float xmin;
  float xmax;
  float xunderflow;
  float xoverflow;

  int nbiny;
  float ymin;
  float ymax;
  float yunderflow;
  float yoverflow;

  int ntitle;
  char *title;

  float dx;
  float dy;

  float *buf;
  float **buf2;

} Hist;


class Hbook {

 private:

  Hist hist[NHIST];
  unsigned int localbuf[NBUF];

 public:

  void hbook1(int id, char *title, int nbinx, float xmin, float xmax);
  void hbook2(int id, char *title, int nbinx, float xmin, float xmax, int nbiny, float ymin, float ymax);
  void hfill(int id, float x, float y, float weight);
  void hreset(int id, char *title);
  void hprint(int id);
  void hunpak(int id, float *content, char *choice, int num);
  float hij(int id, int ibinx, int ibiny);
  void hgive(int id, char *title, int *nbinx, float *xmin, float *xmax, int *nbiny, float *ymin, float *ymax, int *titlelen);
  void hidall(int *ids, int *n);
  int hentries(int id);

  int hist2evio(int id, long *jw);
  int evio2hist(int id, long *jw);

  int hist2ipc(int id, char *myname);

};

#ifdef	__cplusplus
}
#endif



#define _UTHBOOK_

#endif
