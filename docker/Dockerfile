FROM centos:latest

RUN yum -y update && \
yum -y install \
gcc-c++ \
make \
tar \
unzip \
libicu-devel \
libuuid-devel \
perl

ADD https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip /
RUN cd /usr/local/bin && \
unzip /ninja-linux.zip && \
rm /ninja-linux.zip

ADD https://cmake.org/files/v3.13/cmake-3.13.0.tar.gz /.
RUN tar -zxf cmake-3.13.0.tar.gz && \
cd cmake-3.* && \
./bootstrap && \
make && \
make install && \
cd .. && rm -rf cmake-*

RUN yum -y install centos-release-scl && \
yum -y install devtoolset-7

CMD ["/bin/sleep","infinity"]
