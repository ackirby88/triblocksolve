/**
 * File:   Mesh.hxx
 * Author: akirby
 *
 * Created on September 28, 2021, 1:06 PM
 */

#ifndef MESH_HXX
#define MESH_HXX

/* header files */
#include "core.hxx"
#include "Platform.hxx"

#ifdef __cplusplus
extern "C" {
#endif

/* ================== */
/* External Functions */
/* ================== */
void read_mesh_data_(int *nvar,int *nelem,int *nintface,
                     int *eftot,int *nline,int *nlineelem,
                     int *epoint,int *ef,int *fc,int *lines,
                     int *linesize,int *linepoint,int *lineface);

class Mesh {
  public:
    size_t nbytes;

    /* variables */
    int nvar;
    int nelem;
    int nintface;
    int eftot;

    int nline;
    int nlineelem;
    int max_line_nelem;

    std::vector<int> epoint;
    std::vector<int> ef;
    std::vector<int> fc;

    std::vector<int> lines;
    std::vector<int> linesize;
    std::vector<int> lineface;
    std::vector<int> linepoint;

    occa::memory o_epoint;
    occa::memory o_ef;
    occa::memory o_fc;
    occa::memory o_lines;
    occa::memory o_linesize;
    occa::memory o_lineface;
    occa::memory o_linepoint;

    /* constructors */
    Mesh(){};
   ~Mesh(){};

    /* methods */
    bool fromFile();
    void setupDevice(Platform &gpu);
    void toDevice();
    void fromDevice();
};

#ifdef __cplusplus
}
#endif
#endif /* MESH_HXX */
