#
# Makefile for rol/
#

#-----------------------------
# mandatory definitions: MAIN
#-----------------------------
MAIN = rol


#------------------
# generic Makefile
#------------------
include $(CODA)/src/Makefile.include



#-----------------------
# customize definitions
#-----------------------


CLON_FLAGS += -I./inc -I./code.s -I./sfi.s -I../codatt/codatt.s/ -I$(CODA)/common/include

ifeq ("$(OSTYPE)","Linux")
# remove -fwritable-strings (RHEL5 does not like it ???)
  CFLAGS = -g -fPIC -DLinux -DLINUX
  CLON_FLAGS += -I/usr/local/src/tirpci/tirpci_installation/include
  CLON_FLAGS += -I/usr/local/src/tirpci

#check if we are on VME controller; if so, define Linux_vme flag
  FILE=vme_ctl
  ifeq ($(findstring ${FILE},$(wildcard /dev/*)),)
	CFLAGS +=
  else
	CFLAGS += -DLinux_vme -I$(CODA)/src/rol/jvme
  endif

endif

ifeq ("$(OSTYPE)","VXWORKS")
  LIBNAMES +=
else
  LIBNAMES += $(globallib)/libmysql4.1.20.a

#fanuc only ??
  LIBNAMES += $(CODA)/src/rol/jvme/libjvme.so \


#LIBNAMES += $(CERNLIBS)
#LIBNAMES += $(F77LIBS)
  LIBNAMES += $(SYSLIBS)
endif


###############################################################################
###############################################################################

DEFINES = -DVXWORKS -DVXWORKSPPC -DCPU=PPC604 -D_GNU_TOOL 
INCS = -I$(WIND_BASE)/target/h -I./code.s/ -I./sfi.s/ -I../codatt/codatt.s/ -I$(CODA)/common/include/
VXCC = ccppc
VXCFLAGS = -O2 -fno-builtin -mstrict-align -ansi -mlongcall -fvolatile \
	-fvolatile-global -fstrength-reduce -fno-for-scope $(DEFINES) 


ROLOBJS =		./rol/VXWORKS_ppc/obj/vmenrol1.o \
		./rol/VXWORKS_ppc/obj/vmenrol1_standalone.o \
		./rol/VXWORKS_ppc/obj/svt1.o \
		./rol/VXWORKS_ppc/obj/svt1_standalone.o \
		./rol/VXWORKS_ppc/obj/dcrb1.o \
		./rol/VXWORKS_ppc/obj/dcrb1_standalone.o \
		./rol/VXWORKS_ppc/obj/fbrol1.o \
		./rol/VXWORKS_ppc/obj/fbrol1_standalone.o \
		./rol/VXWORKS_ppc/obj/rol2_tt.o \
		./rol/VXWORKS_ppc/obj/rol2_tt_testsetup.o

UROLOBJS =		./rol/$(OSTYPE_MACHINE)/obj/tid1.so \
		./rol/$(OSTYPE_MACHINE)/obj/tid1_master.so \
		./rol/$(OSTYPE_MACHINE)/obj/tid1_slave.so \
		./rol/$(OSTYPE_MACHINE)/obj/fadc1.so \
		./rol/$(OSTYPE_MACHINE)/obj/fadc1_master.so \
		./rol/$(OSTYPE_MACHINE)/obj/dcrb2.so \
		./rol/$(OSTYPE_MACHINE)/obj/tdc2.so \
		./rol/$(OSTYPE_MACHINE)/obj/tdc2_slave.so \
		./rol/$(OSTYPE_MACHINE)/obj/svt2.so \
		./rol/$(OSTYPE_MACHINE)/obj/urol2_tt.so \
		./rol/$(OSTYPE_MACHINE)/obj/urol2_tt_testsetup.so

###		./rol/$(OSTYPE_MACHINE)/obj/urol1.so \


HPSOBJS =		./rol/$(OSTYPE_MACHINE)/obj/hpstrack1.so

###############################################################################
###############################################################################





#-------------------
# customize targets
#-------------------

uincs:
	cp ./inc/*.h $(CODA)/common/include/

#
# must install librol.so for VXWORKS only !!!!!!!!!!
#
install: install_obj install_lib install_bin
#	@for i in $(CODA)/common/bootscripts $(CODA)/common/include ; \
#	do \
#	if [ ! -d $$i ] ; then \
#		echo "Making directory $$i"; \
#		mkdir $$i; \
#		chmod 755 $$i; \
#		else true; \
#	fi; \
#	done;

	rm -rf tmp
	mkdir tmp
	cp code.s/*.h tmp
	chmod 664 tmp/*
	cp tmp/* $(CODA)/common/include
	rm -rf tmp

	rm -rf tmp
	mkdir tmp
	cp ./inc/*.h tmp
	chmod 664 tmp/*
	cp tmp/* $(CODA)/common/include
	rm -rf tmp

	cp ./rol/$(OSTYPE_MACHINE)/obj/* $(CODA)/$(OSTYPE_MACHINE)/rol/

	rm -rf tmp
	mkdir tmp
	cp ./bootscripts/boot_* tmp
	cp ./bootscripts/*.boot tmp
	chmod 664 tmp/*
	cp tmp/* $(CODA)/VXWORKS_ppc/bootscripts/
	rm -rf tmp



#########################################################################################
#
# use: gmake rols VXWORKS
#
rols: $(ROLOBJS)

urols: $(UROLOBJS)

hps: $(HPSOBJS)

#
#unix rol
#

ifeq ("$(OSTYPE)","SunOS")
  UCC = $(CC) $(CFLAGS) $(CLON_FLAGS) -G
  UCCXX = $(CXX) $(CFLAGS) $(CLON_FLAGS) -G
endif

ifeq ("$(OSTYPE)","Linux")
  UCC = $(CC) $(CFLAGS) $(CLON_FLAGS) 
  UCCXX = g++ -shared -g $(CFLAGS) $(CLON_FLAGS)
  ULD   = $(CC) -shared -g
endif

#
#
#
rol/$(OSTYPE_MACHINE)/obj/uvmerol1.o: rol/uvmerol1.cc inc/VME_source.h
	$(UCCXX) -I/usr/src/JLab/PPCLinux/kernel/modules/altro_usb \
	-c -o rol/$(OSTYPE_MACHINE)/obj/uvmerol1.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/uvmerol1.cc
#
rol/$(OSTYPE_MACHINE)/obj/uvmerol1.so: rol/$(OSTYPE_MACHINE)/obj/uvmerol1.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/uvmerol1.so \
					rol/$(OSTYPE_MACHINE)/obj/uvmerol1.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/$(OSTYPE_MACHINE)/lib/libvme.so \
					$(CODA)/$(OSTYPE_MACHINE)/lib/libu2f.so \
					$(CODA)/$(OSTYPE_MACHINE)/lib/libaltro.so \
					$(CODA)/$(OSTYPE_MACHINE)/lib/libtools.so
#
#
#
rol/$(OSTYPE_MACHINE)/obj/uvmerol1_test.o: rol/uvmerol1.cc inc/VME_source.h
	$(UCCXX) -DTEST_SETUP -I/usr/src/JLab/PPCLinux/kernel/modules/altro_usb \
	-c -o rol/$(OSTYPE_MACHINE)/obj/uvmerol1_test.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/uvmerol1.cc
#
rol/$(OSTYPE_MACHINE)/obj/uvmerol1_test.so: rol/$(OSTYPE_MACHINE)/obj/uvmerol1_test.o
	$(UCCXX) -DTEST_SETUP -o rol/$(OSTYPE_MACHINE)/obj/uvmerol1_test.so \
					rol/$(OSTYPE_MACHINE)/obj/uvmerol1_test.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/$(OSTYPE_MACHINE)/lib/libvme.so \
					$(CODA)/$(OSTYPE_MACHINE)/lib/libu2f.so \
					$(CODA)/$(OSTYPE_MACHINE)/lib/libaltro.so \
					$(CODA)/$(OSTYPE_MACHINE)/lib/libtools.so
#
#
#
rol/$(OSTYPE_MACHINE)/obj/bonusrol1.o: rol/bonusrol1.cc
	$(UCCXX) -c -o rol/$(OSTYPE_MACHINE)/obj/bonusrol1.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/bonusrol1.cc
#
rol/$(OSTYPE_MACHINE)/obj/bonusrol1.so: rol/$(OSTYPE_MACHINE)/obj/bonusrol1.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/bonusrol1.so \
					rol/$(OSTYPE_MACHINE)/obj/bonusrol1.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/$(OSTYPE_MACHINE)/lib/libvme.so
#
#
#
rol/$(OSTYPE_MACHINE)/obj/urol1.o: rol/urol1.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/urol1.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/urol1.c
#
rol/$(OSTYPE_MACHINE)/obj/urol1.so: rol/$(OSTYPE_MACHINE)/obj/urol1.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/urol1.so \
					rol/$(OSTYPE_MACHINE)/obj/urol1.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o
#
#
#
rol/$(OSTYPE_MACHINE)/obj/fanuc1.o: rol/fanuc1.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/fanuc1.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/fanuc1.c
#
rol/$(OSTYPE_MACHINE)/obj/fanuc1.so: rol/$(OSTYPE_MACHINE)/obj/fanuc1.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/fanuc1.so \
					rol/$(OSTYPE_MACHINE)/obj/fanuc1.o \
					$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so \
					$(CODA)/src/rol/tirLib/libtir.so \
					$(CODA)/src/rol/jvme/libjvme.so \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttp.o
#
#
#
rol/$(OSTYPE_MACHINE)/obj/tirtid1.o: rol/tirtid1.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/tirtid1.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/tirtid1.c
#
rol/$(OSTYPE_MACHINE)/obj/tirtid1.so: rol/$(OSTYPE_MACHINE)/obj/tirtid1.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/tirtid1.so \
					rol/$(OSTYPE_MACHINE)/obj/tirtid1.o \
					$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so \
					$(CODA)/src/rol/tirLib/libtir.so \
					$(CODA)/src/rol/jvme/libjvme.so \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttp.o
#
#
#
rol/$(OSTYPE_MACHINE)/obj/dcrb2.o: rol/dcrb2.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/dcrb2.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/dcrb2.c
#
rol/$(OSTYPE_MACHINE)/obj/dcrb2.so: rol/$(OSTYPE_MACHINE)/obj/dcrb2.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/dcrb2.so \
					rol/$(OSTYPE_MACHINE)/obj/dcrb2.o \
					$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so \
					$(CODA)/src/rol/jvme/libjvme.so \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttp.o
#
#
#
rol/$(OSTYPE_MACHINE)/obj/tdc2.o: rol/tdc2.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/tdc2.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/tdc2.c
#
rol/$(OSTYPE_MACHINE)/obj/tdc2.so: rol/$(OSTYPE_MACHINE)/obj/tdc2.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/tdc2.so \
					rol/$(OSTYPE_MACHINE)/obj/tdc2.o \
					$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so \
					$(CODA)/src/rol/jvme/libjvme.so \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttp.o
#
#
#
rol/$(OSTYPE_MACHINE)/obj/tdc2_slave.o: rol/tdc2.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/tdc2_slave.o -DNO_X11 -DWITH_ET -DTI_SLAVE \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/tdc2.c
#
rol/$(OSTYPE_MACHINE)/obj/tdc2_slave.so: rol/$(OSTYPE_MACHINE)/obj/tdc2_slave.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/tdc2_slave.so \
					rol/$(OSTYPE_MACHINE)/obj/tdc2_slave.o \
					$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so \
					$(CODA)/src/rol/jvme/libjvme.so \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttp.o
#
#
#
rol/$(OSTYPE_MACHINE)/obj/svt2.o: rol/svt2.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/svt2.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/svt2.c
#
rol/$(OSTYPE_MACHINE)/obj/svt2.so: rol/$(OSTYPE_MACHINE)/obj/svt2.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/svt2.so \
					rol/$(OSTYPE_MACHINE)/obj/svt2.o \
					$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so \
					$(CODA)/src/rol/jvme/libjvme.so \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttp.o
#
#
#
rol/$(OSTYPE_MACHINE)/obj/fadc1.o: rol/fadc1.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/fadc1.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/fadc1.c
#
rol/$(OSTYPE_MACHINE)/obj/fadc1.so: rol/$(OSTYPE_MACHINE)/obj/fadc1.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/fadc1.so \
					rol/$(OSTYPE_MACHINE)/obj/fadc1.o \
					$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so \
					$(CODA)/src/rol/jvme/libjvme.so \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttp.o
#
#
#
rol/$(OSTYPE_MACHINE)/obj/fadc1_master.o: rol/fadc1.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/fadc1_master.o -DNO_X11 -DWITH_ET -DTI_MASTER \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/fadc1.c
#
rol/$(OSTYPE_MACHINE)/obj/fadc1_master.so: rol/$(OSTYPE_MACHINE)/obj/fadc1_master.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/fadc1_master.so \
					rol/$(OSTYPE_MACHINE)/obj/fadc1_master.o \
					$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so \
					$(CODA)/src/rol/jvme/libjvme.so \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttp.o
#
#
#
rol/$(OSTYPE_MACHINE)/obj/tid1.o: rol/tid1.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/tid1.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/tid1.c
#
rol/$(OSTYPE_MACHINE)/obj/tid1.so: rol/$(OSTYPE_MACHINE)/obj/tid1.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/tid1.so \
					rol/$(OSTYPE_MACHINE)/obj/tid1.o \
					$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so \
					$(CODA)/src/rol/jvme/libjvme.so \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttp.o
#
#
#
rol/$(OSTYPE_MACHINE)/obj/tid1_master.o: rol/tid1.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/tid1_master.o -DNO_X11 -DWITH_ET -DTID_MASTER \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/tid1.c
#
rol/$(OSTYPE_MACHINE)/obj/tid1_master.so: rol/$(OSTYPE_MACHINE)/obj/tid1_master.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/tid1_master.so \
					rol/$(OSTYPE_MACHINE)/obj/tid1_master.o \
					$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so \
					$(CODA)/src/rol/jvme/libjvme.so \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttp.o
#
#
rol/$(OSTYPE_MACHINE)/obj/tid1_slave.o: rol/tid1.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/tid1_slave.o -DNO_X11 -DWITH_ET -DTI_SLAVE \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/tid1.c
#
rol/$(OSTYPE_MACHINE)/obj/tid1_slave.so: rol/$(OSTYPE_MACHINE)/obj/tid1_slave.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/tid1_slave.so \
					rol/$(OSTYPE_MACHINE)/obj/tid1_slave.o \
					$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so \
					$(CODA)/src/rol/jvme/libjvme.so \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttp.o
#
#
#
rol/$(OSTYPE_MACHINE)/obj/hpstrack1.o: rol/hpstrack1.c
	$(UCC) -m32 -c -o rol/$(OSTYPE_MACHINE)/obj/hpstrack1.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/hpstrack1.c
#
rol/$(OSTYPE_MACHINE)/obj/hpstrack1.so: rol/$(OSTYPE_MACHINE)/obj/hpstrack1.o
	$(UCCXX) -m32 -o rol/$(OSTYPE_MACHINE)/obj/hpstrack1.so \
					rol/$(OSTYPE_MACHINE)/obj/hpstrack1.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o
#
#
#
rol/$(OSTYPE_MACHINE)/obj/urol1_pci.o: rol/urol1_pci.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/urol1_pci.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/urol1_pci.c
#
rol/$(OSTYPE_MACHINE)/obj/urol1_pci.so: rol/$(OSTYPE_MACHINE)/obj/urol1_pci.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/urol1_pci.so \
					rol/$(OSTYPE_MACHINE)/obj/urol1_pci.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
		/usr/local/src/tirpci/linux/tirpci_lib.o \
		/usr/local/src/tirpci/tirpci_installation/lib/libwdapi921.so \
		$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so
#
#
#
rol/$(OSTYPE_MACHINE)/obj/urol1_pci_testsetup.o: rol/urol1_pci.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/urol1_pci_testsetup.o -DNO_X11 -DWITH_ET -DTEST_SETUP \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/urol1_pci.c
#
rol/$(OSTYPE_MACHINE)/obj/urol1_pci_testsetup.so: rol/$(OSTYPE_MACHINE)/obj/urol1_pci_testsetup.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/urol1_pci_testsetup.so \
					rol/$(OSTYPE_MACHINE)/obj/urol1_pci_testsetup.o \
					$(CODA)/src/codatt/$(OSTYPE_MACHINE)/obj/ttbosio_.o \
		/usr/local/src/tirpci/linux/tirpci_lib.o \
		/usr/local/src/tirpci/tirpci_installation/lib/libwdapi921.so \
		$(CODA)/src/rol/$(OSTYPE_MACHINE)/lib/librol.so \
		$(CODA)/src/codatt/$(OSTYPE_MACHINE)/lib/libcodatt.so
#
#
#
rol/$(OSTYPE_MACHINE)/obj/urol2_tt.o: rol/urol2_tt.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/urol2_tt.o -DNO_X11 -DWITH_ET \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/urol2_tt.c
#
rol/$(OSTYPE_MACHINE)/obj/urol2_tt.so: rol/$(OSTYPE_MACHINE)/obj/urol2_tt.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/urol2_tt.so \
					rol/$(OSTYPE_MACHINE)/obj/urol2_tt.o \
					$(CODA)/src/codatt/Linux_i686/lib/libcodatt.a \
					$(CODA)/src/bosio/Linux_i686/lib/libbosio.a
#
#
#
rol/$(OSTYPE_MACHINE)/obj/urol2_tt_testsetup.o: rol/urol2_tt.c
	$(UCC) -c -o rol/$(OSTYPE_MACHINE)/obj/urol2_tt_testsetup.o -DNO_X11 -DWITH_ET -DTEST_SETUP \
	-DCLON_PARMS=\"/usr/local/clas/parms\" rol/urol2_tt.c
#
rol/$(OSTYPE_MACHINE)/obj/urol2_tt_testsetup.so: rol/$(OSTYPE_MACHINE)/obj/urol2_tt_testsetup.o
	$(UCCXX) -o rol/$(OSTYPE_MACHINE)/obj/urol2_tt_testsetup.so \
					rol/$(OSTYPE_MACHINE)/obj/urol2_tt_testsetup.o \
					$(CODA)/src/codatt/Linux_i686/lib/libcodatt.a \
					$(CODA)/src/bosio/Linux_i686/lib/libbosio.a
#
#
#
#
#
# VXWORKS ROLS
#

rol/VXWORKS_ppc/obj/rol1.o: rol/rol1.c
	$(VXCC) -c $(VXCFLAGS) rol/rol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/rol1.o

rol/VXWORKS_ppc/obj/rol1_vme.o: rol/rol1_vme.c
	$(VXCC) -c $(VXCFLAGS) rol/rol1_vme.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/rol1_vme.o -DROL1_VME

rol/VXWORKS_ppc/obj/fbrol1.o: rol/fbrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/fbrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/fbrol1.o -DFBROL1

rol/VXWORKS_ppc/obj/fbrol1_profile.o: rol/fbrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/fbrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/fbrol1_profile.o -DPROFILE

rol/VXWORKS_ppc/obj/fbrol1_block.o: rol/fbrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/fbrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/fbrol1_block.o -DBLOCK_READ

rol/VXWORKS_ppc/obj/fbrol1_single.o: rol/fbrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/fbrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/fbrol1_single.o -DSINGLE_READ

rol/VXWORKS_ppc/obj/fbrol1_calib.o: rol/fbrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/fbrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/fbrol1_calib.o -DTDC_CALIB

rol/VXWORKS_ppc/obj/fbrol1_laser.o: rol/fbrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/fbrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/fbrol1_laser.o -DTOF_LASER

rol/VXWORKS_ppc/obj/fbrol1_standalone.o: rol/fbrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/fbrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/fbrol1_standalone.o -DSTANDALONE

rol/VXWORKS_ppc/obj/fbvmerol1_standalone.o: rol/fbrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/fbrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/fbvmerol1_standalone.o -DSTANDALONE -DWITHVME

rol/VXWORKS_ppc/obj/vmerol1.o: rol/vmerol1.c
	$(VXCC) -c $(VXCFLAGS) rol/vmerol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/vmerol1.o -DVMEROL1

rol/VXWORKS_ppc/obj/vmenrol1.o: rol/vmenrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/vmenrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/vmenrol1.o -DVMENROL1

rol/VXWORKS_ppc/obj/vmenrol1_standalone.o: rol/vmenrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/vmenrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/vmenrol1_standalone.o -DVMENROL1 -DSTANDALONE

rol/VXWORKS_ppc/obj/dcrb1.o: rol/dcrb1.c
	$(VXCC) -c $(VXCFLAGS) rol/dcrb1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/dcrb1.o -DDCRB1

rol/VXWORKS_ppc/obj/dcrb2.o: rol/dcrb2.c
	$(VXCC) -c $(VXCFLAGS) rol/dcrb2.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/dcrb2.o -DDCRB2

rol/VXWORKS_ppc/obj/dcrb1_standalone.o: rol/dcrb1.c
	$(VXCC) -c $(VXCFLAGS) rol/dcrb1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/dcrb1_standalone.o -DDCRB1 -DSTANDALONE

rol/VXWORKS_ppc/obj/svt1.o: rol/svt1.c
	$(VXCC) -c $(VXCFLAGS) rol/svt1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/svt1.o -DSVT1

rol/VXWORKS_ppc/obj/svt1_standalone.o: rol/svt1.c
	$(VXCC) -c $(VXCFLAGS) rol/svt1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/svt1_standalone.o -DSVT1 -DSTANDALONE

rol/VXWORKS_ppc/obj/fadc1.o: rol/fadc1.c
	$(VXCC) -c $(VXCFLAGS) rol/fadc1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/fadc1.o -DFADC1

rol/VXWORKS_ppc/obj/fadc1_standalone.o: rol/fadc1.c
	$(VXCC) -c $(VXCFLAGS) rol/fadc1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/fadc1_standalone.o -DFADC1 -DSTANDALONE

rol/VXWORKS_ppc/obj/vmearol1.o: rol/vmearol1.c
	$(VXCC) -c $(VXCFLAGS) rol/vmearol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/vmearol1.o -DVMEAROL1

rol/VXWORKS_ppc/obj/ms1.o: rol/ms1.c
	$(VXCC) -c $(VXCFLAGS) rol/ms1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/ms1.o -DMS1

rol/VXWORKS_ppc/obj/fadcrol1.o: rol/fadcrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/fadcrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/fadcrol1.o -DFADCROL1

rol/VXWORKS_ppc/obj/wolfrol1.o: rol/wolfrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/wolfrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/wolfrol1.o -DWOLFROL1

rol/VXWORKS_ppc/obj/ctoftest1.o: rol/ctoftest1.c
	$(VXCC) -c $(VXCFLAGS) rol/ctoftest1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/ctoftest1.o -DCTOFTEST1

rol/VXWORKS_ppc/obj/scaler1.o: rol/scalrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/scalrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/scaler1.o -DSCALER1

rol/VXWORKS_ppc/obj/scaler2.o: rol/scalrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/scalrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/scaler2.o -DSCALER2

rol/VXWORKS_ppc/obj/scaler3.o: rol/scalrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/scalrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/scaler3.o -DSCALER3

rol/VXWORKS_ppc/obj/scaler4.o: rol/scalrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/scalrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/scaler4.o -DSCALER4

rol/VXWORKS_ppc/obj/moller.o: rol/scalrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/scalrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/moller.o -DPOLAR1

rol/VXWORKS_ppc/obj/f1rol1.o: rol/f1rol1.c
	$(VXCC) -c $(VXCFLAGS) rol/f1rol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/f1rol1.o -DF1ROL1

rol/VXWORKS_ppc/obj/caenrol1.o: rol/caenrol1.c
	$(VXCC) -c $(VXCFLAGS) rol/caenrol1.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/caenrol1.o -DCAENROL1

rol/VXWORKS_ppc/obj/rol2_tt.o: rol/rol2_tt.c
	$(VXCC) -c $(VXCFLAGS) rol/rol2_tt.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/rol2_tt.o

rol/VXWORKS_ppc/obj/rol2_tt_testsetup.o: rol/rol2_tt.c
	$(VXCC) -c $(VXCFLAGS) rol/rol2_tt.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/rol2_tt_testsetup.o -DTEST_SETUP

rol/VXWORKS_ppc/obj/rol2_tt_primex.o: rol/rol2_tt.c
	$(VXCC) -c $(VXCFLAGS) rol/rol2_tt.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/rol2_tt_primex.o -DPRIMEX

rol/VXWORKS_ppc/obj/histrol2.o: rol/histrol2.c
	$(VXCC) -c $(VXCFLAGS) rol/histrol2.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/histrol2.o

rol/VXWORKS_ppc/obj/ts2.o: rol/ts2.c
	$(VXCC) -c $(VXCFLAGS) rol/ts2.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/ts2.o

rol/VXWORKS_ppc/obj/ts2_lockroc.o: rol/ts2.c
	$(VXCC) -c $(VXCFLAGS) rol/ts2.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/ts2_lockroc.o -DLOCK_ROC

rol/VXWORKS_ppc/obj/ts2_testsetup.o: rol/ts2.c
	$(VXCC) -c $(VXCFLAGS) rol/ts2.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/ts2_testsetup.o -DTEST_SETUP

rol/VXWORKS_ppc/obj/ts2_testsetup_lockroc.o: rol/ts2.c
	$(VXCC) -c $(VXCFLAGS) rol/ts2.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/ts2_testsetup_lockroc.o \
		-DTEST_SETUP -DLOCK_ROC

rol/VXWORKS_ppc/obj/ts2_primex.o: rol/ts2.c
	$(VXCC) -c $(VXCFLAGS) rol/ts2.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/ts2_primex.o -DPRIMEX

rol/VXWORKS_ppc/obj/ts2_primex_lockroc.o: rol/ts2.c
	$(VXCC) -c $(VXCFLAGS) rol/ts2.c -I./inc \
		$(INCS) -o rol/VXWORKS_ppc/obj/ts2_primex_lockroc.o \
		-DPRIMEX -DLOCK_ROC

#
# several *.s/ directories
#
$(localobj)/%.o: sfi.s/%.c
	$(VXCC) $(VXCFLAGS) $(CLON_FLAGS) -c $< -o $@

$(localobj)/%.o: code.s/%.c
	$(VXCC) $(VXCFLAGS) $(CLON_FLAGS) -c $< -o $@
#
#
#
























