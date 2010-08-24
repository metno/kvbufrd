AM_CPPFLAGS = -D_REENTRANT \
              $(kvcpp_CFLAGS) \
              $(BOOST_CPPFLAGS) \
              $(omniORB4_CFLAGS)  


nodist_idl_sources=kvbufrd.idl 


CLEANFILES = $(nodist_idl_sources:.idl=SK.cc) $(nodist_idl_sources:.idl=.hh) 


%SK.cc %.hh: %.idl
#	$(IDL) -bcxx -Wbinline -I$(KVIDLDIR) $<
	$(IDL) -bcxx -I$(KVIDLDIR) $<


noinst_PROGRAMS = kvBufrEncodeTest

bin_PROGRAMS = kvbufrd kvbufrclt kvbufrdbclt kvbufrdparse kvbufrconf

kvbufrd_SOURCES = kvbufrd.hh kvbufrdSK.cc \
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
                  InitLogger.cc InitLogger.h \
                  getDataReceiver.cc getDataReceiver.h \
                  GetDataThread.cc GetDataThread.h \
                  kvbufrdImpl.h obsevent.h \
                  LoadBufrData.h LoadBufrData.cc \
                  encodebufr.h encodebufr.cc

kvbufrd_CPPFLAGS = $(AM_CPPFLAGS) -DDATADIR="\""$(pkgdatadir)/bufrtables"\"" 
kvbufrd_LDFLAGS = -pthread -export-dynamic $(BOOST_LDFLAGS)

kvbufrd_LDADD = $(kvcpp_LIBS) $(BOOST_THREAD_LIB) \
                -L$(HOME)/lib -lbufr -lgfortran \
                 -lm         
EXTRA_DIST = kvbufrd_schema.sql 

 
kvbufrclt_SOURCES = kvbufrclt.cc \
                     kvbufrCltBufrcbImp.cc kvbufrCltBufrcbImp.h \
                     kvbufrCltApp.cc  kvbufrCltApp.h \
                     kvbufrCorbaThread.cc kvbufrCorbaThread.h \
                     kvbufrd.hh kvbufrdSK.cc
                     
kvbufrclt_CPPFLAGS = $(AM_CPPFLAGS)
kvbufrclt_LDFLAGS = -pthread $(BOOST_LDFLAGS)
kvbufrclt_LDADD = $(kvcpp_LIBS) $(BOOST_THREAD_LIB) \
                   -lm  
                   
kvbufrdbclt_SOURCES = kvbufrdbclt.cc \
                       tblBufr.cc tblBufr.h
kvbufrdbclt_LDFLAGS = -pthread $(BOOST_LDFLAGS) 
kvbufrdbclt_LDADD = $(kvcpp_LIBS) $(BOOST_THREAD_LIB) \
                     -lm 
kvbufrdparse_SOURCES = kvbufrdparse.cc \
                        StationInfo.cc StationInfo.h \
                        StationInfoParse.cc StationInfoParse.h
kvbufrdparse_LDFLAGS = -pthread $(BOOST_LDFLAGS)
kvbufrdparse_LDADD =  $(kvcpp_LIBS) $(BOOST_THREAD_LIB) \
                     -lm
                        
kvbufrconf_SOURCES = StationInfo.cc StationInfo.h \
                     StationInfoParse.cc StationInfoParse.h \
                     tblStInfoSysParam.h tblStInfoSysParam.cc \
					 tblStInfoSysSensorInfo.h tblStInfoSysSensorInfo.cc \
					 tblStInfoSysStation.h tblStInfoSysStation.cc \
					 tblStInfoSysStationOutmessage.h tblStInfoSysStationOutmessage.cc \
					 checkfile.h checkfile.cc \
					 ConfMaker.h ConfMaker.cc \
					 ConfApp.h ConfApp.cc \
					 splitstr.h splitstr.cc \
					 InitLogger.cc InitLogger.h \
					 kvbufrconf.cc
kvbufrconf_LDFLAGS = -pthread $(BOOST_LDFLAGS)
kvbufrconf_LDADD = $(kvcpp_LIBS) $(BOOST_THREAD_LIB) \
                   -lm
             

sqldir=$(datadir)/kvbufrd
sql_DATA=kvbufrd_schema.sql
bin_SCRIPTS = kvbufrd_initdb

TESTS=kvBufrEncodeTest
#noinst_PROGRAMS=kvBufrEncodeTest

check_PROGRAMS=kvBufrEncodeTest

kvBufrEncodeTest_SOURCES = test/kvBufrEncodeTest.cc \
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
		                    bufrencodehelper.h \
		                    encodebufr.cc encodebufr.h \
		                    KvParam.cc KvParam.h \
		                    LoadBufrData.h LoadBufrData.cc

 
kvBufrEncodeTest_CPPFLAGS = $(AM_CPPFLAGS) -DTESTDATADIR="\""$(top_srcdir)/src/test"\""

kvBufrEncodeTest_LDADD =  -lgtest \
						  $(kvcpp_LIBS) \
						  -L$(HOME)/lib -lbufr -lgfortran \
						  -lm
						  