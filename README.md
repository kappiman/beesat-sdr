Software Defined Radio TNC for BEESAT
=====================================

## Contributions
Many thanks to Daniel Estevez (EA4GPZ) who contributed very important fixes and ideas to this repository.

## Installation of gnuradio on Ubuntu/Debian
The following installation instructions were tested under Ubuntu 14.04 and Debian Jessie.

### Ubuntu prerequisite: PPA with recent version of gnuradio:

```bash
sudo apt-add-repository ppa:myriadrf/gnuradio
sudo apt-get update
```

### Installation

Remark:
  Please pay attention that you install swig right from the beginning.
  The build process might be successfull without having swig installed,
  but the compiled and installed module won't work as expected.

```bash
sudo apt-get install gnuradio swig
```

## Compiling and installtion of the BEESAT SDR

Change to the build directory of the sdr tnc package:
```bash
cd beesat-sdr/gr-tnc_nx/build
```

Invoke cmake, additionally give a specific install path:
```bash
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local ../
```

## Compile with make

Compile the package and install it:
```bash
make -j4
sudo make install
sudo ldconfig
```

## GRC files
For decoding BEESAT-2 telemetry start gnuradio-companion and open the file:
```
beesat-sdr/grc/tnc_nx.grc
```

For decoding of BEESAT-1 messages, the file
```
beesat-sdr/grc/tnc_b1.grc
```
must be used.

Both software TNCs allow the input of an wav file for testing purposes.

