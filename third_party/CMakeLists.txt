# 添加vosk语音识别库，由vosk-api-win64-0.3.45.zip解压得到
set(VOSK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vosk-win64-0.3.45")
add_library(libvosk INTERFACE)
target_include_directories(libvosk INTERFACE ${VOSK_ROOT})
target_link_libraries(libvosk INTERFACE ${VOSK_ROOT}/libvosk.lib)

add_custom_target(copy_vosk_dlls ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${VOSK_ROOT}/libgcc_s_seh-1.dll"
    "${VOSK_ROOT}/libstdc++-6.dll"
    "${VOSK_ROOT}/libvosk.dll"
    "${VOSK_ROOT}/libwinpthread-1.dll"
    "${CMAKE_OUTPUT_DIRECTORY_INTDIR}"
)
add_dependencies(copy_vosk_dlls libvosk)
set_target_properties(copy_vosk_dlls PROPERTIES FOLDER vosk_dir)

install(FILES
    "${VOSK_ROOT}/libgcc_s_seh-1.dll"
    "${VOSK_ROOT}/libstdc++-6.dll"
    "${VOSK_ROOT}/libvosk.dll"
    "${VOSK_ROOT}/libwinpthread-1.dll"
    DESTINATION .
)

# 太慢了，先不用
# 添加whisper库，来源https://github.com/ggml-org/whisper.cpp.git
# 可用模型从https://huggingface.co/ggerganov/whisper.cpp/tree/main下载
#add_definitions(-DGGML_BLAS=1)
#add_definitions(-DGGML_VULKAN=1)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/whisper-cpp)
#set_target_properties(ggml PROPERTIES FOLDER whisper_dir)
#set_target_properties(ggml-base PROPERTIES FOLDER whisper_dir)
#set_target_properties(ggml-cpu PROPERTIES FOLDER whisper_dir)
#set_target_properties(whisper PROPERTIES FOLDER whisper_dir)

# 添加sentencepiece库，由vcpkg安装
find_package(absl CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_library(SENTENCEPIECE_LIB sentencepiece)
if(NOT SENTENCEPIECE_LIB OR NOT SENTENCEPIECE_TRAIN_LIB)
    message(FATAL_ERROR "未找到 sentencepiece 库文件")
endif()
add_library(libsentencepiece INTERFACE)
target_include_directories(libsentencepiece INTERFACE ${SENTENCEPIECE_ROOT})
target_link_libraries(libsentencepiece INTERFACE
    absl::flags_usage
    protobuf::libprotobuf
    ${SENTENCEPIECE_LIB}
)

# 添加ctranslate2库，来源https://github.com/OpenNMT/CTranslate2/tree/v4.5.0
find_package(OpenBLAS REQUIRED)
set(BUILD_CLI OFF)
set(WITH_MKL OFF)
set(WITH_OPENBLAS ON)
set(CUDA_DYNAMIC_LOADING ON)
set(OPENMP_RUNTIME "COMP")
set(OPENBLAS_INCLUDE_DIR ${OpenBLAS_INCLUDE_DIRS})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/CTranslate2)
set_target_properties(cpu_features PROPERTIES FOLDER ctranslate2_dir)
set_target_properties(utils PROPERTIES FOLDER ctranslate2_dir)
set_target_properties(ctranslate2 PROPERTIES FOLDER ctranslate2_dir)

