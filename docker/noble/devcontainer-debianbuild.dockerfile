#FROM ubuntu:noble
ARG REGISTRY="registry.met.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild/staging-noble/"
ARG BASE_IMAGE_TAG=latest

FROM ${REGISTRY}kvcpp-dev:${BASE_IMAGE_TAG}
ARG DEBIAN_FRONTEND='noninteractive'
ARG USER=vscode

COPY docker/keys/internrepo-4E8A0C14.asc  /tmp/
RUN apt-get update && apt-get install -y gnupg2 software-properties-common apt-utils

RUN apt-key add /tmp/internrepo-4E8A0C14.asc && rm /tmp/internrepo-4E8A0C14.asc && \
  add-apt-repository 'deb [arch=amd64] http://internrepo.met.no/noble noble main contrib'

RUN apt-get update && apt-get install -y language-pack-nb-base clang gdb libgeo-bufr-perl \
  metno-bufrtables less nano libsqlite3-dev libkvcpp-dev

RUN locale-gen en_US.UTF-8 nb_NO.UTF-8
RUN useradd -ms /bin/bash ${USER}

ENV USER=${USER}

