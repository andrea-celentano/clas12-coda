c  event_monitor.inc

c  ejw, 12-may-1998

c  connects c++ and fortran variables
      integer nevipc,nevproc,nev_this_run,nrun_proc
      integer current_run,current_event
      logical no_cc,no_dc,no_ec,no_sc,no_st,no_lac,
     1     no_trig,no_tg,no_photon,no_call,no_l2,no_scaler,no_hist,
     1     no_timeline
      integer event_length

      common/event_monitor/nevipc,nevproc,nev_this_run,nrun_proc,
     1     current_run,current_event,
     1     no_cc,no_dc,no_ec,no_sc,no_st,no_lac,
     1     no_trig,no_tg,no_photon,no_call,no_l2,no_scaler,no_hist,
     1     no_timeline,event_length

