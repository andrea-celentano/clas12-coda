
/* uthbook.cc - interface to hbook.cc */


#include "hbook.h"
#include "uthbook.h"

static Hbook hbook;

void
uthbook1(int id, char *title, int nbinx, float xmin, float xmax)
{
  hbook.hbook1(id, title, nbinx, xmin, xmax);
}

void
uthfill(int id, float x, float y, float weight)
{
  hbook.hfill(id, x, y, weight);
}

void
uthprint(int id)
{
  hbook.hprint(id);
}

int
uthist2ipc(int id, char *myname)
{
  hbook.hist2ipc(id, myname);
}
