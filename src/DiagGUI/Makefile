#
# File:
#    Makefile
#
# Description:
#
# $Date$
# $Rev$
#

CROSS_COMPILE		=
CC			= $(CROSS_COMPILE)g++
AR                      = ar
RANLIB                  = ranlib
CFLAGS			= -O2 -fno-exceptions -fPIC -I/usr/include \
			-I${LINUXVME_INC} -I. \
                        -I./src \
                        -I./src/CTP \
                        -I./src/DCRB \
                        -I./src/DCRBScalers \
                        -I./src/DSC \
                        -I./src/FADC \
                        -I./src/GTP \
                        -I./src/GTP_HPS \
                        -I./src/SD \
                        -I./src/SSP \
                        -I./src/SSP_HPS \
                        -I./src/TID \
                        -I./src/V1495Pulser \
                        -I./src/VSCM \
			-L${LINUXVME_LIB} -L. -DJLAB \
			`root-config --cflags`
LINKLIBS		= -lrt \
			`root-config --libs` \
			`root-config --glibs`
# LINKLIBS		= -lrt -ljvme -ltir -ltid -lfadc \
# 			`root-config --libs` \
# 			`root-config --glibs`
PROGS			= DiagGUI 
HEADERS			= $(wildcard *.h)
SRC			= ./src/DiagGUI.cxx ./src/main.cxx
OBJS			= $(SRC:.C=.o)

all: $(PROGS) $(HEADERS)

install:
	cp $(PROGS) $(CODA)/$(OSTYPE_MACHINE)/bin

clean distclean:
	@rm -f $(PROGS) *~ *.o outlinkDef.{C,h}

%.o:	%.C Makefile
	@echo "Building $@"
	$(CC) $(CFLAGS) \
	-c $<

$(PROGS): $(OBJS) $(SRC) $(HEADERS) Makefile
	@echo "Building $@"
	$(CC) $(CFLAGS) -o $@ \
	$(LINKLIBS) \
	$(OBJS)


.PHONY: all clean distclean
