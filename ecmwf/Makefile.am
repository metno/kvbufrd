BUFRSRC=$(top_srcdir)/ecmwf/bufrdc_000409.tar.gz
BUFRSRCDIR=$(top_builddir)/ecmwf/bufrdc_000409
FFLAGS_EXTRA ="-fallow-argument-mismatch"

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
		cp $(top_srcdir)/ecmwf/patch.p1 $(BUFRSRCDIR) ; \
		chmod +x $(BUFRSRCDIR)/test.sh ; \
		(cd $(BUFRSRCDIR) && \
		 patch -p1 < patch.p1 && \
		 INSTALLDIR=$(pkgdatadir) R64=no CNAME=_gfortran FFLAGS_EXTRA="$(FFLAGS_EXTRA)" ./my_build_library) ; \
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
	patch.p1 \
	my_build_library \
	test.sh
