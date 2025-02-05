/**
 * File:   Jacobian.hxx
 * Author: akirby
 *
 * Created on September 29, 2021, 9:16 AM
 */

#ifndef JACOBIAN_HXX
#define JACOBIAN_HXX

/* header files */
#include "core.hxx"
#include "Mesh.hxx"
#include "Platform.hxx"

#ifdef __cplusplus
extern "C" {
#endif

/* ================== */
/* External Functions */
/* ================== */
void read_jacobian_data_(int *nvar,int *nelem,int *nintface,
                         double *jacD,double *jacO1,double *jacO2,
                         double *rhs,double *U0);

class bcsr_matrix {
  public:
    int nrows;  /**< number of matrix rows */
    int ncols;  /**< number of matrix columns */
    int bs;     /**< block dimension of each matrix entry */
    int nnz;    /**< number of non-zero matrix blocks */

    std::vector<double> values;     /**< [bs*bs*nnz] */
    std::vector<int> column_idx;    /**< [nnz] */
    std::vector<int> row_ptr;       /**< [nrows+1] */

    /* constructor */
    bcsr_matrix(){};
   ~bcsr_matrix(){};
};

class Jacobian {
  public:
    size_t nbytes;

    /* variables */
    int nvar;
    int nelem;
    int nintface;

    std::vector<double> jacDLU;
    std::vector<double> jacD;
    std::vector<double> jacO1;
    std::vector<double> jacO2;
    std::vector<double> rhs;
    std::vector<double> res;
    std::vector<double> U;
    std::vector<double> U0;
    std::vector<double> dU;

    std::vector<double> A;
    std::vector<double> DinvC;
  //std::vector<double> B;
  //std::vector<double> C;
  //std::vector<int> offmap;

    bcsr_matrix jacCSR;

    occa::memory o_jacDinvC;
    occa::memory o_jacDLU;
    occa::memory o_jacD;
    occa::memory o_jacO1;
    occa::memory o_jacO2;
    occa::memory o_rhs;
    occa::memory o_res;
    occa::memory o_U;
    occa::memory o_U0;
    occa::memory o_dU;

    occa::memory o_A;
  //occa::memory o_B;
  //occa::memory o_C;
  //occa::memory o_offmap;

    /* constructors */
    Jacobian(){};
   ~Jacobian(){};

    /* methods */
    bool fromFile(int nlineelem);
    void assembleTriBlocks(Mesh &mesh);
    void assembleCSR(Mesh &mesh);
    void resizeBlockSize(int nvar_new);
    void setupDevice(Platform &gpu);
    void toDevice();
    void fromDevice();
};

#ifdef __cplusplus
}
#endif
#endif /* JACOBIAN_HXX */