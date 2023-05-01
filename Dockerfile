FROM ubuntu:22.04

LABEL "author"="Yinyi (Shannon) LIU"
LABEL "version"="0.1"
LABEL "about"="Docker env for running systemverilog-python interfaces, supporting svOpenArrayHandle & nested list (numpy), respectively."
LABEL "usage"="kindly use -v <YOUR_PRJECT>:/home/dev/projects/<PROJECT_NAME> to load your project into container"
LABEL "account"="[USER:PASSWD] root:root, dev:dev"

ENV container docker
ENV HOME /home/dev/

WORKDIR ${HOME}/projects

RUN echo 'APT::Get::Assume-Yes "true";' > /etc/apt/apt.conf.d/90-yes && apt-get update -qq && \
    useradd -d ${HOME} -m -G root dev && \
    echo root:root | chpasswd && echo dev:dev | chpasswd && \
    apt-get install -y build-essential make cmake python3 python3-dev python3-pip && \
    apt-get install -y git perl make autoconf g++ flex bison ccache && \
    apt-get install -y libgoogle-perftools-dev numactl perl-doc help2man && \
    apt-get install -y libfl2 libfl-dev zlib1g zlib1g-dev && apt-get clean && \
    mkdir -p ${HOME}/depends/ ${HOME}/projects/ && \
    cd ${HOME}/depends && git clone https://github.com/verilator/verilator && \
    cd ${HOME}/depends/verilator && autoconf && ./configure && make -j`nproc` && \
    make install && make clean && \
    cd ${HOME} && chown -R dev:dev ${HOME}

USER dev
ENTRYPOINT [ "/bin/bash" ]