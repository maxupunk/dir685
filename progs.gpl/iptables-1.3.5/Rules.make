#! /usr/bin/make

all: $(SHARED_LIBS) $(EXTRAS)

experimental: $(EXTRAS_EXP)

# Have to handle extensions which no longer exist.
clean: $(EXTRA_CLEANS)
	rm -f $(SHARED_LIBS) $(EXTRAS) $(EXTRAS_EXP) $(SHARED_LIBS:%.so=%_sh.o)
	rm -f extensions/initext.c extensions/initext6.c
	@find . -name '*.[ao]' -o -name '*.so' -o -name '*.d' | xargs rm -f
	rm -f .makefirst

install: all $(EXTRA_INSTALLS)
	@echo -e "\033[32mInstalling $(APPLET) ...\033[0m"
#	@if [ -f /usr/local/bin/iptables -a "$(BINDIR)" = "/usr/local/sbin" ];\
#	then echo 'Erasing iptables from old location (now /usr/local/sbin).';\
#	rm -f /usr/local/bin/iptables;\
#	fi

install-experimental: $(EXTRA_INSTALLS_EXP)

TAGS:
	@rm -f $@
	find . -name '*.[ch]' | xargs etags -a

dep: $(DEPFILES) $(EXTRA_DEPENDS)
	@echo Dependencies will be generated on next make.
	rm -f $(DEPFILES) $(EXTRA_DEPENDS) .makefirst

$(SHARED_LIBS:%.so=%.d): %.d: %.c
	@-$(CC) -M -MG $(CFLAGS) $< | \
	    sed -e 's@^.*\.o:@$*.d $*_sh.o:@' > $@

$(SHARED_LIBS): %.so : %_sh.o
	$(LD) -shared $(EXT_LDFLAGS) -o $@ $<

%_sh.o : %.c
	$(CC) $(SH_CFLAGS) -o $@ -c $<

.makefirst:
	@echo Making dependencies: please wait...
	@touch .makefirst

# This is useful for when dependencies completely screwed
%.h::
	@echo Something wrong... deleting dependencies.
	@-rm -f $(DEPFILES) $(EXTRA_DEPENDS) .makefirst
	@[ -d $(KERNEL_DIR)/include/linux/netfilter_ipv4 ] || echo -e '\n\n    Please try `make KERNEL_DIR=path-to-correct-kernel'\'.'\n\n'
	@exit 1

-include $(DEPFILES) $(EXTRA_DEPENDS)
-include .makefirst
