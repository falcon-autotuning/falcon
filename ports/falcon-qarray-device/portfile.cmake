vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-qarray-device
    REF v${VERSION}
    SHA512 db78dd628a4445a199d1fa0d0897c3ed1a4a72935c36c911a17547744888738c326fba761b502f2af1633e972206f7e4bacce6a0eef4bc9d2560c11258c0f393
)
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTS=OFF
        -DPython3_EXECUTABLE=${PYTHON_EXECUTABLE}
        -DPython3_ROOT_DIR=${VENV_DIR}
        -DPython3_FIND_STRATEGY=LOCATION
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup()
file(INSTALL "${SOURCE_PATH}/LICENSE"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
     RENAME copyright)
vcpkg_copy_pdbs()
