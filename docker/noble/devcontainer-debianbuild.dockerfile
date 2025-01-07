#FROM ubuntu:noble
ARG REGISTRY="registry.met.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild/staging-noble/"
ARG BASE_IMAGE_TAG=latest

FROM ${REGISTRY}kvcpp-dev:${BASE_IMAGE_TAG}
ARG DEBIAN_FRONTEND='noninteractive'
ARG USER=vscode


RUN apt-get install -y language-pack-nb-base clang gdb libgeo-bufr-perl \
  metno-bufrtables less nano

RUN locale-gen en_US.UTF-8 nb_NO.UTF-8
RUN useradd -ms /bin/bash ${USER}

ENV USER=${USER}

