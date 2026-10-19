vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/mock-hub
    REF v${VERSION}
    SHA512 e0e7efc5fe7d0070730369ead05837a097633ec39fa92abad7321bb415aec3b5e4fef321d661483299ac995c0a790f252dc839b1f25c135bce50f96c6b0dd83e
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTS=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

file(INSTALL "${SOURCE_PATH}/LICENSE"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
     RENAME copyright)

vcpkg_copy_pdbs()
