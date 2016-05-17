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
    cmake ..
    make all


