cmake_minimum_required(VERSION 3.24)

project(
    msi-keyboard
    VERSION 0.0.1
    DESCRIPTION "Linux application for MSI keyboards"
    LANGUAGES CXX
)

include(GNUInstallDirs)

option(MSI_KEYBOARD_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(MSI_KEYBOARD_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core LinguistTools Network Widgets)
qt_standard_project_setup()

add_library(strikepro_project_options INTERFACE)
target_compile_features(strikepro_project_options INTERFACE cxx_std_20)
if(MSVC)
    target_compile_options(strikepro_project_options INTERFACE /W4 /permissive-)
    if(MSI_KEYBOARD_WARNINGS_AS_ERRORS)
        target_compile_options(strikepro_project_options INTERFACE /WX)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        strikepro_project_options
        INTERFACE
            -Wall
            -Wextra
            -Wpedantic
    )
    if(MSI_KEYBOARD_WARNINGS_AS_ERRORS)
        target_compile_options(strikepro_project_options INTERFACE -Werror)
    endif()
    if(MSI_KEYBOARD_ENABLE_SANITIZERS)
        target_compile_options(
            strikepro_project_options
            INTERFACE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )
        target_link_options(
            strikepro_project_options
            INTERFACE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )
    endif()
elseif(MSI_KEYBOARD_ENABLE_SANITIZERS)
    message(FATAL_ERROR "Sanitizers require GCC or Clang")
endif()

add_library(strikepro_i18n STATIC
    src/i18n/LanguageManager.cpp
    src/i18n/LanguageManager.h
)
target_include_directories(strikepro_i18n PUBLIC src)
target_link_libraries(
    strikepro_i18n
    PUBLIC Qt6::Core
    PRIVATE strikepro_project_options
)
qt_add_translations(
    strikepro_i18n
    TS_FILES translations/msi-keyboard_ru.ts
    RESOURCE_PREFIX "/i18n"
    LUPDATE_OPTIONS -no-obsolete
    SOURCES
        src/main.cpp
        src/cli/CliRunner.cpp
        src/device/BatteryDecoder.cpp
        src/device/DeviceCatalog.cpp
        src/device/HidMonitor.cpp
        src/gui/BatteryGauge.cpp
        src/gui/DebugWindow.cpp
        src/gui/MainWindow.cpp
        src/gui/TrayIndicator.cpp
        src/update/UpdaterCore.cpp
        src/update/UpgradeRunner.cpp
)

add_library(strikepro_device STATIC
    src/device/AppPaths.cpp
    src/device/AppPaths.h
    src/device/BatteryDecoder.cpp
    src/device/BatteryDecoder.h
    src/device/DeviceCatalog.cpp
    src/device/DeviceCatalog.h
    src/device/HidDeviceScanner.cpp
    src/device/HidDeviceScanner.h
    src/device/HidMonitor.cpp
    src/device/HidMonitor.h
    src/device/HidTypes.h
)
target_include_directories(strikepro_device PUBLIC src)
target_link_libraries(
    strikepro_device
    PUBLIC Qt6::Core
    PRIVATE strikepro_project_options
)

add_library(strikepro_cli STATIC
    src/cli/CliRunner.cpp
    src/cli/CliRunner.h
)
target_link_libraries(
    strikepro_cli
    PUBLIC strikepro_device
    PRIVATE strikepro_project_options
)

add_library(msi_keyboard_update STATIC
    src/update/UpdaterCore.cpp
    src/update/UpdaterCore.h
    src/update/UpgradeRunner.cpp
    src/update/UpgradeRunner.h
)
target_include_directories(msi_keyboard_update PUBLIC src)
target_link_libraries(
    msi_keyboard_update
    PUBLIC Qt6::Core
    PRIVATE Qt6::Network strikepro_project_options
)

add_library(strikepro_gui STATIC
    src/gui/BatteryGauge.cpp
    src/gui/BatteryGauge.h
    src/gui/DebugWindow.cpp
    src/gui/DebugWindow.h
    src/gui/MainWindow.cpp
    src/gui/MainWindow.h
    src/gui/TrayIndicator.cpp
    src/gui/TrayIndicator.h
)
target_link_libraries(
    strikepro_gui
    PUBLIC strikepro_device strikepro_i18n Qt6::Widgets
    PRIVATE strikepro_project_options
)

qt_add_executable(msi-keyboard src/main.cpp)
qt_add_resources(
    msi-keyboard
    strikepro_assets
    PREFIX "/"
    FILES
        assets/keyboard/strike_pro.webp
        assets/keyboard/strike_pro.png
)
target_link_libraries(
    msi-keyboard
    PRIVATE
        strikepro_cli
        strikepro_gui
        strikepro_i18n
        msi_keyboard_update
        strikepro_project_options
)
target_compile_definitions(
    msi-keyboard
    PRIVATE MSI_KEYBOARD_VERSION="${PROJECT_VERSION}"
)

install(TARGETS msi-keyboard RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES packaging/io.github.kordax.MsiKeyboard.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES packaging/io.github.kordax.MsiKeyboard.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES assets/icons/io.github.kordax.MsiKeyboard.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(FILES LICENSE
        DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/msi-keyboard)
install(FILES LICENSE
        DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/msi-keyboard
        RENAME copyright)
install(
    FILES packaging/70-msi-strike-pro.rules
    DESTINATION lib/udev/rules.d
    COMPONENT udev
)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.4 REQUIRED COMPONENTS Test)

    qt_add_executable(battery-decoder-tests
        tests/BatteryDecoderTest.cpp
    )
    target_link_libraries(
        battery-decoder-tests
        PRIVATE strikepro_device Qt6::Test strikepro_project_options
    )

    add_test(NAME battery-decoder-tests COMMAND battery-decoder-tests)

    qt_add_executable(hid-monitor-tests
        tests/HidMonitorTest.cpp
    )
    target_link_libraries(
        hid-monitor-tests
        PRIVATE strikepro_device Qt6::Test strikepro_project_options
    )

    add_test(NAME hid-monitor-tests COMMAND hid-monitor-tests)

    qt_add_executable(device-catalog-tests
        tests/DeviceCatalogTest.cpp
    )
    target_link_libraries(
        device-catalog-tests
        PRIVATE strikepro_device Qt6::Test strikepro_project_options
    )

    add_test(NAME device-catalog-tests COMMAND device-catalog-tests)

    qt_add_executable(battery-gauge-tests
        tests/BatteryGaugeTest.cpp
    )
    qt_add_resources(
        battery-gauge-tests
        strikepro_test_assets
        PREFIX "/"
        FILES
            assets/keyboard/strike_pro.webp
            assets/keyboard/strike_pro.png
    )
    target_link_libraries(
        battery-gauge-tests
        PRIVATE strikepro_gui Qt6::Test strikepro_project_options
    )

    add_test(NAME battery-gauge-tests COMMAND battery-gauge-tests)

    qt_add_executable(tray-indicator-tests
        tests/TrayIndicatorTest.cpp
    )
    target_link_libraries(
        tray-indicator-tests
        PRIVATE strikepro_gui Qt6::Test strikepro_project_options
    )

    add_test(NAME tray-indicator-tests COMMAND tray-indicator-tests)

    qt_add_executable(desktop-environment-tests
        tests/DesktopEnvironmentTest.cpp
    )
    qt_add_resources(
        desktop-environment-tests
        strikepro_desktop_test_assets
        PREFIX "/"
        FILES
            assets/keyboard/strike_pro.webp
            assets/keyboard/strike_pro.png
    )
    target_link_libraries(
        desktop-environment-tests
        PRIVATE strikepro_gui Qt6::Test strikepro_project_options
    )

    add_test(
        NAME desktop-light-theme-tests
        COMMAND desktop-environment-tests rendersWithLightTheme
    )
    add_test(
        NAME desktop-hidpi-tests
        COMMAND desktop-environment-tests rendersAtHighDpi
    )
    add_test(
        NAME desktop-wayland-tests
        COMMAND
            bash
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/verify-wayland.sh
            $<TARGET_FILE:desktop-environment-tests>
    )
    set_tests_properties(
        desktop-light-theme-tests
        PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_STYLE_OVERRIDE=Fusion"
            LABELS "desktop"
            TIMEOUT 10
    )
    set_tests_properties(
        desktop-hidpi-tests
        PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=2"
            LABELS "desktop"
            TIMEOUT 10
    )
    set_tests_properties(
        desktop-wayland-tests
        PROPERTIES
            LABELS "desktop"
            SKIP_RETURN_CODE 77
            TIMEOUT 30
    )

    qt_add_executable(localization-tests
        tests/LocalizationTest.cpp
    )
    target_link_libraries(
        localization-tests
        PRIVATE strikepro_i18n Qt6::Test strikepro_project_options
    )

    add_test(NAME localization-tests COMMAND localization-tests)

    qt_add_executable(updater-core-tests
        tests/UpdaterCoreTest.cpp
    )
    target_link_libraries(
        updater-core-tests
        PRIVATE msi_keyboard_update Qt6::Test strikepro_project_options
    )

    add_test(NAME updater-core-tests COMMAND updater-core-tests)

    add_test(
        NAME cli-english-default
        COMMAND msi-keyboard --cli --help
    )
    set_tests_properties(
        cli-english-default
        PROPERTIES
            ENVIRONMENT "LANG=ru_RU.UTF-8;LC_ALL=ru_RU.UTF-8"
            LABELS "smoke"
            PASS_REGULAR_EXPRESSION "Run without the GUI."
            TIMEOUT 10
    )

    add_test(
        NAME cli-russian-language
        COMMAND msi-keyboard --cli --language ru --help
    )
    set_tests_properties(
        cli-russian-language
        PROPERTIES
            LABELS "smoke"
            PASS_REGULAR_EXPRESSION "Запустить без графического интерфейса."
            TIMEOUT 10
    )

    add_test(
        NAME cli-headless-help
        COMMAND msi-keyboard --language en --help
    )
    set_tests_properties(
        cli-headless-help
        PROPERTIES
            ENVIRONMENT "DISPLAY=;WAYLAND_DISPLAY=;QT_QPA_PLATFORM="
            LABELS "smoke"
            PASS_REGULAR_EXPRESSION "Linux application for MSI keyboards"
            TIMEOUT 10
    )

    find_program(APPSTREAMCLI_EXECUTABLE NAMES appstreamcli)
    if(APPSTREAMCLI_EXECUTABLE)
        add_test(
            NAME appstream-metadata-tests
            COMMAND
                ${APPSTREAMCLI_EXECUTABLE}
                validate
                --no-net
                --strict
                ${CMAKE_CURRENT_SOURCE_DIR}/packaging/io.github.kordax.MsiKeyboard.metainfo.xml
        )
        set_tests_properties(
            appstream-metadata-tests
            PROPERTIES LABELS "metadata" TIMEOUT 10
        )
    endif()

    find_program(DESKTOP_FILE_VALIDATE_EXECUTABLE NAMES desktop-file-validate)
    if(DESKTOP_FILE_VALIDATE_EXECUTABLE)
        add_test(
            NAME desktop-file-tests
            COMMAND
                ${DESKTOP_FILE_VALIDATE_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/packaging/io.github.kordax.MsiKeyboard.desktop
        )
        set_tests_properties(
            desktop-file-tests
            PROPERTIES LABELS "metadata" TIMEOUT 10
        )
    endif()

    set_tests_properties(
        battery-decoder-tests
        hid-monitor-tests
        device-catalog-tests
        battery-gauge-tests
        tray-indicator-tests
        localization-tests
        updater-core-tests
        PROPERTIES
            LABELS "unit"
            TIMEOUT 10
    )
    set_tests_properties(
        battery-gauge-tests
        tray-indicator-tests
        PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endif()

include(cmake/Packaging.cmake)
