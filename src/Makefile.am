SUBDIRS = gtest bufr

AM_CPPFLAGS = \
	-D_REENTRANT \
   $(kvsubscribe_CFLAGS) \
	-I${KVINCLUDEDIR}/kvalobs/kvskel \
	$(putools_CFLAGS) \
   $(BOOST_CPPFLAGS) \
   $(omniORB4_CFLAGS)  

clean-local:
	if [ -d bufrtables ]; then\
		rm -rf bufrtables ;\
	fi

noinst_PROGRAMS = base64Test kvxmltest kvmd5sum

bin_PROGRAMS = kvbufrd kvbufrdbclt  kvbufrdparse kvbase64 

kvbufrd_SOURCES = \
   kvevents.h \
   KvDataConsumer.h KvDataConsumer.cc \
   KvObsData.h KvObsData.cc \
   DbQuery.h DbQuery.cc \
	 cachedbcleaner.h cachedbcleaner.cc \
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
   LoadBufrData.h LoadBufrData.cc \
   base64.cc base64.h \
   scanstring.cc scanstring.h \
   SemiUniqueName.cc SemiUniqueName.h \
   Strerror.cc Strerror.h \
   isDirWithWritePermission.cc isDirWithWritePermission.h \
   dbhelper.cc dbhelper.h \
	 cachedb.h cachedb.cc \
	 utils.h utils.cc  \
	 LogAppender.h LogAppender.cc 
	 
 
kvbufrd_LDFLAGS = -pthread -export-dynamic $(BOOST_LDFLAGS)

kvxmltest_SOURCES = \
	kvxml_test.cc \
	kvxml_test_data.cc \
	Data.cc Data.h\
	KvObsData.cc KvObsData.h \
	kvDbGateProxyThread.cc kvDbGateProxyThread.h \
	StationInfoParse.cc StationInfoParse.h \
	StationInfo.cc StationInfo.h \
	parseMilogLogLevel.cc parseMilogLogLevel.h

kvxmltest_LDFLAGS = -pthread -export-dynamic $(BOOST_LDFLAGS)

kvxmltest_CPPFLAGS = $(kvcpp_CFLAGS) $(AM_CPPFLAGS) -DDATADIR="\""$(pkgdatadir)/bufrtables"\"" \
	-DCACHEDIR="\""$(localstatedir)/lib/kvbufrd"\""

kvxmltest_LDADD = \
    $(kvsubscribe_LIBS) $(BOOST_THREAD_LIB) \
    $(BOOST_FILESYSTEM_LIB) \
    $(BOOST_DATE_TIME_LIB) \
    $(BOOST_SYSTEM_LIB) \
    -lm -ldl

kvbase64_SOURCES = \
	kvbase64.cc \
  base64.cc base64.h 

kvbase64_LDADD = \
    $(kvsubscribe_LIBS) $(BOOST_THREAD_LIB) \
    $(BOOST_FILESYSTEM_LIB) \
    $(BOOST_DATE_TIME_LIB) \
    $(BOOST_SYSTEM_LIB) \
    -lm -ldl



kvmd5sum_SOURCES = \
	kvmd5sum.cc \
  md5.cc md5.h 

if KVSTATIC
kvbufrd_CPPFLAGS = -I$(KVINCLUDEDIR) $(kvcpp_CFLAGS) $(AM_CPPFLAGS) -DDATADIR="\""$(pkgdatadir)/bufrtables"\"" \
	-DCACHEDIR="\""$(localstatedir)/lib/kvbufrd"\""
kvbufrd_LDADD = \
	$(top_builddir)/src/bufr/libbufr.la \
	$(KVLIBDIR)/${KVLIBPREFIX}_milog.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_corbahelper.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_miconfparser.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvdb.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_miutil.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_fileutil.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvadmininterface.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvskel.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_decoderbase.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvsubscribe.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_decodeutility.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvalobs.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_decoderbase.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvdb.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_fileutil.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_miutil.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_dnmithread.a \
	-lxml++-2.6 \
	-lglibmm-2.4 \
	-lrdkafka++ \
	-lrdkafka \
	$(omniORB4_LIBS) \
	$(BOOST_THREAD_LIB) \
        $(BOOST_FILESYSTEM_LIB) \
        $(BOOST_DATE_TIME_LIB) \
        $(BOOST_SYSTEM_LIB) \
        $(top_builddir)/ecmwf/libbufr.a \
        -lgfortran -lm -ldl
else
kvbufrd_CPPFLAGS = $(kvcpp_CFLAGS) $(AM_CPPFLAGS) -DDATADIR="\""$(pkgdatadir)/bufrtables"\"" \
	-DCACHEDIR="\""$(localstatedir)/lib/kvbufrd"\""
kvbufrd_LDADD = \
    $(top_builddir)/src/bufr/libbufr.la \
    $(kvsubscribe_LIBS) $(BOOST_THREAD_LIB) \
    $(BOOST_FILESYSTEM_LIB) \
    $(BOOST_DATE_TIME_LIB) \
    $(BOOST_SYSTEM_LIB) \
    $(top_builddir)/ecmwf/libbufr.a \
    -lgfortran \
    -lm -ldl
endif         

EXTRA_DIST = kvbufrd_schema.sql 

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
	$(KVLIBDIR)/${KVLIBPREFIX}_miutil.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvadmininterface.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvskel.a \
	$(putools_LIBS) \
	$(omniORB4_LIBS) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_SYSTEM_LIB) \
    -lm -ldl
else
kvbufrdbclt_CPPFLAGS = $(kvcpp_CFLAGS) $(AM_CPPFLAGS)
kvbufrdbclt_LDADD = \
   $(kvsubscribe_LIBS) $(putools_LIBS) $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) \
   $(BOOST_SYSTEM_LIB) \
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
	$(KVLIBDIR)/${KVLIBPREFIX}_miutil.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvadmininterface.a \
	$(KVLIBDIR)/${KVLIBPREFIX}_kvskel.a \
	$(putools_LIBS) \
	$(omniORB4_LIBS) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_SYSTEM_LIB) \
  -lm

else
kvbufrdparse_CPPFLAGS = $(kvcpp_CFLAGS) $(AM_CPPFLAGS)
kvbufrdparse_LDADD = \
   $(kvsubscribe_LIBS) $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) \
   $(BOOST_SYSTEM_LIB) \
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
check_PROGRAMS=kvBufrEncodeTest
TESTS=$(check_PROGRAMS)

kvBufrEncodeTest_SOURCES= \
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
  isDirWithWritePermission.cc isDirWithWritePermission.h \
	kvDbGateProxyThread.cc kvDbGateProxyThread.h

kvBufrEncodeTest_LDFLAGS = -pthread $(BOOST_LDFLAGS)


if KVSTATIC
kvBufrEncodeTest_CPPFLAGS = \
	-I$(KVINCLUDEDIR) \
	$(gtest_CFLAGS) \
	$(AM_CPPFLAGS) \
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
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_SYSTEM_LIB) \
	$(gtest_LIBS) \
	$(top_builddir)/ecmwf/libbufr.a \
	-lgfortran \
	-lm
else
kvBufrEncodeTest_CPPFLAGS = \
	$(gtest_CFLAGS) \
	$(AM_CPPFLAGS) \
	-DTESTDATADIR="\""$(top_srcdir)/src/test"\""\
	-DBUFRTBLDIR="\""$(BUFRTABLES)"\""

#	 -DBUFRTBLDIR="\""$(top_builddir)/src/bufrtables"\""
#    -DBUFRTBLDIR="\""$(top_srcdir)/share/bufrtables"\""

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
	$(top_builddir)/ecmwf/libbufr.a \
	$(putools_LIBS) \
	$(omniORB4_LIBS) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_SYSTEM_LIB) \
	$(gtest_LIBS) \
    -lgfortran -lm -ldl
endif			
endif			
