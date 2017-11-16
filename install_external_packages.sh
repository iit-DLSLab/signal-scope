#!/bin/bash

default_cmake_args="-DCMAKE_BUILD_TYPE:STRING=${Release} -DBUILD_SHARED_LIBS:BOOL=ON -DBUILD_DOCUMENTATION:BOOL=OFF -DENABLE_TESTING:BOOL=OFF"
PID="/usr/include/python2.7/"
PL="/usr/lib/x86_64-linux-gnu/libpython2.7.so"
python_args="-DPYTHON_INCLUDE_DIR:PATH=${PID} -DPYTHON_INCLUDE_DIR2:PATH=${PID} -DPYTHON_LIBRARY:PATH=${PL}"

function install_PythonQt
{
	git clone -b patched-6 https://github.com/commontk/PythonQt.git
	cd PythonQt
	cd build
	cmake ${default_cmake_args} -DPythonQt_Wrap_Qtcore:BOOL=ON -DPythonQt_Wrap_Qtgui:BOOL=ON -DPythonQt_Wrap_Qtuitools:BOOL=ON ../.
	make
	sudo make install
	cd ../../
}

function install_ctkPythonConsole
{
	git clone https://github.com/patmarion/ctkPythonConsole.git
	cd ctkPythonConsole
	git checkout 15988c5
	mkdir build
	cd build
	cmake ${default_cmake_args} ${python_args} ../.
	make
	sudo make install
	cd ../../
}

echo "Installing the dependencies for signal-scope"

# Removing the folder if it's already existing for some reason
if [ -d PythonQt ]; then
	rm -rf PythonQt
fi
install_PythonQt
rm -rf PythonQt

# Removing the folder if it's already existing for some reason
if [ -d ctkPythonConsole ]; then
	rm -rf ctkPythonConsole
fi
install_ctkPythonConsole
rm -rf ctkPythonConsole

