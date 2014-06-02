      subroutine ec_pack_ev(event)
      
      implicit none

      integer event(*)

      integer i,ii,j,k,nev,ntrig
      integer ped,adc
      integer str,lay
      real    sl
      
      include 'lev3.inc'
      include 'ECnt.inc'
      include 'EC2nt.inc'
      include 'ECATnt.inc'
      include 'eccal.inc'
      include 'ec_trans.inc'

      logical good_ec123_1,good_ec123_2,good_ec123_3,good_ec123,good_ec
      logical good_ec123_i,good_ec123_o
      logical good_tst

      integer mec(6)

      integer get_one_pcal_event, status
      integer MAXECALHITS
      parameter (MAXECALHITS=1296)
      integer nhits, slotout(MAXECALHITS), chanout(MAXECALHITS),
     &        tdcout(MAXECALHITS), adcout(MAXECALHITS), rawadcout(100,MAXECALHITS)
      integer newt,n,nsum
      real enev,strp(3)
      character*80 rznout,rzhout
      common/hout/rznout,rzhout

      data ntrig/60/nev/0/newt0/0/

      save nev

      do i=1,6
        mec(i)=0
      enddo

      status=get_one_pcal_event(event,nhits,slotout,chanout,tdcout,adcout,rawadcout)

ccc      print *,'nhits=',nhits    
c      do i=1,nhits
c        print *,'[',i,'] ',slotout(i),chanout(i),tdcout(i),adcout(i),rawadcout(10,i),
c     &    ec_layer_trans(slotout(i),chanout(i)),
c     &    ec_strip_trans(slotout(i),chanout(i))
c      enddo


      if (newt.eq.0) tzero=time()

      nev   = nev+1
      if (nev.gt.999) then
        call level3_done
        call exit
      endif
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
          call hcdir('//PAWC/EC',' ')
          call hrput(0,rzhout,'T')
        endif
      endif

      nec  = 0
      nec2 = 0

      do i = 1,nhits
        if(nec.GE.maxEC) then
          print *,'WARN: nec=',nec,' > maxEC=',maxEC
          print *,'       -> skip the rest of event'
          return
        else            
          lay	= ec_layer_trans(slotout(i),chanout(i))
          str	= ec_strip_trans(slotout(i),chanout(i))
          if (lay.gt.0.and.str.gt.0) then
          ped	= ecped(str,lay,1)
          if (ictl.eq.0) then
            adc	= adcout(i)
            if ((adc-ped).gt.57) then
              nec = nec + 1
              secec(nec)   = 1
              layerec(nec) = lay
              stripec(nec) = str
c              print *,i,tdcout(i)/64,ishft(tdcout(i),-6)
c              print *,iand(tdcout(i),63),str,lay
c              print *, ''
              tdcec(nec) = tdcout(i)
              adcec(nec) = min(143325,adc)
              mec(lay)=mec(lay)+1
            endif
          else
            nec2 = nec2+1
            secec2(nec2)  = 1
            layerec2(nec2) = lay
            stripec2(nec2) = str
            n=0
            nsum=0
            do ii=1,100
              adc=rawadcout(ii,i)-ped/35.
              adcec2(ii,nec2)=rawadcout(ii,i)
              if (rawmod.eq.1.and.adc.gt.6.and.n.le.35) then
              nsum=nsum+adc
              n=n+1
              endif
            enddo
            if (n.gt.0.and.nsum.gt.57) then
              mec(lay)=mec(lay)+1
              strp(lay)=str
            endif
          endif
          endif
        endif
      enddo

      print *, nev,nec2

      good_ec123_1 = mec(1).gt.0.and.mec(2).gt.0.and.mec(3).gt.0
      good_ec123_2 = mec(1).le.2.and.mec(2).le.2.and.mec(3).le.2
      good_ec123_i = mec(1).eq.1.and.mec(2).eq.1.and.mec(3).eq.1
      good_ec123_o = mec(4).eq.1.and.mec(5).eq.1.and.mec(6).eq.1
      good_ec123   = good_ec123_1.and.good_ec123_2

      if (ictl.eq.0) then
        good_ec123 = good_ec123_1.and.good_ec123_2
        if (good_ec123_3) call hfnt(10)
        call ec_store_pul(good_ec123_3)
      else
        good_tst = good_ec123_3.and.strp(2).eq.34.and.strp(3).eq.6
        if (out.eq.1.and.rawmod.eq.0) call hfnt(10)
        if (out.eq.1.and.rawmod.eq.1.and.good_tst) call hfnt(10)
        if (out.eq.2) call ec_store_raw
      endif
      
      return
      end
      
