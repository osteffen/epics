SUBDIRS := $(patsubst %/,%, $(wildcard */) )

.PHONY: all $(SUBDIRS) clean

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

asyn: ipac seq

streamdevice: asyn

clean:
	set -e; for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
