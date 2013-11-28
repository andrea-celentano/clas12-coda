#
# Makefile for rc/runControl/Xui
#

#-----------------------------
# mandatory definitions: MAIN
#-----------------------------
MAIN = Xui


#------------------
# generic Makefile
#------------------
include $(CODA)/src/Makefile.include


#-----------------------
# customize definitions
#-----------------------

#for debugging
ifeq ("$(OSTYPE)","SunOS")
  CFLAGS = -g -KPIC -mt -DSunOS -DOSTYPE="SunOS"
endif

CLON_FLAGS += -I./include -I../Variables/src.s -I../Common/src.s -I../rcClient/src.s \
	-I./pixmaps -I$(CODA)/common/include -I../../cg/cg.s -I../../motif/include \
	-I../../sace3.1/include $(X11INCDIR)



LIBNAMES += ../Variables/$(OSTYPE_MACHINE)/lib/libVariables.a
LIBNAMES += ../rcClient/$(OSTYPE_MACHINE)/lib/librcClient.a
LIBNAMES += ../rcServer/$(OSTYPE_MACHINE)/lib/librcServer.a
LIBNAMES += ../Common/$(OSTYPE_MACHINE)/lib/libCommon.a


LIBNAMES += ../../sace3.1/$(OSTYPE_MACHINE)/lib/libsace3.1.a
LIBNAMES += ../../motif/$(OSTYPE_MACHINE)/lib/libmotif.a
LIBNAMES += ../../cg/$(OSTYPE_MACHINE)/lib/libcg.a

LIBNAMES += $(globallib)/libmysql4.1.20.a


CLON_FLAGS += -DUSE_CREG
LIBNAMES += $(globallib)/libcreg.a


LIBNAMES += $(globallib)/libcodaedit.a

LIBNAMES += $(X11LIBS)
LIBNAMES += $(SYSLIBS)

ifeq ("$(OSTYPE)","SunOS")
  CLON_FLAGS += -DSYSV -DSVR4 -D_CODA_2_0_T \
	-D_CODA_USE_THREADS -DDEBUG -Dsolaris  \
	-DHAVE_UNISTD_H=1 -DHAVE_LIMITS_H=1 -DHAVE_MACROS_H=1 -DSTDC_HEADERS=1 \
	-DTK_FILE_COUNT=_cnt -Dstrtod=fixstrtod -DNEED_MATHERR=1
endif

ifeq ("$(OSTYPE)","Linux")
#  CLON_FLAGS += -D_CODA_2_0_T -DHAVE_UNISTD_H=1 -DHAVE_LIMITS_H=1 -DHAVE_MACROS_H=1 -DSTDC_HEADERS=1 \
#	-DTK_FILE_COUNT=_cnt -Dstrtod=fixstrtod -DNEED_MATHERR=1
  CLON_FLAGS += -D_CODA_2_0_T -DHAVE_UNISTD_H=1 -DHAVE_LIMITS_H=1 -DHAVE_MACROS_H=1 -DSTDC_HEADERS=1 \
	-DTK_FILE_COUNT=_cnt -DNEED_MATHERR=1
endif

ifeq ("$(OSTYPE)","Darwin")
  CLON_FLAGS += -D_CODA_2_0_T -DHAVE_UNISTD_H=1 -DHAVE_LIMITS_H=1 -DHAVE_MACROS_H=1 -DSTDC_HEADERS=1 \
        -DTK_FILE_COUNT=_cnt -DNEED_MATHERR=1
endif

#-------------------
# customize targets
#-------------------
install:
	cp $(localbin)/runcontrol $(CODA)/$(OSTYPE_MACHINE)/bin/

#	@for i in $(CODA)/common/html $(CODA)/common/html/rc ; \
#	do \
#	if [ ! -d $$i ] ; then \
#		echo "Making directory $$i"; \
#		mkdir $$i; \
#		chmod 755 $$i; \
#		else true; \
#	fi; \
#	done;
#	rm -rf tmp
#	mkdir tmp
#	cp ./html/*.html tmp/
#	cp ./html/*.gif tmp/
#	chmod 664 tmp/*
#	cp tmp/* $(CODA)/common/html/rc/
#	rm -rf tmp
#




























