AM_CPPFLAGS = \
	-D_REENTRANT \
	-I.. \
	$(kvcpp_CFLAGS) \
    $(BOOST_CPPFLAGS) 


noinst_LTLIBRARIES = libbufr.la


libbufr_la_SOURCES = \
	bufrdefs.h bufrexceptions.h \
    BufrValueHelper.h BufrValueHelper.cc \
	BufrHelper.cc BufrHelper.h \
    BUFRparam.h BUFRparam.cc \
    fixstring.cc fixstring.h \
     AreaDesignator.cc  AreaDesignator.h \
    GuessBufrTemplate.cc GuessBufrTemplate.h \
    EncodeBufrManager.cc EncodeBufrManager.h \
    EncodeBufrBase.h EncodeBufrBase.cc \
    EncodeBufr301004.cc EncodeBufr301004.h \
    EncodeBufr301011.cc EncodeBufr301011.h \
    EncodeBufr301012.cc EncodeBufr301012.h \
    EncodeBufr301021.cc EncodeBufr301021.h \
    EncodeBufr301036.cc EncodeBufr301036.h \
    EncodeBufr301089.cc EncodeBufr301089.h \
    EncodeBufr301090.cc EncodeBufr301090.h \
    EncodeBufr301093.cc EncodeBufr301093.h \
    EncodeBufr302001.cc EncodeBufr302001.h \
    EncodeBufr302004.cc EncodeBufr302004.h \
    EncodeBufr302005.cc EncodeBufr302005.h \
    EncodeBufr302021.cc EncodeBufr302021.h \
    EncodeBufr302023.cc EncodeBufr302023.h \
    EncodeBufr302024.cc EncodeBufr302024.h \
    EncodeBufr302031.cc EncodeBufr302031.h \
    EncodeBufr302032.cc EncodeBufr302032.h \
    EncodeBufr302033.cc EncodeBufr302033.h \
    EncodeBufr302034.cc EncodeBufr302034.h \
    EncodeBufr302035.cc EncodeBufr302035.h \
    EncodeBufr302036.cc EncodeBufr302036.h \
    EncodeBufr302037.cc EncodeBufr302037.h \
    EncodeBufr302038.cc EncodeBufr302038.h \
    EncodeBufr302039.cc EncodeBufr302039.h \
    EncodeBufr302040.cc EncodeBufr302040.h \
    EncodeBufr302041.cc EncodeBufr302041.h \
    EncodeBufr302042.cc EncodeBufr302042.h \
    EncodeBufr302043.cc EncodeBufr302043.h \
    EncodeBufr302052.cc EncodeBufr302052.h \
    EncodeBufr302053.cc EncodeBufr302053.h \
    EncodeBufr302054.cc EncodeBufr302054.h \
    EncodeBufr302055.cc EncodeBufr302055.h \
    EncodeBufr302056.cc EncodeBufr302056.h \
    EncodeBufr302057.cc EncodeBufr302057.h \
    EncodeBufr302058.cc EncodeBufr302058.h \
    EncodeBufr302059.cc EncodeBufr302059.h \
    EncodeBufr302060.cc EncodeBufr302060.h \
    EncodeBufr307079.cc EncodeBufr307079.h \
    EncodeBufr308009.cc EncodeBufr308009.h \
    EncodeBufr900000_SYNOP.cc EncodeBufr900000_SYNOP.h \
    EncodeBufr900001_SVV.cc EncodeBufr900001_SVV.h \
    EncodeBufr900002_PRECIP_AND_SNOW.cc EncodeBufr900002_PRECIP_AND_SNOW.h \
    EncodeBufr900003_SHIP.cc EncodeBufr900003_SHIP.h 

libbufr_la_CPPFLAGS = \
	$(AM_CPPFLAGS) -DDATADIR="\""$(pkgdatadir)/bufrtables"\"" \
	-D_REENTRANT \
	$(putools_CFLAGS)
	
#libbufr_la_LIBADD = 
	