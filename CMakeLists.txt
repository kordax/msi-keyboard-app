cmake_minimum_required(VERSION 3.24)

project(
    msi-keyboard
    VERSION 0.1.0
    DESCRIPTION "Linux application for MSI keyboards"
    LANGUAGES CXX
)

include(GNUInstallDirs)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

add_library(strikepro_project_options INTERFACE)
target_compile_features(strikepro_project_options INTERFACE cxx_std_20)
if(MSVC)
    target_compile_options(strikepro_project_options INTERFACE /W4 /permissive-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        strikepro_project_options
        INTERFACE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

add_library(strikepro_device STATIC
    src/device/AppPaths.cpp
    src/device/AppPaths.h
    src/device/BatteryDecoder.cpp
    src/device/BatteryDecoder.h
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

add_library(strikepro_gui STATIC
    src/gui/BatteryGauge.cpp
    src/gui/BatteryGauge.h
    src/gui/DebugWindow.cpp
    src/gui/DebugWindow.h
    src/gui/MainWindow.cpp
    src/gui/MainWindow.h
)
target_link_libraries(
    strikepro_gui
    PUBLIC strikepro_device Qt6::Widgets
    PRIVATE strikepro_project_options
)

qt_add_executable(msi-keyboard src/main.cpp)
qt_add_resources(
    msi-keyboard
    strikepro_assets
    PREFIX "/"
    FILES assets/keyboard/strike_pro.png
)
target_link_libraries(
    msi-keyboard
    PRIVATE strikepro_cli strikepro_gui strikepro_project_options
)

install(TARGETS msi-keyboard RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES packaging/io.github.kordax.MsiKeyboard.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES LICENSE
        DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/msi-keyboard)
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

    qt_add_executable(battery-gauge-tests
        tests/BatteryGaugeTest.cpp
    )
    qt_add_resources(
        battery-gauge-tests
        strikepro_test_assets
        PREFIX "/"
        FILES assets/keyboard/strike_pro.png
    )
    target_link_libraries(
        battery-gauge-tests
        PRIVATE strikepro_gui Qt6::Test strikepro_project_options
    )

    add_test(NAME battery-gauge-tests COMMAND battery-gauge-tests)
    set_tests_properties(
        battery-decoder-tests battery-gauge-tests
        PROPERTIES
            LABELS "unit"
            TIMEOUT 10
    )
    set_tests_properties(
        battery-gauge-tests
        PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endif()
