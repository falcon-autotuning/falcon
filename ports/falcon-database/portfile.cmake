vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-database
    REF v${VERSION}
    SHA512 0e2d599e2af5011c6c1f93026a22a1501e10fffdcd1aec75c15258a8712ea1fa2745a51735c40d9a927813ce9d1b57fc1ea29a237f113a227f015d232d637cc3
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
