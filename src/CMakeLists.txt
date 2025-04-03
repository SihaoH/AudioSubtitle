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
)

# 创建可执行文件
add_executable(${TARGET} ${SOURCES})

# 设置库的依赖
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${TARGET}
    PUBLIC
    Qt6::Core
    PRIVATE
    spdlog::spdlog_header_only
    libvosk
)

# 修改从VS调试时的工作目录路径
set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY_INTDIR})

install(TARGETS ${TARGET} RUNTIME DESTINATION .)
# 安装所依赖的dll（需要在vcvarsall.bat打开的环境下使用）
set(APP "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${TARGET}.exe")
set(DIRS "${CMAKE_OUTPUT_INSTALL_INTDIR}")
install(CODE "
    include(BundleUtilities)
    fixup_bundle(\"${APP}\"  \"\"  \"${DIRS}\")
   ")