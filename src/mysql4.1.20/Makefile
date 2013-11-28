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

CLON_FLAGS += -I./src.s -I./src_full.s -I./src_short.s -DCODAMYSQL \
	-DSHAREDIR=\"$(CODA)/common/mysql\" \
	-DDEFAULT_CHARSET_HOME=\"$(CODA)/common/mysql\"

# brakes compilation: -I/usr/include/mysql


# compress/uncompress
ifneq ("$(OSTYPE)","VXWORKS")
ifneq ("$(OSTYPE)","ARM")
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
	-mkdir -p $(CODA)/ARM_i686/include/mysql
	cp tmp/* $(CODA)/ARM_i686/include/mysql
	rm -rf tmp
