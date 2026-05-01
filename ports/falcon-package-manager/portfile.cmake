vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-package-manager
    REF v${VERSION}
    SHA512 eec0035fedfeb603426d2e3423b9e9610c21deb967645a47abf687b1086f74aa3601b1d787dd2223a641bc14daed2a56d5a87b87d32b8fc06a02ea4988e003c4
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
