#!/bin/bash

read -r -p "
#####################################################################
#!!!Warning!!!：                                                    #
#                                                                   #
#  This script is used for Building/Installing EpicSim From Source  #
#  																	#
#   Step 1:Download Sourcecode.										#
#   Step 2:Backup Repo and Install devDependencies.					#
#   Step 3:Building and Installing EpicSim.							#
#   Step 4:Set up the EpicSim Environment Variable.					#
#   Step 5:Install Successfully!!									#
#   																#
#   Test:  Run “epicsim -v” to have a test.							#
#                                                     	            #
#                                      https://www.edagit.com/   	#
#																	#
#####################################################################

Start to Build and Install, Are You Sure ? [Y/n]" input

case $input in
    [yY][eE][sS]|[yY])
                #Download Sourcecode
                yum install -y wget unzip
                wget https://gitee.com/x-epic/EpicSim/repository/archive/master.zip
                unzip master.zip
                
                #Backup "epel repo"
                repo1="/etc/yum.repos.d/epel.repo"
                repo2="/etc/yum.repos.d/epel-testing.repo"
                if [ -f "$repo1" ]; then
                  echo "rename epel.repo to epel.repo.backup"
                  mv /etc/yum.repos.d/epel.repo /etc/yum.repos.d/epel.repo.backup
                fi
                if [ -f "$repo2" ]; then
                  echo "rename epel-testing.repo to epel-testing.repo.backup"
                  mv /etc/yum.repos.d/epel-testing.repo /etc/yum.repos.d/epel-testing.repo.backup
                fi

                #Install devDependencies
                stat=`cat  /etc/redhat-release|sed -r 's/.* ([0-9]+)\..*/\1/'`
                if [ $stat == 6 ];then
                  yum install -y gperf
                  wget -O /etc/yum.repos.d/epel.repo http://mirrors.aliyun.com/repo/epel-6.repo
                  ln -s /bin/bash /usr/bin/bash
                  yum install -y cmake3 make bison flex bzip2-devel zlib zlib-devel ncurses-devel centos-release-scl
                  yum install -y devtoolset-7-gcc devtoolset-7-gcc-c++ devtoolset-7-binutils
                  echo "source /opt/rh/devtoolset-7/enable" >>/etc/profile
                  source /etc/profile
                elif [ $stat == 7  ];then
                  wget -O /etc/yum.repos.d/epel.repo http://mirrors.aliyun.com/repo/epel-7.repo
                  yum install -y gperf cmake3 make gcc-c++ gcc bison flex bzip2-devel zlib zlib-devel ncurses-devel
                elif [ $stat == 8  ];then
                  dnf --enablerepo=PowerTools install -y gperf
                  yum install -y cmake3 make gcc-c++ gcc bison flex bzip2-devel zlib zlib-devel ncurses-devel
                else
                  echo "The version of your system is not adapted yet. The version is:"$stat
                fi

                #Building and Installing EpicSim
                cd EpicSim
                mkdir build && cd build
                cmake3 ..
                make install

                #Set up the EpicSim environment variable
                cd ..
                echo "export PATH=$PATH:`pwd`/install/bin/" >> /etc/profile
                source /etc/profile

                #run "epicsim -v" have a test
                echo "Install successfully,Pleae run “epicsim -v” have a test"
                ;;

    [nN][oO]|[nN])
                echo "Goodbye!"
                ;;

    *)
        echo "Invalid input... Please retry!"
        exit 1
        ;;
esac



