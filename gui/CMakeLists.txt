set(GUI_TARGET "${APP_NAME}-gui")

find_package(Qt5 5.6 REQUIRED COMPONENTS Core DBus Gui Qml Quick)

if(NOT FLAVOR STREQUAL "silica")
  find_package(Qt5 5.6 REQUIRED COMPONENTS Widgets QuickControls2)
endif()

add_executable(${GUI_TARGET}
  ${GUI_SOURCES}
  ${GUI_HEADERS}
  ${FILESELECTOR_SOURCES}
  ${FILESELECTOR_HEADERS})

target_include_directories(${GUI_TARGET} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/fileselector)

target_compile_definitions(${GUI_TARGET} PRIVATE
  APP_NAME=\"${APP_NAME}\"
  APP_VERSION=\"${APP_VERSION}\"
  SERVER_EXE=\"${PREFIX_RUNNING}/${CMAKE_INSTALL_BINDIR}/${APP_NAME}\"
  ${SCOUT_COMMON_COMPILE_DEFINITIONS})

target_link_libraries(${GUI_TARGET} PRIVATE
  Qt5::Core
  Qt5::DBus
  Qt5::Gui
  Qt5::Qml
  Qt5::Quick)

if(FLAVOR STREQUAL "silica")
  # SailfishApp supplies the platform application and QML-view entry points.
  include(FindPkgConfig)
  pkg_search_module(SAILFISH sailfishapp REQUIRED)
  target_include_directories(${GUI_TARGET} PRIVATE ${SAILFISH_INCLUDE_DIRS})
  target_compile_definitions(${GUI_TARGET} PRIVATE
    IS_SAILFISH_OS
    TRANSLATION_FOLDER=\"${PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${APP_NAME}/translations\")
  target_link_libraries(${GUI_TARGET} PRIVATE ${SAILFISH_LDFLAGS})
else()
  target_compile_definitions(${GUI_TARGET} PRIVATE IS_QTCONTROLS_QT)
  target_link_libraries(${GUI_TARGET} PRIVATE
    Qt5::Widgets
    Qt5::QuickControls2)

  target_sources(${GUI_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/qml_main.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/icons.qrc)

  if(FLAVOR STREQUAL "kirigami")
    target_sources(${GUI_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/qml_kirigami.qrc)
    target_compile_definitions(${GUI_TARGET} PRIVATE TRANSLATION_FOLDER=\":/i18n\")
  elseif(FLAVOR STREQUAL "qtcontrols")
    target_sources(${GUI_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/qml_qtcontrols.qrc)
    target_compile_definitions(${GUI_TARGET} PRIVATE TRANSLATION_FOLDER=\":/i18n\")
  elseif(FLAVOR STREQUAL "uuitk")
    target_sources(${GUI_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/qml_uuitk.qrc)
    target_compile_definitions(${GUI_TARGET} PRIVATE
      IS_UUITK
      DEFAULT_FALLBACK_STYLE=\"suru\"
      TRANSLATION_FOLDER=\"./translations\")
  elseif(FLAVOR STREQUAL "lomiri")
    target_sources(${GUI_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/qml_lomiri.qrc)
    target_compile_definitions(${GUI_TARGET} PRIVATE
      IS_LOMIRI
      DEFAULT_FALLBACK_STYLE=\"suru\"
      TRANSLATION_FOLDER=\"./translations\")
  endif()
endif()

install(TARGETS ${GUI_TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${GUI_TARGET}.desktop
  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/icons/86x86/harbour-osmscout-server.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/86x86/apps
  RENAME ${APP_NAME}.png)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/icons/108x108/harbour-osmscout-server.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/108x108/apps
  RENAME ${APP_NAME}.png)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/icons/128x128/harbour-osmscout-server.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps
  RENAME ${APP_NAME}.png)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/icons/256x256/harbour-osmscout-server.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
  RENAME ${APP_NAME}.png)

if(FLAVOR STREQUAL "silica")
  install(FILES ${GUI_QML}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${GUI_TARGET}/qml)
  install(FILES ${GUI_SILICA_QML}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${GUI_TARGET}/qml/platform)
  install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/banner.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/osmscout-server.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${GUI_TARGET}/icons)
else()
  install(FILES ${PROJECT_SOURCE_DIR}/packaging/osmscout-server.appdata.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
endif()

add_osmscout_translations(${GUI_TARGET})
