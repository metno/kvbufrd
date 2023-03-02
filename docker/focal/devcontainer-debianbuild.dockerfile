FROM ubuntu:focal

#Add keys and repo
RUN apt-get update && apt-get install -y gnupg2 software-properties-common apt-utils
COPY docker/keys/internrepo-4E8A0C14.asc /tmp/
RUN apt-key add /tmp/internrepo-4E8A0C14.asc && rm /tmp/internrepo-4E8A0C14.asc && \
  add-apt-repository 'deb [arch=amd64] http://internrepo.met.no/focal focal main contrib'

RUN apt-get update && apt-get -y install \
  debhelper autotools-dev autoconf-archive debconf devscripts fakeroot \
  less mg \
  automake libtool gfortran bison flex sqlite3 libsqlite3-dev libpq-dev python3\
  libxml2-dev libxml++2.6-dev libboost-dev libboost-thread-dev \
  libboost-regex-dev libboost-filesystem-dev libboost-program-options-dev libboost-system-dev libboost-timer-dev \
  libomniorb4-dev  omniidl libperl-dev libdbd-pg-perl libcurl4-gnutls-dev liblog4cpp5-dev libcppunit-dev \
  cmake google-mock  zlib1g-dev libssl-dev libsasl2-dev libzstd-dev \
  librdkafka1 librdkafka++1 librdkafka-dev \
  libgmock-dev \
  libmicrohttpd-dev libgnutls28-dev \
  metlibs-putools-dev


# RUN apt-get install -y libgmock-dev language-pack-nb-base clang gdb libgeo-bufr-perl \
#   metno-bufrtables libkvcpp-dev 

RUN apt-get install -y libgmock-dev language-pack-nb-base clang gdb libgeo-bufr-perl \
  metno-bufrtables libkvcpp-dev less nano

RUN locale-gen en_US.UTF-8 nb_NO.UTF-8

RUN useradd -ms /bin/bash vscode
