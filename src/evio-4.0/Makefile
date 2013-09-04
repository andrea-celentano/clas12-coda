#
# Makefile for evio directory - use scons !!!
#

#------------------
# generic Makefile
#------------------
###include $(CODA)/src/Makefile.include
OSTYPE-MACHINE = $(OSTYPE)-$(MACHINE)


#setenv INSTALL_DIR `pwd`
#setenv LD_LIBRARY_PATH `pwd`/Linux-i686/lib:$LD_LIBRARY_PATH

#setenv INSTALL_DIR `pwd`
#setenv LD_LIBRARY_PATH `pwd`/Linux-x86_64/lib:$LD_LIBRARY_PATH

#setenv INSTALL_DIR `pwd`
#setenv LD_LIBRARY_PATH `pwd`/SunOS-sun4u/lib:$LD_LIBRARY_PATH


all:
	/bin/csh -c "(setenv INSTALL_DIR `pwd`; scons)"

install:
	cp $(OSTYPE-MACHINE)/lib/*.so $(CODA)/$(OSTYPE_MACHINE)/lib/
	cp $(OSTYPE-MACHINE)/bin/* $(CODA)/$(OSTYPE_MACHINE)/bin/

clean:
	rm -f src/execsrc/.$(OSTYPE-MACHINE)/*
	rm -f src/test/.$(OSTYPE-MACHINE)/*
	rm -f src/libsrc/.$(OSTYPE-MACHINE)/*
	rm -f $(OSTYPE-MACHINE)/lib/*
	rm -f $(OSTYPE-MACHINE)/bin/*

distclean: clean
	rm -f $(CODA)/$(OSTYPE_MACHINE)/lib/libevio*
	rm -f $(CODA)/$(OSTYPE_MACHINE)/bin/evio*
