#!/bin/sh
#\
exec /usr/bin/expect -- "$0" ${1+"$@"}
#


set xpos  0
set xinc  530
set ypos  0
set yinc  100
set lines 5

set nlist 23
set roc_list "hps11 hps1 hps2 hps12 hps1gtp hps2gtp dtm0 dtm1 \
              dpm0 dpm1 dpm2 dpm3 dpm4 dpm5 dpm6 dpm7 \
              dpm8 dpm9 dpm10 dpm11 dpm12 dpm13 dpm14"


for {set i 0} {$i < $nlist} {incr i 1} {
  set roc [lindex $roc_list $i]
  puts $roc

  if [string match hps11 $roc] {
    set bg_color "lightblue"
    set fg_color "black"
  } elseif [string match hps1 $roc] {
    set bg_color "lightgreen"
    set fg_color "black"
  } elseif [string match hps2 $roc] {
    set bg_color "lightgreen"
    set fg_color "black"
  } elseif [string match hps12 $roc] {
    set bg_color "lightgreen"
    set fg_color "black"
  } elseif [string match hps1gtp $roc] {
    set bg_color "lightgreen"
    set fg_color "black"
  } elseif [string match hps2gtp $roc] {
    set bg_color "lightgreen"
    set fg_color "black"
  } else {
    set bg_color "DarkGreen"
    set fg_color "white"
  }


  if {[string match hps11 $roc]} {
    exec xterm -title "$roc" -sb -sl 1000 \
               -geometry 80x$lines+$xpos+$ypos -bg $bg_color -fg $fg_color \
               -e prxt_linux_ts.tcl $roc &
  } else {

    if ([string match hps1 $roc]) {
      exec xterm -title "$roc" -sb -sl 1000 \
                 -geometry 80x$lines+$xpos+$ypos -bg $bg_color -fg $fg_color \
                 -e prxt_linux_gef.tcl $roc &

    } elseif ([string match hps2 $roc]) {
      exec xterm -title "$roc" -sb -sl 1000 \
                 -geometry 80x$lines+$xpos+$ypos -bg $bg_color -fg $fg_color \
                 -e prxt_linux_gef.tcl $roc &

    } elseif ([string match hps12 $roc]) {
      exec xterm -title "$roc" -sb -sl 1000 \
                 -geometry 80x$lines+$xpos+$ypos -bg $bg_color -fg $fg_color \
                 -e prxt_linux_gef.tcl $roc &

    } else {
      exec xterm -title "$roc" -sb -sl 1000 \
                 -geometry 80x$lines+$xpos+$ypos -bg $bg_color -fg $fg_color \
                 -e prxt_linux.tcl $roc &
    }
  }


  set ypos [expr $ypos + $yinc]
  if {($i == 7) || ($i == 15)} {
    set xpos [expr $xpos + $xinc]
    set ypos 0
  }
}

