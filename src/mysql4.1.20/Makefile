#
# Makefile for mysql4.1.20/
#

#-----------------------------
# mandatory definitions: MAIN
#-----------------------------
MAIN = mysql4.1.20


#------------------
# generic Makefile
#------------------
include $(CODA)/src/Makefile.include


#-----------------------
# customize definitions
#-----------------------

CLON_FLAGS += -I./src.s -DCODAMYSQL \
	-DSHAREDIR=\"$(CODA)/common/mysql\" \
	-DDEFAULT_CHARSET_HOME=\"$(CODA)/common/mysql\"

#CLON_FLAGS += -I./src.s -I./src_full.s -I./src_short.s -DCODAMYSQL \
#	-DSHAREDIR=\"$(CODA)/common/mysql\" \
#	-DDEFAULT_CHARSET_HOME=\"$(CODA)/common/mysql\"

# brakes compilation: -I/usr/include/mysql

ifeq ("$(OSTYPE_MACHINE)","Linux_armv7l")
CLON_FLAGS += -I$(CODA)/Linux_armv7l/include
endif

ifeq ("$(OSTYPE_MACHINE)","VXWORKS_ppc")
CLON_FLAGS += -I$(CODA)/VXWORKS_ppc/include
endif

# compress/uncompress
ifneq ("$(OSTYPE_MACHINE)","VXWORKS_ppc")
ifneq ("$(OSTYPE_MACHINE)","Linux_armv7l")
  LIBNAMES += -lz
endif
endif

LIBNAMES += $(SYSLIBS)


#-------------------
# customize targets
#-------------------
install: install_lib install_bin
	rm -rf tmp
	mkdir tmp
	cp coda.s/libdb.h tmp
	cp coda.s/libtcp.h tmp
	chmod 664 tmp/*
	-mkdir -p $(CODA)/common/include
	cp tmp/* $(CODA)/common/include
	rm -rf tmp
#
	mkdir tmp
	cp src.s/*.h tmp
	chmod 664 tmp/*
	-mkdir -p $(CODA)/VXWORKS_ppc/include/mysql
	cp tmp/* $(CODA)/VXWORKS_ppc/include/mysql
	rm -rf tmp
#
	mkdir tmp
	cp src.s/*.h tmp
	chmod 664 tmp/*
	-mkdir -p $(CODA)/Linux_armv7l/include/mysql
	cp tmp/* $(CODA)/Linux_armv7l/include/mysql
	rm -rf tmp
