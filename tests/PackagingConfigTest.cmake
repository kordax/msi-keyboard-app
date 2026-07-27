if(NOT DEFINED CPACK_CONFIG OR NOT EXISTS "${CPACK_CONFIG}")
    message(FATAL_ERROR "CPackConfig.cmake was not provided")
endif()
if(NOT DEFINED SOURCE_DIR OR NOT IS_DIRECTORY "${SOURCE_DIR}")
    message(FATAL_ERROR "SOURCE_DIR was not provided")
endif()
if(NOT DEFINED APP_EXECUTABLE)
    message(FATAL_ERROR "APP_EXECUTABLE was not provided")
endif()
get_filename_component(app_executable_name "${APP_EXECUTABLE}" NAME)
if(NOT app_executable_name STREQUAL "msi-keyboard-app")
    message(FATAL_ERROR
        "Installed executable must be msi-keyboard-app, got ${app_executable_name}")
endif()

file(READ "${CPACK_CONFIG}" cpack_config)
foreach(expected IN ITEMS
    "set(CPACK_PACKAGE_NAME \"msi-keyboard-app\")"
    "set(CPACK_PACKAGE_VERSION \"0.1.3\")"
    "set(CPACK_DEBIAN_PACKAGE_BREAKS \"msi-keyboard\")"
    "set(CPACK_DEBIAN_PACKAGE_CONFLICTS \"msi-keyboard\")"
    "set(CPACK_DEBIAN_PACKAGE_REPLACES \"msi-keyboard (<= 0.1.1-1)\")")
    string(FIND "${cpack_config}" "${expected}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Missing package configuration: ${expected}")
    endif()
endforeach()

set(default_icon "${SOURCE_DIR}/assets/icons/io.github.kordax.MsiKeyboard.svg")
set(cinnamon_icon
    "${SOURCE_DIR}/assets/icons/io.github.kordax.MsiKeyboard-cinnamon.svg")
foreach(icon IN ITEMS "${default_icon}" "${cinnamon_icon}")
    if(NOT EXISTS "${icon}")
        message(FATAL_ERROR "Application icon is missing: ${icon}")
    endif()
endforeach()

set(default_desktop
    "${SOURCE_DIR}/packaging/io.github.kordax.MsiKeyboard.desktop")
set(cinnamon_desktop
    "${SOURCE_DIR}/packaging/io.github.kordax.MsiKeyboard.Cinnamon.desktop")
file(READ "${default_desktop}" default_desktop_contents)
file(READ "${cinnamon_desktop}" cinnamon_desktop_contents)
string(FIND
    "${default_desktop_contents}"
    "Icon=io.github.kordax.MsiKeyboard"
    default_icon_position)
if(default_icon_position EQUAL -1)
    message(FATAL_ERROR "Default desktop file has the wrong icon name")
endif()
string(FIND
    "${default_desktop_contents}"
    "Exec=msi-keyboard-app"
    default_exec_position)
if(default_exec_position EQUAL -1)
    message(FATAL_ERROR "Default desktop file has the wrong executable")
endif()
string(FIND
    "${cinnamon_desktop_contents}"
    "Icon=io.github.kordax.MsiKeyboard-cinnamon"
    cinnamon_icon_position)
if(cinnamon_icon_position EQUAL -1)
    message(FATAL_ERROR "Cinnamon desktop file has the wrong icon name")
endif()
string(FIND
    "${cinnamon_desktop_contents}"
    "Exec=msi-keyboard-app"
    cinnamon_exec_position)
if(cinnamon_exec_position EQUAL -1)
    message(FATAL_ERROR "Cinnamon desktop file has the wrong executable")
endif()

foreach(script IN ITEMS
    "${SOURCE_DIR}/packaging/debian/postinst"
    "${SOURCE_DIR}/packaging/debian/postrm"
    "${SOURCE_DIR}/packaging/rpm/post-install.sh"
    "${SOURCE_DIR}/packaging/rpm/post-uninstall.sh")
    file(READ "${script}" script_contents)
    foreach(command IN ITEMS gtk-update-icon-cache update-desktop-database)
        string(FIND "${script_contents}" "${command}" position)
        if(position EQUAL -1)
            message(FATAL_ERROR "${script} does not refresh ${command}")
        endif()
    endforeach()
endforeach()
