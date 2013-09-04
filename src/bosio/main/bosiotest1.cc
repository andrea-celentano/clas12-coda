


#include <iostream>
//#include <clasbanks.h>
//#include <TMath.h>

#include "bos.h"
#include "bosio.h"
#include "bosfun.h"

//#include "defs.h"
//#include "bcs.h"

#define Int_t int
#define NBCS 100000

using namespace std;

int main()
{

  Int_t index;

  Int_t bosStatus = 0;
  Int_t bosReadStatus = 0;
  Int_t headIndex = 0;

  BOSIO *bosFileUnit = NULL;

  Int_t ind = 0;

  Int_t iw[NBCS];

  bosInit(iw,NBCS);

  bosStatus = bosOpen("test1.evt", "w", &bosFileUnit);

  cout<<"Kuku bosStatus = "<<bosStatus<<endl;

 char *name1 = "HEAD";
 char *name2 = "TEST";

 char *format = "2I"; //"2D";
 char *format2 = "8I";

  cout<<"name1 = "<<name1<<endl;

  bosNformat(iw,name1, format2);
  bosNformat(iw,name2, format);


  bosLctl(iw,"E=","HEADTEST");


for( int ii = 0; ii < 100; ii++ )
{
  ind = bosNcreate(iw, name1,0, 8, 1);
  if( ind != 0 )
  {
    cout<<"bosNcreate ind = "<<ind<<endl;
    iw[ind] = 1;
    iw[ind + 1] = 22;
    iw[ind + 2] = ii;
//  iw[ind + 3] = 44;
    iw[ind + 4] = -4;
    iw[ind + 5] = 6;
    iw[ind + 6] = 15;
    iw[ind + 7] = 0;
  }
  else
  {
	cout<<"Preoblem with bosNcreate:  ind = "<<ind<<endl;
  }

  ind = bosNcreate(iw, "TEST",0, 2, 1);
  iw[ind] = 1111;
  iw[ind + 1] = 2222;
  iw[ind + 2] = 3333;

  int status = bosWrite(bosFileUnit,iw,"E");

  bosLdrop(iw,"E");
  bosNgarbage(iw);

}

  int status = bosWrite(bosFileUnit,iw,"0");
  cout<<"bostWriteStatus = "<<status<<endl;


  bosClose(bosFileUnit);
}
