cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include(cmake/prelude.cmake)

project(
    Midas
    VERSION 0.3.0
    DESCRIPTION "Midas - a Multi Dimensional Analysis framework."
    HOMEPAGE_URL "https://github.com/rlalik/Midas"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)
include(cmake/find_or_fetch_package.cmake)

# find ROOT
if (DEFINED ENV{ROOTSYS})
  list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
endif()

find_package(ROOT QUIET REQUIRED COMPONENTS Core Gpad Hist)

set(FMT_INSTALL OFF)
find_or_fetch_package(fmt https://github.com/fmtlib/fmt GIT_TAG 10.0.0 VERSION 10.0.0)

find_package(RootTools REQUIRED)
find_package(Pandora 0.9.4 REQUIRED)
find_package(HelloFitty 0.9.6 REQUIRED)

# find_package(HistAsymmErrors QUIET)
if(HistAsymmErrors_FOUND)
  add_definitions("-DHAVE_HISTASYMMERRORS=1")
endif()

include(cmake/shared_or_static.cmake)
shared_or_static(${PROJECT_NAME})

# ---- Declare library ----
add_library(
    Midas
    ${Midas_LIBRARY_TYPE}
)

target_sources(Midas
    PRIVATE
        source/axis_config.cpp
        source/basic_context.cpp
        source/basic_distribution.cpp
        source/context.cpp
        source/distribution.cpp
        source/observable.cpp
        source/detail.cpp
)

add_library(Midas::Midas ALIAS Midas)

target_link_libraries(Midas PUBLIC ROOT::Core ROOT::Gpad pandora::pandora PRIVATE HelloFitty::HelloFitty)

if (fmt_FETCHED)
  set(FMT_TARGET $<BUILD_INTERFACE:fmt::fmt-header-only>)
else()
  set(FMT_TARGET fmt::fmt)
endif()
target_link_libraries(Midas PRIVATE ${FMT_TARGET})

# if (HelloFitty_FOUND)
#   target_compile_definitions(Midas PRIVATE HELLOFITTY_FOUND)
# endif()

# configure_file(config.h.in midas_config.h)

include(GenerateExportHeader)
generate_export_header(
    Midas
    BASE_NAME Midas
    EXPORT_FILE_NAME export/Midas/midas_export.hpp
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(Midas PUBLIC MIDAS_STATIC_DEFINE)
endif()

set_target_properties(
    Midas PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME Midas
    OUTPUT_NAME Midas
)

target_include_directories(
    Midas ${warning_guard}
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inc>
)

target_include_directories(
    Midas SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
)

option(BUILD_DICTIONARY "Build ROOT dictionary." ON)
if (BUILD_DICTIONARY)
  # cmake-format: off
  root_generate_dictionary(G__Midas_cc midas.hpp
    MODULE Midas
    LINKDEF inc/LinkDef.h)
  # cmake-format: on
endif()

# target_compile_features(Midas PUBLIC cxx_std_11)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${Midas_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(example)
  endif()
endif()

# ---- Developer mode ----

if(NOT Midas_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of Midas"
  )
endif()

include(cmake/dev-mode.cmake)

# Export the package for use from the build-tree (this registers the build-tree
# with a global CMake-registry)
export(PACKAGE Midas)

# messages
message(
  STATUS
    "<<< Configuration >>>
Project:        ${PROJECT_NAME}

Architecture:   ${CMAKE_CXX_LIBRARY_ARCHITECTURE}
Build type      ${CMAKE_BUILD_TYPE}
Install path    ${CMAKE_INSTALL_PREFIX}

Compiler:
C++             ${CMAKE_CXX_COMPILER}

Linker:
Ld              ${CMAKE_LINKER}

Compiler flags:
C++             ${CMAKE_CXX_FLAGS}

Linker flags:
Executable      ${CMAKE_EXE_LINKER_FLAGS}
Module          ${CMAKE_MODULE_LINKER_FLAGS}
Shared          ${CMAKE_SHARED_LINKER_FLAGS}\n")

foreach(p LIB BIN INCLUDE)
  message(STATUS "CMAKE_INSTALL_${p}DIR: ${CMAKE_INSTALL_${p}DIR}")
endforeach()
message(STATUS "Midas_INSTALL_CMAKEDIR: ${Midas_INSTALL_CMAKEDIR}")
