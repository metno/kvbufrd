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

if MUST_COMPILE_GTEST
gtest_version = googletest-release-1.10.0
gtest_src = $(gtest_version).tar.gz


gmockdir=${checkdir}
gtestdir=${checkdir}
gmock_LIBRARIES = libgmock.a libgmock_main.a 
gtest_LIBRARIES = libgtest.a libgtest_main.a
libgmock_a_SOURCES = 
libgmock_main_a_SOURCES = 
libgtest_a_SOURCES = 
libgtest_main_a_SOURCES = 


lib_gtest:
	if [ ! -f $(gtest_version)-stamp ]; then \
		mkdir -p $(abs_top_builddir)/src/gtest ; \
		cp $(top_srcdir)/src/gtest/$(gtest_src) $(abs_top_builddir)/src/gtest ; \
		tar zxpf $(gtest_src) ; \
		ln -s $(gtest_version) gtest ; \
		mkdir -p build ; \
		( cd build ; \
		  cmake -DGTEST_HAS_PTHREAD=0 ../gtest && $(MAKE) ; \
			cp lib/*.a .. ; \
		) ; \
		touch $(gtest_version)-stamp ; \
	fi

libgmock.a: lib_gtest
	
libgmock_main.a: lib_gtest

libgtest.a: lib_gtest

libgtest_main.a: lib_gtest

endif

clean-local:
	rm -rf build
	rm -f gtest
	rm -rf googletest
