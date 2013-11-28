#
# Makefile for coda/src/ directory
# 

MAKE = gmake

all:
	cd scripts;                          $(MAKE) install
	- cd rcscripts;                      $(MAKE) install
	- cd evio-4.0;                       $(MAKE)
	- cd evio-4.0;                       $(MAKE) install
	- cd et-12.0;                        $(MAKE)
	- cd et-12.0;                        $(MAKE) install
	- cd bosio;                          $(MAKE)
	- cd bosio;                          $(MAKE) install
	- cd mysql4.1.20;                    $(MAKE)
	- cd mysql4.1.20;                    $(MAKE) install
	- cd codatcl1.0;                     $(MAKE)
	- cd codatcl1.0;                     $(MAKE) install
	- cd creg;                           $(MAKE)
	- cd creg;                           $(MAKE) install
	- cd codaedit;                       $(MAKE)
	- cd codaedit;                       $(MAKE) install
	- cd codatt;                         $(MAKE)
	- cd codatt;                         $(MAKE) install
	- cd dac;                            $(MAKE)
	- cd dac;                            $(MAKE) install
	- cd rol;                            $(MAKE)
	- cd rol;                            $(MAKE) jvmes
	- cd rol;                            $(MAKE) rols
	- cd rol;                            $(MAKE) install
	- cd rc;                             $(MAKE)
	- cd rc;                             $(MAKE) install

clean:
	cd scripts;                          $(MAKE) clean
	- cd rcscripts;                      $(MAKE) clean
	- cd evio-4.0;                       $(MAKE) clean
	- cd et-12.0;                        $(MAKE) clean
	- cd bosio;                          $(MAKE) clean
	- cd mysql4.1.20;                    $(MAKE) clean
	- cd codatcl1.0;                     $(MAKE) clean
	- cd creg;                           $(MAKE) clean
	- cd codaedit;                       $(MAKE) clean
	- cd codatt;                         $(MAKE) clean
	- cd dac;                            $(MAKE) clean
	- cd rol;                            $(MAKE) clean
	- cd rc;                             $(MAKE) clean


distclean:
	rm -rf $(CODA)/common/*
	rm -rf $(CODA)/$(OSTYPE_MACHINE)/*

vxworks:
	cd mysql4.1.20;                      $(MAKE) all     VXWORKS
	- cd mysql4.1.20;                    $(MAKE) install VXWORKS
	- cd codatt;                         $(MAKE) all     VXWORKS
	- cd codatt;                         $(MAKE) install VXWORKS
	- cd dac;                            $(MAKE) all     VXWORKS
	- cd dac;                            $(MAKE) install VXWORKS
	- cd rol;                            $(MAKE) all     VXWORKS
	- cd rol;                            $(MAKE) rols    VXWORKS
	- cd rol;                            $(MAKE) install VXWORKS

cleanvxworks:
	cd mysql4.1.20;                      $(MAKE) clean   VXWORKS
	- cd codatt;                         $(MAKE) clean   VXWORKS
	- cd dac;                            $(MAKE) clean   VXWORKS
	- cd rol;                            $(MAKE) clean   VXWORKS

arm:
	cd mysql4.1.20;                      $(MAKE) all     ARM
	- cd mysql4.1.20;                    $(MAKE) install ARM
	- cd codatt;                         $(MAKE) all     ARM
	- cd codatt;                         $(MAKE) install ARM
	- cd dac;                            $(MAKE) all     ARM
	- cd dac;                            $(MAKE) install ARM
	- cd rol;                            $(MAKE) all     ARM
	- cd rol;                            $(MAKE) rols    ARM
	- cd rol;                            $(MAKE) install ARM

cleanarm:
	cd mysql4.1.20;                      $(MAKE) clean   ARM
	- cd codatt;                         $(MAKE) clean   ARM
	- cd dac;                            $(MAKE) clean   ARM
	- cd rol;                            $(MAKE) clean   ARM


###########################################



