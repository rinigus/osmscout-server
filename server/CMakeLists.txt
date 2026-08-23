find_package(Qt5 5.6 REQUIRED COMPONENTS Core Network DBus Sql Xml Positioning)
find_package(Boost REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(MICROHTTPD REQUIRED IMPORTED_TARGET libmicrohttpd)
pkg_check_modules(POSTAL REQUIRED IMPORTED_TARGET libpostal)
pkg_check_modules(MARISA REQUIRED IMPORTED_TARGET marisa)
pkg_check_modules(KYOTOCABINET REQUIRED IMPORTED_TARGET kyotocabinet)
pkg_check_modules(ZLIB REQUIRED IMPORTED_TARGET zlib)
pkg_check_modules(SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)

add_executable(${APP_NAME}
  ${SERVER_SOURCES}
  ${SERVER_HEADERS}
  ${UHTTP_SOURCES}
  ${UHTTP_HEADERS}
  ${GEOCODER_NLP_SOURCES}
  ${GEOCODER_NLP_HEADERS})

target_include_directories(${APP_NAME} PRIVATE
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/uhttp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/geocoder-nlp/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/geocoder-nlp/thirdparty/sqlite3pp/headeronly_src)

target_compile_definitions(${APP_NAME} PRIVATE
  APP_NAME=\"${APP_NAME}\"
  APP_VERSION=\"${APP_VERSION}\"
  DEFAULT_DATA_PREFIX=\"${SCOUT_RUNTIME_DATADIR}/\"
  SERVER_EXE=\"${SCOUT_RUNTIME_BINDIR}/${APP_NAME}\"
  GEOCODERNLP_ALIASFILE=\"${SCOUT_RUNTIME_DATADIR}/data/geocoder-npl-tag-aliases.json\"
  MAPBOXGL_STYLEDIR=\"${SCOUT_RUNTIME_DATADIR}/styles/mapboxgl\"
  ${SCOUT_COMMON_COMPILE_DEFINITIONS})

if(FLAVOR STREQUAL "silica")
  target_compile_definitions(${APP_NAME} PRIVATE
    IS_SAILFISH_OS
    TRANSLATION_FOLDER=\"${CMAKE_INSTALL_FULL_DATAROOTDIR}/${APP_NAME}/translations\")
  set_target_properties(${APP_NAME} PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_FULL_DATAROOTDIR}/${APP_NAME}/lib")
elseif(FLAVOR STREQUAL "kirigami" OR FLAVOR STREQUAL "qtcontrols")
  target_compile_definitions(${APP_NAME} PRIVATE
    IS_QTCONTROLS_QT
    TRANSLATION_FOLDER=\":/i18n\")
else()
  target_compile_definitions(${APP_NAME} PRIVATE
    IS_QTCONTROLS_QT
    DEFAULT_FALLBACK_STYLE=\"suru\"
    TRANSLATION_FOLDER=\"./translations\")
  if(FLAVOR STREQUAL "uuitk")
    target_compile_definitions(${APP_NAME} PRIVATE IS_UUITK)
  else()
    target_compile_definitions(${APP_NAME} PRIVATE IS_LOMIRI)
  endif()
endif()

target_link_libraries(${APP_NAME} PRIVATE
  Qt5::Core
  Qt5::Network
  Qt5::DBus
  Qt5::Sql
  Qt5::Xml
  Qt5::Positioning
  PkgConfig::MICROHTTPD
  PkgConfig::POSTAL
  PkgConfig::MARISA
  PkgConfig::KYOTOCABINET
  PkgConfig::ZLIB
  PkgConfig::SQLITE3)

if(WITH_OSMSCOUT)
  # Legacy support retained for existing configurations; CMake discovery is untested.
  find_library(OSMSCOUT_LIBRARY NAMES osmscout)
  find_library(OSMSCOUT_MAP_LIBRARY NAMES osmscout_map)
  if(NOT OSMSCOUT_LIBRARY OR NOT OSMSCOUT_MAP_LIBRARY)
    message(FATAL_ERROR "WITH_OSMSCOUT requires libosmscout and libosmscout_map")
  endif()

  target_compile_definitions(${APP_NAME} PRIVATE USE_OSMSCOUT)
  target_link_libraries(${APP_NAME} PRIVATE ${OSMSCOUT_LIBRARY} ${OSMSCOUT_MAP_LIBRARY})

  if(WITH_OSMSCOUT_MAP_CAIRO)
    pkg_check_modules(PANGO REQUIRED IMPORTED_TARGET pango)
    pkg_check_modules(CAIRO REQUIRED IMPORTED_TARGET cairo)
    find_library(OSMSCOUT_MAP_CAIRO_LIBRARY NAMES osmscout_map_cairo)
    if(NOT OSMSCOUT_MAP_CAIRO_LIBRARY)
      message(FATAL_ERROR "WITH_OSMSCOUT_MAP_CAIRO requires libosmscout_map_cairo")
    endif()
    target_compile_definitions(${APP_NAME} PRIVATE USE_OSMSCOUT_MAP_CAIRO)
    target_link_libraries(${APP_NAME} PRIVATE
      ${OSMSCOUT_MAP_CAIRO_LIBRARY} PkgConfig::PANGO PkgConfig::CAIRO)
  else()
    find_package(Qt5 5.6 REQUIRED COMPONENTS Gui)
    find_library(OSMSCOUT_MAP_QT_LIBRARY NAMES osmscout_map_qt)
    if(NOT OSMSCOUT_MAP_QT_LIBRARY)
      message(FATAL_ERROR "WITH_OSMSCOUT requires libosmscout_map_qt")
    endif()
    target_compile_definitions(${APP_NAME} PRIVATE USE_OSMSCOUT_MAP_QT)
    target_link_libraries(${APP_NAME} PRIVATE Qt5::Gui ${OSMSCOUT_MAP_QT_LIBRARY})
  endif()

  if(NOT FLAVOR STREQUAL "silica")
    find_package(OpenMP REQUIRED)
    target_link_libraries(${APP_NAME} PRIVATE OpenMP::OpenMP_CXX)
  endif()
endif()

if(WITH_MAPNIK)
  find_program(MAPNIK_CONFIG mapnik-config)
  if(NOT MAPNIK_CONFIG)
    message(FATAL_ERROR "WITH_MAPNIK requires mapnik-config")
  endif()
  find_package(ICU REQUIRED COMPONENTS uc)
  execute_process(COMMAND ${MAPNIK_CONFIG} --includes
    OUTPUT_VARIABLE MAPNIK_INCLUDE_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND ${MAPNIK_CONFIG} --libs
    OUTPUT_VARIABLE MAPNIK_LINK_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  separate_arguments(MAPNIK_INCLUDE_FLAGS NATIVE_COMMAND "${MAPNIK_INCLUDE_FLAGS}")
  separate_arguments(MAPNIK_LINK_FLAGS NATIVE_COMMAND "${MAPNIK_LINK_FLAGS}")
  target_compile_definitions(${APP_NAME} PRIVATE USE_MAPNIK)
  target_compile_options(${APP_NAME} PRIVATE ${MAPNIK_INCLUDE_FLAGS})
  target_link_libraries(${APP_NAME} PRIVATE ${MAPNIK_LINK_FLAGS} ICU::uc)

  if(FLAVOR STREQUAL "silica")
    target_compile_definitions(${APP_NAME} PRIVATE
      MAPNIK_FONTS_DIR=\"/usr/share/harbour-osmscout-server-module-fonts/fonts\"
      MAPNIK_INPUT_PLUGINS_DIR=\"/usr/share/${APP_NAME}/lib/mapnik/input\")
  else()
    execute_process(COMMAND ${MAPNIK_CONFIG} --fonts
      OUTPUT_VARIABLE MAPNIK_FONTS_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${MAPNIK_CONFIG} --input-plugins
      OUTPUT_VARIABLE MAPNIK_INPUT_PLUGINS_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    target_compile_definitions(${APP_NAME} PRIVATE
      MAPNIK_FONTS_DIR=\"${MAPNIK_FONTS_DIR}\"
      MAPNIK_INPUT_PLUGINS_DIR=\"${MAPNIK_INPUT_PLUGINS_DIR}\")
  endif()

  if(FLAVOR STREQUAL "uuitk" OR FLAVOR STREQUAL "lomiri")
    pkg_check_modules(PROJ REQUIRED IMPORTED_TARGET proj)
    target_link_libraries(${APP_NAME} PRIVATE PkgConfig::PROJ)
  endif()
endif()

if(WITH_VALHALLA)
  pkg_check_modules(VALHALLA REQUIRED IMPORTED_TARGET libvalhalla)
  target_compile_definitions(${APP_NAME} PRIVATE
    USE_VALHALLA
    VALHALLA_CONFIG_TEMPLATE=\"${SCOUT_RUNTIME_DATADIR}/data/valhalla.json\")
  target_link_libraries(${APP_NAME} PRIVATE PkgConfig::VALHALLA)
endif()

if(WITH_CURL)
  pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)
  target_compile_definitions(${APP_NAME} PRIVATE USE_LIBCURL)
  target_link_libraries(${APP_NAME} PRIVATE PkgConfig::CURL)
endif()

if(WITH_SYSTEMD)
  pkg_check_modules(SYSTEMD REQUIRED IMPORTED_TARGET libsystemd)
  target_compile_definitions(${APP_NAME} PRIVATE USE_SYSTEMD)
  target_link_libraries(${APP_NAME} PRIVATE PkgConfig::SYSTEMD)
endif()

install(TARGETS ${APP_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/styles
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${APP_NAME})
install(FILES
  ${PROJECT_SOURCE_DIR}/data/geocoder-npl-tag-aliases.json
  ${VALHALLA_TEMPLATES}
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${APP_NAME}/data)

if(WITH_DBUS_ACTIVATION)
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/io.github.rinigus.OSMScoutServer.service
    DBUS_SERVICE_CONTENT)
  string(REPLACE "PREFIX/osmscout-server"
    "${SCOUT_RUNTIME_BINDIR}/${APP_NAME}"
    DBUS_SERVICE_CONTENT "${DBUS_SERVICE_CONTENT}")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/io.github.rinigus.OSMScoutServer.service
    "${DBUS_SERVICE_CONTENT}")
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/io.github.rinigus.OSMScoutServer.service
    DESTINATION ${DBUS_SERVICE_PATH})
endif()

add_osmscout_translations(${APP_NAME})
