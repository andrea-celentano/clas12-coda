      integer maxEC2
      parameter (maxEC2=216)
      integer nEC2
      integer secEC2
      integer layerEC2
      integer stripEC2
      integer ADCEC2
*
      common /EC2nt/ nEC2
     &,secEC2(maxEC2)
     &,layerEC2(maxEC2)
     &,stripEC2(maxEC2)
     &,ADCEC2(100,maxEC2)

