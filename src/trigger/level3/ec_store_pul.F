      subroutine ec_store_pul(pix)
      
      include 'lev3.inc'
      include 'ECnt.inc'
      include 'ECstore.inc'
      include 'eccal.inc'
      
      real tdc,adc,chan,ped
      integer s,lay,str,ii,sl,thr,maxstr(6),ntim
      logical pix

      data thr/15/ntim/1/  
      data maxstr/36,36,36,36,36,36/  
      data adccal/0.35/  

      save ntim

      integer nh(6,6)
      real strr(36,6,6),adcr(36,6,6)
      real rs1,rs2,rs3,rs4,rs5,rs6
      real ad1,ad2,ad3,ad4,ad5,ad6

      logical good_ui,good_vi,good_wi
      logical good_uo,good_vo,good_wo
      logical good_uvi,good_uwi,good_vwi
      logical good_uvo,good_uwo,good_vwo
      logical good_uwti,good_wuti,good_vwti
      logical good_uwto,good_wuto,good_vwto
      logical good_uwtti,good_wutti,good_vwtti
      logical good_uwtto,good_wutto,good_vwtto
      logical good_uvw_i,good_uvw_o,good_uvw
      logical good

      call hcdir('//PAWC/EC',' ')

      if(tdiff.ge.60) then
        ntim=ntim+1
        if (ntim.gt.1440) return
        newt0=telap
      endif

c Init: Clear arrays      
      
      do i=1,6
        nh(i,1)=0
        do j=1,36
          strr(j,i,1)=0.
          adcr(j,i,1)=0.
        enddo
      enddo
      
      uvw = 0.
      
c Loop: Subtract pedestals, get hits and fill arrays      

      do i=1,nec
        is=stripec(i)
        il=layerec(i)
        id2 = 100*secec(i)+il
        adc=adccal*(adcec(i)-ecped(is,il,1))
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
        uvw=uvw+uvw_dist_ec(is,il)
        endif
      enddo

      call hf1(777,uvw,1.)      
      
c Logic: Multiplicity tests      
      
      good_ui=nh(1,1).eq.1
      good_vi=nh(2,1).eq.1
      good_wi=nh(3,1).eq.1
      good_uo=nh(4,1).eq.1
      good_vo=nh(5,1).eq.1
      good_wo=nh(6,1).eq.1
      
      if (good_ui)rs1=strr(1,1,1)
      if (good_vi)rs2=strr(1,2,1)
      if (good_wi)rs3=strr(1,3,1)
      if (good_ui)ad1=adcr(1,1,1)
      if (good_vi)ad2=adcr(1,2,1)
      if (good_wi)ad3=adcr(1,3,1)
      if (good_uo)rs4=strr(1,4,1)
      if (good_vo)rs5=strr(1,5,1)
      if (good_wo)rs6=strr(1,6,1)
      if (good_uo)ad4=adcr(1,4,1)
      if (good_vo)ad5=adcr(1,5,1)
      if (good_wo)ad6=adcr(1,6,1)
      
      good_uvi=good_ui.and.good_vi
      good_uwi=good_ui.and.good_wi
      good_vwi=good_vi.and.good_wi
      good_uvo=good_uo.and.good_vo
      good_uwo=good_uo.and.good_wo
      good_vwo=good_vo.and.good_wo
      
c Logic: Rough pixel cut (no Dalitz)      
      
      good_uvw_i=good_ui.and.good_vi.and.good_wi
      good_uvw_o=good_uo.and.good_vo.and.good_wo
      good_uvw=good_uvw_i.and.good_uvw_o
      
c Logic: Next longest strip trigger      
      
      good_uwti=good_uwi.and.rs3.eq.35
      good_wuti=good_uwi.and.rs1.eq.35
      good_vwti=good_uvi.and.rs1.eq.35
      good_uwto=good_uwo.and.rs6.eq.35
      good_wuto=good_uwo.and.rs4.eq.35
      good_vwto=good_uvo.and.rs4.eq.35
      
c Logic: Next longest strip trigger threshold (default=70)  
      
      good_uwtti=good_uwti.and.ad3.gt.70
      good_wutti=good_wuti.and.ad1.gt.70
      good_vwtti=good_vwti.and.ad1.gt.70
      good_uwtto=good_uwto.and.ad6.gt.70
      good_wutto=good_wuto.and.ad4.gt.70
      good_vwtto=good_vwto.and.ad4.gt.70
      
c Histo: Multiplicity per layer and vs. energy sum

      do i=1,6
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
      
c Histo: U vs V, U vs W, V vs W (used for detector map) (inner)      
     
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
      
c Histo: U vs V, U vs W, V vs W (used for detector map) (outer)      
     
      do i=1,nh(4,1)
        iis4=strr(i,4,1)
      do k=1,nh(5,1)
        iis5=strr(k,5,1)
        call hf2(4041,float(iis4),float(iis5),1.)	!uv
        call hf2(4042,float(iis4),float(iis5),ad4)	!uv
      enddo
      do k=1,nh(6,1)
        iis6=strr(k,6,1)
        call hf2(4051,float(iis4),float(iis6),1.)	!uw
        call hf2(4052,float(iis4),float(iis6),ad4)	!uw
      enddo
      enddo
      
      do i=1,nh(5,1)
        iis5=strr(i,5,1)
      do k=1,nh(6,1)
        iis6=strr(k,6,1)
        call hf2(4061,float(iis5),float(iis6),1.)	!vw
        call hf2(4062,float(iis5),float(iis6),ad5)	!vw
        call hf2(4071,float(iis6),float(iis5),1.)	!wv
        call hf2(4072,float(iis6),float(iis5),ad6)	!wv
      enddo
      enddo
      
c Histo: MIP plots using next-to-longest strip trigger    

      if(good_uwti) call hf2(201,adcr(1,1,1),rs1,1.)
      if(good_vwti) call hf2(202,adcr(1,2,1),rs2,1.)
      if(good_wuti) call hf2(203,adcr(1,3,1),rs3,1.)
      if(good_uwto) call hf2(204,adcr(1,4,1),rs4,1.)
      if(good_vwto) call hf2(205,adcr(1,5,1),rs5,1.)
      if(good_wuto) call hf2(206,adcr(1,6,1),rs6,1.)
      
      if (good_uvw) then
      
      if(good_uwti) call hf2(211,adcr(1,1,1),rs1,1.)
      if(good_vwti) call hf2(212,adcr(1,2,1),rs2,1.)
      if(good_wuti) call hf2(213,adcr(1,3,1),rs3,1.)
      if(good_uwto) call hf2(214,adcr(1,4,1),rs4,1.)
      if(good_vwto) call hf2(215,adcr(1,5,1),rs5,1.)
      if(good_wuto) call hf2(216,adcr(1,6,1),rs6,1.)
      
      if(good_uwtti) call hf2(221,adcr(1,1,1),rs1,1.)
      if(good_vwtti) call hf2(222,adcr(1,2,1),rs2,1.)
      if(good_wutti) call hf2(223,adcr(1,3,1),rs3,1.)
      if(good_uwtto) call hf2(224,adcr(1,4,1),rs4,1.)
      if(good_vwtto) call hf2(225,adcr(1,5,1),rs5,1.)
      if(good_wutto) call hf2(226,adcr(1,6,1),rs6,1.)
         
      endif

c Histo: Attenuation lengths (ADC vs strip)      
      
      if(good_uvi) then
         if(ad2.gt.60)call hf2(5100+ifix(rs1),rs2,adcr(1,1,1),1.)
         if(ad1.gt.60)call hf2(5200+ifix(rs2),rs1,adcr(1,2,1),1.)
      endif
      if(good_uwi) then
         if(ad3.gt.60)call hf2(5300+ifix(rs1),rs3,adcr(1,1,1),1.)
         if(ad1.gt.60)call hf2(5400+ifix(rs3),rs1,adcr(1,3,1),1.)
      endif
      if(good_vwi) then
         if(ad3.gt.60)call hf2(5500+ifix(rs2),rs3,adcr(1,2,1),1.)
         if(ad2.gt.60)call hf2(5600+ifix(rs3),rs2,adcr(1,3,1),1.)
      endif   
      
      if(good_uvo) then
         if(ad5.gt.60)call hf2(6100+ifix(rs4),rs5,adcr(1,4,1),1.)
         if(ad4.gt.60)call hf2(6200+ifix(rs5),rs4,adcr(1,5,1),1.)
      endif
      if(good_uwo) then
         if(ad6.gt.60)call hf2(6300+ifix(rs4),rs6,adcr(1,4,1),1.)
         if(ad4.gt.60)call hf2(6400+ifix(rs6),rs4,adcr(1,6,1),1.)
      endif
      if(good_vwo) then
         if(ad6.gt.60)call hf2(6500+ifix(rs5),rs6,adcr(1,5,1),1.)
         if(ad5.gt.60)call hf2(6600+ifix(rs6),rs5,adcr(1,6,1),1.)
      endif   

 100  continue
      
      call hcdir('//PAWC',' ')       
      
      end
