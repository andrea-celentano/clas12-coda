#
# Makefile for coda/ipc/
#

#-----------------------------
# mandatory definitions: MAIN
#-----------------------------
MAIN = ipc


#------------------
# generic Makefile
#------------------
include $(CODA)/src/Makefile.include



#-----------------------
# customize definitions
#-----------------------

#RCDB
CLON_FLAGS += -std=c++11 -DRCDB_MYSQL -I/usr/include/mysql -I$(RCDB_HOME)/cpp/include

CLON_FLAGS += -I./$(MAIN).s -I$(CODA)/common/include -I$(CODA)/common/include -I$(IPC_HOME)/src/main

ifeq ("$(OSTYPE_MACHINE)","Linux_i686")
  CLON_FLAGS += -I/usr/local/include/apr-1
else ifeq ("$(OSTYPE_MACHINE)","Linux_i686_vme")
  CLON_FLAGS += -I/usr/local/include/apr-1
else
  CLON_FLAGS += -I/usr/include/apr-1
endif

LIBNAMES += $(CODA)/$(OSTYPE_MACHINE)/lib/libmysql4.1.20.a
LIBNAMES += $(CODA)/$(OSTYPE_MACHINE)/lib/libet.a
LIBNAMES += $(CODA)/$(OSTYPE_MACHINE)/lib/libevio.a

ifeq ("$(OSTYPE_MACHINE)","Linux_i686")
  LIBNAMES += /usr/local/lib/libactivemq-cpp.so
  LIBNAMES += /usr/local/lib/libapr-1.so
  LIBNAMES += -luuid -lrt -lcrypt -lssl -lcrypto -ldl -lz
  LIBNAMES += -L/usr/kerberos/lib -lpthread -pthread -Wl,-rpath -Wl,/usr/local/lib
else
  LIBNAMES += /usr/local/lib/libactivemq-cpp.so
  LIBNAMES += -lapr-1
  LIBNAMES += -lrt -lcrypt -lssl -lcrypto -ldl -lz
  LIBNAMES += -L/usr/kerberos/lib -lpthread -pthread -Wl,-rpath -Wl,/usr/local/lib
endif


#for 'inet_aton'
LIBNAMES += -lresolv

ifeq ("$(OSTYPE)","SunOS")
  LIBNAMES += -lCrun
endif

ifeq ("$(OSTYPE)","Linux")
  LIBNAMES += -lpthread
endif


LIBNAMES += $(X11LIBS)
LIBNAMES += $(SYSLIBS)



#-------------------
# customize targets
#-------------------
install: install_lib install_bin
	rm -rf tmp
	mkdir tmp
	cp $(MAIN).s/*.h tmp
	chmod 664 tmp/*
	-mkdir -p $(CODA)/common/include
	cp tmp/* $(CODA)/common/include
	rm -rf tmp





















