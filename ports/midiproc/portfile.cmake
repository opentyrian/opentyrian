vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO nikitalita/midiproc
    REF "${VERSION}"
    SHA512 aa885281d92197f68cc12d1b61d2b37f6860f6d54b8e3ed9a6eb6b1d805ccdb0d497f790be76e0482c348961e20550d8cc1979cc574f3a3863dfa7228265c333
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(
    PACKAGE_NAME midiproc
    CONFIG_PATH lib/cmake/midiproc)

vcpkg_fixup_pkgconfig()

vcpkg_copy_pdbs()

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/share"
    "${CURRENT_PACKAGES_DIR}/debug/include"
)
