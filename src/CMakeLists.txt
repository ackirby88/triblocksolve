# ============ #
# Include file #
# ============ #
include_directories($ENV{OCCA_DIR}/include)
find_library(occa_lb NAMES occa PATHS $ENV{OCCA_DIR}/lib)

# ======= #
# Defines #
# ======= #
add_definitions("-DSOLVER_DIR=\"${SOLVER_DIR}\"")
message("[APP] >> SOLVER Directory: ${SOLVER_DIR}")


# ============ #
# Source files #
# ============ #
set(SRC_F90
    F90/read_mesh_data.F90
    F90/read_jacobian_data.F90
)

set(SRC
    Platform.cxx
    Mesh.cxx
    Jacobian.cxx
)

# ==================== #
# Build shared library #
# ==================== #
add_library(triblock SHARED ${SRC} ${SRC_F90})
target_link_libraries(triblock ${occa_lb} ${MPI_C_LIBRARIES})

# ================ #
# Build executable #
# ================ #
add_executable(triblock.exe main.cxx)
target_link_libraries(triblock.exe triblock ${occa_lb} ${MPI_C_LIBRARIES})

# ================================== #
# Install execuatable and shared lib #
# ================================== #
install(TARGETS triblock.exe triblock
        RUNTIME DESTINATION bin/
        LIBRARY DESTINATION lib/
        ARCHIVE DESTINATION lib/
)
