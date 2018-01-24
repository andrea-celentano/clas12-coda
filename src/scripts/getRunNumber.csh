#!/bin/tcsh -f


if ($#argv != 1) then
	echo "use: getRunNumber sessionName"
	exit
endif


set db = daq_$EXPID
set session = $1

echo select runNumber from sessions where name = \"$session\" > tmp.sql

mysql $db < tmp.sql > out.tmp

set runN = (`awk '{print $2}' "out.tmp"`)

set runN = `tail -n1 out.tmp`

echo $runN

rm out.tmp
rm tmp.sql
