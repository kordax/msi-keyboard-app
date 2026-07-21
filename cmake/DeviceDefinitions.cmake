function(msi_keyboard_escape_cpp input output)
    string(REPLACE "\\" "\\\\" escaped "${input}")
    string(REPLACE "\"" "\\\"" escaped "${escaped}")
    set(${output} "${escaped}" PARENT_SCOPE)
endfunction()

function(msi_keyboard_validate_hex_id value field device_id)
    string(LENGTH "${value}" value_length)
    if(NOT value_length EQUAL 4
       OR NOT value MATCHES "^[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]$")
        message(FATAL_ERROR
            "devices.json: ${device_id}.${field} must contain exactly four hexadecimal digits")
    endif()
endfunction()

function(msi_keyboard_generate_device_files json_file output_header output_udev)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${json_file}")
    file(READ "${json_file}" device_json)

    string(JSON schema_version GET "${device_json}" version)
    if(NOT schema_version EQUAL 1)
        message(FATAL_ERROR "devices.json: unsupported version ${schema_version}")
    endif()

    string(JSON device_count LENGTH "${device_json}" devices)
    if(device_count LESS 1)
        message(FATAL_ERROR "devices.json: at least one device is required")
    endif()

    get_filename_component(config_directory "${json_file}" DIRECTORY)
    get_filename_component(project_root "${config_directory}" DIRECTORY)
    set(entries "")
    set(artwork_files "")
    set(seen_ids "")
    set(seen_usb_ids "")
    set(strike_pro_index "")
    set(udev_rules
        "# Generated from config/devices.json. Do not edit.\n# Grant the active desktop session access to configured keyboard HID interfaces.\n")

    math(EXPR last_device_index "${device_count} - 1")
    foreach(index RANGE 0 ${last_device_index})
        string(JSON device_id GET "${device_json}" devices ${index} id)
        string(JSON device_name GET "${device_json}" devices ${index} name)
        string(JSON vendor_id GET "${device_json}" devices ${index} vendor_id)
        string(JSON usb_product_id GET "${device_json}" devices ${index} usb_product_id)
        string(JSON dongle_product_id GET "${device_json}" devices ${index} dongle_product_id)
        string(JSON artwork GET "${device_json}" devices ${index} artwork)
        string(JSON battery_protocol GET "${device_json}" devices ${index} battery_protocol)
        string(JSON battery_interface GET "${device_json}" devices ${index} battery_interface)

        if(NOT device_id MATCHES "^[a-z0-9]+([a-z0-9-]*[a-z0-9])?$")
            message(FATAL_ERROR
                "devices.json: device id '${device_id}' must use lowercase letters, digits, and hyphens")
        endif()
        if(device_name STREQUAL "")
            message(FATAL_ERROR "devices.json: ${device_id}.name must not be empty")
        endif()
        if(NOT artwork MATCHES "^:/")
            message(FATAL_ERROR
                "devices.json: ${device_id}.artwork must be a Qt resource path beginning with :/")
        endif()
        string(SUBSTRING "${artwork}" 2 -1 artwork_file)
        if(NOT EXISTS "${project_root}/${artwork_file}")
            message(FATAL_ERROR
                "devices.json: ${device_id}.artwork does not exist: ${artwork_file}")
        endif()
        list(APPEND artwork_files "${artwork_file}")

        list(FIND seen_ids "${device_id}" duplicate_id_index)
        if(NOT duplicate_id_index EQUAL -1)
            message(FATAL_ERROR "devices.json: duplicate device id '${device_id}'")
        endif()
        list(APPEND seen_ids "${device_id}")

        msi_keyboard_validate_hex_id("${vendor_id}" vendor_id "${device_id}")
        msi_keyboard_validate_hex_id("${usb_product_id}" usb_product_id "${device_id}")
        string(TOLOWER "${vendor_id}" vendor_id)
        string(TOLOWER "${usb_product_id}" usb_product_id)

        set(usb_key "${vendor_id}:${usb_product_id}")
        list(FIND seen_usb_ids "${usb_key}" duplicate_usb_index)
        if(NOT duplicate_usb_index EQUAL -1)
            message(FATAL_ERROR "devices.json: duplicate USB id ${usb_key}")
        endif()
        list(APPEND seen_usb_ids "${usb_key}")

        if(dongle_product_id STREQUAL "")
            set(dongle_cpp "quint16{0}")
        else()
            msi_keyboard_validate_hex_id(
                "${dongle_product_id}" dongle_product_id "${device_id}")
            string(TOLOWER "${dongle_product_id}" dongle_product_id)
            set(dongle_key "${vendor_id}:${dongle_product_id}")
            list(FIND seen_usb_ids "${dongle_key}" duplicate_dongle_index)
            if(NOT duplicate_dongle_index EQUAL -1)
                message(FATAL_ERROR "devices.json: duplicate USB id ${dongle_key}")
            endif()
            list(APPEND seen_usb_ids "${dongle_key}")
            set(dongle_cpp "quint16{0x${dongle_product_id}}")
        endif()

        if(battery_protocol STREQUAL "")
            if(NOT battery_interface EQUAL -1)
                message(FATAL_ERROR
                    "devices.json: ${device_id}.battery_interface must be -1 when no battery protocol is set")
            endif()
        elseif(battery_interface LESS 0)
            message(FATAL_ERROR
                "devices.json: ${device_id}.battery_interface must be non-negative")
        endif()

        msi_keyboard_escape_cpp("${device_id}" device_id_cpp)
        msi_keyboard_escape_cpp("${device_name}" device_name_cpp)
        msi_keyboard_escape_cpp("${artwork}" artwork_cpp)
        msi_keyboard_escape_cpp("${battery_protocol}" battery_protocol_cpp)
        string(APPEND entries
            "    DeviceDefinition{\n"
            "        .id = std::string_view{\"${device_id_cpp}\"},\n"
            "        .displayName = std::string_view{\"${device_name_cpp}\"},\n"
            "        .vendorId = quint16{0x${vendor_id}},\n"
            "        .usbProductId = quint16{0x${usb_product_id}},\n"
            "        .dongleProductId = ${dongle_cpp},\n"
            "        .artworkResource = std::string_view{\"${artwork_cpp}\"},\n"
            "        .batteryProtocol = std::string_view{\"${battery_protocol_cpp}\"},\n"
            "        .batteryInterfaceNumber = ${battery_interface},\n"
            "    },\n")

        string(APPEND udev_rules
            "SUBSYSTEM==\"hidraw\", ATTRS{idVendor}==\"${vendor_id}\", ATTRS{idProduct}==\"${usb_product_id}\", MODE=\"0660\", TAG+=\"uaccess\"\n")
        if(NOT dongle_product_id STREQUAL "")
            string(APPEND udev_rules
                "SUBSYSTEM==\"hidraw\", ATTRS{idVendor}==\"${vendor_id}\", ATTRS{idProduct}==\"${dongle_product_id}\", MODE=\"0660\", TAG+=\"uaccess\"\n")
        endif()

        if(device_id STREQUAL "strike-pro")
            set(strike_pro_index ${index})
        endif()
    endforeach()

    if(strike_pro_index STREQUAL "")
        message(FATAL_ERROR
            "devices.json: the strike-pro entry is required while compatibility aliases exist")
    endif()

    get_filename_component(header_directory "${output_header}" DIRECTORY)
    get_filename_component(udev_directory "${output_udev}" DIRECTORY)
    file(MAKE_DIRECTORY "${header_directory}" "${udev_directory}")

    file(WRITE "${output_header}"
        "#pragma once\n\n"
        "#include <array>\n\n"
        "namespace strikepro::generated {\n\n"
        "inline constexpr std::array<DeviceDefinition, ${device_count}> kDeviceDefinitions{{\n"
        "${entries}"
        "}};\n\n"
        "} // namespace strikepro::generated\n\n"
        "namespace strikepro {\n\n"
        "inline constexpr const DeviceDefinition &kStrikeProDeviceDefinition =\n"
        "    generated::kDeviceDefinitions.at(${strike_pro_index});\n"
        "inline constexpr quint16 kMsiVendorId =\n"
        "    kStrikeProDeviceDefinition.vendorId;\n"
        "inline constexpr quint16 kStrikeProWirelessProductId =\n"
        "    kStrikeProDeviceDefinition.dongleProductId;\n"
        "inline constexpr quint16 kStrikeProWiredProductId =\n"
        "    kStrikeProDeviceDefinition.usbProductId;\n\n"
        "} // namespace strikepro\n")
    file(WRITE "${output_udev}" "${udev_rules}")
    list(REMOVE_DUPLICATES artwork_files)
    set(MSI_KEYBOARD_DEVICE_ARTWORK_FILES
        "${artwork_files}"
        PARENT_SCOPE)
endfunction()
