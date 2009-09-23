PROJECT(ArenaMediaPlayer)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

FIND_PACKAGE(KDE4 REQUIRED)
FIND_PACKAGE(Nepomuk REQUIRED)

INCLUDE(KDE4Defaults)

INCLUDE_DIRECTORIES( ${KDE4_INCLUDES} ${NEPOMUK_INDLUDE_DIR} ${TAGLIB_INCLUDES} ${TAGLIB_INCLUDES}/.. /usr/include/taglib)
ADD_DEFINITIONS( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

SET(BangarangSources mainwindow.cpp main.cpp sensiblewidgets.cpp mediaitemdelegate.cpp nowplayingdelegate.cpp mediaview.cpp infomanager.cpp platform/mediaitemmodel.cpp platform/musiclistengine.cpp platform/filelistengine.cpp platform/playlist.cpp platform/utilities.cpp platform/listengine.cpp platform/listenginefactory.cpp platform/mediaindexer.cpp platform/blur.cpp)

KDE4_ADD_UI_FILES(BangarangSources mainwindow.ui)

kde4_add_app_icon(BangarangSources "${CMAKE_CURRENT_SOURCE_DIR}/hi*.png")


KDE4_ADD_EXECUTABLE(bangarang ${BangarangSources})

TARGET_LINK_LIBRARIES(bangarang ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_PHONON_LIBS} ${NEPOMUK_LIBRARIES} ${SOPRANO_LIBRARIES} ${TAGLIB_LIBRARIES} tag nepomukquery nepomukqueryclient)

########### install files ###############

install(TARGETS bangarang ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES bangarang.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
#install(FILES <bangarangDataFilesHere> DESTINATION ${DATA_INSTALL_DIR}/bangarang)
#install(FILES bangarangrc DESTINATION ${CONFIG_INSTALL_DIR})

kde4_install_icons( ${ICON_INSTALL_DIR}   )

