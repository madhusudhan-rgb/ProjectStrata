cmake_minimum_required(VERSION 3.19)
project(Strata LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(Strata
    WIN32 MACOSX_BUNDLE
    main.cpp
    terminal.h
    terminal.cpp



)

qt_add_translations(
    TARGETS Strata
    TS_FILES Strata_en_US.ts
)

target_link_libraries(Strata
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS Strata
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Strata
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
