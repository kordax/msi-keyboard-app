include_guard(GLOBAL)

set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_PACKAGE_NAME "msi-keyboard")
set(CPACK_PACKAGE_VENDOR "Dmitry Morozov")
set(CPACK_PACKAGE_CONTACT "Dmitry Morozov <kordaxmint@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Manage MSI keyboards on Linux")
set(
    CPACK_PACKAGE_DESCRIPTION
    "MSI Keyboard is a Linux application for supported MSI keyboards."
)
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/kordax/msi-keyboard-app")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "msi-keyboard")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_STRIP_FILES ON)
set(
    CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE
)

set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_PACKAGE_RELEASE "1")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-image-formats-plugins")
set(
    CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${PROJECT_SOURCE_DIR}/packaging/debian/postinst"
    "${PROJECT_SOURCE_DIR}/packaging/debian/postrm"
)

set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")
set(CPACK_RPM_PACKAGE_RELEASE "1")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtimageformats")
set(
    CPACK_RPM_POST_INSTALL_SCRIPT_FILE
    "${PROJECT_SOURCE_DIR}/packaging/rpm/post-install.sh"
)
set(
    CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE
    "${PROJECT_SOURCE_DIR}/packaging/rpm/post-uninstall.sh"
)
list(
    APPEND
    CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
    "/usr/lib/udev"
    "/usr/lib/udev/rules.d"
    "/usr/share/applications"
    "/usr/share/licenses"
    "/usr/share/licenses/msi-keyboard"
)

include(CPack)
