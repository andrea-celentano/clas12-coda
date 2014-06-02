      integer maxPC
      parameter (maxPC=192)
      integer nPC
      integer secPC
      integer layerPC
      integer stripPC
      integer TDCPC
      integer ADCPC
*
      common /PCnt/ nPC
     &,secPC(maxPC)
     &,layerPC(maxPC)
     &,stripPC(maxPC)
     &,TDCPC(maxPC)
     &,ADCPC(maxPC)
