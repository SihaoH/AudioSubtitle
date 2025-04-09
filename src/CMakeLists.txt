set(TARGET app)
set(CMAKE_AUTOMOC ON)

set(SOURCES
    main.cpp
    Logger.h
    Logger.cpp
    AudioCapturer.h
    AudioCapturer.cpp
    AudioConverter.h
    AudioConverter.cpp
    TextTranslator.h
    TextTranslator.cpp
    SubtitleWidget.h
    SubtitleWidget.cpp
    Application.h
    Application.cpp
)

# 创建可执行文件
add_executable(${TARGET} ${SOURCES})

# 设置库的依赖
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets EntryPointPrivate)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${TARGET}
    PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::EntryPointPrivate
    PRIVATE
    spdlog::spdlog_header_only
    libvosk
    libsentencepiece
    ctranslate2
)

add_definitions(-D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
add_definitions(-DALERT_ERROR_LOG)
add_definitions(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}/")

# 修改从VS调试时的工作目录路径
set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY_INTDIR})

# 设置输出文件名
set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

set_target_properties(${TARGET} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")

# 为config.json创建符号链接
add_custom_command(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_OUTPUT_DIRECTORY_INTDIR}/config.json
)

install(TARGETS ${TARGET} RUNTIME DESTINATION .)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config.json DESTINATION .)
# 安装所依赖的dll（需要在vcvarsall.bat打开的环境下使用）
set(APP "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${TARGET}.exe")
set(DIRS "${CMAKE_OUTPUT_INSTALL_INTDIR}")
install(CODE "
    include(BundleUtilities)
    fixup_bundle(\"${APP}\"  \"\"  \"${DIRS}\")
   ")