#!/bin/tcsh -f 
echo "BDXPrestart2_1.tcl : RunNumber is: " $1

#ssh to the pcana and start the DST making
ssh -l clasana bdxana1 'cd $BDXRECO_ROOT ; $BDXRECO_ROOT/scripts/runReconOnET.JLabFluxMeasurement.csh'" $1" 
sleep 2
