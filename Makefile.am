SUBDIRS = src 
ACLOCAL_AMFLAGS = -I m4
sqldir = $(datadir)/kvbufferd
sql_DATA = cleanbufferdb.sql 
bin_SCRIPTS = kvbufferdbadmin check_kvbufferd.sh

SED_SUBST = sed s,KVCONFIG,$(bindir)/kvconfig,g $< > $@ 

kvbufferdbadmin: kvbufferdbadmin.sh 
	$(SED_SUBST)
	
EXTRA_DIST = cleanbufferdb.sql kvbufferdbadmin.sh changelog

CLEANFILES = kvbufferdbadmin

install-data-hook:
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/lib/@PACKAGE_NAME@
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/lib/@PACKAGE_NAME@/debug
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/run/kvalobs
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/log/kvalobs
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/log/kvalobs/kvbuffer
	$(install_sh) -m 755 -d $(DESTDIR)/$(datadir)/@PACKAGE_NAME@

#pkgconfig_DATA = @PACKAGE_TARNAME@.pc
#$(pkgconfig_DATA): config.status

include mk/debian.mk