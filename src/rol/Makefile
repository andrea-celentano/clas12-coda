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


#------------------------
# special building cases
#------------------------

#ifeq ("$(OSTYPE_MACHINE)","Linux_i686_vme")

$(LIB_FILES_ROL): $(SOURCE_FILES_ROL) ./$(OSTYPE_MACHINE)/rol/hps1_master.so \
	./$(OSTYPE_MACHINE)/rol/hps1_slave.so \
	./$(OSTYPE_MACHINE)/rol/hps1test_master.so \
	./$(OSTYPE_MACHINE)/rol/hps1test_slave.so \
	./$(OSTYPE_MACHINE)/rol/hps1mvt_master.so \
	./$(OSTYPE_MACHINE)/rol/hps1mvt_slave.so \
	./$(OSTYPE_MACHINE)/rol/tdc1_master.so \
	./$(OSTYPE_MACHINE)/rol/tdc1_slave.so \
	./$(OSTYPE_MACHINE)/rol/svt1_master.so \
	./$(OSTYPE_MACHINE)/rol/svt1_slave.so \
	./$(OSTYPE_MACHINE)/rol/fadc1_master.so \
	./$(OSTYPE_MACHINE)/rol/fadc1_slave.so \
	./$(OSTYPE_MACHINE)/rol/fadc1_1182.so

#	./$(OSTYPE_MACHINE)/rol/urol2_tt_testsetup.so \
#	./$(OSTYPE_MACHINE)/rol/dcrb1_master.so \
#	./$(OSTYPE_MACHINE)/rol/dcrb1_slave.so \


./$(OSTYPE_MACHINE)/rol/hps1_master.o : rols/hps1.c
	$(MKROLDIR)
	$(CC) -DTI_MASTER $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/hps1_slave.o : rols/hps1.c
	$(MKROLDIR)
	$(CC) -DTI_SLAVE $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/hps1test_master.o : rols/hps1test.c
	$(MKROLDIR)
	$(CC) -DTI_MASTER $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/hps1test_slave.o : rols/hps1test.c
	$(MKROLDIR)
	$(CC) -DTI_SLAVE $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/hps1mvt_master.o : rols/hps1mvt.c
	$(MKROLDIR)
	$(CC) -DTI_MASTER $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/hps1mvt_slave.o : rols/hps1mvt.c
	$(MKROLDIR)
	$(CC) -DTI_SLAVE $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/tdc1_master.o : rols/tdc1.c
	$(MKROLDIR)
	$(CC) -DTI_MASTER $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/tdc1_slave.o : rols/tdc1.c
	$(MKROLDIR)
	$(CC) -DTI_SLAVE $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/svt1_master.o : rols/svt1.c
	$(MKROLDIR)
	$(CC) -DTI_MASTER $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/svt1_slave.o : rols/svt1.c
	$(MKROLDIR)
	$(CC) -DTI_SLAVE $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/fadc1_master.o : rols/fadc1.c
	$(MKROLDIR)
	$(CC) -DTI_MASTER $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/fadc1_slave.o : rols/fadc1.c
	$(MKROLDIR)
	$(CC) -DTI_SLAVE $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/fadc1_1182.o : rols/fadc1.c
	$(MKROLDIR)
	$(CC) -DUSE_ADC1182 $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/dcrb1_master.o : rols/dcrb1.c
	$(MKROLDIR)
	$(CC) -DTI_MASTER $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/dcrb1_slave.o : rols/dcrb1.c
	$(MKROLDIR)
	$(CC) -DTI_SLAVE $(CCROLOPT)

#./$(OSTYPE_MACHINE)/rol/urol2_tt_testsetup.o : rols/urol2_tt.c
#	$(MKROLDIR)
#	$(CC) -DTEST_SETUP $(CCROLOPT)

#endif


#-----------------------
# customize definitions
#-----------------------

#LDCC = $(FC)
#LDFC = $(FC)
LDCC = $(LDCXX)
LDFC = $(LDCXX)

######CLON_FLAGS += -I./inc -I./code.s -I./mvt.s -I../codatt/codatt.s/ -I$(CODA)/common/include
CLON_FLAGS += -I./inc -I./code.s -I./mvt.s -I$(CODA)/common/include


ifeq ("$(OSTYPE)","Linux")
  CLON_FLAGS += -I/usr/local/src/tirpci/tirpci_installation/include
  CLON_FLAGS += -I/usr/local/src/tirpci
endif

ifeq ("$(OSTYPE)","VXWORKS")
  CLON_FLAGS += -I$(CODA)/VXWORKS_ppc/include
  LIBNAMES +=
else ifeq ("$(OSTYPE_MACHINE)","Linux_armv7l")
  CLON_FLAGS += -I$(CODA)/Linux_armv7l/include
  LIBNAMES += -lrt
else ifeq ("$(OSTYPE_MACHINE)","Linux_nios2")
  CLON_FLAGS += -I$(CODA)/Linux_nios2/include
  LIBNAMES +=
else
##########  LIBNAMES += $(CODA)/src/codatt/$(OSTYPE_MACHINE)/lib/libcodatt.a
##########  LIBNAMES += $(CODA)/src/bosio/$(OSTYPE_MACHINE)/lib/libbosio.a
  LIBNAMES += $(CODA)/src/mysql4.1.20/$(OSTYPE_MACHINE)/lib/libmysql4.1.20.a

  LIBNAMES += -lpthread

ifeq ("$(OSTYPE_MACHINE)","Linux_i686_vme")

  LIBNAMES += $(CODA)/src/rol/jvme/libjvme.so

## smartsocket stuff
#  CLON_FLAGS +=	-I$(CLON)/common/include
#  CLON_FLAGS += -I$(RTHOME)/include
#  LIBNAMES += $(CLON)/Linux_i686/lib/libepicsutil.a
#  LIBNAMES += $(CLON)/Linux_i686/lib/libipc.a
#  LIBNAMES += -L$(RTHOME)/lib/$(OSTYPE_MACHINE) -lrtipc -lrtipcl -lrtutil -lrtcxxipc50
## smartsocket stuff

endif

  LIBNAMES += $(SYSLIBS)
endif


#-------------------
# customize targets
#-------------------

jvmes:
	cd jvme; make clean; make

#
# must install librol.so for VXWORKS only !!!!!!!!!!
#install_rol

install: install_obj install_lib install_bin install_rol
	rm -rf tmp
	mkdir tmp
	cp code.s/*.h tmp
	chmod 664 tmp/*
	-mkdir -p $(CODA)/common/include
	cp tmp/* $(CODA)/common/include
	rm -rf tmp

	rm -rf tmp
	mkdir tmp
	cp ./inc/*.h tmp
	chmod 664 tmp/*
	-mkdir -p $(CODA)/common/include
	cp tmp/* $(CODA)/common/include
	rm -rf tmp

	rm -rf tmp
	mkdir tmp
	cp ./bootscripts/boot_* tmp
	cp ./bootscripts/*.boot tmp
	chmod 664 tmp/*
	-mkdir -p $(CODA)/VXWORKS_ppc/bootscripts/
	cp tmp/* $(CODA)/VXWORKS_ppc/bootscripts/
	rm -rf tmp
