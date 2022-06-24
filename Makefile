lib.name := complex

CPPFLAGS:=-I. -std=c++11 -fpermissive
hilbert~.class.sources := hilbert~.cpp
complex.mul~.class.sources := complex.mul~.c

PDLIBBUILDER_DIR=pd-lib-builder
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

