# Tri-Diagonal Block-Matrix Line Solver on GPUs
Line-Implicit Jacobi (LĲ) Block Solver implemented for GPUs via OCCA (https://github.com/libocca/occa).  

This source code was used to generate the results for [2025 AIAA SciTech Conference Paper](https://arc.aiaa.org/doi/abs/10.2514/6.2025-0784):  
```python
"GPU Acceleration of a Two-Dimensional Adjoint-Enabled Real Gas Hypersonic Flow Solver",
Andrew C. Kirby and Dimitri J. Mavriplis,
AIAA 2025-0784,
Session: High-Order Mesh Adaptation and HPC,
Published Online: 3 Jan 2025.
```

## Dependencies
- CMAKE - 3.17 or higher
- OCCA (as git submodule)

## Configure and Build using `./makescript.sh`
![image](https://github.com/user-attachments/assets/544977c4-f68e-448c-8412-4cfe497e68b9)


## Executable and Helper Scripts
After compiling the code, there will be a `bin` directory containing:   

![image](https://github.com/user-attachments/assets/a690f767-71ae-4443-8e7b-6e00d9b44992)


Internally, the program reads data sets from the directories (e.g., `ex10`):  
  1. Mesh File from `gpuline.mesh.data.bin`  
  2. Matrix values from `gpuline.jacobian.data.bin`  

Running `./setdata.sh ex05` sets these symbolic links as seen in the image above.  

## Example Data Sets
Four data sets are available via Git LFS (**PENDING**). These data sets contain Jacobian matrix values generated from a 2D real-gas hypersonic flow solver using a 5-species, 2-temperature gas model for non-ionizing air. The number of variables of each mesh block is of size 9x9, thus for the program input, we say `block_size = 9`. The data sets (meshes) available for benchmarking are listed below. 

| NAME | # MESH ELEM | # MESH LINES | Jacobian Data Size |
| ---- | -----------:| ------------:|:------------------:|
| ex05 |       3,840 |           60 |  13M               |
| ex06 |     138,240 |          360 | 446M               |
| ex10 |     384,000 |          600 | 1.3G               |
| ex20 |   1,536,000 |        1,200 | 4.9G               |

## Running `./triblock.exe <compute_mode> <device_id> <block_size>`  

Solving the matrix problem using:  
| Mesh | <compute_mode> = `1` |  <device_id> = `0` | <block_size> = `9` |
| ---- |:--------------:|:------------------------:|:------------------:|
| ex05 |     HIP Mode   |    AMD RADEON PRO VII    |  9x9 matrix blocks |

![image](https://github.com/user-attachments/assets/83ec256c-77fb-47eb-ad41-e706a2896f80)

## Credit/Citation
Please cite [doi:10.2514/6.2025-0784](https://doi.org/10.2514/6.2025-0784)  
```console
@inproceedings{kirby2025gpu,
  title={GPU Acceleration of a Two-Dimensional Adjoint-Enabled Real Gas Hypersonic Flow Solver},
  author={Kirby, Andrew C and Mavriplis, Dimitri J},
  booktitle={AIAA SCITECH 2025 Forum},
  note={AIAA Paper 2025-0784},
  year={2025}
}
```


