#!/bin/bash

cmake -S . -B build_dedicated -DCMAKE_C_FLAGS="-m32" -DCMAKE_CXX_FLAGS="-m32" -DDEDICATED=ON $@
