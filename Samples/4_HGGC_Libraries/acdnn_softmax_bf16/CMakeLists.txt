add_executable(acdnn_softmax_bf16 acdnn_softmax_bf16.hg)
target_include_directories(acdnn_softmax_bf16 PRIVATE ${COMMON_DIR})
target_link_libraries(acdnn_softmax_bf16 PRIVATE HGGC::toolkit HGGC::acdnn)
