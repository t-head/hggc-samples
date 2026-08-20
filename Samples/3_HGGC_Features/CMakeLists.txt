# Helper: add a ppu001-only sample so that it compiles for ppu_10 only,
# even when the global default includes ppu_15 (ppu0015 does not support
# tc01 Tensor Cell or the ppu001 AIU asynchronous copy instructions).
function(_hg_add_ppu001_only subdir)
  if(CMAKE_HG_ARCHITECTURES MATCHES "ppu_15")
    string(REGEX REPLACE " *--gpu-architecture=ppu_15" "" _hg_flags_local "${CMAKE_HG_FLAGS}")
    if(NOT _hg_flags_local MATCHES "--gpu-architecture=ppu_10")
      set(_hg_flags_local "${_hg_flags_local} --gpu-architecture=ppu_10")
    endif()
    string(STRIP "${_hg_flags_local}" CMAKE_HG_FLAGS)
  endif()
  add_subdirectory(${subdir})
endfunction()

# Compatible with both ppu001 (ppu_10) and ppu0015 (ppu_15).
add_subdirectory(binary_partition_cg)
add_subdirectory(global_to_shmem_async_copy)
add_subdirectory(graph_conditional_nodes)
add_subdirectory(graph_memory_footprint)
add_subdirectory(graph_memory_nodes)
add_subdirectory(new_delete)
add_subdirectory(power_iteration_graph)
add_subdirectory(simple_hggc_graphs)

# ppu001-only: tc01 Tensor Cell / AIU asynchronous copy.
_hg_add_ppu001_only(bf16_tensor_cell_gemm)
_hg_add_ppu001_only(aiu_async_copy)
_hg_add_ppu001_only(aiu_gemm)
_hg_add_ppu001_only(imma_tensor_cell_gemm)
_hg_add_ppu001_only(tf32_tensor_cell_gemm)

# add_subdirectory(warp_aggregated_atomics_cg)
