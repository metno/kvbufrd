BUFRSRC=$(top_srcdir)/ecmwf/bufrdc_000409.tar.gz
BUFRSRCDIR=$(top_builddir)/ecmwf/bufrdc_000409

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
		chmod +x $(BUFRSRCDIR)/test.sh ; \
		(cd $(BUFRSRCDIR) && INSTALLDIR=$(pkgdatadir) R64=no CNAME=_gfortran ./my_build_library) ; \
		cp $(BUFRSRCDIR)/$@ . ;\
	fi 


#		rm $(BUFRSRCDIR)/test.sh ; 
#   cp $(top_srcdir)/ecmwf/test.sh $(BUFRSRCDIR) ; 

#CFLAGS  = $(DEBUG) $(PROFILE) -DLITTLE_ENDIAN -DPOINTER_64 $(CC_A32) $(LOCAL_CFLAGS) $(LARGE_FILE) -DTABLE_PATH=\"/workspace/kvbufrd/build/focal\"
#CFLAGS  = $(DEBUG) $(PROFILE) -DLITTLE_ENDIAN -DPOINTER_64 $(CC_A32) $(LOCAL_CFLAGS) $(LARGE_FILE) -DTABLE_PATH=\"/workspaces/kvbufrd/build/focal/share/kvbufrd/bufrtables\"
#FFLAGS = $(DEBUG) $(PROFILE) $(CC_A32) -fcray-pointer -Dlinux $(USE_NO_POINTERS) -Dgfortran -DTABLE_PATH=\"/workspace/kvbufrd/build/focal\" -DLITTLE_ENDIAN
#gfortran -O2   -fcray-pointer -Dlinux -DPOINTER_64 -Dgfortran -DTABLE_PATH=\"/usr/share/kvbufrd/bufrtables\" -DLITTLE_ENDIAN -c buetab.F

EXTRA_DIST=\
	bufrdc_000409.tar.gz \
	my_build_library \
	test.sh
