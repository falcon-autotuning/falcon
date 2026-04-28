vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-package-manager
    REF v${VERSION}
    SHA512 162a0b4635016910d8adf2268e0f76dd0bc36889c0c1a375375d4b424df627299a4124360b05059cd9d5a39df1287ac206a2680deb54d70619cb071964113f4a
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
vcpkg_copy_tools(TOOL_NAMES falcon-pm AUTO_CLEAN)
vcpkg_copy_pdbs()
