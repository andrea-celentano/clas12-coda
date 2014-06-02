      integer maxPC2
      parameter (maxPC2=192)
      integer nPC2
      integer secPC2
      integer layerPC2
      integer stripPC2
      integer ADCPC2
*
      common /PC2nt/ nPC2
     &,secPC2(maxPC2)
     &,layerPC2(maxPC2)
     &,stripPC2(maxPC2)
     &,ADCPC2(100,maxPC2)

