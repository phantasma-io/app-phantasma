#!/bin/bash
reset;
cd .. &&
make &&
cd unit-tests &&
cmake -Bbuild -H. && make -C build &&
CTEST_OUTPUT_ON_FAILURE=1 make -C build test;
