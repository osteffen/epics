DEPS = libreadline-dev re2c libpcre3-dev build-essential
PACKAGEMANAGER = sudo apt-get
SUBDIRS = base modules extensions

.PHONY: deps all $(SUBDIRS) clean

all: deps $(SUBDIRS)

$(SUBDIRS): deps
	@echo ""
	@echo ""
	@echo "===== Building $@ ====="
	$(MAKE) -C $@

modules: base

extensions: base modules

deps:
	@echo ""
	@echo ""
	@echo "==== Installing Dependencies ===="
	$(PACKAGEMANAGER) install $(DEPS)

clean:
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
