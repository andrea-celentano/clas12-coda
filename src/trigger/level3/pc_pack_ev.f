      subroutine pc_pack_ev(event)
      
      implicit none
      integer event(*)
      
      integer i,ii,j,k,nev,ntrig
      integer ped,adc
      integer str,lay
      real    sl
      
      include 'lev3.inc'
      include 'PCnt.inc'
      include 'PC2nt.inc'
      include 'PCATnt.inc'
      include 'pccal.inc'
      include 'pc_trans.inc'

      logical good_pc123_1,good_pc123_2,good_pc123_3,good_pc123,good_pc
      logical good_tst

      integer mpc(3)

      integer get_one_pcal_event, status
      integer MAXPCALHITS
      parameter (MAXPCALHITS=1000)
      integer nhits, slotout(MAXPCALHITS), chanout(MAXPCALHITS),
     &        tdcout(MAXPCALHITS), adcout(MAXPCALHITS), rawadcout(100,MAXPCALHITS)
      integer newt,n,nsum
      real enev,strp(3)
      character*80 rznout,rzhout
      common/hout/rznout,rzhout

      data ntrig/60/nev/0/newt/0/

      save nev

      do i=1,3
        mpc(i)=0
      enddo

      status=get_one_pcal_event(event,nhits,slotout,chanout,tdcout,adcout,rawadcout)

c      print *,'nhits=',nhits    
c      do i=1,nhits
c        print *,'[',i,'] ',slotout(i),chanout(i),tdcout(i),adcout(i),
c     &    layer_trans(slotout(i),chanout(i)),
c     &    strip_trans(slotout(i),chanout(i))
c      enddo

      if (newt.eq.0) tzero=time()

      nev   = nev+1
      newt  = time()
      telap = newt-tzero
      tdiff = telap-newt0
      dt1   = newt
      dt2   = telap
      
      if (telap.gt.ntrig) then
        enev=sqrt(float(nev))/60.
        print *,'Time: ',telap,' Events: ',nev,
     1' Ev/sec: ',int(nev/60.),' +/- ',int(enev)
        nev=0
        ntrig=ntrig+60  
        if (ictl.eq.0.or.(ictl.eq.1.and.out.eq.2)) then
          call hcdir('//PAWC/PC',' ')
          call hrput(0,rzhout,'T')
        endif
      endif

      npc  = 0
      npc2 = 0

      do i = 1,nhits
        if(npc.GE.maxPC) then
          print *,'WARN: npc=',npc,' > maxPC=',maxPC
          print *,'       -> skip the rest of event'
          return
        else            
          lay	= pc_layer_trans(slotout(i),chanout(i))
          str	= pc_strip_trans(slotout(i),chanout(i))
          ped	= pcped(str,lay,1)
          if (ictl.eq.0) then
            adc	= adcout(i)
            if ((adc-ped).gt.57) then
              npc = npc + 1
              secpc(npc)   = 1
              layerpc(npc) = lay
              strippc(npc) = str
c              print *,i,tdcout(i)/64,ishft(tdcout(i),-6)
c              print *,iand(tdcout(i),63),str,lay
c              print *, ''
              tdcpc(npc) = tdcout(i)
              adcpc(npc) = min(143325,adc)
              mpc(lay)=mpc(lay)+1
            endif
          else
            npc2 = npc2+1
            secpc2(npc2)  = 1
            layerpc2(npc2) = lay
            strippc2(npc2) = str
            n=0
            nsum=0
            do ii=1,100
              adc=rawadcout(ii,i)-ped/35.
              adcpc2(ii,npc2)=rawadcout(ii,i)
              if (rawmod.eq.1.and.adc.gt.6.and.n.le.35) then
              nsum=nsum+adc
              n=n+1
              endif
            enddo
            if (n.gt.0.and.nsum.gt.57) then
              mpc(lay)=mpc(lay)+1
              strp(lay)=str
            endif
          endif
        endif
      enddo


      good_pc123_1 = mpc(1).gt.0.and.mpc(2).gt.0.and.mpc(3).gt.0
      good_pc123_2 = mpc(1).le.2.and.mpc(2).le.2.and.mpc(3).le.2
      good_pc123_3 = mpc(1).eq.1.and.mpc(2).eq.1.and.mpc(3).eq.1
      good_pc123   = good_pc123_1.and.good_pc123_2

      if (ictl.eq.0) then
        good_pc123 = good_pc123_1.and.good_pc123_2
        if (good_pc123_3) call hfnt(10)
        call pc_store_pul(good_pc123_3)
      else
        good_tst = good_pc123_3.and.strp(2).eq.60.and.strp(3).eq.6
        if (out.eq.1.and.rawmod.eq.0) call hfnt(10)
        if (out.eq.1.and.rawmod.eq.1.and.good_tst) call hfnt(10)
        if (out.eq.2) call pc_store_raw
      endif
      
      return
      end
      
