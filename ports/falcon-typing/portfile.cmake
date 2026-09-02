vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-typing
    REF v${VERSION}
    SHA512 b352e54aade4a23f081406c21a1b1272403efc2f999fe50f5064c2358e3a5c8e2ddbc2bb175cbdda9497092623efc272e806bbdfa16d3418f894b033c035b507
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
