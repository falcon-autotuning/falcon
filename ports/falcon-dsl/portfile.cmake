vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-dsl
    REF v${VERSION}
    SHA512 8d01a95e8c9ecd4296d66ea52af8bb30670637ad82e96b45b87e12f8b7969e4e25c236cf7247b60af79893e9a1e2f5092b9241456d8d3b0a784d8e35e0ec23cf
)
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS -DBUILD_TESTS=OFF
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup()
file(INSTALL "${SOURCE_PATH}/LICENSE"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
     RENAME copyright)
vcpkg_copy_pdbs()
