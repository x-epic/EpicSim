Summary: EpicSim
Name: EpicSim
Version: 1.0.0
Release: 0
License: LGPL
URL: https://www.x-epic.com/

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

Provides: EpicSim

%description
EpicSim

%files
%attr(-,root,root) /usr/bin/epicsim
%attr(-,root,root) /usr/bin/epicsim-vvp
%attr(-,root,root) /usr/bin/epicsim-driver
%attr(-,root,root) /usr/bin/epicsim-vpi
%attr(-,root,root) /usr/lib/epicsim/v2005_math.vpi
%attr(-,root,root) /usr/lib/epicsim/v2009.vpi
%attr(-,root,root) /usr/lib/epicsim/va_math.vpi
%attr(-,root,root) /usr/lib/epicsim/vhdl_sys.vpi
%attr(-,root,root) /usr/lib/epicsim/vhdl_textio.vpi
%attr(-,root,root) /usr/lib/epicsim/vhdlpp
%attr(-,root,root) /usr/lib/epicsim/vpi_debug.vpi
%attr(-,root,root) /usr/lib/epicsim/epicsim-compiler
%attr(-,root,root) /usr/lib/epicsim/epicsim-precompiler
%attr(-,root,root) /usr/lib/epicsim/libepicsim_generator.so
%attr(-,root,root) /usr/lib/epicsim/libepicsim_simu_core.a
%attr(-,root,root) /usr/lib/epicsim/vvp.conf
%attr(-,root,root) /usr/lib/epicsim/vvp-s.conf
%attr(-,root,root) /usr/lib/epicsim/system.vpi
%attr(-,root,root) /usr/lib/epicsim/include/constants.vams
%attr(-,root,root) /usr/lib/epicsim/include/disciplines.vams
%attr(-,root,root) /usr/lib/epicsim/libveriuser.a
%attr(-,root,root) /usr/lib/epicsim/libvpi.a
%attr(-,root,root) /usr/include/epicsim/ivl_target.h
%attr(-,root,root) /usr/include/epicsim/ivl_alloc.h
%attr(-,root,root) /usr/include/epicsim/vpi_user.h
%attr(-,root,root) /usr/include/epicsim/sv_vpi_user.h
%attr(-,root,root) /usr/include/epicsim/acc_user.h
%attr(-,root,root) /usr/include/epicsim/veriuser.h
%attr(-,root,root) /usr/include/epicsim/_pli_types.h
