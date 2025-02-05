/**
 * \file    Mesh.cxx
 * \author  akirby
 *
 * \brief Mesh class implementation
 */

/* header files */
#include "Mesh.hxx"

bool Mesh::fromFile(){
    int mesh_data[6];

    /* read mesh data from file */
    FILE *fp = fopen("gpuline.mesh.data.bin", "rb");
    printf("---------------------------------\n");
    if(fp == nullptr){
        printf("\x1B[1;31mERROR: could not find gpuline.mesh.data.bin\x1B[0m\n");
        exit(1);
    } else {
        printf("\x1B[1;92mReading gpuline.mesh.data.bin\x1B[0m\n");
    }

    size_t nread = fread(mesh_data,sizeof(mesh_data),1,fp);
    fclose(fp);

    nvar      = mesh_data[0];
    nelem     = mesh_data[1];
    nintface  = mesh_data[2];
    eftot     = mesh_data[3];
    nline     = mesh_data[4];
    nlineelem = mesh_data[5];

    printf("  Mesh Statistics:\n");
    printf("    nvar: %d\n"
           "    nelem: %d\n"
           "    nintface: %d\n"
           "    eftot: %d\n"
           "    nline: %d\n"
           "    nlineelem: %d\n",
            nvar,nelem,nintface,
            eftot,nline,nlineelem);

    /* allocate host mesh data */
    ef.resize(eftot);
    fc.resize(2*nintface);
    epoint.resize(nelem+1);
    lines.resize(nlineelem);
    linesize.resize(nline);
    linepoint.resize(nline+1);
    lineface.resize(nlineelem);

    nbytes = epoint.size()
           + ef.size()
           + fc.size()
           + linesize.size()
           + linepoint.size()
           + lines.size()
           + lineface.size();

    /* read arrays using fortran */
    read_mesh_data_(&nvar,&nelem,&nintface,
                    &eftot,&nline,&nlineelem,
                    epoint.data(),ef.data(),fc.data(),lines.data(),
                    linesize.data(),linepoint.data(),lineface.data());

    max_line_nelem = 0;
    for(auto i: linesize) max_line_nelem = std::max(max_line_nelem,i);
    printf("  Max Line Element count: %d\n",max_line_nelem);
    printf("Read Mesh Data Complete!\n");
    printf("---------------------------------\n");
    return true;
}

void Mesh::setupDevice(Platform &gpu){
    /* allocate device memory */
    o_epoint = gpu.malloc<int>(epoint.size());
    o_ef = gpu.malloc<int>(ef.size());
    o_fc = gpu.malloc<int>(fc.size());
    o_lines = gpu.malloc<int>(lines.size());
    o_linesize = gpu.malloc<int>(linesize.size());
    o_lineface = gpu.malloc<int>(lineface.size());
    o_linepoint = gpu.malloc<int>(linepoint.size());
}

void Mesh::toDevice(){
    o_epoint.copyFrom(epoint.data());
    o_ef.copyFrom(ef.data());
    o_fc.copyFrom(fc.data());
    o_lines.copyFrom(lines.data());
    o_linesize.copyFrom(linesize.data());
    o_lineface.copyFrom(lineface.data());
    o_linepoint.copyFrom(linepoint.data());
}

void Mesh::fromDevice(){
    o_epoint.copyTo(epoint.data());
    o_ef.copyTo(ef.data());
    o_fc.copyTo(fc.data());
    o_lines.copyTo(lines.data());
    o_linesize.copyTo(linesize.data());
    o_lineface.copyTo(lineface.data());
    o_linepoint.copyTo(linepoint.data());
}