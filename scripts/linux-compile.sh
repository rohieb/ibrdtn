#!/bin/bash -xe
#

DESTDIR="${HOME}/Projects/dtn/buildroot"
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:${DESTDIR}/lib/pkgconfig

COLOR="\033[01;31m"
NOCOLOR="\033[00m"

# set defaults
[ -z "${CLEAN}" ] && CLEAN=1
[ -z "${STRIP}" ] && STRIP=0
[ -z "${DEBUG}" ] && DEBUG=0
[ -z "${PROFILE}" ] && PROFILE=0

CXXFLAGS=""

if [ ${DEBUG} -eq 1 ]; then
    CXXFLAGS+=" -ggdb -g3 -Wall"
    STRIP=0
fi

if [ ${PROFILE} -eq 1 ]; then
    CXXFLAGS+=" -pg"
fi

# clean destdir
#rm -rf ${DESTDIR}

echo -e "${COLOR}Building ibrcommon...${NOCOLOR}"
cd ibrcommon
if [ ${CLEAN} -eq 1 ]; then
    [ -f Makefile ] && make clean
    bash autogen.sh
    ./configure --prefix=${DESTDIR} --with-openssl --with-lowpan
fi
make -j2
make install
cd ..

echo -e "${COLOR}Building ibrdtn/ibrdtn...${NOCOLOR}"
cd ibrdtn/ibrdtn
if [ ${CLEAN} -eq 1 ]; then
    [ -f Makefile ] && make clean
    bash autogen.sh
    ./configure --prefix=${DESTDIR} --with-dtnsec --with-compression
fi
make -j2
make install
cd ..

echo -e "${COLOR}Building ibrdtn/daemon...${NOCOLOR}"
cd daemon
if [ ${CLEAN} -eq 1 ]; then
    [ -f Makefile ] && make clean
    bash autogen.sh
    ./configure --prefix=${DESTDIR} --with-curl --with-lowpan --with-sqlite --with-dtnsec --with-compression --with-tls --with-cppunit 
fi
make -j2
make install
cd ..
cd ..

if [ ${STRIP} -eq 1 ]; then
echo -e "${COLOR}Stripping binaries...${NOCOLOR}"
# strip binaries
BINARIES="sbin/dtnd"
for BINARY in ${BINARIES}; do
    strip ${DESTDIR}/${BINARY}
done
fi

