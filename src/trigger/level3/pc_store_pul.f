      subroutine pc_store_pul(pix)
      
      include 'lev3.inc'
      include 'PCnt.inc'
      include 'PCstore.inc'
      include 'pccal.inc'
      
      real tdc,adc,chan,ped
      integer s,lay,str,ii,sl,thr,maxstr(3),ntim
      logical pix

      data thr/15/ntim/1/  
      data maxstr/68,62,62/  
      data adccal/0.35/  

      save ntim

      integer nh(3,6)
      real strr(68,3,6),adcr(68,3,6)
      real rs1,rs2,rs3
      real ad1,ad2,ad3

      logical good_u,good_v,good_w
      logical good_uv,good_uw,good_vw
      logical good_uwt,good_wut,good_vwt
      logical good_uwtt,good_wutt,good_vwtt
      logical good_uwtc,good_uwttb,good_uwttc
      logical good_uvw
      logical good

      call hcdir('//PAWC/PC',' ')

      if(tdiff.ge.60) then
        ntim=ntim+1
        if (ntim.gt.1440) return
        newt0=telap
      endif

c Init: Clear arrays      
      
      do i=1,3
        nh(i,1)=0
        do j=1,68
          strr(j,i,1)=0.
          adcr(j,i,1)=0.
        enddo
      enddo
      
      uvw = 0.
      
c Loop: Subtract pedestals, get hits and fill arrays      

      do i=1,npc
        is=strippc(i)
        il=layerpc(i)
        id2 = 100*secpc(i)+il
        adc=adccal*(adcpc(i)-pcped(is,il,1))
        call hf2(id2-50,adc,float(is),1.)
        ikey = il*100+is
        if (ikey.ne.key) then
        if (adc.gt.thr) then
          nh(il,1)=nh(il,1)+1
          inh = nh(il,1)
          adcr(inh,il,1)=adc
          strr(inh,il,1)=is
          call hf2(id2,adc,float(is),1.)
          if(pix) call hf2(id2+10,adc,float(is),1.)
        endif
        if (adc.gt.38) then
          call hf2(id2+20,float(is),float(ntim),1.)
          call hf2(id2+30,float(is),float(ntim),adc)
        endif
        uvw=uvw+uvw_dist_pc(is,il)
        endif
      enddo

      call hf1(777,uvw,1.)      
      
c Logic: Multiplicity tests      
      
      good_u=nh(1,1).eq.1
      good_v=nh(2,1).eq.1
      good_w=nh(3,1).eq.1
      
      if (good_u)rs1=strr(1,1,1)
      if (good_v)rs2=strr(1,2,1)
      if (good_w)rs3=strr(1,3,1)
      if (good_u)ad1=adcr(1,1,1)
      if (good_v)ad2=adcr(1,2,1)
      if (good_w)ad3=adcr(1,3,1)
      
      good_uv=good_u.and.good_v
      good_uw=good_u.and.good_w
      good_vw=good_v.and.good_w
      
c Logic: Rough pixel cut (no Dalitz)      
      
      good_uvw=good_u.and.good_v.and.good_w
      
c Logic: Next longest strip trigger      
      
      good_uwt=good_uw.and.rs3.eq.61
      good_wut=good_uw.and.rs1.eq.67
      good_vwt=good_uv.and.rs1.eq.67
      
c Logic: Next longest strip trigger threshold (default=70)  
      
      good_uwtt=good_uwt.and.ad3.gt.70
      good_wutt=good_wut.and.ad1.gt.70
      good_vwtt=good_vwt.and.ad1.gt.70
      
c Histo: Multiplicity per layer and vs. energy sum

      do i=1,3
        sum = 0.
        do k=1,nh(i,1)
          sum=sum+adcr(k,i,1)
        enddo
        call hf1(10+i,float(nh(i,1)),1.)
        call hf2(20+i,float(nh(i,1)),sum,1.)
      enddo
      
      if (good_uvw) then
        call hf1(778,uvw,1.)
      else
        go to 100
      endif
      
c Histo: U vs V, U vs W, V vs W (used for detector map)      
     
      do i=1,nh(1,1)
        iis1=strr(i,1,1)
      do k=1,nh(2,1)
        iis2=strr(k,2,1)
        call hf2(4001,float(iis1),float(iis2),1.)	!uv
        call hf2(4002,float(iis1),float(iis2),ad1)	!uv
      enddo
      do k=1,nh(3,1)
        iis3=strr(k,3,1)
        call hf2(4011,float(iis1),float(iis3),1.)	!uw
        call hf2(4012,float(iis1),float(iis3),ad1)	!uw
      enddo
      enddo
      
      do i=1,nh(2,1)
        iis2=strr(i,2,1)
      do k=1,nh(3,1)
        iis3=strr(k,3,1)
        call hf2(4021,float(iis2),float(iis3),1.)	!vw
        call hf2(4022,float(iis2),float(iis3),ad2)	!vw
        call hf2(4031,float(iis3),float(iis2),1.)	!wv
        call hf2(4032,float(iis3),float(iis2),ad3)	!wv
      enddo
      enddo
      
c Histo: MIP plots using next-to-longest strip trigger    

      if(good_uwt) call hf2(201,adcr(1,1,1),rs1,1.)
      if(good_vwt) call hf2(202,adcr(1,2,1),rs2,1.)
      if(good_wut) call hf2(203,adcr(1,3,1),rs3,1.)
      
      if (good_uvw) then
      
      if(good_uwt) call hf2(211,adcr(1,1,1),rs1,1.)
      if(good_vwt) call hf2(212,adcr(1,2,1),rs2,1.)
      if(good_wut) call hf2(213,adcr(1,3,1),rs3,1.)
      
      if(good_uwtt) call hf2(221,adcr(1,1,1),rs1,1.)
      if(good_vwtt) call hf2(222,adcr(1,2,1),rs2,1.)
      if(good_wutt) call hf2(223,adcr(1,3,1),rs3,1.)
         
      endif

c Histo: Attenuation lengths (ADC vs strip)      
      
      if(good_uv) then
         if(ad2.gt.60)call hf2(5100+ifix(rs1),rs2,adcr(1,1,1),1.)
         if(ad1.gt.60)call hf2(5200+ifix(rs2),rs1,adcr(1,2,1),1.)
      endif
      if(good_uw) then
         if(ad3.gt.60)call hf2(5300+ifix(rs1),rs3,adcr(1,1,1),1.)
         if(ad1.gt.60)call hf2(5400+ifix(rs3),rs1,adcr(1,3,1),1.)
      endif
      if(good_vw) then
         if(ad3.gt.60)call hf2(5500+ifix(rs2),rs3,adcr(1,2,1),1.)
         if(ad2.gt.60)call hf2(5600+ifix(rs3),rs2,adcr(1,3,1),1.)
      endif   

 100  continue
      
      call hcdir('//PAWC',' ')       
      
      end
