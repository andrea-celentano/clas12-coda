      subroutine ec_store_pul(pix)
      
      include 'lev3.inc'
      include 'ECnt.inc'
      include 'ECstore.inc'
      include 'eccal.inc'

      logical pix
          
      integer nh(6,6),nio(2,6) 
      integer hid,hidd,hidd1,rs1,rs2,rs3,tid
      
      real strr(36,6,6),adcr(36,6,6)
      real rs(6),ad(6)
      real maxstr(6),uvw(2,6)
      
      logical good_lay(6)
      logical good_uw(2),good_vu(2),good_wv(2)
      logical good_uwt(2),good_vut(2),good_wvt(2)
      logical good_uwtt(2),good_vutt(2),good_wvtt(2)
      logical good_uvw(2)
      logical good
      
      data thr/15/ntim/1/
      data ilmax/6/
      data iomax/2/
      data maxstr/36,36,36,36,36,36/     
      data tid/100000/
      data iid/10000/
      
      save ntim

c is: sector index
c il: layer index
c ip: pmt index

      call hcdir('//PAWC/EC',' ')

      if(tdiff.ge.60) then
        ntim=ntim+1
        if (ntim.gt.1440) return
        newt0=telap
      endif

      if (nec.eq.0) return   
      
c Init: Clear arrays   

      do is=1,6
        uvw(1,is)=0
        uvw(2,is)=0
        nio(1,is)=0
        nio(2,is)=0
      do il=1,ilmax
        nh(il,is)=0
        do ip=1,maxstr(il)
          strr(ip,il,is)=0.
          adcr(ip,il,is)=0.
        enddo
      enddo
      enddo
      
c Loop: Subtract pedestals, get hits and fill arrays      
        
      do i=1,nec
        is=secec(i)
        ip=stripec(i)
        il=layerec(i)
        io=1
        if (il.gt.3) io=2
        ill=il-3*(io-1)
        hid=1e7*is + 10*tid + io*iid + ill*100
        adc=adcec(i)-ecped(ip,il,is)
        if (adc.gt.thr) then
          nh(il,is)=nh(il,is)+1
          nio(io,is)=nio(io,is)+1
          inh = nh(il,is)
          adcr(inh,il,is)=adc
          strr(inh,il,is)=ip
          call hf2(hid,adc,float(ip),1.)
          if (io.eq.1) uvw(1,is)=uvw(1,is)+ip/maxstr(il)
          if (io.eq.2) uvw(2,is)=uvw(2,is)+ip/maxstr(il)
        endif
      enddo
      
c Loop: Loop over sectors       
      
      do is=1,6
      
      if (nio(1,is).gt.1.and.nio(2,is).gt.1) then
      
      hid=1e7*is
      
c Logic: Limit multiplicity to 1 hit per view   
      
      do il=1,ilmax
        good_lay(il)=nh(il,is).eq.1
        if (good_lay(il)) then
          rs(il)=strr(1,il,is)
          ad(il)=adcr(1,il,is)
        endif
      enddo

c Logic: Good two-view and three view (Dalitz)
             
      do io=1,iomax
        ind=(io-1)*3
        good_uw(io)=good_lay(1+ind).and.good_lay(3+ind)
        good_vu(io)=good_lay(2+ind).and.good_lay(1+ind)
        good_wv(io)=good_lay(3+ind).and.good_lay(2+ind)
        good_uvw(io)=good_vu(io).and.good_lay(3+ind)
      enddo
            
c Logic: Next longest strip trigger and threshold      
      
      do io=1,iomax
        ind=(io-1)*3
        good_uwt(io)=good_uw(io).and.rs(3+ind).eq.33
        good_vut(io)=good_vu(io).and.rs(1+ind).eq.33
        good_wvt(io)=good_wv(io).and.rs(2+ind).eq.33
        good_uwtt(io)=good_uwt(io).and.ad(3+ind).gt.10
        good_vutt(io)=good_vut(io).and.ad(1+ind).gt.10
        good_wvtt(io)=good_wvt(io).and.ad(2+ind).gt.10
      enddo
      
c Histo: Multiplicity per layer and vs. energy sum

      call hf1(hid+7*tid+10000,uvw(1,is),1.)    
      call hf1(hid+7*tid+20000,uvw(2,is),1.)
      
      if (good_uvw(1)) call hf1(hid+8*tid+10000,uvw(1,is),1.)    
      if (good_uvw(2)) call hf1(hid+8*tid+20000,uvw(2,is),1.)
      
      do io=1,iomax
        ind=(io-1)*3
        do il=1,3
          sum = 0.
          do k=1,nh(ind+il,is)
            sum=sum+adcr(k,ind+il,is)
          enddo
          hidd=hid+io*iid+il*100
          call hf1(hidd+1*tid,float(nh(ind+il,is)),1.)
          call hf2(hidd+2*tid,float(nh(ind+il,is)),sum,1.)
          call hf2(100+is,uvw(1,is),uvw(2,is),1.)
          if (good_uvw(io)) then
            call hf2(hidd+11*tid,ad(ind+il),rs(ind+il),1.)
          endif
        enddo
      enddo
      
c Histo: U vs V, U vs W, V vs W (used for detector map)  
 
      do io=1,iomax
      if (good_uvw(io)) then
      
      ind=(io-1)*3
      hidd=hid+40*tid+io*iid
      
      do i=1,nh(1+ind,is)
        ris1=strr(i,1+ind,is)
      do k=1,nh(2+ind,is)
        ris2=strr(k,2+ind,is)
        call hf2(hidd+1201,ris1,ris2,1.)	!uv
        call hf2(hidd+1202,ris1,ris2,ad(1+ind))	!uv
      enddo
      
      do k=1,nh(3+ind,is)
        ris3=strr(k,3+ind,is)
        call hf2(hidd+1301,ris1,ris3,1.) 	!uw
        call hf2(hidd+1302,ris1,ris3,ad(1+ind))	!uw
      enddo
      enddo
      
      do i=1,nh(2+ind,is)
        ris2=strr(i,2+ind,is)
      do k=1,nh(3+ind,is)
        ris3=strr(k,3+ind,is)
        call hf2(hidd+2301,ris2,ris3,1.)	!vw
        call hf2(hidd+2302,ris2,ris3,ad(2+ind))	!vw
        call hf2(hidd+3201,ris3,ris2,1.)	!wv
        call hf2(hidd+3202,ris3,ris2,ad(3+ind))	!wv
      enddo
      enddo
      
      endif
      enddo
      
c Histo: MIP plots using next longest strip trigger    

      do io=1,iomax
      ind=(io-1)*3
      hidd = hid+20*tid+io*iid
      
      if(good_uwt(io)) call hf2(hidd+100,ad(1+ind),rs(1+ind),1.)
      if(good_vut(io)) call hf2(hidd+200,ad(2+ind),rs(2+ind),1.)
      if(good_wvt(io)) call hf2(hidd+300,ad(3+ind),rs(3+ind),1.)
      
      if (good_uvw(io)) then
      
      hidd = hid+21*tid+io*iid
      if(good_uwt(io)) call hf2(hidd+100,ad(1+ind),rs(1+ind),1.)
      if(good_vut(io)) call hf2(hidd+200,ad(2+ind),rs(2+ind),1.) 
      if(good_wvt(io)) call hf2(hidd+300,ad(3+ind),rs(3+ind),1.) 
      
      hidd = hid+22*tid+io*iid
      if(good_uwtt(io)) call hf2(hidd+100,ad(1+ind),rs(1+ind),1.)
      if(good_vutt(io)) call hf2(hidd+200,ad(2+ind),rs(2+ind),1.)
      if(good_wvtt(io)) call hf2(hidd+300,ad(3+ind),rs(3+ind),1.)
         
      endif
      enddo
      
c Histo: Attenuation lengths (ADC vs strip)      
      
      do io=1,iomax
      if (good_uvw(io)) then
      
      ind=(io-1)*3
      hidd  = hid+50*tid+io*iid
      hidd1 = hid+51*tid+io*iid
      
      rs1=rs(1+ind)
      rs2=rs(2+ind)
      rs3=rs(3+ind)
      ad1=ad(1+ind)
      ad2=ad(2+ind)
      ad3=ad(3+ind)
      sum=ad1+ad2+ad3
      
      if(good_vu(io)) then
         if(ad2.gt.15)call hf2(hidd+2100+rs1,float(rs2),ad1,1.)
         if(ad1.gt.15)call hf2(hidd+1200+rs2,float(rs1),ad2,1.)
         if(ad2.gt.15)call hf2(hidd1+2100+rs1,float(rs2),sum,1.)
      endif
      if(good_uw(io)) then
         if(ad3.gt.15)call hf2(hidd+3100+rs1,float(rs3),ad1,1.)
         if(ad1.gt.15)call hf2(hidd+1300+rs3,float(rs1),ad3,1.)
         if(ad3.gt.15)call hf2(hidd1+3100+rs1,float(rs3),sum,1.)
      endif
      if(good_wv(io)) then
         if(ad3.gt.15)call hf2(hidd+3200+rs2,float(rs3),ad2,1.)
         if(ad2.gt.15)call hf2(hidd+2300+rs3,float(rs2),ad3,1.)
         if(ad3.gt.15)call hf2(hidd1+3200+rs2,float(rs3),sum,1.)
      endif  
      
      endif
      enddo 
      
      endif
      enddo

      call hcdir('//PAWC',' ')
      
      end
      
