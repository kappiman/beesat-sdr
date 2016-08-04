Install gnuradio under Ubuntu by using a ppa:

sudo apt-add-repository ppa:myriadrf/gnuradio
sudo apt-get update
sudo apt-get install gnuradio swig

Remark:
  Please pay attention that you install swig right from the beginning.
  The build process might be successfull without having swig installed,
  but the compiled and installed module won't work as expected.

Change to the build directory of the sdr tnc package:
cd sdr-tnc/gr-tnc_nx/build

Invoke cmake, additionally give a specific install path:
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ../

You may get an error like this, when cmake checks for the gnuradio runtime library:

Checking for GNU Radio Module: RUNTIME
-- checking for module 'gnuradio-runtime'
--   found gnuradio-runtime, version 3.7.9
 * INCLUDES=
 * LIBS=gnuradio-runtime;gnuradio-pmt;/usr/lib/x86_64-linux-gnu/libgnuradio-runtime.so;/usr/lib/x86_64-linux-gnu/libgnuradio-pmt.so
-- Could NOT find GNURADIO_RUNTIME (missing:  GNURADIO_RUNTIME_INCLUDE_DIRS) 
GNURADIO_RUNTIME_FOUND = FALSE

Although everything is properly installed the pkg-config tool returns an empty INCLUDE dir because a standard one is used.

Do a workaround and edit the file gnuradio-runtime.pc shipped with the gnuradio-dev package:
Under Ubuntu you may use gedit:
sudo gedit /usr/lib/x86_64-linux-gnu/pkgconfig/gnuradio-runtime.pc
Under Debian:
sudo vim   /usr/lib/i386-linux-gnu/pkgconfig/gnuradio-runtime.pc

Look for last line, which should be:
Cflags: -I${includedir}
change it to:
Cflags: -I${includedir} -IXXX

save it, clear cmake cache and invoke cmake again:
rm CMakeCache.txt
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ../

Compile the package:
make -j4
sudo make install
sudo ldconfig

Start gnuradio-companion and open the software tnc
