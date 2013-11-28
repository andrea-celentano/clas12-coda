#
# Makefile for coda/src/rc/ directory
#

all:
	cd cg;                            gmake
	- cd sace3.1;                     gmake
	- cd motif;                       gmake
	- cd runControl/Common;           gmake
	- cd runControl/Variables;        gmake
	- cd runControl/rcClient;         gmake lib
	- cd runControl/rcServer;         gmake
	- cd runControl/Xui;              gmake
	- cd runControl/rcClient;         gmake

clean:
	cd cg;                            gmake clean
	- cd sace3.1;                     gmake clean
	- cd motif;                       gmake clean
	- cd runControl/Common;           gmake clean
	- cd runControl/Variables;        gmake clean
	- cd runControl/rcClient;         gmake clean
	- cd runControl/rcServer;         gmake clean
	- cd runControl/Xui;              gmake clean

install:
	cd runControl/rcServer;           gmake install
	- cd runControl/Xui;              gmake install
