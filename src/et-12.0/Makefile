#
# Makefile for et/
#

#-----------------------------
# mandatory definitions: MAIN
#-----------------------------
MAIN = et


#------------------
# generic Makefile
#------------------
include $(CODA)/src/Makefile.include



#-----------------------
# customize definitions
#-----------------------

# use C++ as linker instead of CC: we'll link main *.c program with
# SmartSocket's C++ libraries, and using CC will not resolve references
#ifeq ("$(OSTYPE)","Linux")
  LDCC = $(CXX)
#endif

CLON_FLAGS += -I./$(MAIN).s -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS

# for smartsockets
CLON_FLAGS += -I$(RTHOME)/include -I$(CLON)/common/include

#gram heyes use this flag instead of Darwin ???
ifeq ("$(OSTYPE)","Darwin")
  CLON_FLAGS += -D__APPLE__
endif

#evio
LIBNAMES += $(CODA)/$(OSTYPE_MACHINE)/lib/libevio.so

#smartsockets
LIBNAMES += $(CLON_LIB)/libipc.a
LIBNAMES += -L$(RTHOME)/lib/$(OSTYPE_MACHINE) -lrtipc -lrtipcl -lrtutil -lrtcxxipc50

LIBNAMES += -lpthread
LIBNAMES += -lresolv
LIBNAMES += $(SYSLIBS)

ifeq ("$(OSTYPE)","Linux")
  LIBNAMES += -lpthread
# Sergey: -rdynamic needed by Linux, it allows to search for global
#   symbols in main program by dlopen/dlsym
  LDFLAGS += -rdynamic
endif

#-------------------
# customize targets
#-------------------

# user's event selection function in shared lib


ulib: user/$(OSTYPE_MACHINE)/lib/libet_user.so

user/$(OSTYPE_MACHINE)/lib/libet_user.so: user/$(OSTYPE_MACHINE)/obj/et_userfunction.o
	$(SHLD) -o user/$(OSTYPE_MACHINE)/lib/libet_user.so user/$(OSTYPE_MACHINE)/obj/et_userfunction.o
##	ld -G -o user/$(OSTYPE_MACHINE)/lib/libet_user.so user/$(OSTYPE_MACHINE)/obj/et_userfunction.o

user/$(OSTYPE_MACHINE)/obj/et_userfunction.o: user/et_userfunction.c
	$(CC) $(CFLAGS) $(CLON_FLAGS) -c $< -o $@


install: install_lib
	cp $(localbin)/et_start $(CODA)/$(OSTYPE_MACHINE)/bin/
	cp $(localbin)/et_monitor $(CODA)/$(OSTYPE_MACHINE)/bin/
	cp $(localbin)/et_2_et_10_00 $(CODA)/$(OSTYPE_MACHINE)/bin/
	cp $(localbin)/et_2_et_10_01 $(CODA)/$(OSTYPE_MACHINE)/bin/
	cp $(localbin)/et_2_et_10_02 $(CODA)/$(OSTYPE_MACHINE)/bin/
	cp $(localbin)/et_2_et $(CODA)/$(OSTYPE_MACHINE)/bin/

	rm -rf tmp
	mkdir tmp
	cp $(MAIN).s/et.h tmp
	cp $(MAIN).s/et_private.h tmp
	cp $(MAIN).s/etCommonNetwork.h tmp
	chmod 664 tmp/*
	-mkdir -p $(CODA)/common/include
	cp tmp/* $(CODA)/common/include
	rm -rf tmp

	cp user/$(OSTYPE_MACHINE)/lib/libet_user.so $(CODA)/$(OSTYPE_MACHINE)/lib/



distclean: clean
	rm -f $(CODA)/$(OSTYPE_MACHINE)/bin/et_*
	rm -f $(CODA)/$(OSTYPE_MACHINE)/lib/libet*

























