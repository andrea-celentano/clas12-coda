      subroutine pcbookcwnraw(id)
      IMPLICIT NONE
      integer id

      include 'PC2nt.inc'

      call hbname(id,'PC',nPC2,'nPC[0,192]:I  
     1,secPC(nPC)[1,6]:I  
     1,layerPC(nPC)[1,3]:I  
     1,stripPC(nPC)[1,68]:I
     1,ADCPC(100,nPC)[0,4095]:I')

      return

      end
