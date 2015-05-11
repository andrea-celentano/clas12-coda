#
# Makefile for diaggui/
#

#-----------------------------
# mandatory definitions: MAIN
#-----------------------------
MAIN = diaggui


#------------------
# generic Makefile
#------------------
include $(CODA)/src/Makefile.include
include $(CODA)/src/Makefile.arch

#-----------------------
# customize definitions
#-----------------------

ifeq ("$(OSTYPE_MACHINE)","SunOS_i86pc")
#  CLON_FLAGS += -m64
  SHLD += -m64
#  LDCXX += -m64
endif

CLON_FLAGS += -I./src.s \
				-I./inc/CTP \
				-I./inc/DCRB \
				-I./inc/DCRBScalers \
				-I./inc/DSC \
				-I./inc/FADC \
				-I./inc/FADCScalers_HPS \
				-I./inc/GTP \
				-I./inc/GTP_HPS \
				-I./inc/SD \
				-I./inc/SSP \
				-I./inc/SSP_HPS \
				-I./inc/TID \
				-I./inc/V1495Pulser \
				-I./inc/VSCM \
				-I./inc/vetroc \
				-I$(CODA)/common/include

ifneq ("$(OSTYPE)","VXWORKS")
#  LIBNAMES += $(CODA)/$(OSTYPE_MACHINE)/lib/librol.a
#  LIBNAMES += $(CODA)/$(OSTYPE_MACHINE)/lib/libmysql4.1.20.a
#  LIBNAMES += $(CODA)/src/rol/jvme/libjvme.so
  LIBNAMES += $(ROOTGLIBS)
##  LIBNAMES += $(SYSLIBS)
endif


#-------------------
# customize targets
#-------------------
install: install_lib install_bin
#	rm -rf tmp
#	mkdir tmp
###	cp src.s/tt.h tmp
#	chmod 664 tmp/*
#	-mkdir -p $(CODA)/common/include
#	cp tmp/* $(CODA)/common/include
#	rm -rf tmp






























