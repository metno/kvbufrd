SUBDIRS = ecmwf debian_files share/bufrtables src src/kvbufrconf
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

include mk/debian.mk