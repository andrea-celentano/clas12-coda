#
# File:
#    Makefile
#
# Description:
#
# $Date$
# $Rev$
#

ifndef LINUXVME_LIB
	LINUXVME_LIB	= ${CODA}/src/rol/jvme
endif
ifndef LINUXVME_INC
	LINUXVME_INC	= ${CODA}/src/rol/jvme
endif

CROSS_COMPILE		=
CC			= $(CROSS_COMPILE)gcc
AR                      = ar
RANLIB                  = ranlib
CFLAGS			= -O2 -fno-exceptions -fPIC -I/usr/include\
			-I${LINUXVME_INC} -I. \
			-L${LINUXVME_LIB} -L.
# -DJLAB \
#			`root-config --cflags`
#LINKLIBS		= -lrt -ljvme -ltir -ltid -lfadc \
#			`root-config --libs` \
#			`root-config --glibs`
LINKLIBS		= -lrt -ljvme
#			`root-config --libs` \
#			`root-config --glibs`

PROGS			= DiagGuiServer 
HEADERS			= $(wildcard *.h)
SRC			= DiagGuiServer.c CrateMsgServer.c
OBJS			= $(SRC:.C=.o)

all: $(PROGS) $(HEADERS)

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
