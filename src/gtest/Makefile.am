# if MUST_COMPILE_GTEST
# gtestdir=${checkdir}
# gtest_LIBRARIES = libgtest.a libgtest_main.a

# libgtest_a_SOURCES = 
# libgtest_main_a_SOURCES = 

# lib_gtest:
# 	mkdir -p gtest
# 	cd gtest; $(CMAKE) $(gtest_src) && $(MAKE)
		
# libgtest.a: lib_gtest
# 	cp gtest/$@ .

# libgtest_main.a: lib_gtest
# 	cp gtest/$@ .
# endif

if MUST_COMPILE_GMOCK
gmockdir=${checkdir}
gtestdir=${checkdir}
gmock_LIBRARIES = libgmock.a libgmock_main.a 
gtest_LIBRARIES = libgtest.a libgtest_main.a
libgmock_a_SOURCES = 
libgmock_main_a_SOURCES = 
libgtest_a_SOURCES = 
libgtest_main_a_SOURCES = 


lib_gmock:
	mkdir -p gmock
	cd gmock; $(CMAKE) $(gmock_src) && $(MAKE)
		
libgmock.a: lib_gmock
	if [ -f gmock/$@ ]; then \
		cp gmock/$@ . ; \
	elif [ -f gmock/googlemock/$@ ]; then \
		cp gmock/googlemock/$@ . ; \
	fi

libgmock_main.a: lib_gmock
	if [ -f gmock/$@ ]; then \
		cp gmock/$@ . ; \
	elif [ -f gmock/googlemock/$@ ]; then \
		cp gmock/googlemock/$@ . ; \
	fi


libgtest.a: lib_gmock
	if [ -f gtest/$@ ]; then \
		cp gtest/$@ . ; \
	elif [ -f gmock/googlemock/$@ ]; then \
		cp gmock/googlemock/$@ . ; \
	fi

libgtest_main.a: lib_gmock
	if [ -f gmock/$@ ]; then \
		cp gmock/$@ . ; \
	elif [ -f gmock/googlemock/$@ ]; then \
		cp gmock/googlemock/$@ . ; \
	fi


endif

clean-local:
	rm -rf gmock
	rm -rf gtest
