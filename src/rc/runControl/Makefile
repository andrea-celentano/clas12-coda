#
# Makefile for coda/src/rc/ directory
#

all:
	cd Common;           gmake
	- cd Variables;        gmake
	- cd rcClient;         gmake lib
	- cd rcServer;         gmake
	- cd Xui;              gmake
	- cd rcClient;         gmake

clean:
	cd Common;           gmake clean
	- cd Variables;        gmake clean
	- cd rcClient;         gmake clean
	- cd rcServer;         gmake clean
	- cd Xui;              gmake clean

install:
	cd rcServer;           gmake install
	- cd Xui;              gmake install
