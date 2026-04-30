vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-package-manager
    REF v${VERSION}
    SHA512 4c9cd11267d31c083c4dcb5f04dbbbcb67e51d4954ec760ed007f31be27dd6dfd1c8ce2b882a0196db2c491625e325b6ea2177eba46092237f665766a9f2bac7
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
