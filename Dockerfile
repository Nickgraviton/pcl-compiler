FROM ubuntu:20.04

RUN apt-get update
RUN apt-get install -y make llvm clang flex bison && \
  apt-get clean

ADD src /compiler/src
ADD data /compiler/data
COPY compile.sh /compiler

WORKDIR "/compiler"

ENTRYPOINT ["/bin/sh"]
