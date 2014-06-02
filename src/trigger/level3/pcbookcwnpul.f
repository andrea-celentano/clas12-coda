      subroutine pcbookcwnpul(id)
      IMPLICIT NONE
      integer id

      include 'PCnt.inc'
      include 'PCATnt.inc'

      call hbname(id,'PC',nPC,'nPC[0,192]:I  
     1,secPC(nPC)[1,6]:I  
     1,layerPC(nPC)[1,3]:I  
     1,stripPC(nPC)[1,68]:I  
     1,TDCPC(nPC):I  
     1,ADCPC(nPC)[0,16383]:I')

      call hbname(id,'PCAT',dt1,'DT1:I,DT2:I')

      return

      end
