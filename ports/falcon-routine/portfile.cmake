vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-routine
    REF v${VERSION}
    SHA512 54481fa63295013c1458554b2e67457a4299e78b5bc81e813c370d5b8a99c783ddab85fe8bbf667962bcb07279eab07bfb9fb48e9a6b4be5220a371e58ac246a
)


vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

file(INSTALL "${SOURCE_PATH}/LICENSE"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
     RENAME copyright)

vcpkg_copy_pdbs()
