#
# Makefile for coda/src/ directory
# 

MAKE = gmake

all:
	cd scripts;                          $(MAKE) install
	- cd rcscripts;                      $(MAKE) install
	- cd evio-4.3;                       $(MAKE)
	- cd evio-4.3;                       $(MAKE) install
	- cd et-14.0;                        $(MAKE)
	- cd et-14.0;                        $(MAKE) ulib
	- cd et-14.0;                        $(MAKE) install
	- cd mysql4.1.20;                    $(MAKE)
	- cd mysql4.1.20;                    $(MAKE) install
	- cd creg;                           $(MAKE)
	- cd creg;                           $(MAKE) install
	- cd codatcl1.0;                     $(MAKE)
	- cd codatcl1.0;                     $(MAKE) install
	- cd codaedit;                       $(MAKE)
	- cd codaedit;                       $(MAKE) install
	- cd cterm;                          $(MAKE)
	- cd cterm;                          $(MAKE) install
	- cd ipc;                            $(MAKE)
	- cd ipc;                            $(MAKE) install
#	- cd rol;                            $(MAKE) jvmes
	- cd rol/jvme;                       $(MAKE)
	- cd dac;                            $(MAKE)
	- cd dac;                            $(MAKE) install
	- cd rol;                            $(MAKE)
	- cd rol;                            $(MAKE) install
	- cd rol;                            $(MAKE) rols
	- cd rol;                            $(MAKE) install
	- cd rc;                             $(MAKE)
	- cd rc;                             $(MAKE) install
	- cd diaggui;                        $(MAKE)
	- cd diaggui;                        $(MAKE) install

clean:
	cd scripts;                          $(MAKE) clean
	- cd rcscripts;                      $(MAKE) clean
	- cd evio-4.3;                       $(MAKE) clean
	- cd et-14.0;                        $(MAKE) clean
	- cd mysql4.1.20;                    $(MAKE) clean
	- cd codatcl1.0;                     $(MAKE) clean
	- cd creg;                           $(MAKE) clean
	- cd codaedit;                       $(MAKE) clean
	- cd codaterm;                       $(MAKE) clean
	- cd dac;                            $(MAKE) clean
	- cd rol;                            $(MAKE) clean
	- cd rol/jvme;                       $(MAKE) clean
	- cd rc;                             $(MAKE) clean
	- cd diaggui;                        $(MAKE) clean


distclean:
	rm -rf $(CODA)/common/*
	rm -rf $(CODA)/$(OSTYPE_MACHINE)/*

vxworks:
	cd mysql4.1.20;                      $(MAKE) all     VXWORKS
	- cd mysql4.1.20;                    $(MAKE) install VXWORKS
#	- cd codatt;                         $(MAKE) all     VXWORKS
#	- cd codatt;                         $(MAKE) install VXWORKS
	- cd dac;                            $(MAKE) all     VXWORKS
	- cd dac;                            $(MAKE) install VXWORKS
	- cd rol;                            $(MAKE) all     VXWORKS
	- cd rol;                            $(MAKE) rols    VXWORKS
	- cd rol;                            $(MAKE) install VXWORKS

cleanvxworks:
	cd mysql4.1.20;                      $(MAKE) clean   VXWORKS
#	- cd codatt;                         $(MAKE) clean   VXWORKS
	- cd dac;                            $(MAKE) clean   VXWORKS
	- cd rol;                            $(MAKE) clean   VXWORKS

#with cross-compiling
#arm:
#	cd mysql4.1.20;                      make all     ARM
#	- cd mysql4.1.20;                    make install ARM
#	- cd codatt;                         make all     ARM
#	- cd codatt;                         make install ARM
#	- cd dac;                            make all     ARM
#	- cd dac;                            make install ARM
#	- cd rol;                            make all     ARM
#	- cd rol;                            make rols    ARM
#	- cd rol;                            make install ARM
#
#cleanarm:
#	cd mysql4.1.20;                      make clean   ARM
#	- cd codatt;                         make clean   ARM
#	- cd dac;                            make clean   ARM
#	- cd rol;                            make clean   ARM

arm:
	cd mysql4.1.20;                      make
	- cd mysql4.1.20;                    make install 
	- cd evio-4.3;                       make
	- cd evio-4.3;                       make install
	- cd et-14.0;                        make
	- cd et-14.0;                        make ulib
	- cd et-14.0;                        make install
#	- cd codatt;                         make lib
#	- cd codatt;                         make shlib
#	- cd codatt;                         make install 
	- cd dac;                            make     
	- cd dac;                            make install 
	- cd rol;                            make     
	- cd rol;                            make install 
	- cd rol;                            make rols    
	- cd rol;                            make install 

cleanarm:
	cd mysql4.1.20;                      make clean   
	- cd evio-4.3;                       make clean
	- cd et-14.0;                        make clean
#	- cd codatt;                         make clean   
	- cd dac;                            make clean   
	- cd rol;                            make clean   

nios:
	cd mysql4.1.20;                      $(MAKE) all     NIOS
	- cd mysql4.1.20;                    $(MAKE) install NIOS
	- cd evio-4.3;                       $(MAKE) lib     NIOS
	- cd evio-4.3;                       $(MAKE) shlib   NIOS
	- cd evio-4.3;                       $(MAKE) install NIOS
#	- cd codatt;                         $(MAKE) all     NIOS
#	- cd codatt;                         $(MAKE) install NIOS
#	- cd dac;                            $(MAKE) all     NIOS
	- cd dac;                            $(MAKE) Linux_nios2/bin/coda_roc NIOS
	- cd dac;                            $(MAKE) shlib   NIOS
	- cd dac;                            $(MAKE) install NIOS
	- cd rol;                            $(MAKE) lib     NIOS
	- cd rol;                            $(MAKE) shlib   NIOS
	- cd rol;                            $(MAKE) install NIOS
	- cd rol;                            $(MAKE) rols    NIOS
	- cd rol;                            $(MAKE) install NIOS

cleannios:
	cd mysql4.1.20;                      $(MAKE) clean   NIOS
	- cd evio-4.3;                       $(MAKE) clean   NIOS
#	- cd codatt;                         $(MAKE) clean   NIOS
	- cd dac;                            $(MAKE) clean   NIOS
	- cd rol;                            $(MAKE) clean   NIOS


###########################################



