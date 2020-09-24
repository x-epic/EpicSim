#! /bin/bash
# build EpicSim Release rpm for user.
# only tested on centos 7 & 8 now. 
# both x86-64 and arm is supported.

SCRIPTDIR=`dirname $0`
ARCH=`arch`
rm -rf ~/rpmbuild/BUILDROOT/EpicSim-1.0.0-0.${ARCH}/*

if [ ! -d "${SCRIPTDIR}/../install" ]; then
    echo "no install dir, quit.."
    exit 1
fi

mkdir -p ~/rpmbuild/BUILDROOT/EpicSim-1.0.0-0.${ARCH}/usr
cp -rf ${SCRIPTDIR}/../install/* ~/rpmbuild/BUILDROOT/EpicSim-1.0.0-0.${ARCH}/usr

#replace conf VVP_EXECUTABLE
if [ -f ~/rpmbuild/BUILDROOT/EpicSim-1.0.0-0.${ARCH}/usr/lib/epicsim/vvp.conf ]; then
    sed -i '/VVP_EXECUTABLE/d' ~/rpmbuild/BUILDROOT/EpicSim-1.0.0-0.${ARCH}/usr/lib/epicsim/vvp.conf
    echo 'flag:VVP_EXECUTABLE=/usr/bin/epicsim-vvp' >> ~/rpmbuild/BUILDROOT/EpicSim-1.0.0-0.${ARCH}/usr/lib/epicsim/vvp.conf
fi
if [ -f ~/rpmbuild/BUILDROOT/EpicSim-1.0.0-0.${ARCH}/usr/lib/epicsim/vvp-s.conf ]; then
    sed -i '/VVP_EXECUTABLE/d' ~/rpmbuild/BUILDROOT/EpicSim-1.0.0-0.${ARCH}/usr/lib/epicsim/vvp-s.conf
    echo 'flag:VVP_EXECUTABLE=/usr/bin/epicsim-vvp' >> ~/rpmbuild/BUILDROOT/EpicSim-1.0.0-0.${ARCH}/usr/lib/epicsim/vvp-s.conf
fi

rpmbuild -bb ${SCRIPTDIR}/epicsim.spec