find_package(MPI)

if(MPI_FOUND)
    # Mixed compilation: .hg kernel + .cpp MPI host
    add_executable(mpi_ppu_dispatch mpi_ppu_dispatch.hg mpi_ppu_dispatch.cpp)
    target_include_directories(mpi_ppu_dispatch PRIVATE ${COMMON_DIR})
    target_link_libraries(mpi_ppu_dispatch PRIVATE HGGC::toolkit MPI::MPI_CXX)
else()
    message(STATUS "MPI not found - will not build sample 'mpi_ppu_dispatch'")
endif()
