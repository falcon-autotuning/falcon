vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-routine
    REF v${VERSION}
    SHA512 c68e49f8f1357b86d2627596ab80d75d82ee5f814a7a51f9cb7f34aedae94f534114423d151553b22de961e307eb505f85ca621d5fc12b9637b2f19873236fc1
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
