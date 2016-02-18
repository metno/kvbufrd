#bufrtables_source = 
EXTRA_DIST = bufrtables_000400.tar.gz

#Install the bufr tables in src/bufrtables
#This must be done before we run the checks in src.


all:
	if [ ! -d $(top_builddir)/src/bufrtables ]; then \
		( mkdir -p $(top_builddir)/src/bufrtables && \
		cp $(top_srcdir)/share/bufrtables/bufrtables_000400.tar.gz \
		   $(top_builddir)/src/bufrtables );\
		(cd $(top_builddir)/src/bufrtables; tar zxpf bufrtables_000400.tar.gz); \
	fi


clean:
	rm -rf $(top_builddir)/src/bufrtables

distclean: clean

#bufrtablesdir = $(pkgdatadir)/bufrtables
#bufrtables_DATA = $(bufrtables_source)

