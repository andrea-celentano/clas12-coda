#!/bin/tcsh -f


#echo "testPrestart is here"
set runNumber = `getRunNumber.csh bdxtest`
echo "runNumber is: " $runNumber
setenv DISPLAY :0.0

#just to wait the file
sleep 1

#copy the ped file to the ana machine
echo "BEFORE SCP"
scp $CLON_PARMS/peds/bdxdev/run$runNumber.ped 'clasana@bdxana1:$BDXRECO_ROOT/calib/DAQ_pedestals'
echo "AFTER SCP before ssh"
#load to DB
ssh -l clasana bdxana1 'cd $BDXRECO_ROOT/calib ; pwd ; echo $CCDB_USER ; setenv CCDB_USER bdxCT ; echo $CCDB_USER ; python loadPedestals.py --variation JLabFlux0_peds --connection mysql://bdxwriter:e1tocome@jdaq3/BDX --firstRun '"$runNumber"' --lastRun '"$runNumber"
