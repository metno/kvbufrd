FROM registry.met.no/obs/kvalobs/kvbuild/staging/focal-kvcpp-dev:latest

RUN apt-get install -y libgmock-dev language-pack-nb-base

RUN useradd -ms /bin/bash vscode
