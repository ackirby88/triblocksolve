/**
 * \file    Platform.cxx
 * \author  akirby
 *
 * \brief Platform class implementation
 */

/* header files */
#include "Platform.hxx"

void Platform::DeviceConfig(int thread_model,int device_id=0,int plat=0){
    std::string mode;

    if(thread_model == HIP_MODE){
        mode = "{mode: 'HIP'}";
    } else
    if(thread_model == CUDA_MODE){
        mode = "{mode: 'CUDA'}";
    } else
    if(thread_model == OPENCL_MODE){
        mode = "{mode: 'OpenCL', platform_id : " + std::to_string(plat) +"}";
    } else
    if(thread_model == OPENMP_MODE){
        mode = "{mode: 'OpenMP'}";
    } else {
        mode = "{mode: 'Serial'}";
    }

    if (thread_model == CUDA_MODE
      ||thread_model == HIP_MODE
      ||thread_model == OPENCL_MODE) {

        //for testing a single device, run with 1 rank and specify DEVICE NUMBER
        if (nrank > 1) {
            /* find out how many ranks and devices are on this system */
            char* hostnames = (char *) ::malloc(nrank*sizeof(char)*MPI_MAX_PROCESSOR_NAME);
            char* hostname = hostnames+rank*MPI_MAX_PROCESSOR_NAME;

            int namelen;
            MPI_Get_processor_name(hostname,&namelen);

            MPI_Allgather(MPI_IN_PLACE , MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                          hostnames, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, MPI_COMM_WORLD);

            int localRank = 0;
            int localSize = 0;
            for(int n=0; n<rank; n++){
                if(!strcmp(hostname, hostnames+n*MPI_MAX_PROCESSOR_NAME)) localRank++;
            }
            for(int n=0; n<nrank; n++){
                if(!strcmp(hostname, hostnames+n*MPI_MAX_PROCESSOR_NAME)) localSize++;
            }

            device_id = localRank;

            //check for over-subscribing devices
            int deviceCount = occa::getDeviceCount(mode);
            if (deviceCount>0 && localRank>=deviceCount) {
                std::stringstream ss;
                ss << "Rank " << rank << " oversubscribing device " << device_id%deviceCount << " on node \"" << hostname<< "\"";
                device_id = device_id%deviceCount;
            }
            MPI_Barrier(comm);
            free(hostnames);
    }

        // add device_id to setup string
        mode.pop_back();
        mode += ", device_id: " + std::to_string(device_id) + "}";
    }

    device.setup(mode);

    char pwd[256];
    char *ret = getcwd(pwd, 256);
    std::string occaCacheDir = std::string(pwd) + "/.occa";
    occa::env::setOccaCacheDir(occaCacheDir);
}
