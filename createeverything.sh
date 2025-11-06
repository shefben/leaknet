#!/bin/bash

cmake -S . -B build -DCMAKE_C_FLAGS="-m32" -DCMAKE_CXX_FLAGS="-m32" -DBUILD_GROUP="everything" $@
