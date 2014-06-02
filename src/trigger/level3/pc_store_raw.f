      subroutine pc_store_raw
      
      include 'PC2nt.inc'
      include 'PCstore.inc'
      include 'pccal.inc'

      real vfa(100,68,3),vfas(100,68,3),vfb(100,68,3)
      real vft(100,68,3)
      real vfh(68,3)

      data nid1/0/nid2/0/nid3/0/k/0/

      k=k+1

      if (k.ge.101) then
        do il=1,3
        do is=1,68
        sum=0.
        do i=1,100
          sum=sum+vft(i,is,il)
        enddo
        pcped(is,il,1)=sum/100.
        enddo
        enddo
        k=1
      endif

      do i=1,npc2
        il=layerpc2(i)
        is=strippc2(i)
        vsum=0.
        psum=0.
        do ii=1,100
          vfa(ii,is,il)=adcpc2(ii,i)
          vfas(ii,is,il)=vfa(ii,is,il)-pcped(is,il,1)
          vsum=vsum+vfas(ii,is,il)
          if (ii.le.25) psum=psum+vfa(ii,is,il)
        enddo
        vft(k,is,il)=psum/25
        if(vsum.gt.400.and.vfh(is,il).ne.1) then
          vfh(is,il)=1
          do ii=1,100
            vfb(ii,is,il)=vfas(ii,is,il)
          enddo
        endif

      enddo

      call hfpak1(770,nid1,vfa,20400)
      call hfpak1(771,nid2,vfas,20400)
      call hfpak1(772,nid3,vfb,204)       

      return
      end
