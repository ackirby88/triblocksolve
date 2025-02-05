# Tri-diagonal matrix line solver on GPUs

## Dependencies
- OCCA
- CMAKE - 3.17 or higher

## Configure and Build (serial mode on CPU)
```
./makescript.sh
```
Use option "-help" to see all available options.

## Configure and Build Device (e.g., HIP)
```
./makescript.sh -hip HIP_ROOT=<path_to_hip_library>
```

## Executable and Helper Scripts
After compiling the code, there will be a "bin" directory, which contains the following executables:   
```
triblock.exe    <main executable>  
setdata.sh      <script to set symbolic links (gpuline.jacobian.data.bin & gpuline.mesh.data.bin)>

```

Internally, the program reads example data sets:  
  1. Mesh File from "gpuline.mesh.data.bin"  
  2. Jacobian Matrix values from "gpuline.jacobian.data.bin"  

The script "setdata.sh" makes symbolic links with these names based on the example case:  
     NAME : # MESH ELEM :  # MESH LINES : Jacobian Data Size  
  1. ex05 :       3,840 :            60 :  13M  
  2. ex06 :     138,240 :           360 : 446M  
  3. ex10 :     384,000 :           600 : 1.3G  
  4. ex20 :   1,536,000 :         1,200 : 4.9G  

Example Usage of setdata.sh (e.g., mesh ex05):  
```
./setdata.sh ex05
```

## Running the Executable (triblock.exe)
```
./triblock.exe --help  # displays help message
>>> ./triblock.exe <block_size> <compute_mode> <device_id>
```



