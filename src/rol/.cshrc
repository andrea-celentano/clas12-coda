# @(#)Cshrc 1.6 91/09/05 SMI

#  .cshrc for clasrun account
#
#  do not modify this file without permission from the online group!
#
##echo ">> $PATH <<"

###echo "starting .cshrc"

# clas software tree
###setenv CLAS /usr/local/clas/release/current

###setenv CLAS /usr/local/clas/clas12
setenv CLAS /usr/local/clas12/release/0.1

if ( ($HOST == "ltcc0.jlab.org") || ($HOST == "ltcc0") ) then
  setenv CLAS /usr/local/clas12/release/0.1
else if ( ($HOST == "clon00.jlab.org") || ($HOST == "clon00") ) then
  setenv CLAS /usr/local/clas/devel
else if ( ($HOST == "pcal0.jlab.org") || ($HOST == "pcal0") ) then
  setenv CLAS /usr/local/clas12/release/0.1
else if ( ($HOST == "dcrb1.jlab.org") || ($HOST == "dcrb1") ) then
  setenv CLAS /usr/local/clas12/release/0.1
else if ( ($HOST == "svt2.jlab.org") || ($HOST == "svt2") ) then
  setenv CLAS /usr/local/clas12/release/0.1
else if ( ($HOST == "svt5.jlab.org") || ($HOST == "svt5") ) then
  setenv CLAS /usr/local/clas12/release/0.1
else if ( ($HOST == "adcecal1.jlab.org") || ($HOST == "adcecal1") ) then
  setenv CLAS /usr/local/clas12/release/0.1
# For RootSpy
  setenv CMSGROOT /home/clasrun/ecal/pcal/paw/CODA
  setenv LD_LIBRARY_PATH ${CMSGROOT}/lib
  setenv PATH "$PATH":/home/clasrun/ecal/jre1.7.0_45/bin:/home/clasrun/ecal/pcal/paw/RootSpy/src/RootSpy
endif

setenv OSTYPE `uname`
setenv MACHINE `uname -m`

#
# Solaris on sparc
#
if ( ($OSTYPE == "SunOS") && ($MACHINE == "sun4u") ) then

#define PATH
  setenv PATH /bin
  setenv PATH "$PATH":/usr/bin
  setenv PATH "$PATH":/usr/sbin
  setenv PATH "$PATH":/opt/SUNWspro/bin
  setenv PATH "$PATH":/opt/sfw/bin
  setenv PATH "$PATH":/usr/sfw/bin
  setenv PATH "$PATH":/usr/ucb
  setenv PATH "$PATH":/usr/ccs/bin
  setenv PATH "$PATH":/usr/dt/bin
  setenv PATH "$PATH":/usr/openwin/bin
  setenv PATH "$PATH":/usr/local/system
  setenv PATH "$PATH":/usr/local/bin
  setenv PATH "$PATH":/apps/bin
  setenv PATH "$PATH":/site/bin
  setenv PATH "$PATH":/home/clasrun/bin

#define LD_LIBRARY_PATH
  setenv LD_LIBRARY_PATH /usr/dt/lib
  setenv LD_LIBRARY_PATH "$LD_LIBRARY_PATH":/usr/openwin/lib
  setenv LD_LIBRARY_PATH "$LD_LIBRARY_PATH":/usr/lib
  setenv LD_LIBRARY_PATH "$LD_LIBRARY_PATH":/opt/SUNWspro/lib
  setenv LD_LIBRARY_PATH "$LD_LIBRARY_PATH":/opt/sfw/lib

endif

#
# Solaris on Intel
#
if ( ($OSTYPE == "SunOS") && ($MACHINE == "i86pc") ) then

  setenv PATH /bin
  setenv PATH "$PATH":/usr/bin
  setenv PATH "$PATH":/usr/sbin
  setenv PATH "$PATH":/opt/SUNWspro/bin
  setenv PATH "$PATH":/usr/ucb
  setenv PATH "$PATH":/usr/ccs/bin
  setenv PATH "$PATH":/apps/bin
  setenv PATH "$PATH":/usr/openwin/bin
  setenv PATH "$PATH":/opt/sfw/bin
  setenv PATH "$PATH":/usr/sfw/bin
  setenv PATH "$PATH":/usr/local/bin
  setenv PATH "$PATH":/site/bin

  setenv LD_LIBRARY_PATH /usr/dt/lib
  setenv LD_LIBRARY_PATH "$LD_LIBRARY_PATH":/usr/openwin/lib
  setenv LD_LIBRARY_PATH "$LD_LIBRARY_PATH":/usr/lib
  setenv LD_LIBRARY_PATH "$LD_LIBRARY_PATH":/opt/SUNWspro/lib
  setenv LD_LIBRARY_PATH "$LD_LIBRARY_PATH":/opt/sfw/lib

endif

#
# Linux on Intel
#
if ( ($OSTYPE == "Linux") && ($MACHINE == "i686") ) then

###setenv XAUTHORITY /tmp/.Xauthority.$LOGNAME

endif

#
# Linux on Opteron (64-bit)
#
if ( ($OSTYPE == "Linux") && ($MACHINE == "x86_64") ) then
  setenv LD_LIBRARY_PATH /lib64
endif

#
# Linux on PowerPC
#
if ( ($OSTYPE == "Linux") && ($MACHINE == "ppc") ) then
  setenv LD_LIBRARY_PATH /usr/lib
endif


#setup smartsockets
###  source /apps/smartsockets/ss68/bin/rtinit.csh



#
# CLAS software setup
#
source $CLAS/.setup

#for devel only !!!
setenv EXPID clasdev
setenv SESSION clastest


if ( ($HOST == "svt1") || ($HOST == "svt1.jlab.org") ) then
  setenv EXPID classvt
  setenv SESSION classvt1
#  setenv MYSQL_HOST svt1
endif
if ( ($HOST == "svt2") || ($HOST == "svt2.jlab.org") ) then
  setenv EXPID classvt
  setenv SESSION classvt2
#  setenv MYSQL_HOST svt2
endif
if ( ($HOST == "svt3") || ($HOST == "svt3.jlab.org") ) then
  setenv EXPID classvt
  setenv SESSION classvt3
#  setenv MYSQL_HOST svt3
endif
if ( ($HOST == "svt4") || ($HOST == "svt4.jlab.org") ) then
  setenv EXPID classvt
  setenv SESSION classvt4
#  setenv MYSQL_HOST svt4
endif
if ( ($HOST == "svt5") || ($HOST == "svt5.jlab.org") ) then
  setenv EXPID classvt
  setenv SESSION classvt5
  # Added by beng for using elog API
  setenv PATH ${PATH}:/apps/jdk1.6.0_31/bin  
#  setenv MYSQL_HOST svt5
endif
if ( ($HOST == "svt6") || ($HOST == "svt6.jlab.org") ) then
  setenv EXPID classvt
  setenv SESSION classvt6
#  setenv MYSQL_HOST svt6
endif

if ( ($HOST == "pcal0") ) then
  setenv MYSQL_HOST pcal0
endif
if ( ($HOST == "pcal0.jlab.org") ) then
  setenv MYSQL_HOST pcal0
endif




setenv PATH "$PATH":.

# do we need it ? here ?
setenv RECON_HIST_DIR /work/hist/recon
alias xt "xterm -sb -sl 5000 -ls"
alias ll "ls -al"
alias vi vim
alias l "ls -lh"

#  aliases that can't be in .alias or .login...ejw
alias account "ssh -l epicsb jlabs1 ""/u/group/accsft/bta/btaCUE.csh b"" >& /dev/null &"
alias onlb "online_buttons -a clasprod -session clasprod >& /dev/null &"

## replaced by script
##alias reset_all_hist 'tclipc -a clasprod -c "reset 0" mon_clasprod'
##alias reset_hist     'tclipc -a clasprod -c "reset 0" mon_clasprod'

# login into clonwin3
alias clonwin3 'ssh clonmon2 rdesktop -g 1024x768 -u clasrun -p e1ToCome clonwin3'

# very important for shared files...ejw
umask 002

# default printer in counting house...ejw
setenv LPDEST clonhp
setenv PSPRINTER clonhp

# misc tcsh stuff...ejw
#set prompt="%m:%C> "
set prompt="%B%n@%m:%C>%b"
set rmstar
set history=1000
set savehist=1000
set autologout=0
unset autologout
set ignoreeof
set notify
set autolist


# for editor...ejw
setenv EDITOR 'emacs'
setenv VERSION_CONTROL t


#
# CERN and ROOT
#

#if ( ($OSTYPE == "SunOS") && ($MACHINE == "sun4u") ) then
#  setenv ROOTSYS /apps/root/2.24.05
#  setenv PATH ${ROOTSYS}/bin:${PATH}
#  setenv LD_LIBRARY_PATH ${ROOTSYS}/lib:${LD_LIBRARY_PATH}
#endif

if ($HOST == "clon01") then
  setenv ROOTSYS /apps/root/5.20.00
  setenv PATH ${ROOTSYS}/bin:${PATH}
  setenv LD_LIBRARY_PATH "${ROOTSYS}/lib:${LD_LIBRARY_PATH}:/home/clasrun/g14/cbrem/setup_tools"
endif

#
#
if ($HOST == "clasonl1") then
  setenv EXPID clasdev
  setenv SESSION claspcal
  setenv MSQL_TCP_HOST clasonl1
endif
#
if ($HOST == "wolfram") then
  setenv EXPID clasdev
  setenv SESSION claswolf
  setenv MSQL_TCP_HOST wolfram
endif
#
#

###echo "exiting .cshrc"



# nerses
###setenv MEDM_EXEC_LIST "Make Log Entry; $APP/general/scripts/make_elog_entry.pl &X &T"

