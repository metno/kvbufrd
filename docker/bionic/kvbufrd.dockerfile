FROM registry.met.no/obs/kvalobs/kvbuild/staging/bionic-kvcpp-dev AS build
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y gfortran 

COPY compile config.h.in configure configure.ac config.sub config.guess \
  check_kvbufrd.sh.in changelog cleanbufrdb.sql \
  depcomp install-sh kvbufrdbadmin.sh   ltmain.sh Makefile.am Makefile.in missing \
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
COPY bufr-20200420.tar /src


WORKDIR /src

RUN ls -l && mkdir -p build && cd build && ../configure --prefix=/usr \
	--localstatedir=/var \
	--mandir=/usr/share/man \
	--with-boost-libdir=/usr/lib/x86_64-linux-gnu/ \
	LDFLAGS="-Wl,-z,defs"  &&\
  make && make install

ENTRYPOINT [ "/bin/bash" ]


FROM registry.met.no/obs/kvalobs/kvbuild/staging/bionic-kvcpp-runtime AS kvbufrd
ARG DEBIAN_FRONTEND=noninteractive
ARG kvuser=kvalobs
ARG kvuserid=5010

#Create a runtime user for kvalobs
RUN addgroup --gid $kvuserid $kvuser && \
  adduser --uid $kvuserid  --gid $kvuserid --disabled-password --disabled-login --gecos '' $kvuser

RUN apt-get update && apt-get install -y sqlite3 metno-bufrtables

RUN mkdir -p /etc/kvalobs && chown ${kvuser}:${kvuser}  /etc/kvalobs
RUN mkdir -p /var/log/kvalobs && chown ${kvuser}:${kvuser}  /var/log/kvalobs
RUN mkdir -p /var/lib/kvalobs/kvbufrd && chown ${kvuser}:${kvuser}  /var/lib/kvalobs/kvbufrd
RUN mkdir -p /var/lib/kvalobs/kvbufrd/bufr2norcom && chown ${kvuser}:${kvuser}  /var/lib/kvalobs/kvbufrd/bufr2norcom
RUN mkdir -p /var/lib/kvalobs/kvbufrd/debug && chown ${kvuser}:${kvuser}  /var/lib/kvalobs/kvbufrd/debug

COPY --from=build /usr/bin/kvbufrd  /usr/bin/
COPY --from=build /usr/share/kvbufrd /usr/share/kvbufrd/
COPY docker/entrypoint.sh /usr/bin/ 
RUN chmod +x /usr/bin/entrypoint.sh

VOLUME /etc/kvalobs
VOLUME /var/lib/kvalobs/kvbufrd
VOLUME /var/log/kvalobs

USER ${kvuser}:${kvuser}

ENTRYPOINT [ "tini", "--", "/usr/bin/entrypoint.sh" ]



FROM kvbufrd AS kvbufrd-svv
ARG DEBIAN_FRONTEND=noninteractive
ARG kvuser=kvalobs
ARG kvuserid=5010


COPY --from=build /usr/bin/kvbufrd  /usr/bin/kvbufrd-svv

VOLUME /etc/kvalobs
VOLUME /var/lib/kvalobs/kvbufrd
VOLUME /var/log/kvalobs

USER ${kvuser}:${kvuser}

ENTRYPOINT [ "tini", "--", "/usr/bin/entrypoint.sh" ]
