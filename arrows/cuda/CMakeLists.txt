set(header_files
  integrate_depth_maps.h
  )

set(cxx_files
  integrate_depth_maps.cxx
  )

kwiver_install_headers(
  SUBDIR     arrows/cuda
  ${header_files}
  )

set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} --gpu-architecture sm_30)
cuda_compile( CUDA_COMPILE_O integrate_depth_maps.cu OPTIONS -G)

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_arrows_cuda_export.h
  NOPATH   SUBDIR     arrows/cuda
  )

kwiver_add_library(kwiver_arrows_cuda ${header_files} ${cxx_files} ${CUDA_COMPILE_O})
target_link_libraries(kwiver_arrows_cuda kwiver_arrows_core ${CUDA_LIBRARIES}
)

algorithms_create_plugin( kwiver_arrows_cuda
  register_algorithms.cxx
  )

