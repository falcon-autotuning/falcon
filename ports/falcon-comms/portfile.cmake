vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-comms
    REF v${VERSION}
    SHA512 a31bd7a152cd5c39d58395654a3cfcac3fd24f608b0b3340f970884b4b9983d1ca0f87c8f3a4d502743f62188b5d97bc3d4b8fe6a6c764a4431c1d739fa8ba53
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
