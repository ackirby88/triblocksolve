/**
 * \file    Jacobian.cxx
 * \author  akirby
 *
 * \brief Jacobian class implementation
 */

/* header files */
#include "Jacobian.hxx"

bool Jacobian::fromFile(int nlineelem){
    int jac_data[3];

    /* read Jacobian size data from file */
    FILE *fp = fopen("gpuline.jacobian.data.bin", "rb");
    if(fp == nullptr){
        printf("\x1B[1;31mERROR: could not find gpuline.jacobian.data.bin\x1B[0m\n");
        exit(1);
    } else {
        printf("\x1B[1;92mReading gpuline.jacobian.data.bin\x1B[0m\n");
    }
    size_t nread = fread(jac_data,sizeof(jac_data),1,fp);
    fclose(fp);

    nvar     = jac_data[0];
    nelem    = jac_data[1];
    nintface = jac_data[2];

    /* mesh sizes */
    std::cout << "Read Jac Stats:\n";
    printf("  nvar: %d\n"
           "  nelem: %d\n"
           "  nintface: %d\n",
           nvar,nelem,nintface);

    /* allocate host Jacobian data */
    jacDLU.resize(nvar*nvar*nelem);
    jacD.resize(nvar*nvar*nelem);
    jacO1.resize(nvar*nvar*nintface);
    jacO2.resize(nvar*nvar*nintface);
    rhs.resize(nvar*nelem);
    U0.resize(nvar*nelem);

    U.resize(nvar*nelem);
    dU.resize(nvar*nelem);
    res.resize(nvar*nelem);

    DinvC.resize(nvar*nvar*nelem);
    A.resize(nvar*nvar*nlineelem);
  //B.resize(nvar*nvar*nlineelem);
  //C.resize(nvar*nvar*nlineelem);
  //offmap.resize(nlineelem);

    nbytes = jacD.size()
           + jacO1.size()
           + jacO2.size()
           + rhs.size()
           + U0.size()
           + U.size()
           + dU.size()
           + res.size();

    /* clear res */
    for(auto i:res) i = 0.0;
    for(auto i:dU) i = 0.0;
    for(auto i:U) i = 0.0;

    /* read arrays using fortran */
    read_jacobian_data_(&nvar,&nelem,&nintface,
                        jacD.data(),jacO1.data(),jacO2.data(),
                        rhs.data(),U0.data());
    std::cout << "Read Jacobian Data Complete!\n";
    return true;
}

void Jacobian::assembleTriBlocks(Mesh &mesh){

//    int ind = 0;
//    for(int k = 0; k < mesh.max_line_nelem; k++){
//        for(int l = 0; l < mesh.nline; ++l){
//            const int nelem_line = mesh.linesize[l];
//
//            if(k < nelem_line){
//                int m = mesh.linepoint[l] + k;
//                int e = mesh.lines[m];
//                offmap[e] = ind++;
//            }
//        }
//    }

    int blocksize = nvar*nvar;
    for(int l = 0; l < mesh.nline; ++l){
        const int nelem_line = mesh.linesize[l];

        for(int k = 1; k < nelem_line; ++k){
            /* copy A matrix into packed data */
            int m = mesh.linepoint[l] + k;
            int f = mesh.lineface[m];
            int e = mesh.lines[m];
            int e1 = mesh.fc[2*f];

            double *Aptr = (e==e1) ? &jacO2[blocksize*f]:&jacO1[blocksize*f];
            memcpy(&A[blocksize*e], Aptr, blocksize*sizeof(double));
        }

//        for(int k = 0; k < nelem_line-1; ++k){
//            /* copy C matrix into packed data */
//            int m = mesh.linepoint[l] + k;
//            int f = mesh.lineface[m+1];
//            int e = mesh.lines[m];
//            int e1 = mesh.fc[2*f];
//
//            double *Cptr = (e==e1) ? &jacO2[blocksize*f]:&jacO1[blocksize*f];
//            memcpy(&C[blocksize*e], Cptr, blocksize*sizeof(double));
//        }
    }
}

void Jacobian::assembleCSR(Mesh &mesh){

    jacCSR.bs = nvar;
    jacCSR.nrows = mesh.nelem;
    jacCSR.ncols = mesh.nelem;

    int blocksize = nvar*nvar;
    for(int l = 0; l < mesh.nline; ++l){
        const int nelem_line = mesh.linesize[l];

//        for(int k = 1; k < nelem_line; ++k){
//            /* copy A matrix into packed data */
//            int m = mesh.linepoint[l] + k;
//            int f = mesh.lineface[m];
//            int e = mesh.lines[m];
//            int e1 = mesh.fc[2*f];
//
//            double *Aptr = (e==e1) ? &jacO2[blocksize*f]:&jacO1[blocksize*f];
//            memcpy(&A[blocksize*e], Aptr, blocksize*sizeof(double));
//        }
//
//        for(int k = 0; k < nelem_line-1; ++k){
//            /* copy C matrix into packed data */
//            int m = mesh.linepoint[l] + k;
//            int f = mesh.lineface[m+1];
//            int e = mesh.lines[m];
//            int e1 = mesh.fc[2*f];
//
//            double *Cptr = (e==e1) ? &jacO2[blocksize*f]:&jacO1[blocksize*f];
//            memcpy(&C[blocksize*e], Cptr, blocksize*sizeof(double));
//        }
    }
}

void Jacobian::resizeBlockSize(int nvar_new){
    int nvar_old = nvar;
    if(nvar_new != nvar){
        printf("Resizing NVAR: old=%d new=%d ...",nvar,nvar_new);
    } else {
        return;
    }
    nvar = nvar_new;
    int min_nvar = (nvar < nvar_old) ? nvar:nvar_old;

    std::vector<double> jacD_old(jacD);
    std::vector<double> jacO1_old(jacO1);
    std::vector<double> jacO2_old(jacO2);
    std::vector<double> rhs_old(rhs);
    std::vector<double> U0_old(U0);

    jacD.resize(nvar*nvar*nelem);
    jacO1.resize(nvar*nvar*nintface);
    jacO2.resize(nvar*nvar*nintface);
    U0.resize(nvar*nelem);
    U.resize(nvar*nelem);

    jacDLU.resize(nvar*nvar*nelem);
    rhs.resize(nvar*nelem);
    dU.resize(nvar*nelem);
    res.resize(nvar*nelem);

    A.resize(nvar*nvar*nelem);
  //B.resize(nvar*nvar*nelem);
  //C.resize(nvar*nvar*nelem);

    for(auto i:jacD) i = 0.0;
    for(auto i:jacO1) i = 0.0;
    for(auto i:jacO2) i = 0.0;
    for(auto i:rhs) i = 0.0;
    for(auto i:U0) i = 0.0;
    for(auto i:U) i = 0.0;

    /* set diag */
    for(int e = 0; e < nelem; ++e){
        for(int j = 0; j < nvar; ++j){
            jacD[e*nvar*nvar+nvar*j+j] = 10.0;
        }
    }
    for(int f = 0; f < nintface; ++f){
        for(int j = 0; j < nvar; ++j){
            jacO1[f*nvar*nvar+nvar*j+j] = 10.0;
        }
    }
    for(int f = 0; f < nintface; ++f){
        for(int j = 0; j < nvar; ++j){
            jacO2[f*nvar*nvar+nvar*j+j] = 10.0;
        }
    }

    /* fill vectors with old data */
    for(int e = 0; e < nelem; ++e){
        for(int j = 0; j < min_nvar; ++j){
            for(int i = 0; i < min_nvar; ++i){
                jacD[e*nvar*nvar+nvar*j+i] = jacD_old[e*nvar_old*nvar_old+nvar_old*j+i];
            }
        }
    }
    for(int e = 0; e < nintface; ++e){
        for(int j = 0; j < min_nvar; ++j){
            for(int i = 0; i < min_nvar; ++i){
                jacO1[e*nvar*nvar+nvar*j+i] = jacO1_old[e*nvar_old*nvar_old+nvar_old*j+i];
            }
        }
    }
    for(int e = 0; e < nintface; ++e){
        for(int j = 0; j < min_nvar; ++j){
            for(int i = 0; i < min_nvar; ++i){
                jacO2[e*nvar*nvar+nvar*j+i] = jacO2_old[e*nvar_old*nvar_old+nvar_old*j+i];
            }
        }
    }
    for(int eij = 0; eij < nelem*min_nvar; ++eij) rhs[eij] = rhs_old[eij];
    for(int eij = 0; eij < nelem*min_nvar; ++eij) U0[eij] = U0_old[eij];

    /* clear res */
    for(auto i:res) i = 0.0;
    for(auto i:dU) i = 0.0;
    for(auto i:U) i = 0.0;
    printf(" done!\n");
}

void Jacobian::setupDevice(Platform &gpu){
    /* allocate device memory */
    o_jacDLU = gpu.malloc<double>(jacDLU.size());
    o_jacD = gpu.malloc<double>(jacD.size());
    o_jacO1 = gpu.malloc<double>(jacO1.size());
    o_jacO2 = gpu.malloc<double>(jacO2.size());
    o_rhs = gpu.malloc<double>(rhs.size());
    o_U0 = gpu.malloc<double>(U0.size());

    o_jacDinvC = gpu.malloc<double>(jacDLU.size());

    o_U = gpu.malloc<double>(U.size());
    o_dU = gpu.malloc<double>(dU.size());
    o_res = gpu.malloc<double>(res.size());

    o_A = gpu.malloc<double>(A.size());
  //o_B = gpu.malloc<double>(B.size());
  //o_C = gpu.malloc<double>(C.size());
  //o_offmap= gpu.malloc<int>(offmap.size());
}

void Jacobian::toDevice(){
    o_jacDLU.copyFrom(jacDLU.data());
    o_jacD.copyFrom(jacD.data());
    o_jacO1.copyFrom(jacO1.data());
    o_jacO2.copyFrom(jacO2.data());
    o_rhs.copyFrom(rhs.data());
    o_U0.copyFrom(U0.data());

    o_U.copyFrom(U.data());
    o_res.copyFrom(res.data());
    o_dU.copyFrom(dU.data());

    o_A.copyFrom(A.data());
  //o_B.copyFrom(B.data());
  //o_C.copyFrom(C.data());
  //o_offmap.copyFrom(offmap.data());
}

void Jacobian::fromDevice(){
    o_jacDLU.copyTo(jacDLU.data());
    o_jacD.copyTo(jacD.data());
    o_jacO1.copyTo(jacO1.data());
    o_jacO2.copyTo(jacO2.data());
    o_rhs.copyTo(rhs.data());
    o_U0.copyTo(U0.data());

    o_U.copyTo(U.data());
    o_dU.copyTo(dU.data());
    o_res.copyTo(res.data());
}
