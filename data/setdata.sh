#!/bin/bash
unlink gpuline.jacobian.data.bin 2>/dev/null; ln -sf ./$1/gpuline.jacobian.data.bin
unlink gpuline.mesh.data.bin     2>/dev/null; ln -sf ./$1/gpuline.mesh.data.bin
