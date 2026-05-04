vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-comms
    REF v${VERSION}
    SHA512 51e2315de2eaa37b0fafe8ccd93b60393945715737ac661c6e64cd4380a56cc03b952e8143cb4d4e676e556e5727132166398c9ef289f3954a0fb28b3359879f
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
