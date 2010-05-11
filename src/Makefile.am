AM_CPPFLAGS = -D_REENTRANT \
              $(kvcpp_CFLAGS) \
              $(BOOST_CPPFLAGS) \
              $(omniORB4_CFLAGS)  


nodist_idl_sources=kvbufferd.idl 


CLEANFILES = $(nodist_idl_sources:.idl=SK.cc) $(nodist_idl_sources:.idl=.hh) 


%SK.cc %.hh: %.idl
#	$(IDL) -bcxx -Wbinline -I$(KVIDLDIR) $<
	$(IDL) -bcxx -I$(KVIDLDIR) $<


noinst_PROGRAMS = kvBufferEncodeTest

bin_PROGRAMS = kvbufferd kvbufferclt kvbufferdbclt kvbufferdparse

kvbufferd_SOURCES = kvbufferd.hh kvbufferdSK.cc \
				   kvbufferd.cc \
                   App.cc App.h \
                   DataReceiver.cc DataReceiver.h \
                   Data.cc Data.h \
                   BufferWorker.cc BufferWorker.h \
                   BufferData.cc BufferData.h \
                   DataList.cc DataList.h \
                   StationInfo.cc StationInfo.h \
                   StationInfoParse.cc StationInfoParse.h \
                   tblBuffer.cc tblBuffer.h \
                   kvbufferdImpl.cc kvbufferdImpl.h \
                   Replay.cc Replay.h \
                   buffer.cc buffer.h \
                   Validate.cc Validate.h \
                   tblWaiting.cc tblWaiting.h \
                   Waiting.cc Waiting.h \
                   delaycontrol.cc delaycontrol.h \
                   tblKeyVal.cc tblKeyVal.h \
                   InitLogger.cc InitLogger.h \
                   getDataReceiver.cc getDataReceiver.h \
                   GetDataThread.cc GetDataThread.h \
                   kvbufferdImpl.h obsevent.h \
                   LoadBufferData.h LoadBufferData.cc 

kvbufferd_CPPFLAGS = $(AM_CPPFLAGS) 
kvbufferd_LDFLAGS = -pthread -export-dynamic $(BOOST_LDFLAGS)

kvbufferd_LDADD = $(kvcpp_LIBS) $(BOOST_THREAD_LIB) \
                 -lm         
EXTRA_DIST = kvbufferd_schema.sql
 
kvbufferclt_SOURCES = kvbufferclt.cc \
                     kvbufferCltBuffercbImp.cc kvbufferCltBuffercbImp.h \
                     kvbufferCltApp.cc  kvbufferCltApp.h \
                     kvbufferCorbaThread.cc kvbufferCorbaThread.h \
                     kvbufferd.hh kvbufferdSK.cc
                     
kvbufferclt_CPPFLAGS = $(AM_CPPFLAGS)
kvbufferclt_LDFLAGS = -pthread $(BOOST_LDFLAGS)
kvbufferclt_LDADD = $(kvcpp_LIBS) $(BOOST_THREAD_LIB) \
                   -lm  
                   
kvbufferdbclt_SOURCES = kvbufferdbclt.cc \
                       tblBuffer.cc tblBuffer.h
kvbufferdbclt_LDFLAGS = -pthread $(BOOST_LDFLAGS) 
kvbufferdbclt_LDADD = $(kvcpp_LIBS) $(BOOST_THREAD_LIB) \
                     -lm 
kvbufferdparse_SOURCES = kvbufferdparse.cc \
                        StationInfo.cc StationInfo.h \
                        StationInfoParse.cc StationInfoParse.h
kvbufferdparse_LDFLAGS = -pthread $(BOOST_LDFLAGS)
kvbufferdparse_LDADD =  $(kvcpp_LIBS) $(BOOST_THREAD_LIB) \
                     -lm
                        
sqldir=$(datadir)/kvbufferd
sql_DATA=kvbufferd_schema.sql
bin_SCRIPTS = kvbufferd_initdb

TESTS=kvBufferEncodeTest
#noinst_PROGRAMS=kvBufferEncodeTest

check_PROGRAMS=kvBufferEncodeTest

kvBufferEncodeTest_SOURCES = test/kvBufferEncodeTest.cc \
							test/kvBufferEncodeTestConf.h \
							test/ReadDataFile.h test/ReadDataFile.cc \
							Data.cc Data.h\
							DataList.cc DataList.h \
		                    BufferData.cc BufferData.h \
		                    StationInfo.cc StationInfo.h \
        		            StationInfoParse.cc StationInfoParse.h \
		                    buffer.cc buffer.h \
		                    Validate.cc Validate.h \
		                    InitLogger.cc InitLogger.h \
		                    LoadBufferData.h LoadBufferData.cc 
		                    
 
kvBufferEncodeTest_CPPFLAGS = $(AM_CPPFLAGS) -DTESTDATADIR="\""$(top_srcdir)/src/test"\""

kvBufferEncodeTest_LDADD = -lgtest \
						  $(kvcpp_LIBS)