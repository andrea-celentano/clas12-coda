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

$(LIB_FILES_ROL): $(SOURCE_FILES_ROL) ./$(OSTYPE_MACHINE)/rol/tdc2_master.so \
	./$(OSTYPE_MACHINE)/rol/tdc2_slave.so \
	./$(OSTYPE_MACHINE)/rol/svt2_master.so \
	./$(OSTYPE_MACHINE)/rol/svt2_slave.so \
	./$(OSTYPE_MACHINE)/rol/fadc1_master.so \
	./$(OSTYPE_MACHINE)/rol/fadc1_slave.so \
	./$(OSTYPE_MACHINE)/rol/dcrb2_master.so \
	./$(OSTYPE_MACHINE)/rol/dcrb2_slave.so \
	./$(OSTYPE_MACHINE)/rol/urol2_tt_testsetup.so


./$(OSTYPE_MACHINE)/rol/tdc2_master.o : rols/tdc2.c
	$(MKROLDIR)
	$(CC) -DTI_MASTER $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/tdc2_slave.o : rols/tdc2.c
	$(MKROLDIR)
	$(CC) -DTI_SLAVE $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/svt2_master.o : rols/svt2.c
	$(MKROLDIR)
	$(CC) -DTI_MASTER $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/svt2_slave.o : rols/svt2.c
	$(MKROLDIR)
	$(CC) -DTI_SLAVE $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/fadc1_master.o : rols/fadc1.c
	$(MKROLDIR)
	$(CC) -DTI_MASTER $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/fadc1_slave.o : rols/fadc1.c
	$(MKROLDIR)
	$(CC) -DTI_SLAVE $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/dcrb2_master.o : rols/dcrb2.c
	$(MKROLDIR)
	$(CC) -DTI_MASTER $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/dcrb2_slave.o : rols/dcrb2.c
	$(MKROLDIR)
	$(CC) -DTI_SLAVE $(CCROLOPT)

./$(OSTYPE_MACHINE)/rol/urol2_tt_testsetup.o : rols/urol2_tt.c
	$(MKROLDIR)
	$(CC) -DTEST_SETUP $(CCROLOPT)

#endif


#-----------------------
# customize definitions
#-----------------------

CLON_FLAGS += -I./inc -I./code.s -I../codatt/codatt.s/ -I$(CODA)/common/include

ifeq ("$(OSTYPE)","Linux")
  CLON_FLAGS += -I/usr/local/src/tirpci/tirpci_installation/include
  CLON_FLAGS += -I/usr/local/src/tirpci
endif

ifeq ("$(OSTYPE)","VXWORKS")
  CLON_FLAGS += -I$(CODA)/VXWORKS_ppc/include
  LIBNAMES +=
else ifeq ("$(OSTYPE)","ARM")
  LIBNAMES +=
else
  LIBNAMES += $(CODA)/src/codatt/$(OSTYPE_MACHINE)/lib/libcodatt.a
  LIBNAMES += $(CODA)/src/bosio/$(OSTYPE_MACHINE)/lib/libbosio.a
  LIBNAMES += $(CODA)/src/mysql4.1.20/$(OSTYPE_MACHINE)/lib/libmysql4.1.20.a

ifeq ("$(OSTYPE)","Linux")
  LIBNAMES += $(CODA)/src/rol/jvme/libjvme.so
endif

  LIBNAMES += $(SYSLIBS)
endif


#-------------------
# customize targets
#-------------------

jvmes:
	- cd jvme;	make all

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
