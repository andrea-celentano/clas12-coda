      subroutine ecbookcwnraw(id)
      IMPLICIT NONE
      integer id

      include 'EC2nt.inc'

      call hbname(id,'EC',nEC2,'nEC[0,216]:I  
     1,secEC(nEC)[1,6]:I  
     1,layerEC(nEC)[1,6]:I  
     1,stripEC(nEC)[1,36]:I
     1,ADCEC(100,nEC)[0,4095]:I')

      return

      end
