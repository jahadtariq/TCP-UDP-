# Subject:  Pocitacove komunikace a site
# Project:  Projekt 3 - Implementace zretezeneho RDT
# Author:   Radim Loskot, xlosko01@stud.fit.vutbr.cz
# Date:     28. 4. 2011
# 
# Usage:
#	- make          compiles project - release version
#	- make pack     pack all project files
#	- make client   compiles only client
#	- make server   compiles only server
#	- make clean    clean temp compilers files
#

MK=gmake
PACKAGE_NAME=xlosko01
SRCFILES=objs src readme.txt Makefile Makefile-client Makefile-server

# Calls GNU make
all:
	$(MK) -f Makefile-server
	$(MK) -f Makefile-client

.PHONY: server client clean pack

server:
	$(MK) -f Makefile-server

client:
	$(MK) -f Makefile-client

clean:
	$(MK) -f Makefile-server clean
	$(MK) -f Makefile-client clean

pack:
	tar -cvf $(PACKAGE_NAME).tar $(SRCFILES)
	gzip $(PACKAGE_NAME).tar
