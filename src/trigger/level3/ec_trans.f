      subroutine ec_trans()
      
      include 'ec_trans.inc'
      
      integer ecmap(6,2)
      data ecmap/36,36,36,36,36,36,1,2,3,4,5,6/

c ECAL translation tables: 18 slots, 16 channels, we have slots 3-10,13-19
c Mapping is UI,VI,WI,UO,VO,WO:All 36 channels

      do i = 1,18
        do j = 1,16
          ec_layer_trans(i,j) = 0
          ec_strip_trans(i,j) = 0
        enddo
      enddo

      layer = 1
      strip = 1
      do i = 1,18
        if(i>=3.AND.i<=10.OR.i>=13.AND.i<=18) then
          imax=16
          if (i.eq.9.or.i.eq.18) imax=12
          do j = 1,imax
            ec_layer_trans(i,j) = ecmap(layer,2)
            ec_strip_trans(i,j) = strip
            strip = strip + 1
            if(strip.gt.ecmap(layer,1)) then
              layer=layer+1
              strip = 1
            endif
          enddo
        endif
      enddo

      return
      end
