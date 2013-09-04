#
#  copies rcscripts to $(CODA)/common/rcscripts directories
#

SHELL = /bin/csh

all: 
	@echo Nothing to do for all in rcscripts

install:
	@echo "Install rcscripts"
	-mkdir -p $(CODA)/common/rcscripts
	-mkdir -p $(CODA)/common/rcscripts/main
	cp run_dispatch      $(CODA)/common/rcscripts
	chmod ug+rwx         $(CODA)/common/rcscripts/run_dispatch
	chmod a+x            $(CODA)/common/rcscripts/run_dispatch
	@cp -f main/* $(CODA)/common/rcscripts/main

clean:
	@echo Nothing to clean in rcscripts

distclean:
	rm -rf $(CODA)/common/rcscripts
