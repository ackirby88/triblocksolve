/**
 * File:   main.hxx
 * Author: akirby
 *
 * Created on February 4, 2025
 */

/* header files */
#include "Platform.hxx"
#include "Jacobian.hxx"
#include "Mesh.hxx"

int main(int argc,char **argv){
    occa::streamTag start,end;

    /* Default Values */
    int compute_mode = SERIAL_MODE;
    int block_size = 9;
    int device_id = 0;
    int iters = 30;

    /* initialize MPI */
    MPI_Init(&argc,&argv);

    /* ===================== */
    /* Parse Input Arguments */
    /* ===================== */
    std::cout << "+================================================================================+" << std::endl;
    printf(" >>>>         " GREEN "./triblock.exe" COLOR_OFF " <compute_mode> <device_id> <block_size>         <<<< \n" COLOR_OFF);
    printf(SPACEBLK1 "<compute_mode> SERIAL (%d): enabled=? %d\n",SERIAL_MODE,occa::modeIsEnabled("Serial"));
    printf(SPACEBLK2 "   HIP (%d): enabled=? %d\n",   HIP_MODE,occa::modeIsEnabled("HIP"));
    printf(SPACEBLK2 "  CUDA (%d): enabled=? %d\n",  CUDA_MODE,occa::modeIsEnabled("CUDA"));
    printf(SPACEBLK2 "OpenCL (%d): enabled=? %d\n",OPENCL_MODE,occa::modeIsEnabled("OpenCL"));
    printf(SPACEBLK2 "OpenMP (%d): enabled=? %d\n",OPENMP_MODE,occa::modeIsEnabled("OpenMP"));
    printf(SPACEBLK2 " DPC++ (%d): enabled=? %d\n", DPCPP_MODE,occa::modeIsEnabled("dpcpp"));
    printf(SPACEBLK2 " Metal (%d): enabled=? %d\n", METAL_MODE,occa::modeIsEnabled("Metal"));

    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "--help" || arg1 == "-help") {
            // Print usage information
            std::cout <<              "Arguments:\n"
                         "  compute_mode: 0=Serial, 1=HIP, 2=CUDA, 3=OpenCL, 4=OpenMP, 5=DPC++, 6=Metal (apple)\n"
                         "  device_id:    Device ID on node\n"
                         "  block_size:   Size of the matrix sub-block (i.e., # of variables/eqns; e.g., 9x9 default)\n";
            std::cout << "+================================================================================+" << std::endl;
            MPI_Finalize();
            return 0;
        }
    }

    if(argc > 1) compute_mode = std::stoi(argv[1]);
    if(argc > 2) device_id    = std::stoi(argv[2]);
    if(argc > 3) block_size   = std::stoi(argv[3]);
    int nvar = block_size;

    std::cout << " -------------------------------------------------------------------------------- " << std::endl;
    std::cout << GREEN " TRIDIAGONAL BLOCK SOLVER: "
              << COLOR_OFF "Block size: "     GREEN << block_size
              << COLOR_OFF ", Compute mode: " GREEN <<
                    ((compute_mode == SERIAL_MODE) ? "SERIAL MODE":
                     (compute_mode ==    HIP_MODE) ?    "HIP MODE":
                     (compute_mode ==   CUDA_MODE) ?   "CUDA MODE":
                     (compute_mode == OPENCL_MODE) ? "OpenCL MODE":
                     (compute_mode == OPENMP_MODE) ? "OpenMP MODE":
                     (compute_mode ==  DPCPP_MODE) ?  "DPC++ MODE":
                     (compute_mode ==  METAL_MODE) ?  "Metal MODE":"UNKNOWN")
              << COLOR_OFF ", Device ID: " GREEN << device_id
              << COLOR_OFF << std::endl;
    std::cout << " -------------------------------------------------------------------------------- " << std::endl;

    /* display available OCCA Mode and Device information available */
    std::cout << "    OCCA DEVICE INFO:" << std::endl;
    occa::printModeInfo();
    std::cout << "+================================================================================+" << std::endl;

    /* ========================== */
    /* Initialize Device and Data */
    /* ========================== */
    Platform gpu(MPI_COMM_WORLD,compute_mode,device_id);

    Mesh mesh;
    mesh.fromFile();
    mesh.setupDevice(gpu);
    mesh.toDevice();
    gpu.device.finish();

    Jacobian Jac;
    Jac.fromFile(mesh.nlineelem);
    Jac.assembleTriBlocks(mesh);
    Jac.resizeBlockSize(nvar);
    Jac.setupDevice(gpu);
    Jac.toDevice();
    gpu.device.finish();

    std::cout << "=========================================\n";
    std::cout << "GPU Memory Allocated (MB): "
              << (gpu.device.memoryAllocated()/1024./1024.)
              << std::endl;
    std::cout << "=========================================\n";

    /* ======================================= */
    /* Build Compute Kernels (JIT-compilation) */
    /* ======================================= */
    occa::properties kernelProps;
    kernelProps["defines/NVAR"] = nvar;
    kernelProps["defines/MAX_LINE_ELEM"] = mesh.max_line_nelem;
    kernelProps["defines/p_Nblock"] = (nvar+9-1)/9;
    printf("p_Nblock = %d\n",(nvar+9-1)/9);


    std::cout << GREEN "Compiling Device Kernels..." COLOR_OFF;
    double t1 = MPI_Wtime();

    /* utility functions */
    occa::kernel copyAtoBjac = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_v5.okl","copyAtoBjac",kernelProps);
    occa::kernel copyAtoB    = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_v5.okl","copyAtoB",kernelProps);
    occa::kernel addAtoB     = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_v5.okl","addAtoB",kernelProps);

    /* line factorization */
    //occa::kernel lineLU_v1   = gpu.buildKernel(SOLVER_DIR "/okl/lineLU_v1.okl","lineLU",kernelProps);
    occa::kernel lineLU_v2   = gpu.buildKernel(SOLVER_DIR "/okl/lineLU_v2.okl","lineLU",kernelProps);

    /* matrix solve */
    //occa::kernel solveDU_v5  = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_v5.okl","linesmoothLU_solveDU",kernelProps);
    //occa::kernel solveDU_v6  = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_v6.okl","linesmoothLU_solveDU",kernelProps);
    //occa::kernel solveDU_v7  = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_v7.okl","linesmoothLU_solveDU",kernelProps);
    //occa::kernel solveDU_v8  = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_TriBlock_v1.okl","triblock_solveDU",kernelProps);
    //occa::kernel solveDU_v9  = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_TriBlock_v2.okl","triblock_solveDU",kernelProps);
    occa::kernel solveDU_v10 = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_TriBlock_v3.okl","triblock_solveDU",kernelProps);

    /* linear residual calculation */
    //occa::kernel lineRes_v5  = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_v5.okl","linesolver_lineRes",kernelProps);
    //occa::kernel lineRes_v6  = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_v6.okl","linesolver_lineRes",kernelProps);
    //occa::kernel lineRes_v7  = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_v7.okl","linesolver_lineRes",kernelProps);
    //occa::kernel lineRes_v8  = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_TriBlock_v1.okl","triblock_lineRes",kernelProps);
    //occa::kernel lineRes_v9  = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_TriBlock_v2.okl","triblock_lineRes",kernelProps);
    occa::kernel lineRes_v10 = gpu.buildKernel(SOLVER_DIR "/okl/linesmoothLU_TriBlock_v3.okl","triblock_lineRes",kernelProps);

    double t2 = MPI_Wtime();
    std::cout << GREEN "done: " COLOR_OFF << t2-t1 << " seconds." << std::endl;

    /* ===================================================== */
    /* Compute Memory Loads/Stores for Bandwidth Calculation */
    /* ===================================================== */
    double linesolver_mem = (mesh.nbytes + Jac.nbytes)*sizeof(double);
    linesolver_mem /= (double)1e9; // GB

    double duMem = 1*Jac.jacDLU.size()*sizeof(double)
                 + 1*Jac.DinvC.size()*sizeof(double)
                 + 1*Jac.A.size()*sizeof(double)
                 + 4*Jac.dU.size()*sizeof(double) // 2 loads, 2 stores
                 + 1*Jac.res.size()*sizeof(double)
                 + 1*mesh.linesize.size()*sizeof(int)
                 + 1*mesh.linepoint.size()*sizeof(int)
                 + 1*mesh.lines.size()*sizeof(int)
                 + 1*mesh.lineface.size()*sizeof(int);
    duMem *= iters / (double)1e9; // GB

    double cpMem = (5*Jac.dU.size()*sizeof(double))*(iters + 2./5.);
    cpMem /= (double)1e9; // GB

    double lrMem = Jac.jacD.size()*sizeof(double)
                 + Jac.jacO1.size()*sizeof(double)
                 + Jac.jacO2.size()*sizeof(double)
                 + 5*mesh.nelem*nvar*sizeof(double) // U: spMV w/ 5 blocks for each element
                 + 2*Jac.res.size()*sizeof(double)  // 1 load, 1 store
                 + mesh.epoint.size()*sizeof(int)
                 + mesh.ef.size()*sizeof(int)
                 + mesh.fc.size()*sizeof(int)
                 + mesh.linesize.size()*sizeof(int)
                 + mesh.linepoint.size()*sizeof(int)
                 + mesh.lines.size()*sizeof(int)
                 + mesh.lineface.size()*sizeof(int);
    lrMem *= iters / (double)1e9; // GB

    /* ====================================================================== */
    /* Factor Block Jacobian Diagonals                                        */
    /* ====================================================================== */
    gpu.device.finish();
    start = gpu.device.tagStream();
        copyAtoBjac(mesh.nelem,Jac.o_jacD,Jac.o_jacDLU);
        lineLU_v2(mesh.nelem,mesh.nintface,mesh.nline,
                  mesh.o_fc,mesh.o_linesize,mesh.o_linepoint,mesh.o_lines,mesh.o_lineface,
                  Jac.o_jacDLU,Jac.o_jacO1,Jac.o_jacO2,Jac.o_jacDinvC);
    end = gpu.device.tagStream();
    gpu.device.finish();
    double LU_time = gpu.device.timeBetween(start, end);
    printf("   LU OCCA Time: %f\n",LU_time);

    /* ====================================================================== */
    /* Iterate Line Solver                                                    */
    /* ====================================================================== */
    /* ---------- */
    /* Version 10 */
    /* ---------- */
    double dU_time = 0.0;
    double LR_time = 0.0;
    double cp_time = 0.0;

    gpu.device.finish();
    start = gpu.device.tagStream();
        copyAtoB(mesh.nelem,Jac.o_rhs,Jac.o_res);
    end = gpu.device.tagStream();
    cp_time += gpu.device.timeBetween(start, end);

    for(int p = 0; p < iters; ++p){
        start = gpu.device.tagStream();
            solveDU_v10(mesh.nelem,mesh.nintface,mesh.eftot,mesh.nline,mesh.nlineelem,
                        mesh.o_linesize,mesh.o_linepoint,mesh.o_lines,
                        Jac.o_jacDLU,Jac.o_jacDinvC,Jac.o_A,Jac.o_dU,Jac.o_res);
        end = gpu.device.tagStream();
        dU_time += gpu.device.timeBetween(start, end);

        start = gpu.device.tagStream();
            addAtoB(mesh.nelem,Jac.o_dU,Jac.o_U);
            copyAtoB(mesh.nelem,Jac.o_rhs,Jac.o_res);
        end = gpu.device.tagStream();
        cp_time += gpu.device.timeBetween(start, end);

        start = gpu.device.tagStream();
            lineRes_v10(mesh.nelem,mesh.nintface,mesh.eftot,mesh.nline,mesh.nlineelem,
                        mesh.o_epoint,mesh.o_ef,mesh.o_fc,
                        mesh.o_linesize,mesh.o_linepoint,mesh.o_lines,
                        Jac.o_jacD,Jac.o_jacO1,Jac.o_jacO2,
                        Jac.o_U,Jac.o_res);
        end = gpu.device.tagStream();
        LR_time += gpu.device.timeBetween(start, end);
    }
    gpu.device.finish();
    double v10_time = dU_time + cp_time + LR_time;

    double du_bytes = (Jac.jacDLU.size()
                     + Jac.jacD.size()
                     + Jac.A.size()
                     + Jac.dU.size()
                     + Jac.res.size())*sizeof(double)/1024.0;
    printf("dU KiB per line: %.2f\n",du_bytes/mesh.nline);
    printf("dU KiB per elem: %.2f\n",du_bytes/mesh.nelem);
    printf("dU KiB    total: %.2f\n",du_bytes);

    std::cout << "-----------------------------------------\n";
    printf("[v10]OCCA Time: %f %f GB/s\n",v10_time,(duMem + cpMem + lrMem)/v10_time);
    printf("       du Time: %f %f GB/s\n",dU_time,duMem/dU_time);
    printf("    U+=du Time: %f %f GB/s\n",cp_time,cpMem/cp_time);
    printf("   linRes Time: %f %f GB/s\n",LR_time,lrMem/LR_time);
    /* ====================================================================== */

    std::cout << "-----------------------------------------\n";
    printf("[v10] Iterations: %d\n",iters);
    printf("[v10] Total Time: %f\n",v10_time+LU_time);
    std::cout << "-----------------------------------------\n";

    /* ====================================================================== */
    /* Copy and Display Jacobian Values                                       */
    /* ====================================================================== */
//    MPI_Barrier(gpu.comm);
//    Jac.fromDevice();
//    for(int j = 0; j < mesh.nelem; ++j){
//        for(int i = 0; i < mesh.nvar; ++i){
//            (Jac.dU[mesh.nvar*j+i]<0) ?
//                printf("  %.16e ",Jac.dU[mesh.nvar*j+i]):
//                printf("   %.16e ",Jac.dU[mesh.nvar*j+i]);
//        }
//        printf("\n");
//    }
    /* ====================================================================== */
    MPI_Finalize();
}