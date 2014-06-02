      subroutine pc_trans()
      
      include 'pc_trans.inc'
      
      integer pcmap(3,2)
      data pcmap/68,62,62,1,2,3/

c PCAL translation tables: 18 slots, 16 channels, we have slots 4-10,13-17
c Mapping is U,V,W:U=68,V,W=62

      do i = 1,18
        do j = 1,16
          pc_layer_trans(i,j) = 0
          pc_strip_trans(i,j) = 0
        enddo
      enddo

      layer = 1
      strip = 1
      do i = 1,18
        if(i>=4.AND.i<=10.OR.i>=13.AND.i<=17) then
          do j = 1,16
            pc_layer_trans(i,j) = pcmap(layer,2)
            pc_strip_trans(i,j) = strip
            strip = strip + 1
            if(strip.gt.pcmap(layer,1)) then
              layer=layer+1
              strip = 1
            endif
          enddo
        endif
      enddo

      return
      end
