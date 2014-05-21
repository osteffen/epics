SUBDIRS = $(wildcard */)

.PHONY: all $(SUBDIRS) clean

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

asyn: ipac seq

clean:
	set -e; for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
