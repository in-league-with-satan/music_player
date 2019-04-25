FROM ubuntu:latest
#FROM fedora:latest

#RUN dnf --setopt=install_weak_deps=False install -y gcc-c++ make autoconf automake libtool cmake git-core pkg-config && \
#RUN dnf --setopt=install_weak_deps=False install -y gcc-c++ make automake libtool git && \
#RUN dnf --setopt=install_weak_deps=False install -y gcc-c++ make git && \
#    dnf clean all
#    dnf remove *perl* && dnf clean all

RUN apt-get update && apt-get upgrade -y \
    && apt-get install -y autoconf automake libtool build-essential cmake git pkg-config \
    && apt-get autoremove -y && apt-get autoclean -y \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

CMD ["/bin/bash"]
