#!/bin/tcsh -f

ssh -l clasana bdxana1 'cd $BDXRECO_ROOT ;  killall -INT bdxReco'
