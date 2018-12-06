#!/usr/bin/env bash

ver_dir="$HOME/.verify"
trs_dir="$ver_dir/trusted"
tmp_dir="$ver_dir/tmp"
bin_dir="$ver_dir/bin"

mkdir -p $ver_dir
mkdir -p $trs_dir
mkdir -p $tmp_dir
mkdir -p $bin_dir
mkdir -p build
cd build/ && cmake ..
make

if [ $? -eq 0 ]; then
    cd .. && cp build/verify $bin_dir && cp build/libverifier.a $bin_dir
    echo ""
    echo "== Verify v0.0.1 =="
    echo "       Kidev"
    echo "==================="
    echo "Don't forget to run"
    echo "export PATH=\$PATH:$bin_dir"
    echo "for an easier access to the program"
else
    echo "Error while building"
fi

rm -rf build
