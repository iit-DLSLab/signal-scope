
default_cmake_args="-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} -DBUILD_SHARED_LIBS:BOOL=ON -DBUILD_DOCUMENTATION:BOOL=OFF -DENABLE_TESTING:BOOL=OFF"

function install_PythonQt
{
	git clone https://github.com/commontk/PythonQt.git
	cd PythonQt
	git checkout 00e6c6b2
	cd build
	cmake ${default_cmake_args} ../.
	make
	sudo make install
	cd ..
	cd ..
}

function install_ctkPythonConsole
{
	git clone https://github.com/patmarion/ctkPythonConsole.git
	cd ctkPythonConsole
	git checkout 15988c5
	mkdir build
	cd build
	cmake ${default_cmake_args} ../.
	make
	sudo make install
	cd ..
	cd ..
}

echo "Installing the dependencies for signal-scope"
install_PythonQt
install_ctkPythonConsole
rm -rf PythonQt
rm -rf ctkPythonConsole

