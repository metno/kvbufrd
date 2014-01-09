SUBDIRS = gtest bufr

AM_CPPFLAGS = -D_REENTRANT \
			  $(kvcpp_CFLAGS) \
			  $(putools_CFLAGS) \
              $(BOOST_CPPFLAGS) \
              $(omniORB4_CFLAGS)  


nodist_idl_sources=kvbufrd.idl 


CLEANFILES = $(nodist_idl_sources:.idl=SK.cc) $(nodist_idl_sources:.idl=.hh) 


%SK.cc %.hh: %.idl
#	$(IDL) -bcxx -Wbinline -I$(KVIDLDIR) $<
	$(IDL) -bcxx -I$(KVIDLDIR) $<

clean-local:
	if [ -d bufrtables ]; then\
		rm -rf bufrtables ;\
	fi

noinst_PROGRAMS = kvBufrEncodeTest base64Test

bin_PROGRAMS = kvbufrd kvbufrclt kvbufrdbclt kvbufrdparse kvbufrconf kvbufrconfcmp

kvbufrd_SOURCES = kvbufrd.hh kvbufrdSK.cc \
                  defines.h \
		          kvbufrd.cc \
                  App.cc App.h \
                  DataReceiver.cc DataReceiver.h \
                  Data.cc Data.h \
                  BufrWorker.cc BufrWorker.h \
                  BufrData.cc BufrData.h \
                  KvParam.cc KvParam.h \
                  DataElementList.cc DataElementList.h \
                  DataList.cc DataList.h \
                  StationInfo.cc StationInfo.h \
                  StationInfoParse.cc StationInfoParse.h \
                  tblBufr.cc tblBufr.h \
                  kvbufrdImpl.cc kvbufrdImpl.h \
                  Replay.cc Replay.h \
                  bufr.cc bufr.h \
                  Validate.cc Validate.h \
                  tblWaiting.cc tblWaiting.h \
                  Waiting.cc Waiting.h \
                  delaycontrol.cc delaycontrol.h \
                  tblKeyVal.cc tblKeyVal.h \
                  parseMilogLogLevel.cc parseMilogLogLevel.h \
                  InitLogger.cc InitLogger.h \
                  Indent.h \
                  kvDbGateProxyThread.cc kvDbGateProxyThread.h \
                  kvDbGateProxy.cc kvDbGateProxy.h \
                  getDataReceiver.cc getDataReceiver.h \
                  GetDataThread.cc GetDataThread.h \
                  kvbufrdImpl.h obsevent.h \
                  LoadBufrData.h LoadBufrData.cc \
                  encodebufr.h encodebufr.cc \
                  base64.cc base64.h \
                  scanstring.cc scanstring.h \
                  SemiUniqueName.cc SemiUniqueName.h \
                  Strerror.cc Strerror.h \
                  isDirWithWritePermission.cc isDirWithWritePermission.h
                  
 
kvbufrd_LDFLAGS = -pthread -export-dynamic $(BOOST_LDFLAGS)

if KVSTATIC
kvbufrd_CPPFLAGS = -I$(KVINCLUDEDIR) $(kvcpp_CFLAGS) $(AM_CPPFLAGS) -DDATADIR="\""$(pkgdatadir)/bufrtables"\""
kvbufrd_LDADD = \
	$(top_builddir)/src/bufr/libbufr.la \
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
    $(BOOST_FILESYSTEM_LIB) \
    $(BOOST_DATE_TIME_LIB) \
    $(BOOST_SYSTEM_LIB) \
    -L$(HOME)/lib -lbufr \
    -lgfortran -lm -ldl
else
kvbufrd_CPPFLAGS = $(kvcpp_CFLAGS) $(AM_CPPFLAGS) -DDATADIR="\""$(pkgdatadir)/bufrtables"\""
kvbufrd_LDADD = \
   	$(top_builddir)/src/bufr/libbufr.la \
	$(kvcpp_LIBS) $(BOOST_THREAD_LIB) \
    $(BOOST_FILESYSTEM_LIB) \
    $(BOOST_DATE_TIME_LIB) \
    $(BOOST_SYSTEM_LIB) \
    -L$(HOME)/lib -lbufr -lgfortran \
    -lm -ldl
endif         

EXTRA_DIST = kvbufrd_schema.sql 

 
kvbufrclt_SOURCES = kvbufrclt.cc \
                     kvbufrCltBufrcbImp.cc kvbufrCltBufrcbImp.h \
                     kvbufrCltApp.cc  kvbufrCltApp.h \
                     kvbufrCorbaThread.cc kvbufrCorbaThread.h \
                     kvbufrd.hh kvbufrdSK.cc
kvbufrclt_LDFLAGS = -pthread -export-dynamic $(BOOST_LDFLAGS)

if KVSTATIC
kvbufrclt_CPPFLAGS = -I$(KVINCLUDEDIR) $(kvcpp_CFLAGS) $(AM_CPPFLAGS)
kvbufrclt_LDADD = \
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
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_SYSTEM_LIB) \
    -lm

else
kvbufrclt_CPPFLAGS = $(kvcpp_CFLAGS) $(AM_CPPFLAGS)
kvbufrclt_LDADD = $(kvcpp_LIBS) $(BOOST_THREAD_LIB) \
				  $(BOOST_FILESYSTEM_LIB) \
				  $(BOOST_DATE_TIME_LIB) \
				  $(BOOST_SYSTEM_LIB) \
                  -lm
endif  
                   
kvbufrdbclt_SOURCES = kvbufrdbclt.cc \
                       tblBufr.cc tblBufr.h
kvbufrdbclt_LDFLAGS = -pthread $(BOOST_LDFLAGS)

if KVSTATIC
kvbufrdbclt_CPPFLAGS = -I$(KVINCLUDEDIR) $(kvcpp_CFLAGS) $(AM_CPPFLAGS)
kvbufrdbclt_LDADD = \
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
kvbufrdbclt_CPPFLAGS = $(kvcpp_CFLAGS) $(AM_CPPFLAGS)
kvbufrdbclt_LDADD = $(kvcpp_LIBS) $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) \
                     -lm -ldl
endif                     
                     
kvbufrdparse_SOURCES = kvbufrdparse.cc \
                        StationInfo.cc StationInfo.h \
                        parseMilogLogLevel.cc parseMilogLogLevel.h \
                        StationInfoParse.cc StationInfoParse.h
kvbufrdparse_LDFLAGS = -pthread $(BOOST_LDFLAGS)

if KVSTATIC
kvbufrdparse_CPPFLAGS = -I$(KVINCLUDEDIR) $(kvcpp_CFLAGS) $(AM_CPPFLAGS)
kvbufrdparse_LDADD = \
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
kvbufrdparse_CPPFLAGS = $(kvcpp_CFLAGS) $(AM_CPPFLAGS)
kvbufrdparse_LDADD =  $(kvcpp_LIBS) $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) \
                     -lm
endif                     
                        
kvbufrconf_SOURCES = StationInfo.cc StationInfo.h \
                     StationInfoParse.cc StationInfoParse.h \
                     tblStInfoSysParam.h tblStInfoSysParam.cc \
                     tblStInfoSysNetworkStation.h tblStInfoSysNetworkStation.cc \
					 tblStInfoSysSensorInfo.h tblStInfoSysSensorInfo.cc \
					 tblStInfoSysStation.h tblStInfoSysStation.cc \
					 tblStInfoSysStationOutmessage.h tblStInfoSysStationOutmessage.cc \
					 tblStInfoSysObsPgmH.h tblStInfoSysObsPgmH.cc \
					 checkfile.h checkfile.cc \
					 ConfMaker.h ConfMaker.cc \
					 ConfApp.h ConfApp.cc \
					 splitstr.h splitstr.cc \
					 InitLogger.cc InitLogger.h \
					 parseMilogLogLevel.cc parseMilogLogLevel.h \
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
                   
kvbufrconfcmp_SOURCES = StationInfo.cc StationInfo.h \
	                    StationInfoParse.cc StationInfoParse.h \
     	         		checkfile.h checkfile.cc \
					    splitstr.h splitstr.cc \
					    InitLogger.cc InitLogger.h \
					    parseMilogLogLevel.cc parseMilogLogLevel.h \
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

sqldir=$(datadir)/kvbufrd
sql_DATA=kvbufrd_schema.sql
bin_SCRIPTS = kvbufrd_initdb


base64Test_SOURCES = base64Test.cc \
                     base64.cc base64.h
base64Test_CPPFLAGS = $(AM_CPPFLAGS) 
base64Test_LDADD = -lm


if HAVE_GTEST
TESTS=kvBufrEncodeTest
#noinst_PROGRAMS=kvBufrEncodeTest

check_PROGRAMS=kvBufrEncodeTest


kvBufrEncodeTest_SOURCES = \
	test/kvBufrEncodeTest.cc \
	test/kvBufrEncodeTestConf.h \
	test/ReadDataFile.h test/ReadDataFile.cc \
	Data.cc Data.h\
	DataList.cc DataList.h \
	DataElementList.cc DataElementList.h \
	BufrData.cc BufrData.h \
	StationInfo.cc StationInfo.h \
    StationInfoParse.cc StationInfoParse.h \
	bufr.cc bufr.h \
	Validate.cc Validate.h \
	InitLogger.cc InitLogger.h \
	parseMilogLogLevel.cc parseMilogLogLevel.h \
	bufrencodehelper.h \
	KvParam.cc KvParam.h \
	LoadBufrData.h LoadBufrData.cc \
	scanstring.cc scanstring.h \
	SemiUniqueName.cc SemiUniqueName.h \
	Strerror.cc Strerror.h \
    isDirWithWritePermission.cc isDirWithWritePermission.h

kvBufrEncodeTest_LDFLAGS = -pthread $(BOOST_LDFLAGS)


if KVSTATIC
kvBufrEncodeTest_CPPFLAGS = \
	-I$(KVINCLUDEDIR) \
	$(AM_CPPFLAGS) \
	$(gtest_CFLAGS) \
	-DTESTDATADIR="\""$(top_srcdir)/src/test"\""\
	-DBUFRTBLDIR="\""$(BUFRTABLES)"\""
	
#    -DBUFRTBLDIR="\""$(top_builddir)/src/bufrtables"\""
#-DBUFRTBLDIR="\""$(top_srcdir)/share/bufrtables"\""

kvBufrEncodeTest_LDADD =   \
	$(top_builddir)/src/bufr/libbufr.la \
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
	-L$(HOME)/lib \
	$(gtest_LIBS) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_SYSTEM_LIB) \
	-lbufr -lgfortran \
	-lm
else
kvBufrEncodeTest_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(gtest_CFLAGS) \
	-DTESTDATADIR="\""$(top_srcdir)/src/test"\""\
	-DBUFRTBLDIR="\""$(BUFRTABLES)"\""
	
#	 -DBUFRTBLDIR="\""$(top_builddir)/src/bufrtables"\""
#    -DBUFRTBLDIR="\""$(top_srcdir)/share/bufrtables"\""

kvBufrEncodeTest_LDADD =   \
	$(top_builddir)/src/bufr/libbufr.la \
	$(kvcpp_LIBS) \
	-L$(HOME)/lib \
	$(gtest_LIBS) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_SYSTEM_LIB) \
	-lbufr -lgfortran \
	-lm
endif			
endif			