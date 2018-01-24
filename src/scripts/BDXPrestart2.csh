#!/bin/tcsh -f

set runNumber = `getRunNumber.csh bdxtest`
echo "runNumber is: " $runNumber
setenv DISPLAY :0.0
set tmp = `xterm -T RECON -e BDXPrestart2_1.tcl $runNumber >& /dev/null &`
