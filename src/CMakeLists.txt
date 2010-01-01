PROJECT(Bangarang)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

FIND_PACKAGE(KDE4 REQUIRED)
FIND_PACKAGE(Nepomuk REQUIRED)
FIND_PACKAGE(KdeMultimedia REQUIRED)

INCLUDE(KDE4Defaults)

ADD_DEFINITIONS( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

add_subdirectory( platform/nepomukwriter )

INCLUDE_DIRECTORIES( 
    ${KDE4_INCLUDES} 
    ${KDEMULTIMEDIA_INCLUDE_DIR} 
    ${NEPOMUK_INCLUDE_DIR} 
    ${TAGLIB_INCLUDES} 
    ${TAGLIB_INCLUDES}/.. 
    /usr/include/taglib 
)

SET(BangarangSources 
    mainwindow.cpp 
    main.cpp 
    sensiblewidgets.cpp 
    mediaitemdelegate.cpp 
    nowplayingdelegate.cpp 
    mediaview.cpp 
    infomanager.cpp 
    savedlistsmanager.cpp 
    actionsmanager.cpp
    videosettings.cpp
    platform/mediaitemmodel.cpp
    platform/musiclistengine.cpp
    platform/videolistengine.cpp
    platform/filelistengine.cpp
    platform/cdlistengine.cpp
    platform/dvdlistengine.cpp
    platform/medialistsengine.cpp
    platform/audiostreamlistengine.cpp
    platform/nepomuklistengine.cpp
    platform/semanticslistengine.cpp
    platform/playlist.cpp
    platform/utilities.cpp
    platform/listengine.cpp
    platform/savedlistsengine.cpp
    platform/listenginefactory.cpp
    platform/mediaindexer.cpp
    platform/mediavocabulary.cpp
    platform/mediaquery.cpp
    platform/medialistcache.cpp
    platform/cachelistengine.cpp
    platform/audioclipslistengine.cpp
)

KDE4_ADD_UI_FILES(BangarangSources mainwindow.ui)

KDE4_ADD_EXECUTABLE(bangarang ${BangarangSources})

kde4_add_app_icon(BangarangSources "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*.png")

TARGET_LINK_LIBRARIES(bangarang 
    ${KDE4_KDEUI_LIBS} 
    ${KDE4_KPARTS_LIBS} 
    ${KDE4_PHONON_LIBS} 
    ${NEPOMUK_LIBRARIES} 
    ${KDEMULTIMEDIA_LIBRARIES} 
    ${SOPRANO_LIBRARIES} 
    ${TAGLIB_LIBRARIES} 
    tag 
)

########### install files ###############

install(TARGETS bangarang ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(FILES <bangarangDataFilesHere> DESTINATION ${DATA_INSTALL_DIR}/bangarang)
#install(FILES bangarangrc DESTINATION ${CONFIG_INSTALL_DIR})
kde4_install_icons( ${ICON_INSTALL_DIR}   )
