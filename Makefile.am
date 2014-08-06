SUBDIRS = share/bufrtables src src/kvbufrconf
#SUBDIRS = src 
ACLOCAL_AMFLAGS = -I m4
sqldir = $(datadir)/kvbufrd
sql_DATA = cleanbufrdb.sql 
bin_SCRIPTS = kvbufrdbadmin check_kvbufrd.sh

SED_SUBST = sed s,__KVCONFIG__,$(bindir)/kvconfig,g $< > $@ 

kvbufrdbadmin: kvbufrdbadmin.sh 
	$(SED_SUBST)
	
EXTRA_DIST = cleanbufrdb.sql kvbufrdbadmin.sh changelog 

CLEANFILES = kvbufrdbadmin

install-data-hook:
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/lib/@PACKAGE_NAME@
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/lib/@PACKAGE_NAME@/debug
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/run/kvalobs
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/log/kvalobs
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/log/kvalobs/kvbufr
	$(install_sh) -m 755 -d $(DESTDIR)/$(datadir)/@PACKAGE_NAME@
	$(install_sh) -m 755 -d $(DESTDIR)/$(pkgdatadir)/bufrtables
	tar zxpf $(top_srcdir)/share/bufrtables/bufrtables_000400.tar.gz -C $(DESTDIR)/$(pkgdatadir)/bufrtables
	
#	(cd $(DESTDIR)/$(pkgdatadir)/bufrtables; \
#	 tar zxpf $(top_srcdir)/share/bufrtables/bufrtables_000400.tar.gz)

#Install the bufr tables in src/bufrtables
#This must be done before we run the checks in src.
#TESTS=true
#check-local:
#	echo "CWDIR: $(pwd)"
#	mkdir -p src/bufrtables
#	if [ -d src/bufrtables ]; then \
#		if [ ! -f src/bufrtables/bufrtables.tar.gz ]; then \
#			cp $(top_srcdir)/bufrtables_000400.tar.gz src/bufrtables \
#			(cd src/bufrtables; tar zxpf bufrtables_000400.tar.gz) \
#		fi \
#	fi



#pkgconfig_DATA = @PACKAGE_TARNAME@.pc
#$(pkgconfig_DATA): config.status

include mk/debian.mk