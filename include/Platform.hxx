/**
 * File:   Platform.hxx
 * Author: akirby
 *
 * Created on September 29, 2021, 2:50 PM
 */

#ifndef PLATFORM_HXX
#define PLATFORM_HXX

/*header files */
#include "core.hxx"

/* system header files */
#include <unistd.h>

#define SERIAL_MODE 0
#define HIP_MODE    1
#define CUDA_MODE   2
#define OPENCL_MODE 3
#define OPENMP_MODE 4
#define DPCPP_MODE  5
#define METAL_MODE  6

class Platform {
  public:
    MPI_Comm comm;

    occa::properties props;
    occa::device device;

    int rank, nrank;

    /* constructors */
    Platform(MPI_Comm _comm,int thread_model,int device_id=0,int platform=0):
        comm(_comm)
    {
        MPI_Comm_rank(_comm, &rank);
        MPI_Comm_size(_comm, &nrank);
        DeviceConfig(thread_model,device_id,platform);
    }

   ~Platform(){}

    /* methods */
    occa::kernel buildKernel(const std::string &fileName,
                             const std::string &kernelName,
                             const occa::json  &props = occa::json()){
        occa::kernel kernel;

        /* build on root first */
        if(!rank) kernel = device.buildKernel(fileName,kernelName,props);
        MPI_Barrier(comm);

        /* remaining ranks find the cached version (ideally) */
        if(rank) kernel = device.buildKernel(fileName,kernelName,props);
        MPI_Barrier(comm);

        return kernel;
    }

    template <class T>
    occa::memory malloc(const occa::dim_t entries,
                        const void *src = NULL,
                        const occa::json &prop = occa::properties()) {
        return device.malloc<T>(entries, src, prop);
    }

    template <class T>
    occa::memory malloc(const occa::dim_t entries,
                        const occa::memory &src,
                        const occa::json &prop = occa::properties()) {
        return device.malloc<T>(entries, src, prop);
    }

    template <class T>
    occa::memory malloc(const occa::dim_t entries,
                        const occa::json &prop) {
        return device.malloc<T>(entries, prop);
    }

    void *hostMalloc(const size_t bytes,
                     const void *src,
                     occa::memory &h_mem){
        occa::properties hostProp;
        hostProp["host"] = true;
        h_mem = device.malloc(bytes, src, hostProp);
        return h_mem.ptr();
    }

  private:
    void DeviceConfig(int thread_model,int device_id,int plat);
};

#endif /* PLATFORM_HXX */