AM_CPPFLAGS = -D_REENTRANT \
           -I$(top_srcdir)/src \
			  $(kvcpp_CFLAGS) \
			  $(putools_CFLAGS) \
           $(BOOST_CPPFLAGS) \
           $(omniORB4_CFLAGS)  


bin_PROGRAMS = kvbufrconf kvbufrconfcmp
                        
kvbufrconf_SOURCES = \
		../StationInfo.cc ../StationInfo.h \
      ../StationInfoParse.cc ../StationInfoParse.h \
		../splitstr.h ../splitstr.cc \
		../InitLogger.cc ../InitLogger.h \
		../parseMilogLogLevel.cc ../parseMilogLogLevel.h \
		../checkfile.h ../checkfile.cc \
      tblStInfoSysParam.h tblStInfoSysParam.cc \
      tblStInfoSysNetworkStation.h tblStInfoSysNetworkStation.cc \
		tblStInfoSysSensorInfo.h tblStInfoSysSensorInfo.cc \
		tblStInfoSysStation.h tblStInfoSysStation.cc \
		tblStInfoSysStationOutmessage.h tblStInfoSysStationOutmessage.cc \
		tblStInfoSysObsPgmH.h tblStInfoSysObsPgmH.cc \
		ConfMaker.h ConfMaker.cc \
		ConfApp.h ConfApp.cc \
		kvbufrconfOptions.cc kvbufrconfOptions.h \
		kvbufrconf.cc
kvbufrconf_LDFLAGS = -pthread $(BOOST_LDFLAGS)

if KVSTATIC
kvbufrconf_CPPFLAGS = -I$(KVINCLUDEDIR) $(kvcpp_CFLAGS) $(AM_CPPFLAGS)
kvbufrconf_LDADD = \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvcpp.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_milog.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvalobs.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_corbahelper.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_miconfparser.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_dnmithread.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvdb.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_miutil.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_fileutil.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvadmininterface.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvskel.a \
	$(putools_LIBS) \
	$(omniORB4_LIBS) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_DATE_TIME_LIB) \
   	-lm -ldl
else
kvbufrconf_CPPFLAGS = $(kvcpp_CFLAGS) $(AM_CPPFLAGS)
kvbufrconf_LDADD = $(kvcpp_LIBS) $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) \
                   -lm -ldl
endif
                   
kvbufrconfcmp_SOURCES = \
	../StationInfo.cc ../StationInfo.h \
	../StationInfoParse.cc ../StationInfoParse.h \
   ../checkfile.h ../checkfile.cc \
	../splitstr.h ../splitstr.cc \
	../InitLogger.cc ../InitLogger.h \
	../parseMilogLogLevel.cc ../parseMilogLogLevel.h \
	kvbufrconfcmpOptions.cc kvbufrconfcmpOptions.h \
	kvbufrconfcmp.cc
kvbufrconfcmp_LDFLAGS = -pthread $(BOOST_LDFLAGS)

if KVSTATIC
kvbufrconfcmp_CPPFLAGS = -I$(KVINCLUDEDIR) $(kvcpp_CFLAGS) $(AM_CPPFLAGS)
kvbufrconfcmp_LDADD = \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvcpp.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_milog.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvalobs.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_corbahelper.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_miconfparser.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_dnmithread.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvdb.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_miutil.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_fileutil.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvadmininterface.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvskel.a \
	$(putools_LIBS) \
	$(omniORB4_LIBS) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_DATE_TIME_LIB) \
    -lm
else
kvbufrconfcmp_CPPFLAGS = $(kvcpp_CFLAGS) $(AM_CPPFLAGS)
kvbufrconfcmp_LDADD = \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvcpp.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_milog.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvalobs.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_corbahelper.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_miconfparser.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_dnmithread.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvdb.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_miutil.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_fileutil.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvadmininterface.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvskel.a \
	$(putools_LIBS) \
	$(omniORB4_LIBS) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_DATE_TIME_LIB) \
    -lm
endif
