# EZCA Example


LIBS = -lca -lCom -lezca -rdynamic -Wl,-rpath,$(EPICS_BASE)/lib/$(EPICS_HOST_ARCH):$(EPICS_EXTENSIONS)/lib/$(EPICS_HOST_ARCH)
LIBDIRS = -L$(EPICS_BASE)/lib/$(EPICS_HOST_ARCH) -L$(EPICS_EXTENSIONS)/lib/$(EPICS_HOST_ARCH)
INCDIRS = -I$(EPICS_EXTENSIONS)/include -I$(EPICS_BASE)/include/ -I$(EPICS_BASE)/include/compiler/gcc -I$(EPICS_BASE)/include/os/Linux


# Check if paths are set in environment

ifndef EPICS_BASE
$(error EPICS_BASE not defined. Set up the path to your EPICS installation.)
endif
ifndef EPICS_EXTENSIONS
$(error EPICS_EXTENSIONS not defined. Set up the path to your EPICS installation.)
endif


all: epics_example

epics_example:
	gcc main.c -o example $(LIBS) $(LIBDIRS) $(INCDIRS)

clean:
	rm -fr example
