PROJECT(Bangarang)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

FIND_PACKAGE(KDE4 REQUIRED)
FIND_PACKAGE(Nepomuk REQUIRED)
FIND_PACKAGE(KdeMultimedia REQUIRED)

INCLUDE(KDE4Defaults)

ADD_DEFINITIONS( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

macro_ensure_version( "4.3.80" ${KDE_VERSION} HAVEKDE4_4 )
macro_bool_to_01(HAVEKDE4_4 HAVE_KSTATUSNOTIFIERITEM)
configure_file(config-bangarang.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-bangarang.h )

add_subdirectory( platform/nepomukwriter )

INCLUDE_DIRECTORIES( 
    ${QT_QTSCRIPT_INCLUDES}
    ${KDE4_INCLUDES} 
    ${KDEMULTIMEDIA_INCLUDE_DIR} 
    ${NEPOMUK_INCLUDE_DIR} 
    ${TAGLIB_INCLUDES} 
    ${TAGLIB_INCLUDES}/.. 
    ${KROSS_INCLUDES}
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
    infoitemdelegate.cpp
    infoartistdelegate.cpp
    savedlistsmanager.cpp 
    actionsmanager.cpp
    videosettings.cpp
    pluginexport.cpp
    scriptconsole.cpp
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
    platform/taglistengine.cpp
    platform/bangarangvideowidget.cpp
    platform/infoitemmodel.cpp
    platform/infoartistmodel.cpp
    platform/dbpediaquery.cpp
)

KDE4_ADD_UI_FILES(BangarangSources mainwindow.ui)

KDE4_ADD_EXECUTABLE(bangarang ${BangarangSources})

kde4_add_app_icon(BangarangSources "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*.png")
# Executable
TARGET_LINK_LIBRARIES(bangarang 
    ${KDE4_KDEUI_LIBS} 
    ${KDE4_KPARTS_LIBS} 
    ${KDE4_PHONON_LIBS} 
    ${KDE4_KROSSCORE_LIBS}
    ${NEPOMUK_LIBRARIES} 
    ${KDEMULTIMEDIA_LIBRARIES} 
    ${KDE4_KROSSCORE_LIBS}
    ${SOPRANO_LIBRARIES} 
    ${TAGLIB_LIBRARIES} 
    ${KDE4_KROSSUI_LIBS}
    ${KDE4_KROSSCORE_LIBS}
    tag 
)

# # Plugin-Module
# kde4_add_plugin(krossmodulebangarangmod  ${BangarangSources})
# target_link_libraries( krossmodulebangarangmod
#     ${KDE4_KDEUI_LIBS} 
#     ${KDE4_KPARTS_LIBS} 
#     ${KDE4_PHONON_LIBS} 
#     ${KDE4_KROSSCORE_LIBS}
#     ${NEPOMUK_LIBRARIES} 
#     ${KDEMULTIMEDIA_LIBRARIES} 
#     ${SOPRANO_LIBRARIES} 
#     ${TAGLIB_LIBRARIES} 
#     tag 
#     ${KDE4_KROSSCORE_LIBS}
#     )
 
if( NOT HAVEKDE4_4 )
TARGET_LINK_LIBRARIES(bangarang knotificationitem-1)
endif( NOT HAVEKDE4_4 )

########### install files ###############

install(TARGETS bangarang ${INSTALL_TARGETS_DEFAULT_ARGS})
# install(TARGETS  krossmodulebangarangmod DESTINATION ${PLUGIN_INSTALL_DIR}) # modules one might want to link against in c++ plugins (using dynamic libload magic) 
#install(FILES <bangarangDataFilesHere> DESTINATION ${DATA_INSTALL_DIR}/bangarang) # We might need that once we can load plugins 
#install(FILES bangarangrc DESTINATION ${CONFIG_INSTALL_DIR})
kde4_install_icons( ${ICON_INSTALL_DIR}   )
