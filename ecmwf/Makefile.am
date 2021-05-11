BUFRSRC=$(top_srcdir)/ecmwf/bufrdc_000409.tar.gz
BUFRSRCDIR=bufrdc_000409

libbufrdir=${bufrdir}
libbufr_LIBRARIES = libbufr.a
libbufr_a_SOURCES = 

unpack_bufr:
	if test ! -d $(BUFRSRCDIR) ; then \
		tar zxpf $(BUFRSRC); \
  fi

libbufr.a: unpack_bufr
	if ! test -f $@ ; then \
		cp $(top_srcdir)/ecmwf/my_build_library $(BUFRSRCDIR) ; \
		cp $(top_srcdir)/ecmwf/test.sh $(BUFRSRCDIR) ; \
		chmod +x $(BUFRSRCDIR)/test.sh ; \
		(cd $(BUFRSRCDIR) && INSTALLDIR=$(pkgdatadir)/bufrtables R64=no CNAME=_gfortran ./my_build_library) ; \
		cp $(BUFRSRCDIR)/$@ . ;\
	fi 

EXTRA_DIST=\
	bufrdc_000409.tar.gz \
	my_build_library \
	test.sh
