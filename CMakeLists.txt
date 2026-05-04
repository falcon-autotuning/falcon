cmake_minimum_required(VERSION 3.20)
project(
  falcon
  VERSION 1.1.3
  DESCRIPTION "Full falcon docker image for dsl control and development"
  HOMEPAGE_URL "https://github.com/falcon-autotuning/falcon"
  LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build Options
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build example plugins" ON)

# Performance options
option(USE_CCACHE "Enable ccache or sccache if available" ON)
option(ENABLE_PCH
       "Enable target_precompile_headers to speed up builds (opt-in)" ON)

# Windows-specific definitions
if(WIN32)
  add_compile_definitions(NOMINMAX)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Try to auto-detect a compiler launcher (prefer sccache if present)
if(USE_CCACHE)
  find_program(SCCACHE_PROGRAM sccache)
  if(SCCACHE_PROGRAM)
    message(STATUS "Using sccache: ${SCCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
  else()
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
      message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
      set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    else()
      message(
        STATUS "No ccache/sccache found (fine, builds will just be normal).")
    endif()
  endif()
endif()

find_package(falcon-core              CONFIG REQUIRED)
find_package(falcon-database          CONFIG REQUIRED)
find_package(falcon-routine           CONFIG REQUIRED)
find_package(falcon-comms             CONFIG REQUIRED)
find_package(falcon-dsl               CONFIG REQUIRED)
find_package(falcon-package-manager   CONFIG REQUIRED)
find_package(falcon-typing            CONFIG REQUIRED)

# ============================================================================
# Installation Prefix Configuration
# ============================================================================
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(WIN32)
    set(CMAKE_INSTALL_PREFIX "C:/falcon" CACHE PATH "Installation prefix" FORCE)
  else()
    set(CMAKE_INSTALL_PREFIX "/opt/falcon" CACHE PATH "Installation prefix" FORCE)
  endif()
endif()
# ============================================================================
# CPack General Settings
# ============================================================================
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_SOURCE_IGNORE_FILES
  build/
  ".*~$"
  vcpkg/
  vcpkg_installed/
)
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

# ============================================================================
# Platform-Specific CPack Configuration
# ============================================================================
if(WIN32)
  set(CPACK_GENERATOR "ZIP")
  set(CPACK_ARCHIVE_THREADS 0)
else()
  # Linux: TGZ format (extractable via curl | tar xz)
  set(CPACK_GENERATOR "TGZ")
  set(CPACK_ARCHIVE_THREADS 0)
endif()

# ============================================================================
# Installation Directory Structure
# ============================================================================
set(INSTALL_BIN_DIR "bin")
set(INSTALL_LIB_DIR "lib")
set(INSTALL_INCLUDE_DIR "include")
set(INSTALL_SHARE_DIR "share")
set(INSTALL_DOC_DIR "doc")

# ============================================================================
# Install Targets
# ============================================================================

install(
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
  DESTINATION "${INSTALL_DOC_DIR}"
)

# ============================================================================
# Install vcpkg dependencies
# ============================================================================
if(WIN32)
  file(GLOB_RECURSE WIN_DLLS "${VCPKG_INSTALLED_DIR}/x64-win-llvm/bin/*.dll")
  install(
    PROGRAMS ${WIN_DLLS}
    DESTINATION "${INSTALL_BIN_DIR}"
  )
  file(GLOB_RECURSE WIN_LIBS "${VCPKG_INSTALLED_DIR}/x64-win-llvm/lib/*.lib")
  install(
    FILES ${WIN_LIBS}
    DESTINATION "${INSTALL_LIB_DIR}"
  )
  install(
    DIRECTORY "${VCPKG_INSTALLED_DIR}/x64-win-llvm/include/"
    DESTINATION "${INSTALL_INCLUDE_DIR}"
  )
  install(
    DIRECTORY "${VCPKG_INSTALLED_DIR}/x64-win-llvm/share/"
    DESTINATION "${INSTALL_SHARE_DIR}"
  )
else()
  file(GLOB_RECURSE LINUX_BINS "${VCPKG_INSTALLED_DIR}/x64-linux-dynamic/bin/*")
  install(
    PROGRAMS ${LINUX_BINS}
    DESTINATION "${INSTALL_BIN_DIR}"
  )
  file(GLOB_RECURSE LINUX_LIBS_A "${VCPKG_INSTALLED_DIR}/x64-linux-dynamic/lib/*.a")
  file(GLOB_RECURSE LINUX_LIBS_SO "${VCPKG_INSTALLED_DIR}/x64-linux-dynamic/lib/*.so*")
  install(
    FILES ${LINUX_LIBS_A} ${LINUX_LIBS_SO}
    DESTINATION "${INSTALL_LIB_DIR}"
  )
  install(
    DIRECTORY "${VCPKG_INSTALLED_DIR}/x64-linux-dynamic/include/"
    DESTINATION "${INSTALL_INCLUDE_DIR}"
  )
  install(
    DIRECTORY "${VCPKG_INSTALLED_DIR}/x64-linux-dynamic/share/"
    DESTINATION "${INSTALL_SHARE_DIR}"
  )
endif()

include(CPack)
