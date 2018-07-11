FROM ubuntu:latest

RUN apt-get update && apt-get upgrade -y \
    && apt-get install -y autoconf automake libtool build-essential cmake git subversion mercurial pkg-config \
    && apt-get autoremove -y && apt-get autoclean -y \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

CMD ["/bin/bash"]
