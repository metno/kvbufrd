#syntax=docker/dockerfile:1.2

ARG REGISTRY
ARG BASE_IMAGE_TAG=

#FROM registry.met.no/obs/kvalobs/kvbuild/staging/focal-kvcpp-dev AS build
FROM ${REGISTRY}focal-kvcpp-dev:${BASE_IMAGE_TAG} AS build
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y gfortran gpg software-properties-common apt-utils

COPY configure.ac  \
  check_kvbufrd.sh.in changelog cleanbufrdb.sql \
  kvbufrdbadmin.sh Makefile.am Makefile.in  \
  test-driver \
  AUTHORS NEWS README \
  /src/

COPY debian_files /src/debian_files
COPY ecmwf /src/ecmwf
COPY etc /src/etc
COPY m4 /src/m4
COPY mk /src/mk
COPY share /src/share
COPY src /src/src 
#COPY bufr-20200420.tar.gz /src

VOLUME /src
VOLUME /build
WORKDIR /build

# RUN --mount=type=cache,target=/build cd /src/ && autoreconf -if && cd /build && \
#       /src/configure --prefix=/usr --mandir=/usr/share/man --infodir=/usr/share/info  \
# 	    --localstatedir=/var --sysconfdir=/etc  \
#       CFLAGS=-g && make && make install


ENTRYPOINT [ "/bin/bash" ]

RUN --mount=type=cache,target=/build cd /src \
  && autoreconf -if && cd /build &&\
  /src/configure \
  --prefix=/usr \
 	--localstatedir=/var \
 	--mandir=/usr/share/man \
   --sysconfdir=/etc \
 	--with-boost-libdir=/usr/lib/x86_64-linux-gnu/ \
 	LDFLAGS="-Wl,-z,defs" CFLAGS=-g &&\
  make && make install

# ENTRYPOINT [ "/bin/bash" ]


#Get metno-bufrtables from the repository
FROM ubuntu:focal AS bufrtables
RUN apt-get update && apt-get install -y git
#Get the metno-bufrtables

RUN mkdir -p /usr/share/metno-bufrtables/tmp && cd /usr/share/metno-bufrtables/tmp && \
  git clone https://gitlab.met.no/it-geo/metno-bufrtables.git && \
  cp metno-bufrtables/bufrtables/* .. && cd .. && rm -rf tmp/



FROM ${REGISTRY}focal-kvcpp-runtime:${BASE_IMAGE_TAG} AS kvbufrd
ARG DEBIAN_FRONTEND=noninteractive
ARG kvuser=kvalobs
ARG kvuserid=5010

RUN apt-get install -y gpg software-properties-common apt-utils

#Add intertn repos and postgres repo 
RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 4e8a0c1418494cf45d1b8533799e9fe74bb0156c &&\
  add-apt-repository 'deb [arch=amd64] http://internrepo.met.no/focal focal main contrib'

RUN apt-get update && apt-get install -y sqlite3 libgfortran5
#metno-bufrtables

RUN apt-get install -y gdb

#Create a runtime user for kvalobs
RUN useradd -ms /bin/bash --uid ${kvuserid} --user-group  ${kvuser}

RUN mkdir -p /etc/kvalobs && chown ${kvuser}:${kvuser}  /etc/kvalobs
RUN mkdir -p /var/log/kvalobs && chown ${kvuser}:${kvuser}  /var/log/kvalobs
RUN mkdir -p /var/lib/kvbufrd && chown ${kvuser}:${kvuser}  /var/lib/kvbufrd
RUN mkdir -p /var/lib/kvalobs/kvbufrd/bufr2norcom && chown ${kvuser}:${kvuser}  /var/lib/kvalobs/kvbufrd/bufr2norcom
RUN mkdir -p /var/lib/kvalobs/kvbufrd/debug && chown ${kvuser}:${kvuser}  /var/lib/kvalobs/kvbufrd/debug

COPY --from=build /usr/bin/kvbufrd  /usr/bin/
COPY --from=build /usr/share/kvbufrd /usr/share/kvbufrd/
COPY docker/entrypoint.sh /usr/bin/ 

#Create a link for kvbufrd-svv
RUN ln -s /usr/bin/kvbufrd /usr/bin/kvbufrd-svv

RUN chmod +x /usr/bin/entrypoint.sh

#Get the metno-bufrtables
RUN mkdir -p /usr/share/metno-bufrtables
COPY --from=bufrtables /usr/share/metno-bufrtables/* /usr/share/metno-bufrtables/

COPY GITREF /usr/share/kvbufrd/VERSION

VOLUME /etc/kvalobs
VOLUME /var/lib/kvalobs/kvbufrd
VOLUME /var/log/kvalobs

RUN mkdir /cores && chmod 0777 /cores

VOLUME /cores


USER ${kvuser}:${kvuser}

ENTRYPOINT ["/usr/bin/entrypoint.sh" ]


# FROM kvbufrd AS kvbufrd-svv
# ARG DEBIAN_FRONTEND=noninteractive
# ARG kvuser=kvalobs
# ARG kvuserid=5010


# COPY --from=build /usr/bin/kvbufrd  /usr/bin/kvbufrd-svv

# VOLUME /etc/kvalobs
# VOLUME /var/lib/kvalobs/kvbufrd
# VOLUME /var/log/kvalobs

# USER ${kvuser}:${kvuser}

# ENTRYPOINT ["/usr/bin/entrypoint.sh" ]
