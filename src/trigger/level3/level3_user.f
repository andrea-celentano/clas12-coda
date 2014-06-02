      subroutine level3_init()
      
      implicit none

      include 'event_monitor.inc'
      include 'lev3.inc'
      
      integer istat,ntupid
      
      integer space
      parameter (space=20000000)
      integer hmemor,iquest
      common/pawc/hmemor(space)
      common/quest/iquest(100)
      character*80 rznout,rzhout
      common/hout/rznout,rzhout

      data ntupid/10/

      ictl   = 0
      out    = 1
      rawmod = 0
      
      print *, 'Enter ECAL (1) or PCAL (2):'
      read(5,*) ecpc 

      print *, 'Enter 0 (PULSE) or 1 (RAW):'
      read(5,*) ictl

      if (ictl.eq.1) then
        print *, 'Enter 1 (NTUP) or 2 (HIST):'
        read(5,*) out
        if (out.eq.1) then
          print *, 'Enter 0 (PED) or 1 (TST):'
          read(5,*) rawmod
        endif
      endif

      print *, 'Enter run number: '
      read(5,*) current_run

      print *, 'Enter hot strip key (100*lay+strip):'
      read(5,*) key

      if (ictl.eq.0) print *, 'LEVEL3_INIT: PULSE MODE'
      if (ictl.eq.1) print *, 'LEVEL3_INIT: RAW MODE'

      call hlimit(space)
      
      if (ecpc.eq.1) then
      if (current_run.lt.1000) then
        write (rznout,1011) current_run
        write (rzhout,1012) current_run
      else
        write (rznout,1001) current_run
        write (rzhout,1002) current_run
      endif
      endif
      
      if (ecpc.eq.2) then
      write (rznout,1003) current_run
      write (rzhout,1004) current_run
      endif
            
c 1001 format('/work/pcal/ecal_',i4,'-0.rzn')
c 1002 format('/work/pcal/ecal_',i4,'-0.rzh')
c 1011 format('/work/pcal/ecal_0',i3,'-0.rzn')
c 1012 format('/work/pcal/ecal_0',i3,'-0.rzh')
c 1003 format('/work/pcal/pcal_',i4,'-0.rzn')
c 1004 format('/work/pcal/pcal_',i4,'-0.rzh')

 1001 format('./ecal1_',i1,'.rzn')
 1002 format('./ecal2_',i1,'.rzh')
 1011 format('./ecal3_',i1,'.rzn')
 1012 format('./ecal4_',i1,'.rzh')
 1003 format('./pcal1_',i1,'.rzn')
 1004 format('./pcal2_',i1,'.rzh')

      if (out.eq.1) then
      
      iquest(10) = 65000
      call hcdir('//PAWC',' ')
      call hropen(2,'CWN',rznout,'NQ',8191,istat)
      call hbset('BSIZE',8176,istat)
      call hbnt(ntupid,'ECPC',' ')
      call hbname(ntupid,' ',0,'$clear')
      
      if (ecpc.eq.1) then
      if (ictl.eq.0) call ecbookcwnpul(ntupid)  
      if (ictl.eq.1) call ecbookcwnraw(ntupid) 
      if (ictl.eq.0) call ecbookrzhpul
      call ec_bor
      call ec_trans
      endif
      
      if (ecpc.eq.2) then
      if (ictl.eq.0) call pcbookcwnpul(ntupid)  
      if (ictl.eq.1) call pcbookcwnraw(ntupid) 
      if (ictl.eq.0) call pcbookrzhpul
      call pc_bor
      call pc_trans
      endif
      
      endif
      
c      elseif (out.eq.2) then
c      rzout = '/work/stage_in/pcal.rzh'
c      call hlimap(space,'COSM')
c      call hropen(1,'HIS',rzout,'NQ',1024,istat)

      if (out.eq.2) call pcbookrzhraw   

      return
      end

c------------------------------------------------------------------------

      subroutine level3_packev(ctl,event,evlen)

      implicit none

      include 'lev3.inc'
      
      integer ctl(4)
      integer event(*)
      integer evlen

      if (ecpc.eq.1) call ec_pack_ev(event)
      if (ecpc.eq.2) call pc_pack_ev(event)
      
      return
      end
      
c------------------------------------------------------------------------
      
      subroutine level3_analyze()
            
c      call pc_store
      
      end
      
c------------------------------------------------------------------------
      
      real function uvw_dist_pc(is,il)
      
      if (il.eq.1.and.is.le.52) uvw=is/84.
      if (il.eq.1.and.is.gt.52) uvw=(52+(is-52)*2)/84.
      if (il.eq.2.and.is.le.15) uvw=2*is/77.
      if (il.eq.2.and.is.gt.15) uvw=(30+(is-15))/77.
      if (il.eq.3.and.is.le.15) uvw=2*is/77.
      if (il.eq.3.and.is.gt.15) uvw=(30+(is-15))/77.
      
      uvw_dist_pc = uvw
      
      end
      
      real function uvw_dist_ec(is,il)
      
      uvw_dist_ec=is/36.
      
      end

c-----------------------------------------------------------------------


      subroutine level3_done

      include 'lev3.inc'
      
      integer icycle
      character*80 rznout,rzhout
      common/hout/rznout,rzhout

      print *, 'LEVEL3_DONE:CLOSING HBOOK FILES',out
     
      if(out.gt.0) then
      call hcdir('//PAWC',' ')
      call hrput(0,rzhout,'T')      
      call hcdir('//CWN',' ')
      call hldir('//CWN',' ')
      call hrout(10,icycle,' ')
      call hrend('CWN')
      endif

      return
      end



