vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-comms
    REF v${VERSION}
    SHA512 11ee64b269160d04bbd0481875e13880fcedcb45e1be9c00fdab5b229cfe3ffb30456a854e5d875b9053bc0c4c30c4f787899323dbb995ee080f480601be6603
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
