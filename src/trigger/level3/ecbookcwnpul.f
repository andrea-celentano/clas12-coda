      subroutine ecbookcwnpul(id)
      IMPLICIT NONE
      integer id

      include 'ECnt.inc'
      include 'ECATnt.inc'

      call hbname(id,'EC',nEC,'nEC[0,216]:I  
     1,secEC(nEC)[1,6]:I  
     1,layerEC(nEC)[1,6]:I  
     1,stripEC(nEC)[1,36]:I  
     1,TDCEC(nEC):I  
     1,ADCEC(nEC)[0,16383]:I')

      call hbname(id,'ECAT',dt1,'DT1:I,DT2:I')

      return

      end
