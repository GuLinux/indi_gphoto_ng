INDI GPhoto Driver - Next Generation
====================================

This is a full rewrite of the original indi gphoto driver, making use of modern c++11 programming style.
It is still in a very early stage, and lots of feature of the original driver are missing.

Get the sources
---------------

    git clone https://github.com/GuLinux/indi_gphoto_ng.git
    cd indi_gphoto_ng
    git submodule init && git submodule update # Initialize submodules.


Compilation
-----------

    cd indi_gphoto_ng
    mkdir build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr
    make all


Running
-------

The driver itself must be launched by indiserver.
If correctly installed, it will appear as "GPhoto NG CCD" in your indi client devices menu.
For testing purpouses, you may launch it manually:

    indiserver -v indi_gphoto_ng_ccd
