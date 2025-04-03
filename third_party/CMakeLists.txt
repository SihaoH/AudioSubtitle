# 添加vosk语音识别库
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
set_target_properties(copy_vosk_dlls PROPERTIES FOLDER runtime)

install(FILES
    "${VOSK_ROOT}/libgcc_s_seh-1.dll"
    "${VOSK_ROOT}/libstdc++-6.dll"
    "${VOSK_ROOT}/libvosk.dll"
    "${VOSK_ROOT}/libwinpthread-1.dll"
    DESTINATION .
)
