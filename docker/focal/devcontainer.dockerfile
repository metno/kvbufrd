
FROM registry.met.no/met/obsklim/kvalobs/kvbuild/staging/kvcpp-dev:latest

RUN apt-get install -y libgmock-dev language-pack-nb-base clang gdb libgeo-bufr-perl \
  metno-bufrtables less mg

RUN locale-gen en_US.UTF-8

RUN useradd -ms /bin/bash vscode
