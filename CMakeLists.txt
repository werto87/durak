#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
cmake_minimum_required(VERSION 3.17)
project(project VERSION 1.0.0
  DESCRIPTION project
  LANGUAGES CXX
  ) 
cmake_policy(SET CMP0015 NEW)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

########## enable cache system
include(cmake/Cache.cmake)


########## link against the libs to apply the configurations to the target
# compiler warnings
set(PROJECT_WARNINGS project_warning)
add_library(${PROJECT_WARNINGS} INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(${PROJECT_WARNINGS})
# compiler options
set(PROJECT_OPTIONS project_options)


add_library(${PROJECT_OPTIONS} INTERFACE)
target_compile_features(${PROJECT_OPTIONS} INTERFACE cxx_std_20)
# target_precompile_headers(${PROJECT_OPTIONS} INTERFACE 
# )


# include(cmake/Sanitizers.cmake)
# enable_sanitizers(${PROJECT_OPTIONS})

# allow for static analysis options
#include(cmake/StaticAnalyzers.cmake) 
 
######## conan setup
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)


set(TARGET_NAME durak)
add_library(${TARGET_NAME} INTERFACE)

######## add sub dir
add_subdirectory(durak) 


####### link


target_include_directories(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${TARGET_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

  
target_compile_definitions(${TARGET_NAME} INTERFACE 
$<$<CONFIG:DEBUG>:DEBUG>
)


target_link_libraries(${TARGET_NAME} INTERFACE
    ${PROJECT_WARNINGS}
    ${PROJECT_OPTIONS}
    CONAN_PKG::range-v3
    CONAN_PKG::pipes
    CONAN_PKG::magic_enum
    CONAN_PKG::fmt
    CONAN_PKG::boost
    CONAN_PKG::confu_json

  )
enable_testing()
add_subdirectory(test)