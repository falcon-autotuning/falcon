vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-dsl
    REF v${VERSION}
    SHA512 ecd99eacb0807fa046d280249acc1b895b4285283e871aea01304ab16b96543b00faebd954a2f60832e6c41ea80fceeb979d4135d1e2ee4f74174ac554bc01bc
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
vcpkg_copy_tools(TOOL_NAMES falcon-run falcon-test AUTO_CLEAN)
vcpkg_copy_pdbs()
