      subroutine ec_bor
      include 'eccal.inc'

      character*132 file_in
      integer str,lay,slot,chan
      integer ped,s,i,spar,nstr(3)

      file_in='./ecped.vec'

      open(unit=12,file=file_in,status='old')

      do s=1,6
        do lay=1,6
          do  str=1,36
            read(12,*) ecped(str,lay,s)
          enddo
        enddo
      enddo

      close(12)

      return
      end
