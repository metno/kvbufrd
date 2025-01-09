#bufrtables_source = 
#EXTRA_DIST = bufrtables_000400.tar.gz
#tables=bufrtables_000400.tar.gz
#tables=bufrdc_tables_wmo_v33.tar.gz
tables=bufrdc-wmo-v40-v41.tar.gz

EXTRA_DIST = $(tables)

#Install the bufr tables in src/bufrtables
#This must be done before we run the checks in src.


all:
	if [ ! -d $(top_builddir)/src/bufrtables ]; then \
		( mkdir -p $(top_builddir)/src/bufrtables && \
		cp $(top_srcdir)/share/bufrtables/$(tables) \
		   $(top_builddir)/src/bufrtables );\
		(cd $(top_builddir)/src/bufrtables; tar zxpf $(tables) --strip-components=1); \
	fi


clean:
	rm -rf $(top_builddir)/src/bufrtables

distclean: clean

#bufrtablesdir = $(pkgdatadir)/bufrtables
#bufrtables_DATA = $(bufrtables_source)

