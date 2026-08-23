cmake_minimum_required(VERSION 3.10.0)

project(osmscout-server
  VERSION 3.1.7
  DESCRIPTION "Offline map services")

set(APP_VERSION "${PROJECT_VERSION}")
if(DEFINED VERSION)
  set(APP_VERSION "${VERSION}")
endif()
set(VERSION "${PROJECT_VERSION}" CACHE STRING "Application version")
set(FLAVOR "kirigami" CACHE STRING "Platform flavor: silica, kirigami, qtcontrols, uuitk, or lomiri")
set(APP_NAME "" CACHE STRING "Application name")
set(DBUS_SERVICE_PATH "/usr/share/dbus-1/services" CACHE PATH
  "D-Bus activation service directory")

set(VALID_FLAVORS silica kirigami qtcontrols uuitk lomiri)
if(NOT FLAVOR IN_LIST VALID_FLAVORS)
  message(FATAL_ERROR
    "FLAVOR must be one of: silica, kirigami, qtcontrols, uuitk, lomiri")
endif()

if(NOT APP_NAME)
  if(FLAVOR STREQUAL "silica")
    set(APP_NAME "harbour-osmscout-server" CACHE STRING "Application name" FORCE)
  else()
    set(APP_NAME "osmscout-server" CACHE STRING "Application name" FORCE)
  endif()
endif()

option(WITH_OSMSCOUT "Build libosmscout support" OFF)
option(WITH_OSMSCOUT_MAP_CAIRO "Use the Cairo libosmscout renderer" OFF)
option(WITH_MAPNIK "Build Mapnik support" ON)
option(WITH_VALHALLA "Build Valhalla support" ON)
option(WITH_CURL "Build libcurl support" OFF)
option(WITH_SYSTEMD "Build systemd support" ON)
option(WITH_DBUS_ACTIVATION "Install D-Bus activation support" OFF)

if(WITH_OSMSCOUT_MAP_CAIRO AND NOT WITH_OSMSCOUT)
  message(FATAL_ERROR "WITH_OSMSCOUT_MAP_CAIRO requires WITH_OSMSCOUT")
endif()

set(SCOUT_COMMON_COMPILE_DEFINITIONS QT_DEPRECATED_WARNINGS)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  list(APPEND SCOUT_COMMON_COMPILE_DEFINITIONS
    QT_NO_WARNING_OUTPUT
    QT_NO_DEBUG_OUTPUT)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)

if(FLAVOR STREQUAL "uuitk" OR FLAVOR STREQUAL "lomiri")
  # Click apps run from their package root, where binaries and data are local.
  set(SCOUT_RUNTIME_BINDIR ".")
  set(SCOUT_RUNTIME_DATADIR "./${CMAKE_INSTALL_DATAROOTDIR}/${APP_NAME}")
else()
  set(SCOUT_RUNTIME_BINDIR "${CMAKE_INSTALL_FULL_BINDIR}")
  set(SCOUT_RUNTIME_DATADIR "${CMAKE_INSTALL_FULL_DATAROOTDIR}/${APP_NAME}")
endif()

# Discover only production sources. Target CMake files consume these lists so a
# source addition does not also require build-system maintenance.
file(GLOB SERVER_SOURCES server/src/*.cpp)
file(GLOB SERVER_HEADERS server/src/*.h server/src/*.hpp)
file(GLOB UHTTP_SOURCES server/src/uhttp/*.cpp)
file(GLOB UHTTP_HEADERS server/src/uhttp/*.h)
file(GLOB GEOCODER_NLP_SOURCES server/src/geocoder-nlp/src/*.cpp)
file(GLOB GEOCODER_NLP_HEADERS server/src/geocoder-nlp/src/*.h)
file(GLOB GUI_SOURCES gui/src/*.cpp)
file(GLOB GUI_HEADERS gui/src/*.h)
file(GLOB FILESELECTOR_SOURCES gui/src/fileselector/*.cpp)
file(GLOB FILESELECTOR_HEADERS gui/src/fileselector/*.h)
file(GLOB GUI_QML gui/qml/*.qml)
file(GLOB GUI_SILICA_QML gui/qml/platform.silica/*.qml)
file(GLOB GUI_PLATFORM_QML gui/qml/platform.*/*.qml)
file(GLOB VALHALLA_TEMPLATES data/valhalla.json-*)
find_package(Qt5 5.6 REQUIRED COMPONENTS Core LinguistTools)

add_custom_target(osmscout-update-translations
  COMMAND $<TARGET_FILE:Qt5::lupdate>
    ${SERVER_SOURCES}
    ${SERVER_HEADERS}
    ${UHTTP_SOURCES}
    ${UHTTP_HEADERS}
    ${GUI_SOURCES}
    ${GUI_HEADERS}
    ${FILESELECTOR_SOURCES}
    ${FILESELECTOR_HEADERS}
    ${GUI_QML}
    ${GUI_PLATFORM_QML}
    -ts ${PROJECT_SOURCE_DIR}/translations/harbour-osmscout-server.ts
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Updating the Transifex source translation catalog")

function(add_osmscout_translations target)
  get_property(translation_outputs GLOBAL PROPERTY OSMSCOUT_TRANSLATION_OUTPUTS)
  if(NOT translation_outputs)
    set(translation_catalog_prefix "harbour-osmscout-server")
    file(GLOB translation_sources
      "${PROJECT_SOURCE_DIR}/translations/${translation_catalog_prefix}-*.ts")
    list(REMOVE_ITEM translation_sources
      "${PROJECT_SOURCE_DIR}/translations/${translation_catalog_prefix}-sk.ts")

    if(FLAVOR STREQUAL "silica")
      set(translation_runtime_prefix "${APP_NAME}")
    else()
      # Desktop code retains this fixed lookup prefix independently of APP_NAME.
      set(translation_runtime_prefix "osmscout-server")
    endif()

    foreach(translation_source ${translation_sources})
      get_filename_component(translation_name "${translation_source}" NAME_WE)
      string(REPLACE "${translation_catalog_prefix}-" "${translation_runtime_prefix}-"
        translation_name "${translation_name}")
      set(translation_output "${CMAKE_BINARY_DIR}/translations/${translation_name}.qm")
      add_custom_command(
        OUTPUT "${translation_output}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/translations"
        COMMAND $<TARGET_FILE:Qt5::lrelease> -removeidentical "${translation_source}" -qm "${translation_output}"
        DEPENDS "${translation_source}"
        VERBATIM)
      list(APPEND translation_outputs "${translation_output}")
    endforeach()
    add_custom_target(osmscout-translations ALL DEPENDS ${translation_outputs})
    set_property(GLOBAL PROPERTY OSMSCOUT_TRANSLATION_OUTPUTS "${translation_outputs}")
  endif()
  add_dependencies(${target} osmscout-translations)

  if(FLAVOR STREQUAL "kirigami" OR FLAVOR STREQUAL "qtcontrols")
    set(translation_qrc "${CMAKE_CURRENT_BINARY_DIR}/${target}-translations.qrc")
    file(WRITE "${translation_qrc}" "<RCC>\n  <qresource prefix=\"/i18n\">\n")
    foreach(translation_output ${translation_outputs})
      get_filename_component(translation_resource_name "${translation_output}" NAME)
      file(APPEND "${translation_qrc}"
        "    <file alias=\"${translation_resource_name}\">${translation_output}</file>\n")
    endforeach()
    file(APPEND "${translation_qrc}" "  </qresource>\n</RCC>\n")
    qt5_add_resources(translation_resources "${translation_qrc}")
    target_sources(${target} PRIVATE ${translation_resources})
  elseif(FLAVOR STREQUAL "silica" AND "${target}" STREQUAL "${APP_NAME}")
    install(FILES ${translation_outputs}
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${APP_NAME}/translations)
  endif()
endfunction()

message(STATUS "OSM Scout Server ${PROJECT_VERSION}")
message(STATUS "  Flavor: ${FLAVOR}")
message(STATUS "  App name: ${APP_NAME}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Runtime data directory: ${SCOUT_RUNTIME_DATADIR}")
message(STATUS "  libosmscout: ${WITH_OSMSCOUT}")
message(STATUS "  Mapnik: ${WITH_MAPNIK}")
message(STATUS "  Valhalla: ${WITH_VALHALLA}")
message(STATUS "  libcurl: ${WITH_CURL}")
message(STATUS "  systemd: ${WITH_SYSTEMD}")
message(STATUS "  D-Bus activation: ${WITH_DBUS_ACTIVATION}")
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  message(STATUS "  Compile commands: enabled")
endif()

add_subdirectory(server)
add_subdirectory(gui)
