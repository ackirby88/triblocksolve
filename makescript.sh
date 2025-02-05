#!/bin/bash -e
# Folder structure:
# ================
# project_root
#   install            #install directory
#   builds             #build directory
#   src                #source codes


# ======================= #
# project directory paths
# ======================= #
CURRENT_PATH="$(pwd)"
MY_PATH="$( cd "$( dirname "$0" )" && pwd )"
PROJECT_ROOT=${MY_PATH}

# ====================== #
# folder directory paths
# ====================== #
INSTALL_DIRECTORY=${PROJECT_ROOT}/install
INSTALL_APP_DIRECTORY=${INSTALL_DIRECTORY}/app
BUILD_DIRECTORY=${PROJECT_ROOT}/builds
BUILD_APP_DIRECTORY=${BUILD_DIRECTORY}/app

# ================ #
# SOURCE CODE PATH #
# ================ #
APP_SOURCE_DIRECTORY=${PROJECT_ROOT}
SOLVER_SRC_DIRECTORY=${PROJECT_ROOT}/src


# ======================== #
# compiler option defaults
# ======================== #
MAKE_CMD="make -j4 install"
BUILD_SUFFIX="_release"
BUILD_TYPE="Release"

# ================== #
# compiling defaults
# ================== #
BUILD_OCCA=0
BUILD_APP=0
BUILD_CLEAN=0
CLEAN_DIST=0
COMPILE_FAIL=0

# ================= #
# compiler defaults
# ================= #
FC=mpif90
CC=mpicc
CXX=mpicxx

C_FLAGS=
CXX_FLAGS=
Fortran_FLAGS=

# ========== #
# OCCA Modes #
# ========== #
HIP="OFF"
CUDA="OFF"
OPENMP="OFF"
OPENCL="OFF"
DPCPP="OFF"
METAL="OFF"

APP_HIP_ROOT=
APP_CUDATookit_ROOT=
APP_OpenCL_ROOT=
APP_SYCL_ROOT=

# ============== #
# print strings
# ============== #
opt_str="[OPTION] "

eC="\x1B[0m"
bC="\x1B[0;34m"
GC="\x1B[1;32m"
yC="\x1B[0;33m"
aC="\x1B[0;96m"
rC="\x1B[0;41m"
gC="\x1B[0;32m"
oC="\x1B[3;93m"
mC="\x1B[0;43m"

help() {

    echo " "
    echo -e "${GC} Usage:${eC} $0 [OPTION]"
    echo " "
    echo -e " ${aC}Recommended Options:${eC}"
    echo -e "    Default: ./makescript.sh"
    echo -e "       Builds app and the occa library"
    echo " "
    echo -e " ${aC}Options List:${eC}"
    echo "  [OPTION]:"
    echo "    --OCCA      -occa   build the occa library"
    echo "    --APP       -app    build the app executable"
    echo " "
    echo "    --help      -h      displays this help message"
    echo "    --clean     -c      removes local build directories"
    echo "    --distclean -dc     removes builds and install directories"
    echo "    --release   -opt    compile the project in optimized mode"
    echo "    --debug     -deb    compile the project in debug mode"
    echo " "
    echo "  [COMPILER OPTIONS]:"
    echo "     CC=<arg>   cc=<arg>    sets the C compiler"
    echo "    CXX=<arg>  cxx=<arg>    sets the C++ compiler"
    echo "     FC=<arg>   fc=<arg>    sets the Fortran compiler"
    echo " "
    echo "      C_FLAGS=<arg>    c_flags=<arg>    sets the C compiler flags"
    echo "    CXX_FLAGS=<arg>  cxx_flags=<arg>    sets the C++ compiler flags"
    echo "     FC_FLAGS=<arg>   fc_flags=<arg>    sets the Fortran compiler flags"
    echo " "
    echo "   [OCCA MODES]:"
    echo "      --hip     -hip      enable HIP mode"
    echo "      --cuda    -cuda     enable CUDA mode"
    echo "      --opencl  -opencl   enable OpenCL mode"
    echo "      --openmp  -openmp   enable OpenMP mode"
    echo "      --dpcpp   -dpcpp    enable DPC++ mode"
    echo "      --metal   -metal    enable Metal mode"
    echo " "
    echo "      CUDATookit_ROOT=<arg>   sets path to the NVIDIA CUDA Toolkit"
    echo "      HIP_ROOT=<arg>          sets path to the AMD HIP Toolkit"
    echo "      OpenCL_ROOT=<arg>       sets path to the OpenCL headers and library"
    echo "      SYCL_ROOT=<arg>         sets path to the SYCL headers and library"
}

# ----------------------------- #
# Start the compilation process #
# ----------------------------- #
cd $PROJECT_ROOT

# ============ #
# parse inputs
# ============ #
if [[ $# -lt 1 ]]; then
  BUILD_OCCA=1
  BUILD_APP=1
fi

for var in "$@"
do
  if [ "$var" == "--help" -o "$var" == "-help" -o "$var" == "-h" ]; then
    help
    exit 0

  elif [ "$var" == "--APP" -o "$var" == "-app" ]; then
    echo -e "Found known argument: ${gC}$var${eC}"
    BUILD_APP=1

  elif [ "$var" == "--distclean" -o "$var" == "-distclean" -o "$var" == "-dc" ]; then
    echo -e "Found known argument: ${gC}$var${eC}"
    echo ${opt_str} "Cleaning the distribution..."
    CLEAN_DIST=1

  elif [ "$var" == "--OCCA" -o "$var" == "-occa" ]; then
    echo -e "Found known argument: ${gC}$var${eC}"
    BUILD_OCCA=1

  elif [ "$var" == "--release" -o "$var" == "-release" -o "$var" == "-opt" ]; then
    echo -e "Found known argument: ${gC}$var${eC}"
    BUILD_SUFFIX="_release"
    BUILD_TYPE="Release"

  elif [ "$var" == "--debug" -o "$var" == "-debug" -o "$var" == "-deb" ]; then
    echo -e "Found known argument: ${gC}$var${eC}"
    BUILD_SUFFIX="_debug"
    BUILD_TYPE="Debug"

  elif [ "${var:0:3}" == "CC=" -o "${var:0:3}" == "cc=" ]; then
    CC=${var:3}
    echo -e "[OPTION]       C Compiler: $yC$CC$eC"
  elif [ "${var:0:4}" == "CXX=" -o "${var:0:4}" == "cxx=" ]; then
    CXX=${var:4}
    echo -e "[OPTION]     CXX Compiler: $yC$CXX$eC"
  elif [ "${var:0:3}" == "FC=" -o "${var:0:3}" == "fc=" ]; then
    FC=${var:3}
    F77=${FC}
    echo -e "[OPTION] Fortran Compiler: $yC$FC$eC"

  elif [ "${var:0:8}" == "C_FLAGS=" -o "${var:0:8}" == "c_flags=" ]; then
    C_FLAGS=${var:8}
    echo -e "[OPTION]       C Compiler Flags: $yC$C_FLAGS$eC"
  elif [ "${var:0:10}" == "CXX_FLAGS=" -o "${var:0:10}" == "cxx_flags=" ]; then
    CXX_FLAGS=${var:10}
    echo -e "[OPTION]     CXX Compiler Flags: $yC$CXX_FLAGS$eC"
  elif [ "${var:0:9}" == "FC_FLAGS=" -o "${var:0:9}" == "fc_flags=" ]; then
    FC=${var:9}
    echo -e "[OPTION] Fortran Compiler Flags: $yC$FC_FLAGS$eC"

  elif [ "${var:0:10}" == "CUDA_ROOT_ROOT=" ]; then
    APP_CUDATookit_ROOT=${var:10}
    echo -e "[OPTION]     CUDA Toolkit Path: $yC$APP_CUDA__ROOT$eC"
  elif [ "${var:0:9}" == "HIP_ROOT=" ]; then
    APP_HIP_ROOT=${var:9}
    echo -e "[OPTION]     HIP Toolkit Path: $yC$APP_HIP_ROOT$eC"
  elif [ "${var:0:12}" == "OpenCL_ROOT=" ]; then
    APP_OpenCL_ROOT=${var:12}
    echo -e "[OPTION]     OpenCL Library Path: $yC$APP_OpenCL_ROOT$eC"
  elif [ "${var:0:10}" == "SYCL_ROOT=" ]; then
    APP_SYCL_ROOT=${var:10}
    echo -e "[OPTION]     OpenCL Library Path: $yC$APP_SYCL_ROOT$eC"

  elif [ "$var" == "--hip" -o "$var" == "-hip" ]; then
    echo -e "Found known argument: ${gC}$var${eC}"
    HIP="ON"
  elif [ "$var" == "--cuda" -o "$var" == "-cuda" ]; then
    echo -e "Found known argument: ${gC}$var${eC}"
    CUDA="ON"
  elif [ "$var" == "--opencl" -o "$var" == "-opencl" ]; then
    echo -e "Found known argument: ${gC}$var${eC}"
    OPENCL="ON"
  elif [ "$var" == "--openmp" -o "$var" == "-openmp" ]; then
    echo -e "Found known argument: ${gC}$var${eC}"
    OPENMP="ON"
  elif [ "$var" == "--dpcpp" -o "$var" == "-dpcpp" ]; then
    echo -e "Found known argument: ${gC}$var${eC}"
    DPCPP="ON"
  elif [ "$var" == "--metal" -o "$var" == "-metal" ]; then
    echo -e "Found known argument: ${gC}$var${eC}"
    METAL="ON"

  elif [ "$var" == "--clean" -o "$var" == "-clean" -o "$var" == "-c" -o \
         "${var:0:3}" == "CC=" -o "${var:0:3}" == "cc=" -o \
         "${var:0:4}" == "CXX=" -o "${var:0:4}" == "cxx=" -o \
         "${var:0:3}" == "FC=" -o "${var:0:3}" == "fc=" -o \
         "${var:0:8}" == "C_FLAGS=" -o "${var:0:8}" == "c_flags=" -o \
         "${var:0:10}" == "CXX_FLAGS=" -o "${var:0:10}" == "cxx_flags=" -o \
         "${var:0:9}" == "FC_FLAGS=" -o "${var:0:9}" == "fc_flags=" -o \
         "${var:0:9}" == "HIP_ROOT=" -o \
         "${var:0:10}" == "CUDA_ROOT=" -o \
         "${var:0:12}" == "OpenCL_ROOT=" -o \
         "${var:0:10}" == "SYCL_ROOT=" -o \
         "$var" == "--OCCA" -o "$var" == "-occa" -o \
	 "$var" == "--APP"  -o "$var" == "-app" ]; then
    echo -e "Found known argument: ${gC}$var${eC}"

  else
    echo -e "${oC}Unknown option:${eC}  ${rC}$var${eC}"
    echo "See available options: ./makescript.sh -help"
    exit 0
  fi
done

# ========================= #
# display command line args
# ========================= #
echo "$0 $@"
cmd_args="${@:1}"

if [ $CLEAN_DIST == 1 ]; then
  rm -rf install
  rm -rf builds
  echo "Clean complete!"
  exit 0
fi

if [[ $BUILD_OCCA == 0 && $BUILD_APP == 0 ]]; then
  BUILD_OCCA=1
  BUILD_APP=1
fi


# ----------------------------------------------------- #
# After reading in cmd arg options, set remaining paths #
# ----------------------------------------------------- #
# ====================================== #
# install/build location compiled source
# ====================================== #
COMPILE_INSTALL_APP_DIRECTORY="${INSTALL_APP_DIRECTORY}${BUILD_SUFFIX}"
COMPILE_BUILD_APP_DIRECTORY="${BUILD_APP_DIRECTORY}${BUILD_SUFFIX}"

# ============== #
# compiler paths
# ============== #
FC_PATH="`which $FC`"
CC_PATH="`which $CC`"
CXX_PATH="`which $CXX`"
LD_PATH="`which ld`"

# ====================== #
# check source directory
# ====================== #
if [ ! -d "${APP_SOURCE_DIRECTORY}" ]; then
  echo " "
  echo "Error:"
  echo "${APP_SOURCE_DIRECTORY} does not exist."
  exit 1
fi

# ======================= #
# check install directory
# ======================= #
if [ ! -d "${INSTALL_DIRECTORY}" ]; then
  echo  "${INSTALL_DIRECTORY} does not exist. Making it..."
  mkdir "${INSTALL_DIRECTORY}"
fi

# ====================== #
# check builds directory
# ====================== #
if [ ! -d "${BUILD_DIRECTORY}" ]; then
  echo  "${BUILD_DIRECTORY} does not exist. Making it..."
  mkdir "${BUILD_DIRECTORY}"
fi

# =============================================================== #
# BUILD 3RD Party Library: OCCA (https://github.com/libocca/occa)
# =============================================================== #
COMPILE_FAIL=0
INSTALL_OCCA_DIRECTORY=${INSTALL_DIRECTORY}/occa
if [ $BUILD_OCCA == 1 ]; then
  echo " "
  echo -e "${mC} ===== Building OCCA ===== ${eC}"
  echo " Compiling Options:"
  echo "        Build Type: ${BUILD_TYPE}"
  echo "  Install Location: ${INSTALL_OCCA_DIRECTORY}"
  echo " "
  echo "                CC: ${CC}"
  echo "               CXX: ${CXX}"
  echo "                FC: ${FC}"
  echo " "
  echo "      Device Modes:"
  echo "               HIP: ${HIP}         HIP_ROOT: ${APP_HIP_ROOT}"
  echo "              CUDA: ${CUDA}       CUDA_ROOT: ${APP_CUDATookit_ROOT}"
  echo "            OpenMP: ${OPENMP}"
  echo "            OpenCL: ${OPENCL}     OpenCL_ROOT: ${APP_OpenCL_ROOT}"
  echo "             DPC++: ${DPCPP}       SYCL_ROOT: ${APP_SYCL_ROOT}"
  echo "             Metal: ${METAL}"
  echo -e "${mC} ========================= ${eC}"

  # pull OCCA git repo #
  git submodule init
  git submodule update
  cd occa

  # configure OCCA
  HIP_ROOT=${APP_HIP_ROOT}               \
  CUDATookit_ROOT=${APP_CUDATookit_ROOT} \
  OpenCL_ROOT=${APP_OpenCL_ROOT}         \
  SYCL_ROOT=${APP_SYCL_ROOT}             \
  INSTALL_DIR=${INSTALL_OCCA_DIRECTORY}  \
  OCCA_ENABLE_HIP=${HIP}                 \
  OCCA_ENABLE_CUDA=${CUDA}               \
  OCCA_ENABLE_OPENCL=${OPENCL}           \
  OCCA_ENABLE_OPENMP=${OPENMP}           \
  OCCA_ENABLE_DPCPP=${DPCPP}             \
  OCCA_ENABLE_METAL=${METAL}             \
  ./configure-cmake.sh

  # build OCCA
  cd build
  ${MAKE_CMD}
  cd ${PROJECT_ROOT}
fi

# ================================== #
# BUILD and INSTALL APPLICATION CODE
# ================================== #
if [ $BUILD_APP == 1 ]; then
  echo " "
  echo -e "${mC} ===================== Building APP ==================== ${eC}"
  echo         "   Compiling Options:"
  echo         "          Build Type: ${BUILD_TYPE}"
  echo         "          Unit Tests: ${UNIT_TEST}"
  echo         " "
  echo         "                  CC: ${CC}"
  echo         "                 CXX: ${CXX}"
  echo         "                  FC: ${FC}"
  echo         " "
  echo         "            CC Flags: ${C_FLAGS}"
  echo         "           CXX Flags: ${CXX_FLAGS}"
  echo         "            FC Flags: ${FC_FLAGS}"
  echo         " "
  echo         "          Build Type: ${BUILD_TYPE}"
  echo         "      Build Location: ${COMPILE_BUILD_APP_DIRECTORY}"
  echo         "    Install Location: ${COMPILE_INSTALL_APP_DIRECTORY}"
  echo         " Executable Location: ${COMPILE_BUILD_APP_DIRECTORY}/bin"
  echo -e "${mC} ======================================================== ${eC}"
  echo " "

  # move to the build directory
  cd $BUILD_DIRECTORY

  if [ ! -d $COMPILE_BUILD_APP_DIRECTORY ]; then
    echo  "${COMPILE_BUILD_APP_DIRECTORY} does not exist. Making it..."
    mkdir $COMPILE_BUILD_APP_DIRECTORY
  fi
  cd $COMPILE_BUILD_APP_DIRECTORY

  cmake -D CMAKE_C_COMPILER=${CC_PATH}                              \
        -D CMAKE_CXX_COMPILER=${CXX_PATH}                           \
        -D CMAKE_Fortran_COMPILER=${FC_PATH}                        \
        -D CMAKE_C_FLAGS=${C_FLAGS}                                 \
        -D CMAKE_CXX_FLAGS=${CXX_FLAGS}                             \
        -D CMAKE_Fortran_FLAGS=${Fortran_FLAGS}                     \
        -D CMAKE_LINKER=${LD_PATH}                                  \
        -D CMAKE_INSTALL_PREFIX=${COMPILE_INSTALL_APP_DIRECTORY}    \
        -D CMAKE_BUILD_TYPE=${BUILD_TYPE}                           \
        -D occa_dir=${INSTALL_OCCA_DIRECTORY}                       \
        -D SOLVER_DIR=${SOLVER_SRC_DIRECTORY}                       \
        -G "Unix Makefiles" ${APP_SOURCE_DIRECTORY} | tee cmake_config.out

  ${MAKE_CMD}
  cd ${PROJECT_ROOT}

  if [ ! -d "${COMPILE_INSTALL_APP_DIRECTORY}" ]; then
    echo "ERROR:"
    echo "${COMPILE_INSTALL_APP_DIRECTORY} does not exist."
    COMPILE_FAIL=1
  fi
fi


if [ ${COMPILE_FAIL} == 0 ]; then
  echo " "
  echo -e " ========================================================== "
  echo -e " ${gC}APP build successful! ${eC}"
  echo    "   Compiling Options:"
  echo    "          Build Type: ${BUILD_TYPE}"
  echo    "          Unit Tests: ${UNIT_TEST}"
  echo    " "
  echo    "                  CC: ${CC}"
  echo    "                 CXX: ${CXX}"
  echo    "                  FC: ${FC}"
  echo    " "
  echo    "             C Flags: ${C_FLAGS}"
  echo    "           CXX Flags: ${CXX_FLAGS}"
  echo    "            FC Flags: ${FC_FLAGS}"
  echo    " "
  echo    "          Build Type: ${BUILD_TYPE}"
  echo    "      Build Location: ${COMPILE_BUILD_APP_DIRECTORY}"
  echo -e "                    : ${GC}make clean; make -j install${eC} in this directory"
  echo    "    Install Location: ${COMPILE_INSTALL_APP_DIRECTORY}"
  echo    " Executable Location: ${COMPILE_BUILD_APP_DIRECTORY}/bin"
  echo -e " ========================================================== "
  echo    " "
  EXE_LOC=${COMPILE_BUILD_APP_DIRECTORY}/bin
  DATA_LOC=${PROJECT_ROOT}/data

  cd ${PROJECT_ROOT}
  echo "$(pwd)"

  # Create hyperlink to bin directory
  ln -sf ${EXE_LOC}

  # Create hyperlines to example meshes/data
  pushd ${EXE_LOC}
  ln -sf ${DATA_LOC}/ex05 2>/dev/null
  ln -sf ${DATA_LOC}/ex06 2>/dev/null
  ln -sf ${DATA_LOC}/ex10 2>/dev/null
  ln -sf ${DATA_LOC}/ex20 2>/dev/null
  
  # copy symlink script and execute for ex05 (smallest)
  cp ${DATA_LOC}/setdata.sh ${COMPILE_BUILD_APP_DIRECTORY}/bin/.
  ./setdata.sh ex05
  popd

else
  echo " "
  echo         "==================="
  echo -e "${rC} APP build FAILED! ${eC}"
  echo         "==================="
  echo " "
  exit 1
fi

cd ${CURRENT_PATH}
