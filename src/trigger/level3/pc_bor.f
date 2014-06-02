      subroutine pc_bor
      include 'pccal.inc'

      character*132 file_in
      integer str,lay,slot,chan
      integer ped,s,i,spar,nstr(3)

      file_in='./pcped.vec'

      open(unit=12,file=file_in,status='old')

      do s=1,1
        do lay=1,3
          do  str=1,68
            read(12,*) pcped(str,lay,s)
          enddo
        enddo
      enddo

      close(12)

      return
      end
