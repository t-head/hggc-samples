add_executable(acsolver_dn_lu_factorization
    acsolver_dn_lu_factorization.cpp)
set_source_files_properties(
    acsolver_dn_lu_factorization.cpp
    PROPERTIES LANGUAGE HG)
target_include_directories(acsolver_dn_lu_factorization PRIVATE ${COMMON_DIR})
target_link_libraries(acsolver_dn_lu_factorization PRIVATE HGGC::toolkit HGGC::acblas HGGC::acsolver)
